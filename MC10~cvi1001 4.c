//////////////////////////////////////////////////////////////////////////////////////////
//                                                        					            //
//                     Zero Field Magnetometer Controller v 1.0                         //
//					   Paul Ganssle, 07/19/2011                                         //
//																						//
//   This program is intended as a controller for the zero field atomic magnetometers   //
// in the Pines and Budker Labs. It is intended for use with the Spincore PulseBlaster  //
// USB version with 24 TTLs and the National Instruments M-Series USB 6229 DAQ.         //
//                                                                                      //
//																					    //
//////////////////////////////////////////////////////////////////////////////////////////

 /***************************  Version History  ***************************

1.0		I may be re-using version numbers, because, as with everything else
		in this program, the early versions were made extremely inexpertly.
		Since my intention is to make this version the first decently coded
		version without memory leaks and such, I've called it 1.0.
		
		Changes so far:
		-Core Changes
		-- Switched over to the netCDF data serialization format for saving
		   data, pulse programs and preferences.
		
		-- Rather than using the CVI style panel/control typedefs, I've 
		   added my own UIControls.h library containing structs related to
		   the various panels and controls. Where useful, I've stored these
		   as 2-int arrays containing the panel the thing is on along with 
		   the control identifier. This will make it much easier to move controls
		   between panels if need be.
		
		-- I've broken up the functions into libraries tailored to specific
		   tasks, such as the PulseProgramLib for dealing with pulse program
		   stuff and the MathParserLib for parsing math.
		   
		
		-Functional Changes
		-- Added in the ability to make arbitrary ND pulse sequences in a
		   number of ways. First, either or both of the instr_data and 
		   instruction delay times can now be incremented/decremented in
		   the simple linear function. Additionally, one can now write an
		   arbitrary expression for either instruction delays or instruction
		   data or both. Finally, you can just enter or load from file a
		   sequence of values for either or both, allowing it to be truly
		   arbitrary.
		   
		-- Added in the ability to phase cycle as many instructions as one
		   would like in a maximum of 8 possible cycles (this can be expanded
		   as high as 24 without causing problems with integer overflow).
		   This also allows you to cycle through different instruction values
		   or TTL values.
		   
		-- Added the ability to make user-defined functions which can be used
		   by any program. This will be useful for things like defining a 
		   function such as "90x", which can be calibrated and changed without
		   fucking up all your saved programs that involve 90s.
		 
*****************************************************************************/
		
/***************************         To Do       ****************************
Project	: Updated Pulse Programming
Prior.	: High
Progress: 60%
Descrip	: Need to update how pulses are saved and loaded, as well as implement
		the numerous changes to the user interface, function building, etc.
		
		Milestones:
		
		Implement new Save/Load Protocol  				Progress: 90%
		Implement user-defined functions  				Progress: 10%
		Implement expanded ND programs	  				Progress: 30%
		Implement phase cycling			  				Progress: 90%

Project	: Implement new user interface controls
Progress: 20%
Priority: High
Descrip	: Since changing over to this new system, you've broken all the 
	old user interface functions. Now you need to re-implement them.
	
	Milestones:
	Phase Cycling UI				  				Progress:100%
	
	ND UI											Progress: 60%
		ND On/Off									 Progress:100%
		Change number of dims						 Progress: 80%
		Change number of steps						 Progress: 75%
		update_nd_state()							 Progress: 95%
		Setup skip condition						 Progress:  0%
		
	User Defined Functions UI						Progress:  5%
		Setup main UI								 Progress:  5%


Project	: Update new data saving
Prior.	: High
Progress: 0%
Descrip	: Use the netCDF serialization format to read and write data to files.
		
		Milestones:
		Implement save									Progress: 0%
		Implement load									Progress: 0%
		

Project	: Update User Preference Files
Prior.	: Moderate/High
Progress: 2%
Descrip	: Use the netCDF serialization format to save user preferences as well
		as the configuration of the user interface when the program was last
		closed. I'll also include in this that we should probably add a load
		screen to the program. Should be easy.
		
		Milestones:
		Implement user-defined preferences				Progress: 0%
		Implement new save/load of configuration		Progress: 0%
			Save programs to config file				 Progress: 0%
			Load programs from config file				 Progress: 0%
			Save settings to config file				 Progress: 0%
			Load settings to config file				 Progress: 0%

***************************************************************************/

/******************************* Includes *********************************/

#include <userint.h>					// Start with the standard libraries
#include <cvirte.h>		
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <windows.h>
#include <analysis.h>
#include <utility.h>
#include <ansi_c.h>
#include "pathctrl.h"
#include "toolbox.h"
#include "asynctmr.h"
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include <spinapi.h>					// Then the external libraries
#include <netcdf.h>
#include <NIDAQmx.h>

#include <Magnetometer Controller.h>	// Then the libraries specific to
#include <PulseProgramTypes.h>			// this software.
#include <MathParserLib.h>
#include <UIControls.h>
#include <MCUserDefinedFunctions.h>
#include <MC10.h>  
#include <PulseProgramLib.h>

static TaskHandle acquireSignal, counterTask;    

// Structures for containing UI information (if this were C++, we'd make some classes for this sort of thing..)
maincontrols *mc;	// The Main controls structure
ppcontrols *pc; 	// The UI controls structure
datacontrols *dc; 	// The Data controls structure
ui_ppconfig *uipc;	// The current program configuration structure
all_funcs *af;		// Holder for all current functions.

// Multithreading variable declarations
DefineThreadSafeScalarVar(int, QuitUpdateStatus, 0); 
DefineThreadSafeScalarVar(int, QuitIdle, 0);
DefineThreadSafeScalarVar(int, DoubleQuitIdle, 0);
DefineThreadSafeScalarVar(int, Status, 0);
int update_thread = NULL, idle_thread = NULL;	// Threads
int lock_pb, lock_pp, lock_DAQ, lock_plot; 		// Thread locks

//////////////////////////////////////////////////////
//                                                  //
//              Main Program Functions              //
//                                                  //
//////////////////////////////////////////////////////

int main (int argc, char *argv[])
{
	//Generate locks, thread safe variables
	CmtNewLock(NULL, OPT_TL_PROCESS_EVENTS_WHILE_WAITING, &lock_pb);
	CmtNewLock(NULL, OPT_TL_PROCESS_EVENTS_WHILE_WAITING, &lock_pp);
	CmtNewLock(NULL, OPT_TL_PROCESS_EVENTS_WHILE_WAITING, &lock_DAQ);
	CmtNewLock(NULL, OPT_TL_PROCESS_EVENTS_WHILE_WAITING, &lock_plot);
	
	InitializeQuitUpdateStatus();
	InitializeQuitIdle();
	InitializeDoubleQuitIdle();
	InitializeStatus();
	idle_thread = NULL;
	update_thread = NULL;
	
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	
	//Main panel
	char *uifname = "Magnetometer Controller.uir"; // So it's easy to change this if we need to.
	
	if(load_ui(uifname)) // This function loads the UI and creates the structs.
		return -1;

	RunUserInterface ();
	DiscardPanel (mc->mp);
	
	//Discard Locks
	CmtDiscardLock(lock_pb);
	CmtDiscardLock(lock_pp);
	CmtDiscardLock(lock_DAQ);
	CmtDiscardLock(lock_plot);
	UninitializeQuitIdle();
	UninitializeDoubleQuitIdle();
	UninitializeStatus();
	UninitializeQuitUpdateStatus();

	return 0;
}


int load_ui(char *uifname) { // Function for creating the ppcontrols structure
	// This loads the user interface and populates the structs that contain pointers that can be
	// passed selectively to library functions. These structs are globals, so they don't need to
	// be passed as arguments here.
	
	// Allocate the UIControls structs
	mc = malloc(sizeof(maincontrols));
	dc = malloc(sizeof(datacontrols));
	pc = malloc(sizeof(ppcontrols));
	
	// The uifile will probably be needed later when we make more instructions, so it goes in pc
	pc->uifname = malloc(strlen(uifname)+1);
	strcpy(pc->uifname, uifname);
	
	pc->pulse_inst = PulseInstP;
	pc->md_inst = MDInstr;
	
	
	// Start building the tabs now.
	
	if ((mc->mp = LoadPanel (0, uifname, MainPanel)) < 0)
		return 1;
	DisplayPanel (mc->mp);
	
	// First get the tab panels.
	GetPanelHandleFromTabPage(mc->mp, MainPanel_MainTabs, 0, &dc->fid);
	GetPanelHandleFromTabPage(mc->mp, MainPanel_MainTabs, 1, &dc->spec);
	GetPanelHandleFromTabPage(mc->mp, MainPanel_MainTabs, 2, &pc->PProgPan);
	GetPanelHandleFromTabPage(mc->mp, MainPanel_MainTabs, 3, &pc->PPConfigPan);

	// Then the menu bars
	mc->mainmenu = GetPanelMenuBar(mc->mp); 	// Main menu
	mc->rcmenu = LoadMenuBar(0, uifname, RCMenus);	// Right click menu
	

	// Then the container panels								 
	pc->PProgCPan = LoadPanel(pc->PProgPan, uifname, PPPanel); // Pulse program instr container
	pc->PPConfigCPan = LoadPanel(pc->PPConfigPan, uifname, PPConfigP); // ND instr container
	
	
	// Create two panels for containing the current location in acquisition space.
	dc->cloc[0] = LoadPanel(dc->fid, uifname, CurrentLoc);
	dc->cloc[1] = LoadPanel(dc->spec, uifname, CurrentLoc);
	
	// Move everything to where it belongs
	SetPanelPos(pc->PProgCPan, 5, 2); 	// Move the instruction container where it should go
	DisplayPanel(pc->PProgCPan); 		// Display the instruction container
	
	SetPanelPos(pc->PPConfigCPan, 32, 8); 	// Move MD instruction container to where it belongs
	DisplayPanel(pc->PPConfigCPan);		// Display the MD instruction container
	SetPanelAttribute(pc->PPConfigCPan, ATTR_DIMMED, 1);
		
	int i;
	for(i = 0; i<2; i++) { // Place the current location panels and display them
		SetPanelPos(dc->cloc[i], 250, 1200);
		DisplayPanel(dc->cloc[i]);
	}
	
	// Now all the constant stuff
	initialize_uicontrols();
	
	initialize_program();
	return 0;
}

void initialize_program() {
	// Gets the initial values for uipc. For now it just initializes to
	// a blank program, but eventually this will load a program from file
	
	// First things first, create the uipc variable
	uipc = malloc(sizeof(ui_ppconfig));
	
	/*  Uncomment this when you get session saving working.
	char last_prog[16] = "SavedSession.nc";
	PPROGRAM *p = malloc(sizeof(PPROGRAM));
	LoadPulseProgram(last_prog, p, "program");
	*/
	
	uipc->ni = 1;
	uipc->max_ni = 1;
	
	uipc->nd = 0;
	uipc->max_nd = 0;
	uipc->ndins = 0;
	uipc->dim_steps = NULL;
	uipc->dim_ins = NULL;
	uipc->ins_dims = NULL;
	uipc->ins_state = NULL;
	uipc->nd_delays = NULL;
	uipc->nd_data = NULL;
	
	uipc->nc = 0;
	uipc->max_nc = 0;
	uipc->ncins = 0;
	uipc->cyc_steps = NULL;
	uipc->cyc_ins = NULL;
	uipc->ins_cycs = NULL;
	uipc->c_instrs = NULL;
	uipc->max_cinstrs = NULL;
	
	//	Now allocate memory for the instruction arrays, then create one of each
	pc->inst = malloc(sizeof(int));
	pc->cinst = malloc(sizeof(int));
	
	pc->inst[0] = LoadPanel(pc->PProgCPan, pc->uifname, PulseInstP);
	pc->cinst[0] = LoadPanel(pc->PPConfigCPan, pc->uifname, MDInstr);

	SetPanelPos(pc->inst[0], 25, 7);	// Move the first instruction to where it belongs
	DisplayPanel(pc->inst[0]);			// Display the first instruction
	SetCtrlAttribute(pc->inst[0], pc->xbutton, ATTR_DISABLE_PANEL_THEME, 1);
	
	SetPanelPos(pc->cinst[0], 25, 7); 	// Move the first MD instruction to where it belongs
	DisplayPanel(pc->cinst[0]);			// Display the first MD instruction
	SetPanelAttribute(pc->cinst[0], ATTR_DIMMED, 1);
}

void initialize_uicontrols() {
	// Function that populates the static portions of mc, pc and dc;
	int i;
	// Populate the main panel controls
	mc->pathb[0] = MainPanel_DirectorySelect;
	mc->path[0] = MainPanel_Path;
	mc->basefname[0] = MainPanel_Filename;
	mc->pbrun[0] = MainPanel_Running;
	mc->pbwait[0] = MainPanel_Waiting;
	mc->pbstop[0] = MainPanel_Stopped;
	mc->mainstatus[0] = MainPanel_IsRunning;
	mc->startbut[0] = MainPanel_Start;
	mc->stopbut[0] = MainPanel_Stop;
	
	// Then populate the main panel panels
	mc->pathb[1] = mc->mp;
	mc->path[1] = mc->mp;
	mc->basefname[1] = mc->mp;
	mc->pbrun[1] = mc->mp;
	mc->pbwait[1] = mc->mp;
	mc->pbstop[1] = mc->mp;
	mc->mainstatus[1] = mc->mp;
	mc->startbut[1] = mc->mp;
	mc->stopbut[1] = mc->mp;

	
	// Populate the data tab controls
	dc->fgraph = FID_Graph;		//FID controls first
	dc->fauto = FID_Autoscale;
	dc->fxpos = FID_CursorX;
	dc->fypos = FID_CursorY;
	dc->fcring = FID_ChanPrefs;
	dc->fccol = FID_ChanColor;
	dc->fcgain = FID_Gain;
	dc->fcoffset = FID_Offset;
	dc->fpolysub = FID_PolySubtract;
	dc->fpsorder = FID_PolyFitOrder;
	
	dc->fchans[0] = FID_Chan1;	// FID channel controls
	dc->fchans[1] = FID_Chan2;
	dc->fchans[2] = FID_Chan3;
	dc->fchans[3] = FID_Chan4;
	dc->fchans[4] = FID_Chan5;
	dc->fchans[5] = FID_Chan6;
	dc->fchans[6] = FID_Chan7;
	dc->fchans[7] = FID_Chan8;

	dc->sgraph = Spectrum_Graph;	//Next Spectrum controls
	dc->sauto = Spectrum_Autoscale;
	dc->sxpos = Spectrum_CursorX;
	dc->sypos = Spectrum_CursorY;
	dc->scring = Spectrum_ChanPrefs;
	dc->sccol = Spectrum_ChanColor;
	dc->scgain = Spectrum_Gain;
	dc->scoffset = Spectrum_Offset;
	dc->spolysub = Spectrum_PolySubtract;
	dc->spsorder = Spectrum_PolyFitOrder;
	
	dc->schans[0] = Spectrum_Chan1;	// Spectrum channel controls
	dc->schans[1] = Spectrum_Chan2;
	dc->schans[2] = Spectrum_Chan3;
	dc->schans[3] = Spectrum_Chan4;
	dc->schans[4] = Spectrum_Chan5;
	dc->schans[5] = Spectrum_Chan6;
	dc->schans[6] = Spectrum_Chan7;
	dc->schans[7] = Spectrum_Chan8;
	
	dc->sphase[0] = Spectrum_PhaseKnob;
	dc->sphorder[0] = Spectrum_PhaseCorrectionOrder;
	dc->sfftring[0] = Spectrum_Channel;

	dc->sphase[1] = dc->spec;	// Initialize the panels for the dc
	dc->sphorder[1] = dc->spec;
	dc->sfftring[1] = dc->spec;
	
	dc->ctrans[0] = CurrentLoc_TransientNum; 	// Now the current location controls
	dc->idrings[0][0] = CurrentLoc_IDVal1;		// Ring control
	dc->idlabs[0][0] = CurrentLoc_ID1;			// Label
	dc->idrings[1][0] = CurrentLoc_IDVal2;		
	dc->idlabs[1][0] = CurrentLoc_ID2;	
	dc->idrings[2][0] = CurrentLoc_IDVal3;		
	dc->idlabs[2][0] = CurrentLoc_ID3;	
	dc->idrings[3][0] = CurrentLoc_IDVal4;		
	dc->idlabs[3][0] = CurrentLoc_ID4;	
	dc->idrings[4][0] = CurrentLoc_IDVal5;		
	dc->idlabs[4][0] = CurrentLoc_ID5;	
	dc->idrings[5][0] = CurrentLoc_IDVal6;		
	dc->idlabs[5][0] = CurrentLoc_ID6;	
	dc->idrings[6][0] = CurrentLoc_IDVal7;		
	dc->idlabs[6][0] = CurrentLoc_ID7;	
	dc->idrings[7][0] = CurrentLoc_IDVal8;		
	dc->idlabs[7][0] = CurrentLoc_ID8;
	
	dc->ctrans[1] = dc->spec;			// The panels now
	for(i = 0; i<8; i++) {
		dc->idrings[i][1] = dc->spec;
		dc->idlabs[i][1] = dc->spec;
	}
	
	// Finally populate the pulse program controls
	pc->trig_ttl[0] = PulseProg_Trigger_TTL;
	pc->ninst[0] = PulseProg_NumInst;
	pc->rc[0] = PulseProg_ContinuousRun;
	pc->numcycles[0] = PulseProg_PhaseCycles;
	pc->trans[0] = PulseProg_TransientNum;

	// And the panel bits
	pc->trig_ttl[1] = pc->PProgPan;
	pc->ninst[1] = pc->PProgPan;
	pc->rc[1] = pc->PProgPan;
	pc->numcycles[1] = pc->PProgPan;
	pc->trans[1] = pc->PProgPan;

	pc->idrings[0][0] = PulseProg_IDVal1;		// Ring control
	pc->idlabs[0][0] = PulseProg_ID1;			// Label
	pc->idrings[1][0] = PulseProg_IDVal2;		
	pc->idlabs[1][0] = PulseProg_ID2;	
	pc->idrings[2][0] = PulseProg_IDVal3;		
	pc->idlabs[2][0] = PulseProg_ID3;	
	pc->idrings[3][0] = PulseProg_IDVal4;		
	pc->idlabs[3][0] = PulseProg_ID4;	
	pc->idrings[4][0] = PulseProg_IDVal5;		
	pc->idlabs[4][0] = PulseProg_ID5;	
	pc->idrings[5][0] = PulseProg_IDVal6;		
	pc->idlabs[5][0] = PulseProg_ID6;	
	pc->idrings[6][0] = PulseProg_IDVal7;		
	pc->idlabs[6][0] = PulseProg_ID7;	
	pc->idrings[7][0] = PulseProg_IDVal8;		
	pc->idlabs[7][0] = PulseProg_ID8;
	
	for(i = 0; i<8; i++) {
		pc->idrings[i][1] = pc->PProgPan;
		pc->idlabs[i][1] = pc->PProgPan;
	}
	
	pc->ndon[0] = PPConfig_NDimensionalOn;		// Now the Program Config Tab
	pc->ndims[0] = PPConfig_NumDimensions;
	pc->sr[0] = PPConfig_SampleRate;
	pc->nt[0] = PPConfig_NTransients;
	pc->np[0] = PPConfig_NPoints;
	pc->at[0] = PPConfig_AcquisitionTime;
	pc->dev[0] = PPConfig_Device;
	pc->nc[0] = PPConfig_NumChans;
	pc->ic[0] = PPConfig_AcquisitionChannel;
	pc->cc[0] = PPConfig_CounterChan;
	pc->curchan[0] = PPConfig_ChannelGain;
	pc->min[0] = PPConfig_InputMin;
	pc->max[0] = PPConfig_InputMax;
	pc->trigc[0] = PPConfig_Trigger_Channel;
	pc->trige[0] = PPConfig_TriggerEdge;
	pc->skip[0] = PPConfig_SkipCondition;
	pc->skiptxt[0] = PPConfig_SkipConditionExpr;
	pc->timeest[0] = PPConfig_EstimatedTime;
	
	pc->ndon[1] = pc->PPConfigPan;		// And the panel bits
	pc->ndims[1] = pc->PPConfigPan;
	pc->sr[1] = pc->PPConfigPan;
	pc->nt[1] = pc->PPConfigPan;
	pc->np[1] = pc->PPConfigPan;
	pc->at[1] = pc->PPConfigPan;
	pc->dev[1] = pc->PPConfigPan;
	pc->nc[1] = pc->PPConfigPan;
	pc->ic[1] = pc->PPConfigPan;
	pc->cc[1] = pc->PPConfigPan;
	pc->curchan[1] = pc->PPConfigPan;
	pc->min[1] = pc->PPConfigPan;
	pc->max[1] = pc->PPConfigPan;
	pc->trigc[1] = pc->PPConfigPan;
	pc->trige[1] = pc->PPConfigPan;
	pc->skip[1] = pc->PPConfigPan;;
	pc->skiptxt[1] = pc->PPConfigPan;
	pc->timeest[1] = pc->PPConfigPan;
	
	pc->ins_num = PulseInstP_InstNum;	// The inst panel controls
	pc->instr = PulseInstP_Instructions;
	pc->instr_d = PulseInstP_Instr_Data;
	pc->delay = PulseInstP_InstDelay;
	pc->delayu = PulseInstP_TimeUnits;
	pc->pcon = PulseInstP_PhaseCyclingOn;
	pc->pcstep = PulseInstP_PhaseCycleStep;
	pc->pclevel = PulseInstP_PhaseCycleLevel;
	pc->pcsteps = PulseInstP_NumCycles;
	pc->scan = PulseInstP_Scan;
	pc->uparrow = PulseInstP_UpButton;
	pc->downarrow = PulseInstP_DownButton;
	pc->xbutton = PulseInstP_xButton;
	
	pc->TTLs[0] = PulseInstP_TTL0;	 	// Each TTL is its own control
	pc->TTLs[1] = PulseInstP_TTL1;	 	
	pc->TTLs[2] = PulseInstP_TTL2;	 	
	pc->TTLs[3] = PulseInstP_TTL3; 		
	pc->TTLs[4] = PulseInstP_TTL4; 		
	pc->TTLs[5] = PulseInstP_TTL5;	 	
	pc->TTLs[6] = PulseInstP_TTL6; 		
	pc->TTLs[7] = PulseInstP_TTL7; 		
	pc->TTLs[8] = PulseInstP_TTL8;	 	
	pc->TTLs[9] = PulseInstP_TTL9; 		
	pc->TTLs[10] = PulseInstP_TTL10;	
	pc->TTLs[11] = PulseInstP_TTL11;	
	pc->TTLs[12] = PulseInstP_TTL12;	
	pc->TTLs[13] = PulseInstP_TTL13;	
	pc->TTLs[14] = PulseInstP_TTL14;	
	pc->TTLs[15] = PulseInstP_TTL15;	
	pc->TTLs[16] = PulseInstP_TTL16;	
	pc->TTLs[17] = PulseInstP_TTL17;	
	pc->TTLs[18] = PulseInstP_TTL18;	
	pc->TTLs[19] = PulseInstP_TTL19;	
	pc->TTLs[20] = PulseInstP_TTL20; 	
	pc->TTLs[21] = PulseInstP_TTL21; 	
	pc->TTLs[22] = PulseInstP_TTL22; 	
	pc->TTLs[23] = PulseInstP_TTL23;
	
	pc->cins_num = MDInstr_InstrNum;	// The cinst panel controls now
	pc->cinstr = MDInstr_Instructions;
	
	pc->disp_init = MDInstr_InitDisplay;
	pc->del_init = MDInstr_InitTime;
	pc->delu_init = MDInstr_InitTimeUnits;
	pc->dat_init = MDInstr_InitInstrData;
	pc->disp_inc = MDInstr_IncDisplay;
	pc->del_inc = MDInstr_IncTime;
	pc->delu_inc = MDInstr_IncTimeUnits;
	pc->dat_inc = MDInstr_IncInstrData;
	pc->disp_fin = MDInstr_FDisplay;
	pc->del_fin = MDInstr_FTime;
	pc->delu_fin = MDInstr_FTimeUnits;
	pc->dat_fin = MDInstr_FInstrData;
	
	pc->cexpr_delay = MDInstr_IncDelayExpression;
	pc->cexpr_data  = MDInstr_IncDataExpression;
	pc->nsteps = MDInstr_NumSteps;
	pc->dim = MDInstr_Dimension;
	pc->vary = MDInstr_VaryInstr;

}

//////////////////////////////////////////////////////
//                                      	        //
//          Pulse Instruction UI Functions 	  		//
//                                                  //
//////////////////////////////////////////////////////

void ui_cleanup() {
	// Function for cleaning up the input so it's consistent - i.e. turning off ND if there are no varied instrs.
	int i, dim, cyc, nl, on;
	int ndims=0, npc=0, dimf=0, cycf=0, ndins=0, ncins=0;
	int *dim_steps = malloc(sizeof(int)*uipc->nd), *cyc_steps = malloc(sizeof(int)*uipc->nc);
	int *dim_ins = malloc(sizeof(int)*uipc->ndins), *ins_dims = malloc(sizeof(int)*uipc->ndins);
	int *cyc_ins = malloc(sizeof(int)*uipc->ncins), *ins_cycs = malloc(sizeof(int)*uipc->ncins);
	
	// Update the uipc function from the controls and reduce the nominal dimensionality of the experiment
	// if there are cycles/dimensions with only 1 step or which appear in no instructions. 
	for(i = 0; i<uipc->ni; i++) {
		// ND Bit first
		GetNumListItems(pc->cinst[i], pc->dim, &nl);
		on = get_nd_state(i);
		if(nl > 0 && on) {
			GetCtrlVal(pc->cinst[i], pc->pclevel, &dim);
			if(uipc->dim_steps[dim]) {
				dim_ins[ndins] = i;
				ins_dims[ndins++] = dim;
				if((dimf - (dimf=(dimf|(int)pow(2,dim)))) < 0) 	 
					GetCtrlVal(pc->cinst[i], pc->nsteps, &dim_steps[ndims++]);
			}
		} else if(on) 
			update_nd_state(i, 0);
		
		// Now phase cycling
		GetNumListItems(pc->inst[i], pc->pclevel, &nl);
		on = get_pc_state(i);
		if(nl > 0 && on) {
			GetCtrlVal(pc->inst[i], pc->pclevel, &cyc);
			if(uipc->cyc_steps[cyc] > 1) {
				cyc_ins[ncins] = i;
				ins_cycs[ncins++] = cyc;
				if((cycf - (cycf=(cycf|(int)pow(2,cyc)))) < 0) // Adds the flag and returns negative if that was necessary
					GetCtrlVal(pc->inst[i], pc->pcsteps, &cyc_steps[npc++]);
			} else
				update_pc_state(i, 0);
		} else if(on)
			update_pc_state(i, 0);
	}
	
	// Now if we deleted dimensions or cycles, the nominal dimension of everything higher than the deleted
	// dimensions will be reduced. We need to update the uipc variable according.
	for(int j=0; j<uipc->nd; j++) {
		if(!(dimf & (int)pow(2, j))) {
			for(i=0; i<ndins; i++) {
				if(ins_dims[i] > j)	// The == condition should never be met, so no worries there
					ins_dims[i]--;	
			}
		}
	}
	
	for(int j=0; j<uipc->nc; j++) {	// Same thing, but for cycles, obviously.
		if(!(cycf & (int)pow(2, j))) {
			for(i=0; i<ncins; i++) {
				if(ins_cycs[i] > j)	
					ins_cycs[i]--;	
			}
		}
	}
	
	// Now we need to update the uipc and controls appropriately.
	uipc->nd = uipc->max_nd = ndims;
	uipc->nc = uipc->max_nc = npc;
	uipc->ndins = ndins;
	uipc->ncins = ncins;
	
	// Tighten up the memory allocation of these arrays
	dim_steps = realloc(dim_steps, sizeof(int)*ndims);
	dim_ins = realloc(dim_ins, sizeof(int)*ndins);
	ins_dims = realloc(ins_dims, sizeof(int)*ndins);
	cyc_steps = realloc(cyc_steps, sizeof(int)*npc);
	cyc_ins = realloc(cyc_ins, sizeof(int)*ncins);
	ins_cycs = realloc(ins_cycs, sizeof(int)*ncins);
 
	// Free the old arrays - if you don't do this there will be memory leaks when you reassign the pointer
	free(uipc->dim_steps);
	free(uipc->dim_ins);
	free(uipc->ins_dims);
	free(uipc->cyc_steps);
	free(uipc->cyc_ins);
	free(uipc->ins_cycs);
	
	// Reassign the pointers to the new arrays.
	uipc->dim_steps = dim_steps;
	uipc->dim_ins = dim_ins;
	uipc->ins_dims = ins_dims;
	uipc->cyc_steps = cyc_steps;
	uipc->cyc_ins = cyc_ins;
	uipc->ins_cycs = ins_cycs;

	// Update the UI.
	populate_dim_points();
	populate_cyc_points();
	SetCtrlVal(pc->ndims[1], pc->ndims[0], uipc->nd);
	SetCtrlVal(pc->ndon[1], pc->ndon[0], uipc->nd);
	SetCtrlVal(pc->numcycles[1], pc->numcycles[0], uipc->nc);
}

/************* Instruction Parameter Manipulation **************/ 
int move_ttl(int num, int to, int from) {
	// Move a TTL from "from" to "to", and shift all the others in response.
	// This is a "safe" function, and will skip broken ttls. If the initial or
	// final value is marked broken, the operation will fail and return -1.
	int panel = pc->inst[num];
	
	if(to == from)
		return 0;
	
	int high, low, buff1, buff2, i;
	if(to > from) {
		high = to;
		low = from;
	} else {
		high = from;
		low = to;
	}

	int broken_ttls = 0;
	for(i = low; i<high; i++) {
		GetCtrlAttribute(panel, pc->TTLs[i], ATTR_DIMMED, &buff1);
		if(buff1) 
			broken_ttls += (int)pow(2, i);
	}
	
	if(broken_ttls && ((int)pow(2, low) | (int)pow(2, high)) & broken_ttls)
		return -1;
	
	GetCtrlVal(panel, pc->TTLs[low], &buff1);
	i = low;
	int next = i;
	while(i<high) {
		next++;
		while(broken_ttls && broken_ttls & (int)pow(2, next)) { // Skip over any ttls that are broken.
			next++;
			if(next >= high)
				break;
		}
		
		GetCtrlVal(panel, pc->TTLs[next], &buff2);
		SetCtrlVal(panel, pc->TTLs[i], buff2);
		i = next;
	}
	
	return 0;
}

void change_instruction(int num) {
	// Dims instr_data if necessary and sets up the minimum
	int ind;
	int panel = pc->inst[num];
	
	GetCtrlVal(panel, pc->instr, &ind);
	
	// Change the corresponding ring control on the ND side.
	SetCtrlVal(pc->cinst[num], pc->cinstr, ind);
	
	// Set up the minimum
	int min = instr_data_min(ind);
	SetCtrlAttribute(panel, pc->instr_d, ATTR_MIN_VALUE, min);
  
	int nd_ctrls[4] = {pc->dat_init, pc->dat_inc, pc->dat_fin, pc->cexpr_data};
	int ndnum = 4;
	
	if(takes_instr_data(ind)) {
		SetCtrlAttribute(panel, pc->instr_d, ATTR_DIMMED, 0);
		change_control_mode(pc->cinst[num], nd_ctrls, ndnum, VAL_HOT);
	} else {
		SetCtrlVal(panel, pc->instr_d, 0);
		SetCtrlAttribute(panel, pc->instr_d, ATTR_DIMMED, 1);
		change_control_mode(pc->cinst[num], nd_ctrls, ndnum, VAL_INDICATOR);
	}
}
/************ ND Instruction Parameter Manipulation ************/  
void toggle_nd() {		// Toggles whether or not multi-dimensional instructions are on.
	int dimmed, val, nd, i;
	GetCtrlVal(pc->ndon[1], pc->ndon[0], &val);
	val = !val; // This is what it means to toggle something
	
	if(val) {
		dimmed = 0;
		change_num_dims();
	} else {
		dimmed = 1;
		uipc->nd = 0;
	}
	
	// Set the controls appropriately.
	SetCtrlVal(pc->ndon[1], pc->ndon[0], val);
	SetPanelAttribute(pc->PPConfigCPan, ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pc->ndims[1], pc->ndims[0], ATTR_DIMMED, dimmed);
	for(i = 0; i<uipc->max_ni; i++)
		SetPanelAttribute(pc->cinst[i], ATTR_DIMMED, dimmed);
	
}

void update_nd_state(int num, int state) {	// Changes the state of a given ND control.   
	// State 0 = Off
	// State 1 = On/Red    	=> Linear delay/instr increment
	// State 2 = On/Blue   	=> Expression mode
	// State == -1 -> Figure out what state we're in for us.
	
	int panel = pc->cinst[num];	// Convenience
	
	// Get the state if we need it
	if(state < 0) {
		state = get_nd_state(num);
	}
	
	// Set up the uipc variable.
	int ind = -1, i;			
	for(i=0; i<uipc->ndins; i++) {		// Get the place in the dim_ins array
		if(uipc->dim_ins[i] == num) {   // -1 if it's not there.
			ind = i;
			break;
		}
	}
	
	if(!state) {
		if(ind >=0) {
			// Remove the index from the lists and decrement the number of instructions
			remove_array_item(uipc->dim_ins, ind, uipc->ndins);
			remove_array_item(uipc->ins_state, ind, uipc->ndins);
			remove_array_item(uipc->ins_dims, ind, uipc->ndins);
			
			if(uipc->nd_data[ind] != NULL)
				free(uipc->nd_data[ind]);
			if(uipc->nd_delays[ind] != NULL)
				free(uipc->nd_delays[ind]);
			
			remove_array_item_void(uipc->nd_data, ind, uipc->ndins, 2);
			remove_array_item_void(uipc->nd_delays, ind, uipc->ndins, 3);
			
			// Now reallocate the size of the arrays
			if(--uipc->ndins == 0) {
				free(uipc->dim_ins);
				free(uipc->ins_state);
				free(uipc->ins_dims);
				free(uipc->nd_data);
				free(uipc->nd_delays);
				
				uipc->dim_ins = NULL;
				uipc->ins_state = NULL;
				uipc->ins_dims = NULL;
				uipc->nd_data = NULL;
				uipc->nd_delays = NULL;
			} else {
				uipc->dim_ins = realloc(uipc->dim_ins, sizeof(int)*uipc->ndins);
				uipc->ins_dims = realloc(uipc->ins_dims, sizeof(int)*uipc->ndins);
				uipc->ins_state = realloc(uipc->ins_state, sizeof(int)*uipc->ndins);
				uipc->nd_data = realloc(uipc->nd_data, sizeof(int*)*uipc->ndins);
				uipc->nd_delays = realloc(uipc->nd_delays, sizeof(double*)*uipc->ndins);	
			}
			
		}
	} else {
		if(ind < 0) {	// only need to update if we're not already there.
			uipc->ndins++;			// Increment the number of instructions varied 
			
			// Memory allocation is important.
			if(uipc->dim_ins == NULL)
				uipc->dim_ins = malloc(sizeof(int)*uipc->ndins);
			else
				uipc->dim_ins = realloc(uipc->dim_ins, sizeof(int)*uipc->ndins);
			
			if(uipc->ins_dims == NULL)
				uipc->ins_dims = malloc(sizeof(int)*uipc->ndins);
			else
				uipc->ins_dims = realloc(uipc->ins_dims, sizeof(int)*uipc->ndins);
			
			if(uipc->ins_state == NULL)
				uipc->ins_state = malloc(sizeof(int)*uipc->ndins);
			else
				uipc->ins_state = realloc(uipc->ins_state, sizeof(int)*uipc->ndins);
			
			if(uipc->nd_data == NULL)
				uipc->nd_data = malloc(sizeof(int*)*uipc->ndins);
			else
				uipc->nd_data = realloc(uipc->nd_data, sizeof(int*)*uipc->ndins);
			
			if(uipc->nd_delays == NULL)
				uipc->nd_delays = malloc(sizeof(double*)*uipc->ndins);
			else
				uipc->nd_delays = realloc(uipc->nd_delays, sizeof(double*)*uipc->ndins);
	
			// Need to update the list controls
			int nl;
			GetNumListItems(panel, pc->dim, &nl);
			if(nl < uipc->nd) {
				int elements;
				char **c = generate_char_num_array(1, uipc->nd, &elements);
				
				for(i=nl; i<uipc->nd; i++) 
					InsertListItem(panel, pc->dim, -1, c[i], i);
				
				if(c != NULL) {
					for(i = 0; i<elements; i++)
						free(c[i]);
					free(c);
				}
				
			} else if(nl > uipc->nd) 
				DeleteListItem(panel, pc->dim, uipc->nd, -1);
				  
			int dim;
			GetCtrlVal(panel, pc->dim, &dim);
			
			// Now insert our instruction at the end.
			uipc->ins_dims[uipc->ndins-1] = dim;
			uipc->dim_ins[uipc->ndins-1] = num;
			uipc->ins_state[uipc->ndins-1] = state;
			uipc->nd_data[uipc->ndins-1] = NULL; 
			uipc->nd_delays[uipc->ndins-1] = NULL; 
		}
	}
	
	
	// Set up the controls that need to be dimmed or hidden
	int inc_ctrls[4] = {pc->del_inc, pc->delu_inc,
						pc->dat_inc, pc->disp_inc};
	int exprs_ctrls[2] = {pc->cexpr_data, pc->cexpr_delay};
	int all_ctrls[10] = {pc->del_init, pc->delu_init, pc->dat_init, 
						 pc->disp_init, pc->disp_fin, pc->del_fin, 
						 pc->delu_fin, pc->dat_fin, pc->nsteps, pc->dim};
	int inc_num = 4, exprs_num = 2, all_num = 10, main_num = 2;
	int main_ctrls[2] = {pc->delay, pc->instr_d};
	int exprs = MC_HIDDEN, inc = MC_HIDDEN, main = MC_DIMMED, all = 0;
	int color = VAL_RED;	// This will change as necessary
	
	if(state > 0) {		// A varied state
		// If it's on, we want to switch the border to blue.
		SetCtrlVal(panel, pc->vary, 1);
		SetCtrlAttribute(pc->inst[num], pc->ins_num, ATTR_DISABLE_PANEL_THEME, 1);
	}
	
	// Dim/Hide the controls that need to be dimmed and/or hidden
 	if(state == 0) {
		// In the 0 state, we're off, so that should be dimmed time increment stuff
		SetCtrlAttribute(pc->inst[num], pc->ins_num, ATTR_FRAME_COLOR, 14737379);	// The default color, for whatever reason
		SetCtrlAttribute(pc->inst[num], pc->ins_num, ATTR_DISABLE_PANEL_THEME, 0);
		SetCtrlVal(panel, pc->vary, 1);
		
		main = 0;
		all = MC_DIMMED;
		inc = MC_DIMMED;
	} else if(state == 1) {
		inc = 0;		   // No color change needed
		SetCtrlAttribute(pc->inst[num], pc->ins_num, ATTR_FRAME_COLOR, VAL_RED);
	} else if(state == 2) {
		exprs = 0;
		color = VAL_BLUE;
		SetCtrlAttribute(pc->inst[num], pc->ins_num, ATTR_FRAME_COLOR, VAL_BLUE);
	}
	
	// Set up the visibility modes
	change_visibility_mode(pc->inst[num], main_ctrls, main_num, main);
	change_visibility_mode(panel, all_ctrls, all_num, all);
	change_visibility_mode(panel, inc_ctrls, inc_num, inc);
	change_visibility_mode(panel, exprs_ctrls, exprs_num, exprs);
	
	SetCtrlAttribute(panel, pc->vary, ATTR_ON_COLOR, color); // Change color to red.
	SetCtrlVal(panel, pc->vary, state);
}

void change_num_dims() { 	// Updates the number of dimensions in the experiment
	int i, j, nd;
	GetCtrlVal(pc->ndims[1], pc->ndims[0], &nd);
	if(--nd == uipc->nd)
		return;
	
	// Update the UI elements
	if(uipc->ndins) {
		char **c = NULL;
		int elements = 0;
		if(nd > uipc->nd)
		c = generate_char_num_array(1, nd, &elements);
		
		// We are going to be updating uipc as we go, so we want a local copy so that as we
		// iterate through it we don't end up skipping instructions and such
		int ndins = uipc->ndins;
		int *ins_dims = malloc(sizeof(int)*ndins), *dim_ins = malloc(sizeof(int)*ndins);
		for(i = 0; i<ndins; i++) {
			ins_dims[i] = uipc->ins_dims[i];
			dim_ins[i] = uipc->dim_ins[i];
		}
		
		for(i = 0; i<ndins; i++) {
			if(ins_dims[i] >= nd) {
				update_nd_state(dim_ins[i], 0);
				continue;
			}
		
			if(nd < uipc->nd) {
				DeleteListItem(pc->cinst[dim_ins[i]], pc->dim, nd, -1); // Delete the rest of them.
			} else {
				for(j=uipc->nd; j<nd; j++)
					InsertListItem(pc->cinst[dim_ins[i]], pc->dim, -1, c[j], j);
			}
		}
		
		free(dim_ins);
		free(ins_dims);
		if(c != NULL) {
			for(i = 0; i<elements; i++)
				free(c[i]);
			free(c);
		}
	}
	
	// Update the uipc variable
	uipc->nd = nd;
	if(uipc->max_nd < nd) {
		if(uipc->dim_steps == NULL)
			uipc->dim_steps = malloc(sizeof(int)*nd);
		else
			uipc->dim_steps = realloc(uipc->dim_steps, sizeof(int)*nd);
		
		for(i=uipc->max_nd; i<nd; i++)
			uipc->dim_steps[i] = 2;
		
		uipc->max_nd = nd;
	}
}

void change_num_dim_steps(int dim, int steps) { // Updates number of steps in the given dimension
	int i, j, nl, cp;
	
	if (steps == uipc->dim_steps[dim])
		return;	// No change
	
	// Update the controls
	for(i = 0; i<uipc->ndins; i++) {
		if(uipc->ins_dims[i] == dim)
			change_nd_steps_instr(uipc->dim_ins[i], steps);
	}
	
	// Now update the uipc variable and we're done.
	uipc->dim_steps[dim] = steps;
 
}

void change_nd_steps_instr(int num, int steps) { // Changes the number of steps for a given instr
	int panel = pc->cinst[num];
	
	SetCtrlVal(panel, pc->nsteps, steps);
	
	// Now we need to update the controls.
	int state = get_nd_state(num);
	if(state == 1) {	// Generate from the simple increments.
		// We'll keep initial and final constant here.
		update_nd_increment(num, MC_INC);
	} else if (state == 2) {
		update_nd_from_exprs(num);	
	}
	
}

void change_dimension (int num) {	// Change the dimension of a given instruction
	int dim, ind, i;
	int panel = pc->cinst[num];
	
	// Pretty much all you need to do is update uipc (for now)
	GetCtrlVal(panel, pc->dim, &dim);		// Get the new dimension
	
	for(i=0; i<uipc->ndins; i++) {
		if(uipc->dim_ins[i] == num)
			break;
	}
	
	if(i == uipc->ndins) 
		return;				// Something's wrong, return.
	
	
	ind = i;
	// Update ins_dims and change the number of steps for the control.
	uipc->ins_dims[ind] = dim;

	change_nd_steps_instr(num, uipc->dim_steps[dim]);
}

void populate_dim_points() {	// Function for updating the UI with the values from the uipc var
	int j, on, nl, dim;
	
	// A char array of labels
	int elements;
	char **c = generate_char_num_array(1, uipc->nd, &elements);
	
	int panel;
	for(int i = 0; i<uipc->ndins; i++) {
		panel = pc->cinst[uipc->dim_ins[i]];
		on = get_nd_state(i);
		if(on) {	// This condition should never be met, but it's not so bad to have it
			// First make the number of dimensions per control correct
			GetNumListItems(panel, pc->dim, &nl);
			if(nl < uipc->nd) {
				for(j=nl; j<uipc->nd; j++)
					InsertListItem(panel, pc->dim, -1, c[j], j);
			} else if (nl > uipc->nd) {
				DeleteListItem(panel, pc->dim, uipc->nd, -1);
			}
			
			// Now update the number of steps.
			GetCtrlIndex(panel, pc->dim, &dim);
			SetCtrlVal(panel, pc->nsteps, uipc->dim_steps[dim]);
		}
	}
	
	for(j=0; j<elements; j++)
		free(c[j]);
	free(c);
}

void update_nd_increment(int num, int mode) { // Updates Initial, Increment and Final controls
	// Modes:
	// MC_INIT = Change initial, leave the other two
	// MC_INC = Change increment, leave the other two
	// MC_FINAL = Change final
	
	double init, inc, fin; 		// Times
	int initu, incu, finu;  	// Final times
	int instr;					// The instruction
	int steps;					// The number of steps
	int panel = pc->cinst[num]; // Convenience
	
	// Get the values that are there now.
	GetCtrlVal(panel, pc->del_init, &init);
	GetCtrlVal(panel, pc->del_inc, &inc);
	GetCtrlVal(panel, pc->del_fin, &fin);
	GetCtrlVal(panel, pc->delu_init, &initu);
	GetCtrlVal(panel, pc->delu_inc, &incu);
	GetCtrlVal(panel, pc->delu_fin, &finu);
	GetCtrlVal(panel, pc->cinstr, &instr);
	
	// Convert everything to nanoseconds.
	init *= pow(1000, initu);
	inc *= pow(1000, incu);
	fin *= pow(1000, finu);
	
	// Get the dimension it varies along
	GetCtrlVal(panel, pc->nsteps, &steps);

	 // First the delays
	steps--; 	// Temporary decrement, because the step indexing is 0-based.
	switch(mode) {
		case MC_INIT:
			init = fin-(inc*steps);
			
			if(init < 0) {		// Init can't be negative.
				init = 0;
				fin = steps*inc;
			}
			break;
		case MC_INC:
			inc = (fin-init)/steps;
			break;
		case MC_FINAL:
			fin = init+(inc*steps);
			
			if(fin < 0) {		// Only possible when inc is negative.
				fin = 0;
				init = -inc*steps;
			}
			break;
	}
	steps++;
	
	// Update the uipc variable.
	int ind, i;
	for(i=0; i<uipc->ndins; i++) {
		if(uipc->dim_ins[i] == num)
			break;
	}
	if(i < uipc->ndins)
		ind = i;
	else
		ind = -1;
	
	if(ind >= 0) {
		if(uipc->nd_delays[ind] == NULL) 
			uipc->nd_delays[ind] = malloc(sizeof(double)*steps);
		else
			uipc->nd_delays[ind] = realloc(uipc->nd_delays[ind], sizeof(double)*steps);
	
		for(i=0; i<steps; i++) 
			uipc->nd_delays[ind][i] = init+inc*i;
	}
	
	// Convert back to the appropriate units
	init /= pow(1000, initu);
	inc /= pow(1000, incu);
	fin /= pow(1000, finu);
	
	// Set the controls to the new values
	SetCtrlVal(panel, pc->del_init, init);
	SetCtrlVal(panel, pc->del_inc, inc);
	SetCtrlVal(panel, pc->del_fin, fin);
	
	// Now if necessary, update the data increment controls
	if(takes_instr_data(instr)) {
		int initd, incd, find;	// Data

		GetCtrlVal(panel, pc->dat_init, &initd);
		GetCtrlVal(panel, pc->dat_inc, &incd);
	 	GetCtrlVal(panel, pc->dat_fin, &find);
		steps--;
		switch(mode) {
			case MC_INIT:
				initd = find-(incd*steps);
				
				if(initd < 0) {
					initd = 0;
					find = (incd*steps);
				}
				break;
			case MC_INC:
				incd = (int)((find-initd)/steps); // Round down
				
				// Keep init the same and recalculate final to account for rounding
				find = initd+(incd*steps);
				break;
			case MC_FINAL:
				find = initd+(incd*steps);
				
				if(find < 0) {
					find = 0;
					initd = find-(incd*steps);
				}
				break;
		}
		steps++;
		
		// Update the uipc variable
		if(ind >= 0) {
			if(uipc->nd_delays[ind] == NULL) 
				uipc->nd_data[ind] = malloc(sizeof(int)*steps);
			else
				uipc->nd_data[ind] = realloc(uipc->nd_data[ind], sizeof(int)*steps);
	
			for(int i=0; i<steps; i++) 
				uipc->nd_data[ind][i] = initd+incd*i;
		}
		
		SetCtrlVal(panel, pc->dat_init, initd);
		SetCtrlVal(panel, pc->dat_inc, incd);
	 	SetCtrlVal(panel, pc->dat_fin, find);
		
	}
}

/************ Phase Cycling Parameter Manipulation *************/ 
void update_pc_state(int num, int state) {
	// Right now there are only two states, on and off.
	int all_ctrls[3] = {pc->pcsteps, pc->pclevel, pc->pcstep};
	int all_num = 3, all;
	int panel = pc->inst[num];
	int i, ind = -1;

	// Find out if it's already in the list.
	if(uipc->cyc_ins != NULL) {
		for(i = 0; i<uipc->ncins; i++) {
			if(uipc->cyc_ins[i] == num) {
				ind = i;
				break;
			}
		}
	}
	
	if(!state) {
		all = MC_DIMMED;
		if(ind>=0) {
			// Remove the index from the lists.
			remove_array_item(uipc->cyc_ins, ind, uipc->ncins);
			remove_array_item(uipc->ins_cycs, ind, uipc->ncins);
			
			// Now we're going to decrement ncins and free the memory from those lists.
			if(--uipc->ncins == 0) {
				free(uipc->cyc_ins);
				free(uipc->ins_cycs);
				
				uipc->cyc_ins = NULL;
				uipc->ins_cycs = NULL;
			} else {
				uipc->cyc_ins = realloc(uipc->cyc_ins, sizeof(int)*uipc->ncins);
				uipc->ins_cycs = realloc(uipc->cyc_ins, sizeof(int)*uipc->ncins);
			}
		}
	} else {
		all = 0;
		// If we haven't turned on phase cycling yet, add a cycle
		if(!uipc->nc) {
			SetCtrlVal(pc->numcycles[1], pc->numcycles[0], 1);
			change_num_cycles();
		}
		
		if(ind < 0) {  	// If this isn't true, that's an error (for now).
			// Populate the rings.
			int nl;
			GetNumListItems(panel, pc->pclevel, &nl);
			int elements;
			char **c;
			
			if(uipc->nc > nl) {
				c = generate_char_num_array(1, uipc->nc, &elements);
			
				for(i = nl; i<uipc->nc; i++) 
					InsertListItem(panel, pc->pclevel, -1, c[i], i); 	// Insert the cycles
				
				if(c != NULL) {
					for(i = 0; i<elements; i++)
						free(c[i]);
					free(c);
				}
			} else if(uipc->nc < nl) 
				DeleteListItem(panel, pc->pclevel, uipc->nc, -1);
			
			int cyc;
			GetCtrlVal(panel, pc->pclevel, &cyc);
		
			GetNumListItems(panel, pc->pcstep, &nl);
			if(nl > uipc->cyc_steps[cyc]) 
				DeleteListItem(panel, pc->pcstep, nl, -1);
			else if(nl < uipc->cyc_steps[cyc]) {
				c = generate_char_num_array(1, uipc->cyc_steps[cyc], &elements);

				for(i = nl; i<uipc->cyc_steps[cyc]; i++) 
					InsertListItem(panel, pc->pcstep, -1, c[i], i);	
				
				if(c != NULL) {
					for(i = 0; i<elements; i++)
						free(c[i]);	
					free(c);
				}
			}
			
			// Now we need to update the uipc var.
			uipc->ncins++;
			if(uipc->cyc_ins == NULL)
				uipc->cyc_ins = malloc(uipc->ncins*sizeof(int));
			else
				uipc->cyc_ins = realloc(uipc->cyc_ins, uipc->ncins*sizeof(int));
			
			if(uipc->ins_cycs == NULL)
				uipc->ins_cycs = malloc(uipc->ncins*sizeof(int));
			else
				uipc->ins_cycs = realloc(uipc->ins_cycs, uipc->ncins*sizeof(int));
			
			uipc->ins_cycs[uipc->ncins-1] = cyc;
			uipc->cyc_ins[uipc->ncins-1] = num;
		} else {
			all = MC_DIMMED;
		}
	}
	
	change_visibility_mode(panel, all_ctrls, all_num, all);
	SetCtrlVal(panel, pc->pcon, state);
}

void change_num_cycles() {
	// Function called if the cycle number control is changed.
	int nc, i, j;
	GetCtrlVal(pc->numcycles[1], pc->numcycles[0], &nc);
	
	if(nc == uipc->nc)
		return;
	
	// Update the UI elements
	if(uipc->ncins) {
		char **c = NULL;
		int elements = 0;
		if(nc > uipc->nc)
		c = generate_char_num_array(1, nc, &elements);

		// We are going to be updating uipc as we go, so we want a local copy so that as we
		// iterate through it we don't end up skipping instructions and such
		int ncins = uipc->ncins;
		int *ins_cycs = malloc(sizeof(int)*ncins), *cyc_ins = malloc(sizeof(int)*ncins);
		for(i=0; i<ncins; i++){
			ins_cycs[i] = uipc->ins_cycs[i];
			cyc_ins[i] = uipc->cyc_ins[i];
		}
	
		for(j = 0; j<ncins; j++) {
			if(ins_cycs[j] >= nc) {
				update_pc_state(cyc_ins[j], 0);
				continue;
			}
		
			if(nc < uipc->nc) {
				DeleteListItem(pc->inst[cyc_ins[j]], pc->pclevel, nc, -1);	// Delete the rest of them	
			} else {
				for(i=uipc->nc; i<nc; i++) 
					InsertListItem(pc->inst[cyc_ins[j]], pc->pclevel, -1, c[i], i);
			}
		}
	
		free(cyc_ins);
		free(ins_cycs);
	
		if(c != NULL) {
			for(i = 0; i<elements; i++)
				free(c[i]);
			free(c);
		}
	}
	
	// Only if this has never been allocated - change_num_instructions will
	// take care of reallocation otherwise.
	if(uipc->c_instrs == NULL) {
		uipc->c_instrs = malloc(sizeof(PINSTR**)*uipc->max_ni);
		uipc->max_cinstrs = malloc(sizeof(int)*uipc->max_ni);
		for(i = 0; i<uipc->max_ni; i++) {
			uipc->c_instrs[i] = NULL;	// Null initialization
			uipc->max_cinstrs[i] = 0;	// -1 initialization
		}
	}

	// Update the uipc variable
	uipc->nc = nc;
	if(uipc->max_nc < nc) {								// We keep everything allocated as a form
		if(uipc->cyc_steps == NULL)						// of history, so there's no need to do  
			uipc->cyc_steps = malloc(sizeof(int)*nc);	// this unless you go past the highest
		else											// value we've ever assigned
			uipc->cyc_steps = realloc(uipc->cyc_steps, sizeof(int)*nc);

		for(i=uipc->max_nc; i<nc; i++) 
			uipc->cyc_steps[i] = 2;

		uipc->max_nc = nc;
	}
}

void change_cycle(int num) {
	// Updates controls and such when you switch from one cycle to another
	int nl, cyc;
	int panel = pc->inst[num];

	GetNumListItems(panel, pc->pclevel, &nl);
	if(nl < 1)
		return;	// Some kind of weird error.
	
	// First we'll update the uipc.
	GetCtrlVal(panel, pc->pclevel, &cyc);		// Should be 0-based index
	int i;
	for(i = 0; i<uipc->ncins; i++) {
		if(uipc->cyc_ins[i] == num) {			// Find the place in the cyc_ins array
			if(uipc->ins_cycs[i] == cyc)
				return;							// No change
			uipc->ins_cycs[i] = cyc;
			break;
		}
	}
	
	GetNumListItems(panel, pc->pcstep, &nl);	// Need to know how many to add or remove
	
	int steps = uipc->cyc_steps[cyc];
	if(steps == nl)								// It's all good.
		return;
	
	// The easy one, since -1 deletes everything from steps on.
	if(steps < nl) {
		DeleteListItem(panel, pc->pcstep, steps, -1);
	}
	
	// The harder one, need to add some instructions
	if(steps > nl) {
		int elements;
		char **c = generate_char_num_array(nl+1, steps, &elements);
		for(i=0; i<elements; i++) {
			InsertListItem(panel, pc->pcstep, -1, c[i], i+nl);
			free(c[i]);
		}
		free(c);
	}
	
	if(nl == 0) {
		SetCtrlIndex(panel, pc->pcstep, 0);
		SetCtrlAttribute(panel, pc->pcstep, ATTR_DFLT_INDEX, 0);
	}
}

void change_cycle_step(int num) {
	// Feed this an instruction number and it will update uipc and
	// update the phase cycle accordingly.
	
	int to, from, nl;
	int panel = pc->inst[num];
	
	GetNumListItems(panel, pc->pcstep, &nl);
	if(nl == 0)
		return;		// Weird error.
	
	// The old value is stored as the default value so we know where it came from
	GetCtrlVal(panel, pc->pcstep, &to);
	GetCtrlAttribute(panel, pc->pcstep, ATTR_DFLT_INDEX, &from);
	
	if(from < 0)
		from = 0;
	
	// Now we need to make sure that the uipc->c_instrs array is up to date.
	int cyc = -1, i;
	for(i = 0; i<uipc->ncins; i++) {
		if(uipc->cyc_ins[i] == num) 
			cyc = uipc->ins_cycs[i];
	}
	if(cyc < 0)
		return;	// Error.
	
	// Once you commit, we allocate you an array in c_instrs. The array size
	// is conservative, so it's only initially as big as it needs to be, but
	// it's also persistent for history reasons, so it never gets any smaller
	// than the biggest it's ever been.
	
	int steps = uipc->cyc_steps[cyc];
	
	if(uipc->c_instrs[num] == NULL) {
		uipc->c_instrs[num] = malloc(sizeof(PINSTR*)*(from+1));
	} else if (uipc->max_cinstrs[num] <= from) {
		uipc->c_instrs[num] = realloc(uipc->c_instrs[num], sizeof(PINSTR*)*(from+1));
	}
	
	if(from >= uipc->max_cinstrs[num]) {
		for(i=uipc->max_cinstrs[num]; i<=from; i++) 
			uipc->c_instrs[num][i] = NULL;
		
		uipc->max_cinstrs[num] = from+1;
	}
	
	if(uipc->c_instrs[num][from] == NULL)
		uipc->c_instrs[num][from] = malloc(sizeof(PINSTR));
	
	get_instr(pc, uipc->c_instrs[num][from], num); 		// Save the old instruction
	
	// If it's already been set, change the phase cycle instruction to what it used to be
	if(uipc->max_cinstrs[num] > to && uipc->c_instrs[num][to] != NULL)
		set_instr(pc, num, uipc->c_instrs[num][to]);
	
	// Now it's all done, so we just set the default index to the new index.
	SetCtrlAttribute(panel, pc->pcstep, ATTR_DFLT_INDEX, to);
	
	/************TO DO***************
	TODO: I should also implement a "check box" for what phase cycles have already
	been set up. Do this later.
	********************************/
}

void change_cycle_num_steps(int cyc, int steps) {
	// Function for changing the number of steps in a phase cycle.
	int i, j, nl, cp;

	if(steps == uipc->cyc_steps[cyc])
		return;			//No change

	int elements;
	char **c = NULL;
	if(steps > uipc->cyc_steps[cyc])
		c = generate_char_num_array(1, steps, &elements);
	
	for(i=0; i<uipc->ncins; i++) {
		if(uipc->ins_cycs[i] != cyc)
			continue;
		
		cp = pc->inst[uipc->cyc_ins[i]];
		
		if(steps < uipc->cyc_steps[cyc]) {
			GetNumListItems(cp, pc->pcstep, &nl);
			if(nl >= steps)
				DeleteListItem(cp, pc->pcstep, steps, -1);
		} else {
			GetNumListItems(cp, pc->pcstep, &nl);
			for(j=nl; j<steps; j++)
				InsertListItem(cp, pc->pcstep, -1, c[j], j);
		}
		
		SetCtrlVal(cp, pc->pcsteps, steps);
	}
	
	if(c != NULL) {
		for(i = 0; i<elements; i++)
			free(c[i]);
		free(c);
	}
	
	uipc->cyc_steps[cyc] = steps;
}

void populate_cyc_points()
{	 
	// Function for updating the UI with the values from the uipc var
	int j, on, nl, cyc;
	
	// A char array of labels
	int elements;
	char **c = generate_char_num_array(1, uipc->nc, &elements); 
	int panel;
	for(int i = 0; i<uipc->ncins; i++) {
		panel = pc->cinst[uipc->cyc_ins[i]];
		on = get_pc_state(i);
		if(on) {	// This condition should never be met, but it's not so bad to have it
			// First make the number of dimensions per control correct
			GetNumListItems(panel, pc->pclevel, &nl);
			if(nl < uipc->nc) {
				for(j=nl; j<uipc->nc; j++)
					InsertListItem(panel, pc->pclevel, -1, c[j], j);
			} else if (nl > uipc->nc) {
				DeleteListItem(panel, pc->pclevel, uipc->nc, -1);
			}
			
			// Now update the number of steps.
			GetCtrlIndex(panel, pc->pclevel, &cyc);
			SetCtrlVal(panel, pc->pcsteps, uipc->cyc_steps[cyc]);
		}
	}
	
	for(j=0; j<elements; j++)
		free(c[j]);
	free(c);
}
/****************** Instruction Manipulation *******************/ 
int move_instruction(int to, int from)
{
	// Moves an instruction somewhere else in the list and shifts everything around
	// accordingly.
	int diff = (int)fabs(to-from) + 1, inst_buffer[diff], inst_top[diff], cinst_buffer[diff], cinst_top[diff], i, start;
	
	if(to == from)
		return 0;
	
	if(to<from)
	{
		inst_buffer[0] = pc->inst[from];
		cinst_buffer[0] = pc->cinst[from];
		start = to;
		
		for (i = 1; i<diff; i++)
		{
			inst_buffer[i] = pc->inst[to+i-1];
			cinst_buffer[i] = pc->cinst[to+i-1];
		}
	}
	else
	{
		inst_buffer[diff-1] = pc->inst[from];
		cinst_buffer[diff-1] = pc->cinst[from];
		start = from;
														;
		for (i = 0; i<diff-1; i++)
		{
			inst_buffer[i] = pc->inst[from+i+1];
			cinst_buffer[i] = pc->cinst[from+i+1];
		}
	}
	
	// Check if it's in a loop before and after, and if there's a change, then you shoul
	int ninstructions = uipc->ni;
	int loop_locations[ninstructions][3]; // An array with all the loop locations and their corresponding end-points.
	int j = 0, ins, end_ins;
	for(i = 0; i<ninstructions; i++) {
		GetCtrlVal(pc->inst[i], pc->instr, &ins);
		if(ins == LOOP) {
			end_ins = find_end_loop(i);
			if(end_ins >= 0) {
				loop_locations[j][0] = i; // Which instruction is the loop.
				loop_locations[j++][1] = end_ins; // Which instruction is the end of the loop;
			}
		}
	}
	
	// Now that we know where all the loops are, we can just simply search through it and make changes that we need to.
	int c_l, c_e;
	for(i = 0; i<j; i++) {
		// Convenience variables
		c_l = loop_locations[i][0];
		c_e = loop_locations[i][1];
		
		// Check if it will affect the END_LOOP instruction data and modify accordingly.
		if((c_l > to && c_l > from) || (c_l < to && c_l < from)) {
			// Nothing happens in this case.
			continue;
		} else if(c_l == from) {
			SetCtrlVal(pc->inst[c_e], pc->instr_d, to);
		} else if (c_l > to) {
			SetCtrlVal(pc->inst[c_e], pc->instr_d, c_l+1);
		} else if (c_l > from) {
			SetCtrlVal(pc->inst[c_e], pc->instr_d, c_l-1);
		}

	}
	
	// Update the uipc var.
	int end = start+diff;
	// ND Instructions
	for(i = 0; i<uipc->ndins; i++) {
		ins = uipc->dim_ins[i];
		if(ins < start || ins > end)
			continue;
		
		if(ins == from)
			uipc->dim_ins[i] = to;
		else if (to > from)
			uipc->dim_ins[i]++;
		else if (from < to)
			uipc->dim_ins[i]--;
	}
	
	// Phase cycled instructions
	for(i = 0; i<uipc->ncins; i++) {
		ins = uipc->cyc_ins[i];
		if(ins < start || ins > end)
			continue;
		
		if(ins == from)
			uipc->cyc_ins[i] = to;
		else if (to > from)
			uipc->cyc_ins[i]--;
		else if (to < from)
			uipc->cyc_ins[i]++;
	}

	// Now the actual moving and updating of things.
	for (i = 0; i<diff; i++)
	{
		GetPanelAttribute(pc->inst[start+i], ATTR_TOP, &inst_top[i]);
		GetPanelAttribute(pc->cinst[start+i], ATTR_TOP, &cinst_top[i]);
	}
	
	for(i = 0; i<diff; i++)
	{ 
		pc->inst[start+i] = inst_buffer[i];
		pc->cinst[start+i] = cinst_buffer[i];
		SetPanelAttribute(pc->inst[start+i], ATTR_TOP, inst_top[i]);
		SetPanelAttribute(pc->cinst[start+i], ATTR_TOP, cinst_top[i]);
		SetCtrlVal(pc->inst[start+i], pc->ins_num, start+i);
		SetCtrlVal(pc->cinst[start+i], pc->cins_num, start+i);
	}

	// Now that it's moved, we want to recalculate if necessary.
	for(i = 0; i<j; i++) {
		// Convenience variables
		c_l = loop_locations[i][0];
		c_e = loop_locations[i][1];
		
		// If we came from inside a loop or moved to outside of a loop, adjust accordingly.
		if((c_l < to && c_e > to) || (c_l < from && c_e > from)) {
			int state = get_nd_state(c_l);
			if(state == 2) {
			//	update_instr_data_nd(pc->cinst[c_l], 2); // This will update the times.
			}
		}
	}
	
	return 0; 
}

void clear_instruction(int num) {
	// Takes an instruction and restores it to defaults.
	
	set_instr(pc, num, NULL);		// This sets everything to defaults
	
	// If this was a phase cycled instruction, this is the only time we want
	// to clear that, so we're going to free up the memory and such.
	if(get_pc_state(num))
		update_pc_state(num, 0);
	
	if(uipc->max_cinstrs != NULL)
	{
		if(uipc->c_instrs[num] != NULL) {
			for(int i = 0; i<uipc->max_cinstrs[num]; i++) {
				if(uipc->c_instrs[num][i] != NULL) {
					free(uipc->c_instrs[num][i]);
				}
			}
			free(uipc->c_instrs[num]);
			uipc->c_instrs[num] = NULL;
		}
		uipc->max_cinstrs[num] = 0;
	}
	
	int nl;
	GetNumListItems(pc->inst[num], pc->pcstep, &nl);
	if(nl)							   
		DeleteListItem(pc->inst[num], pc->pcstep, 0, -1);
	
	GetNumListItems(pc->inst[num], pc->pclevel, &nl);
	if(nl)							   
		DeleteListItem(pc->inst[num], pc->pclevel, 0, -1);
	
	GetNumListItems(pc->cinst[num], pc->dim, &nl);
	if(nl)							   
		DeleteListItem(pc->cinst[num], pc->dim, 0, -1);
	
}

void change_number_of_instructions() {
	// Gets "num" from pc->ninst and changes the number of instructions. If num < uipc->ni, t
	// the instructions at the end are hidden. if uipc->max_ni > num > uipc > uipc->ni, the 
	// old instructions are made visible again. If num > uipc->max_ni, a new instruction is 
	// created at the end and max_ni is incremented.
	
	int num, i;
	GetCtrlVal(pc->ninst[1], pc->ninst[0], &num);
	
	if(num == uipc->ni)
		return;			// No change needed
	
	if(num<uipc->ni) {						// In this case, we just hide the panels
		for(i = num; i<uipc->max_ni; i++) {
			HidePanel(pc->inst[i]);
			HidePanel(pc->cinst[i]);
		}
		uipc->ni = num;
		return;
	}
	
	if(num>uipc->max_ni) {		// The only really important part is to make new panels if there aren't any left		
		int top, left, height, ctop, cleft, cheight; 	// Getting the GUI values if we need them. 
		GetPanelAttribute(pc->inst[uipc->max_ni-1], ATTR_TOP, &top);	     	// Need these vals for both the
		GetPanelAttribute(pc->cinst[uipc->max_ni-1], ATTR_TOP, &ctop); 		 	// ND instrs and the pulse ones
		GetPanelAttribute(pc->inst[uipc->max_ni-1], ATTR_LEFT, &left);		 
		GetPanelAttribute(pc->cinst[uipc->max_ni-1], ATTR_LEFT, &cleft);
		GetPanelAttribute(pc->inst[uipc->max_ni-1], ATTR_HEIGHT, &height);
		GetPanelAttribute(pc->cinst[uipc->max_ni-1], ATTR_HEIGHT, &cheight);

		pc->inst = realloc(pc->inst, sizeof(int)*num);
		pc->cinst = realloc(pc->cinst, sizeof(int)*num);
		for(i=uipc->max_ni; i<num; i++) {
			pc->inst[i] = LoadPanel(pc->PProgCPan, pc->uifname, pc->pulse_inst);	// Make a new instruction
			SetPanelPos(pc->inst[i], top+=height+5, left);		// Place it and increment "top"
			SetCtrlAttribute(pc->inst[i], pc->xbutton, ATTR_DISABLE_PANEL_THEME, 1);
			
			pc->cinst[i] = LoadPanel(pc->PPConfigCPan, pc->uifname, pc->md_inst);	// Make a new ND instr
			SetPanelPos(pc->cinst[i], ctop+=cheight+5, cleft);
		
			// Update the instruction numbers
			SetCtrlVal(pc->inst[i], pc->ins_num, i);
			SetCtrlVal(pc->cinst[i], pc->cins_num, i);
		}

		if(uipc->c_instrs != NULL) {
			uipc->c_instrs = realloc(uipc->c_instrs, sizeof(PINSTR**)*num);
			uipc->max_cinstrs = realloc(uipc->max_cinstrs, sizeof(int)*num);
			for(i=uipc->max_ni; i<num; i++) {
				uipc->c_instrs[i] = NULL;
				uipc->max_cinstrs[i] = 0;
			}
		}

		uipc->max_ni = num;		// We've increased the max number of instructions, so increment it.
	}
	
	int ndon;
	GetCtrlVal(pc->ndon[1], pc->ndon[0], &ndon);
	
	for(i=uipc->ni; i<num; i++) {	// Now we can show the remaining panels.
		DisplayPanel(pc->inst[i]);
		DisplayPanel(pc->cinst[i]);
		SetPanelAttribute(pc->cinst[i], ATTR_DIMMED, !ndon);
	}
	
	uipc->ni = num;	// And update the uipc value.
}

void delete_instruction(int num) {
	// Deletes an instruction by clearing the value, moving it to the end and
	// reducing the number of instructions available.
	
	clear_instruction(num);

	if(uipc->ni == 1)
		return;			// Can't have 0 instructions
	
	move_instruction(uipc->max_ni-1, num);
	SetCtrlVal(pc->ninst[1], pc->ninst[0], uipc->ni-1);
	change_number_of_instructions();
}

/********************** Math Evaluation ************************/

int update_nd_from_exprs(int num) {// Generate the experiment from the exprs (state == 2)
	// Return value is 0 or a positive error.
	int state = get_nd_state(num);
	int i, *cstep = malloc(sizeof(int)*(uipc->nc+uipc->nd));
	int panel = pc->cinst[num];
	int dim, ind;
	
	for(i=0; i<uipc->ndins; i++) {
		if(uipc->dim_ins[i] == num)
			break;
	}
	
	if(i== uipc->ndins) // Index not found
		return 8;
	ind = i;
	
	int eval_data = 0, eval_delay = 0; // Whether or not to evaluate these things.
	int instr, len_data, len_delays, len_default;
	char *expr_delay, *expr_data, *default_val;
	
	GetCtrlVal(pc->inst[num], pc->instr, &instr);   // The instruction
	GetCtrlVal(panel, pc->dim, &dim);				// What dimension we're varying along.
	if(takes_instr_data(instr)) {
		GetCtrlValStringLength(panel, pc->cexpr_data, &len_data);
		GetCtrlAttribute(panel, pc->cexpr_data, ATTR_DFLT_VALUE_LENGTH, &len_default);
		
		expr_data = malloc(len_data+1);
		GetCtrlVal(panel, pc->cexpr_data, expr_data);
		
		default_val = malloc(len_default+1);
		GetCtrlAttribute(panel, pc->cexpr_data, ATTR_DFLT_VALUE, default_val);
		
		if(strcmp(default_val, expr_data) != 0)
			eval_data = 1;
		else
			free(expr_data);
		
		free(default_val);
	}
	
	GetCtrlValStringLength(panel, pc->cexpr_delay, &len_delays);
	GetCtrlAttribute(panel, pc->cexpr_delay, ATTR_DFLT_VALUE_LENGTH, &len_default);
	
	expr_delay = malloc(len_delays+1);
	GetCtrlVal(panel, pc->cexpr_delay, expr_delay);
	
	default_val = malloc(len_default+1);
	GetCtrlAttribute(panel, pc->cexpr_delay, ATTR_DFLT_VALUE, default_val);
	
	if(strcmp(default_val, expr_delay) != 0)
		eval_delay = 1;
	else
		free(expr_delay);
	
	free(default_val);
	
	// At this point we know which one(s) to evaluate, and we have the expressions.
	
	if(!eval_delay && !eval_data)
		return 0;	// Nothing to do.
	
	// Set up the uipc var if necessary.
	int steps = uipc->dim_steps[dim];
	if(eval_delay) {
		if(uipc->nd_delays[ind] == NULL)
			uipc->nd_delays[ind] = malloc(sizeof(double)*steps);
		else
			uipc->nd_delays[ind] = realloc(uipc->nd_delays[ind], sizeof(double)*steps);
	}
	
	if(eval_data) {
		if(uipc->nd_data[ind] == NULL)
			uipc->nd_data[ind] = malloc(sizeof(int)*steps);
		else
			uipc->nd_data[ind] = realloc(uipc->nd_data[ind], sizeof(int)*steps);
	}
	
	
	// Build the basic cstep.
	for(i = 0; i<(uipc->nc+uipc->nd); i++)
		cstep[i] = 0;	// Just set everything we don't care about to the first one, it's not important
	
	
	int err_del = 0, err_dat = 0;					// Error values
	constants *c = setup_constants(); 		// Initializes the static constants
	for(i=0; i<steps; i++) {
		cstep[dim]++;
		update_constants(c, cstep);
		
		if(eval_delay && !err_del) {
			uipc->nd_delays[ind][i] = parse_math(expr_delay, c, &err_del, 0);
			
			if(err_del && (!eval_data || err_dat))
					break;
		}
		
		if(eval_data && !err_dat) {
			uipc->nd_data[ind][i] = (int)parse_math(expr_data, c, &err_dat, 0);
			
			if(err_dat && (!eval_delay || err_del)) 
					break;
		}
	}
	
	// If the evaluation was unsuccessful, the border should turn red.
	// If it was successful, the border should turn green, otherwise it is
	// switched back to the windows style with no specific border color.
	if(eval_delay) {
		free(expr_delay);
		if(err_del) {
			uipc->err_del = err_del;
			uipc->err
			SetCtrlAttribute(panel, pc->cexpr_delay, ATTR_TEXT_BGCOLOR, VAL_RED);
		}
		else 
			SetCtrlAttribute(panel, pc->cexpr_delay, ATTR_TEXT_BGCOLOR, VAL_GREEN);
	} else 
		SetCtrlAttribute(panel, pc->cexpr_delay, ATTR_TEXT_BGCOLOR, VAL_WHITE);
	
	if(eval_data) {
		free(expr_data);
		if(err_dat)
			SetCtrlAttribute(panel, pc->cexpr_data, ATTR_FRAME_COLOR, VAL_RED);	
		else 
			SetCtrlAttribute(panel, pc->cexpr_data, ATTR_FRAME_COLOR, VAL_DK_GREEN);
	} else 
		SetCtrlAttribute(panel, pc->cexpr_data, ATTR_DISABLE_PANEL_THEME, 0);
	
	
	
	return 0;
}


constants *setup_constants() {				// Setup all the static constants in the experiment
	constants *c = malloc_constants();		// First create the constants variable
		
	// We'll initialize the constants now.
	// Eventually we'll read these from a preferences file, but for now set them in stone
	add_constant(c, "nd", C_INT, &uipc->nd);				// Number of dimensions
	add_constant(c, "nc", C_INT, &uipc->nc);				// Number of ccles.
		
	// Now the steps per dimension and the steps per cycle
	char *current_var = malloc(5);
	int i;
	
	for(i=0;i<uipc->nc; i++) {
		sprintf(current_var, "mcs%d", i);				// Maximum cycle step for cycle i
		add_constant(c, current_var, C_INT, &uipc->cyc_steps[i]);
	}
		
	for(i=0;i<uipc->nd; i++){
		sprintf(current_var, "mds%d", i);				// Max dim step for dimension i
		add_constant(c, current_var, C_INT, &uipc->dim_steps[i]);
	}
	
	// Now setup units
	double micro = 1000.0, milli = 1000000.0, second = 1000000000.0;
	add_constant(c, "us", C_DOUBLE, &micro);
	add_constant(c, "ms", C_DOUBLE, &milli);
	add_constant(c, "s", C_DOUBLE, &second);
	
	free(current_var);
	
	return c;
}

void update_constants(constants *c, int *cstep) { // Updates the constants for a given position in acq. space
	// cstep must be a position in acqusition space of size uipc->nd+uipc->nc
	char *current_var = malloc(5);
	int i;
	
	// change_constant redirects to add_constant if the constant is not found.
	for(i=0;i<uipc->nc; i++) {
			sprintf(current_var, "ccs%d", i);					// Current cycle step for cycle i
			change_constant(c, current_var, C_INT, &cstep[i]);
		}
	
	for(i=0;i<uipc->nd; i++) {
		sprintf(current_var, "cds%d", i);					// Current dim step for dimension i
		change_constant(c, current_var, C_INT, &cstep[i+uipc->nc]);
	}
	
	free(current_var);
}



/******************** Convenience Functions ********************/

int find_end_loop (int instr) {
	// Feed this an instruction which is a loop, it finds the corresponding END_LOOP instruction.
	// If no such instruction is found, returns -1;
	
	int ins;
	GetCtrlVal(pc->inst[instr], pc->instr, &ins);
	
	if(ins != LOOP) {
		return -1;
	}
	
	// Search for the END_LOOP, check if it matches. Simple.
	int e_i_d, ni;
	GetCtrlVal(pc->ninst[1], pc->ninst[0], &ni);
	for(int i = instr+1; i<ni; i++)
	{
		GetCtrlVal(pc->inst[i], pc->instr, &ins);
		if(ins == END_LOOP)
		{
			GetCtrlVal(pc->inst[i], pc->instr_d, &e_i_d);
			if(e_i_d == instr)
			{
				return i;
			}
		}
	}
	
	return -1;
}

int in_loop(int instr, int big) {
	// Tells you if the instruction you're looking at is in a loop or not.
	// It will return no if it's in a mal-formed loop (i.e. if END_LOOP is after STOP, doesn't point to the right place, etc.)
	// Returns the instruction number of the loop if you are in one, negative numbers otherwise.
	
	// If big = 1, find the largest loop that our instruction is in.
	// If big = 0, find the smallest loop that our instruction is in.
	
	int end_instr;
	
	
	// Find all the loops, find their corresponding END_LOOPs, and if this is within that, then yeah, it's a loop.
	if (big) {
		for(int i = 0; i<=instr; i++) {
			end_instr = find_end_loop(i); // Returns -1 if it's a malformed loop or if it's not a loop at all.
			if(end_instr >= instr) {
				return i; // Yep, you're in a loop. If you're in a loop within a loop, then this returns the biggest loop you're in.
			}
		}
	} else {
		for(int i = instr; i>=0; i--) {
			end_instr = find_end_loop(i); // Returns -1 if it's a malformed loop or if it's not a loop at all.
			if(end_instr >= instr) {
				return i; // Yep, you're in a loop.
			}
		}
	}
	
	return -1; // If we didn't find that we were in a loop before, we're not in a loop.
}

int takes_instr_data(int func_index) {
	switch(func_index) {	// These are the atomic functions that take instr_data.
		case LOOP:
		case END_LOOP:
		case JSR:
		case BRANCH:
		case LONG_DELAY:
			return 1;
			break;
	}
	
	// TODO: Update this for compound functions
	
	return 0;
}

int instr_data_min(int func_index) {
	switch(func_index) {
		case LOOP:
		case LONG_DELAY:
			return 1;
			break;
	}
	
	// TODO: Update this for user-defined functions.
	return 0;
}

void change_visibility_mode (int panel, int *ctrls, int num, int mode) {
 	// Toggles the visiblity mode of all the controls in ctrls, an array of
	// size num, according to the flags set in "mode" (bitwise | for both)
	// MC_HIDDEN = Hidden
	// MC_DIMMED = Dimmed  
	
	int dimmed = 0, hidden = 1;
	if(mode & MC_DIMMED)
		dimmed = 1;
	if(mode & MC_HIDDEN)
		hidden = 0;
	
	
	for(int i = 0; i<num; i++) {
		SetCtrlAttribute(panel, ctrls[i], ATTR_DIMMED, dimmed);
		SetCtrlAttribute(panel, ctrls[i], ATTR_VISIBLE, hidden);
	}
}


void change_control_mode (int panel, int *ctrls, int num, int mode) {
	// Toggles whether the controls are an indicator or hot.
	// Just pass it VAL_HOT, VAL_NORMAL, VAL_INDICATOR or VAL_VALIDATE
	for(int i = 0; i<num; i++) 
		SetCtrlAttribute(panel, ctrls[i], ATTR_CTRL_MODE, mode);

}

void remove_array_item(int *array, int index, int num_items) {
	// Removes the item at index from *array, an array of size num_items
	// This does not re-allocate space for the array, so if the item you
	// are removing is the last one, it doesn't do anything, otherwise
	// it moves everything over one.
	
	for(int i=index; i<num_items-1; i++)
		array[i] = array[i+1];
	
}

void remove_array_item_void(void *array, int index, int num_items, int type) {
	// Same as remove_array_item, but for any type.
	// Pass 0 for double
	// Pass 1 for char
	// Pass 2 for int*
	// Pass 3 for double*
	
	if(type == 0) {
		double *ar = (double*)array;
	 	for(int i=index; i<num_items-1; i++)
			ar[i] = ar[i+1];
	} else if(type == 1) {
		char *ar = (char*)array;
		for(int i=index; i<num_items-1; i++)
			ar[i] = ar[i+1];
	} else if(type == 2) {
		int **ar = (int**)array;
		for(int i=index; i<num_items-1; i++)
			ar[i] = ar[i+1];
	} else if(type == 3) {
		double **ar = (double**)array;
		for(int i=index; i<num_items-1; i++)
			ar[i] = ar[i+1];
	}

}

char **generate_char_num_array(int first, int last, int *elems) {
	// Generates an array of strings that are just numbers, starting with
	// first, ending with last.
	// *c needs to be freed. The return value is the number of items in
	// the list - make sure to iterate through c and free all the elements
	
	// Some people don't understand that the last thing comes AFTER the
	// first thing, fix it if that's a problem.
	if(first > last) {
		int buff = first;
		first = last;
		last = buff;
	}
	
	// Need the length of the longest entry.
	int maxlen;
	if(last == 0 && last == first)			// log10(0) is undefined.
		maxlen = 2;		
	if(last <= 0)
		maxlen=(int)log10(abs(first))+2; 	// Need to allocate space for the sign
	else									// and the null termination
		maxlen=(int)log10(abs(last))+1;
	
	int elements = last-first+1;
	char **c = malloc(sizeof(char*)*elements);
	for(int i=0; i<elements; i++) {
		c[i] = malloc(maxlen+1);
		sprintf(c[i], "%d", first+i);
	}
	
	if(elems != NULL)
		elems[0] = elements;
	
	return c;
}

//////////////////////////////////////////////////////
//                                      	        //
//         		User Function Functions	 	  		//
//                                                  //
//////////////////////////////////////////////////////

//////////////////////////////////////////////////////
//                                      	        //
//            	Main Panel Callbacks	 	  		//
//                                                  //
//////////////////////////////////////////////////////

int CVICALLBACK StartProgram (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK StopProgram (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK QuitCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				QuitUserInterface(0); 
			break;
	}
	return 0;
}

void CVICALLBACK QuitCallbackMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	QuitUserInterface(0);
}

void CVICALLBACK SaveConfig (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

void CVICALLBACK SaveConfigToFile (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

void CVICALLBACK LoadConfigurationFromFile (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

int CVICALLBACK DirectorySelect (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

//////////////////////////////////////////////////////
//                                      	        //
//          Pulse Instruction Callbacks	 	  		//
//                                                  //
//////////////////////////////////////////////////////

/************* UI Configuration Callbacks **************/

int CVICALLBACK MoveInst (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// First find the old panel.
			
			int to, from;
			int i;
			for(i=0; i<uipc->ni; i++) {
				if(pc->inst[i] == panel) {
					from = i;
					break;
				}
			}
			
			if(i == uipc->ni)
				break;	// Error.
			
			GetCtrlVal(panel, pc->ins_num, &to);
			
			move_instruction(to, from);
			break;
	}
	return 0;
}

int CVICALLBACK MoveInstButton (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// Where is the cursor now
			POINT pos;
			GetCursorPos(&pos);
			
			// What instruction called this
			int num;
			GetCtrlVal(panel, pc->ins_num, &num);
			
			// How far apart are the instructions - they're evenly spaced, so pick the first two.
			if (uipc->ni > 1) {
				int top1, top2, diff;
				
				GetPanelAttribute(pc->inst[0], ATTR_TOP, &top1);	
				GetPanelAttribute(pc->inst[1], ATTR_TOP, &top2);
				
				diff = top1-top2; // This is a positive number, so if we're moving up, move the cursor by this amount, otherwise move it by the negative of this amount.
				
				// Now we just decide if it was called by the up button or the down button.
				if(control == pc->uparrow) {
					move_instruction(num-1, num);
					SetCursorPos(pos.x, pos.y+diff);				
				} else if(control == pc->downarrow) {
					move_instruction(num+1, num);
					SetCursorPos(pos.x, pos.y-diff);
				}
			}
			break;
	}
	return 0;
}

int CVICALLBACK DeleteInstructionCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				int num;
				GetCtrlVal(panel, pc->ins_num, &num);
				
				delete_instruction(num);
			break;
	}
	return 0;
}


/*********** Instruction Property Callbacks *************/ 

int CVICALLBACK ChangeInstDelay (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
} // Needs to retrieve the time of the instruction.

int CVICALLBACK InstrCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int num;
			GetCtrlVal(panel, pc->ins_num, &num);
			
			change_instruction(num);
			break;
	}
	return 0;
}

int CVICALLBACK ChangeTUnits (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
} //

int CVICALLBACK InstrDataCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
} // Needs to sync with the ND stuff.

int CVICALLBACK Change_Scan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
} // Need to setup the trigger TTL again.

/******** Multidimensional Instruction Callbacks *********/

int CVICALLBACK ChangeInstrVary (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int up = 1;
	switch (event)
	{
		case EVENT_RIGHT_CLICK:
		case EVENT_RIGHT_DOUBLE_CLICK:
			up = 0;	// This will only be seen if they right clicked
		case EVENT_LEFT_CLICK:			// Because there was no break statement, these
		case EVENT_LEFT_DOUBLE_CLICK:   // conditions will also be met by right clicks
			int num;
			GetCtrlVal(panel, pc->cins_num, &num);
			
			int state = get_nd_state(num);
			
			if(up)
				state++;
			else
				state--;
			
			state = (state+3)%3; 	// In C, mod is signed, so we need to add 4 first here
			
			int val;
			GetCtrlAttribute(pc->inst[num], pc->ins_num, ATTR_FRAME_COLOR, &val);
			
			if(val == VAL_OFFWHITE)
				return -1;
			
			update_nd_state(num, state);	// Finally we can update the state.
			break;
	}
	return 0;
}


int CVICALLBACK ChangeNumSteps (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int dim, steps;
			
			GetCtrlVal(panel, pc->dim, &dim);
			GetCtrlVal(panel, pc->nsteps, &steps);
			
			change_num_dim_steps(dim, steps);
			break;
	}
	return 0;
} 

int CVICALLBACK ChangeDimension (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int num;
			GetCtrlVal(panel, pc->cins_num, &num);
			
			change_dimension(num);
			break;
	}
	return 0;
} 

int CVICALLBACK ChangeInitOrFinal (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// When you change init or final, change the increment
			int num;
			GetCtrlVal(panel, pc->cins_num, &num);
			
			update_nd_increment(num, MC_INC);
			break;
	}
	return 0;
} //

int CVICALLBACK ChangeInc (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// When you change the increment, update final in response.
			int num;
			GetCtrlVal(panel, pc->cins_num, &num);
			
			update_nd_increment(num, MC_FINAL);
			break;
	}
	return 0;
} 
  

/*************** Phase Cycling Callbacks ****************/ 

int CVICALLBACK PhaseCycleInstr (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_RIGHT_CLICK:
		case EVENT_RIGHT_DOUBLE_CLICK:
		case EVENT_LEFT_CLICK:
		case EVENT_LEFT_DOUBLE_CLICK:
			// Toggle the phase cycling step
			int num;
			GetCtrlVal(panel, pc->ins_num, &num);
			
			int state = get_pc_state(num);
			
			update_pc_state(num, !state);
			break;
	}
	return 0;
} 

int CVICALLBACK ChangeNumCycles (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				change_num_cycles();
			break;
	}
	return 0;
}


int CVICALLBACK InstrChangeCycleNum (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int cyc, steps;
			GetCtrlVal(panel, pc->pcsteps, &steps);
			GetCtrlVal(panel, pc->pclevel, &cyc);
			change_cycle_num_steps(cyc, steps);
			break;
	}
	return 0;
}

int CVICALLBACK ChangePhaseCycleLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int num;
			GetCtrlVal(panel, pc->ins_num, &num);
			
			change_cycle(num);
			break;
	}
	return 0;
}

int CVICALLBACK ChangePhaseCycleStep (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int num;
			GetCtrlVal(panel, pc->ins_num, &num);
			change_cycle_step(num);
			break;
	}
	return 0;
}

//////////////////////////////////////////////////////
//                                      	        //
//            Pulse Prog Tabs Callbacks	 	  		//
//                                                  //
//////////////////////////////////////////////////////

/*************** Basic Prog Setup Callbacks ****************/

int CVICALLBACK Change_Trigger (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ContinuousRunCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK InstNumChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			change_number_of_instructions();
			break;
	}
	return 0;
}

/*************** Load/Save/New Prog Callbacks ****************/

int CVICALLBACK LoadProgram (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK NewProgram (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK SaveProgram (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

/************** Sampling Space Position Callbacks ****************/

int CVICALLBACK ProgChangeIDPos (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ResetIDPos (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}


/******************** Basic Config Callbacks ********************/ 
int CVICALLBACK ChangeAcquisitionTime (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeNPoints (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeSampleRate (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeTransients (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

/************** Multidimensional Setup Callbacks ******************/  

int CVICALLBACK NumDimensionCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				change_num_dims();	
			break;
	}
	return 0;
}

int CVICALLBACK ToggleND (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
		case EVENT_RIGHT_CLICK:
		case EVENT_LEFT_DOUBLE_CLICK:
		case EVENT_RIGHT_DOUBLE_CLICK:
			toggle_nd();
			break;
	}
	return 0;
}

int CVICALLBACK ChangeNDTimeUnits (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeNDTime (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK EditExpression (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int len, dlen;
	switch (event)
	{
		case EVENT_GOT_FOCUS:
			// The background colors can be annoying, so fix them when you first get focus
			SetCtrlAttribute(panel, control, ATTR_TEXT_BGCOLOR, VAL_WHITE);
			SetCtrlAttribute(panel, control, ATTR_TEXT_BOLD, 0);
			SetCtrlAttribute(panel, control, ATTR_TEXT_COLOR, VAL_BLACK);
			
			// We want the label to be deleted when we first try to edit
			GetCtrlValStringLength(panel, control, &len);
			GetCtrlAttribute(panel, control, ATTR_DFLT_VALUE_LENGTH, &dlen);
			if(len == dlen) {
				char *val = malloc(len+1);
				char *dval = malloc(len+1);
				
				GetCtrlVal(panel, control, val);
				GetCtrlAttribute(panel, control, ATTR_DFLT_VALUE, dval);
				
				if(strcmp(val, dval) == 0)
					SetCtrlVal(panel, control, "");
				
				free(val);
				free(dval);
			}
			break;
		case EVENT_LOST_FOCUS:
			// If we click out of it, we want to restore the default value
			// if there was a blank string in there. If not, we want to go
			// ahead and evaluate the contents.
			GetCtrlValStringLength(panel, control, &len);
			if(len == 0) {
				GetCtrlAttribute(panel, control, ATTR_DFLT_VALUE_LENGTH, &dlen);
				
				char *dval = malloc(dlen+1);
				GetCtrlAttribute(panel, control, ATTR_DFLT_VALUE, dval);
				SetCtrlVal(panel, control, dval);
				free(dval);
				
				break;
			}
		case EVENT_COMMIT:
			int num;
			GetCtrlVal(panel, pc->cins_num, &num);
				
			update_nd_from_exprs(num);
			break;
			
		case EVENT_RIGHT_CLICK:
			
			break;
	}
	return 0;
}

int CVICALLBACK SetupSkipCondition (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

/************** Multidimensional Setup Callbacks ******************/ 

int CVICALLBACK ChangeAcquisitionChannel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeCounterEdge (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeDevice (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeTriggerEdge (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

// Channel gain setup
int CVICALLBACK ChangeChannelGain (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeInputMinMax (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

//////////////////////////////////////////////////////
//                                      	        //
//            Pulse Prog Menu Callbacks	 	  		//
//                                                  //
//////////////////////////////////////////////////////
void CVICALLBACK NewAcquisitionMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

void CVICALLBACK LoadProgramMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

void CVICALLBACK NewProgramMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

void CVICALLBACK SaveProgramMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

void CVICALLBACK BrokenTTLs (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

void CVICALLBACK ChangeNDPointMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

void CVICALLBACK ViewProgramChart (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

void CVICALLBACK UpdateDAQMenuCallback (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}


//////////////////////////////////////////////////////
//                                      	        //
//         User Function Editor Callbacks	 		//
//                                                  //
//////////////////////////////////////////////////////

int CVICALLBACK FuncEditQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK FENewFunc (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK FuncEditSave (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK FuncEditSaveAndClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK FEChangeDelayInstr (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK FEChangeInstrDataInstr (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK FEEnableDelay (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK FEEnableInstrData (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

//////////////////////////////////////////////////////
//                                      	        //
//            Data Viewing Callbacks	 	  		//
//                                                  //
//////////////////////////////////////////////////////
void CVICALLBACK LoadDataMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

/********* Acquisition Space Navigation ***********/ 
int CVICALLBACK DatChangeIDPos (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeViewingTransient (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

void CVICALLBACK ChangeTransientView (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

/************** Fitting Callbacks *****************/ 

int CVICALLBACK ChangePolySubtract (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangePolyFitOrder (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

/***************** FID Specific ******************/ 
 int CVICALLBACK FIDGraphClick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeFIDChannel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeFIDChanPrefs (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeFIDOffset (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeFIDGain (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeFIDChanColor (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

/***************** FFT Specific ******************/

int CVICALLBACK SpectrumCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeSpectrumChannel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeSpectrumChan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeSpecChanPrefs (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangePhaseCorrectionOrder (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangePhaseKnob (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeSpectrumOffset (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeSpectrumGain (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeSpectrumChanColor (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

/*************** Zoom/Pan Controls ****************/ 
 
 void CVICALLBACK AutoscalingOnOff (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

void CVICALLBACK ZoomGraph (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}

void CVICALLBACK PanGraph (int menuBar, int menuItem, void *callbackData,
		int panel)
{
}


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


int CVICALLBACK ControlHidden (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ColorVal (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}



