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
	uipc->
	uipc->total_time = 0.0;
	
	uipc->nd = 0;
	uipc->max_nd = 0;
	uipc->ndins = 0;
	uipc->dim_steps = NULL;
	uipc->dim_ins = NULL;
	uipc->ins_dims = NULL;
	uipc->ins_state = NULL;
	
	uipc->nd_delays = NULL;
	uipc->nd_data = NULL;
	uipc->err_dat = 0;
	uipc->err_dat_size = 0;
	uipc->err_del = 0;
	uipc->err_dat_size = 0;
	uipc->err_dat_pos = NULL;
	uipc->err_del_pos = NULL;
	
	uipc->max_n_steps = 0;
	uipc->skip_err = 0;
	uipc->skip_err_size = 0;
	uipc->skip_err_pos = NULL;
	uipc->skip_locs = NULL;
	
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
//            	Main Panel Callbacks	 	  		//
//                                                  //
//////////////////////////////////////////////////////

int CVICALLBACK StartProgram (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			PPROGRAM *p = get_current_program();
			
			free_pprog(p);
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
			double val;
			int units;

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
			int num;
			GetCtrlVal(panel, pc->ins_num, &num);
			
			change_instr_units(num);
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
int CVICALLBACK NewProgram (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			clear_program();
			break;
	}
	return 0;
}

int CVICALLBACK LoadProgram (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			char *path = malloc(MAX_FILENAME_LEN+MAX_PATHNAME_LEN);
			FileSelectPopup("Programs", ".nc", ".nc;.txt", "Load Program From File", VAL_LOAD_BUTTON, 0, 0, 1, 1, path);
			
			int err_val;
			PPROGRAM *p = LoadPulseProgram(path, &err_val, NULL);
			
			if(err_val)
				display_netcdf_error(err_val);
			
			if(p != NULL)
				set_current_program(p);
			
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
			char *path = malloc(MAX_FILENAME_LEN+MAX_PATHNAME_LEN);
			int rv = FileSelectPopup("Programs", ".nc", ".nc;.txt", "Save Program To File", VAL_SAVE_BUTTON, 0, 0, 1, 1, path);
			if(rv == VAL_NO_FILE_SELECTED)
				break;
			
			PPROGRAM *p = get_current_program();
			
			int err_val = SavePulseProgram(path, p, NULL);
			
			if(err_val) 
				display_netcdf_error(err_val);
			
			free_pprog(p);
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

int CVICALLBACK EditExpression (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int len, dlen;
	switch (event)
	{
		case EVENT_GOT_FOCUS:
		case EVENT_VAL_CHANGED:
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
			// Right clicking on either control gives a popup with the error, if there was an error.
			int dat = 0, del = 0;
			if(control == pc->cexpr_data)
				dat = 1;
			else if(control == pc->cexpr_delay)
				del = 1;
			
			if((del && uipc->err_del) || (dat && uipc->err_dat)) {
				char *err_message;
				int size, i, *pos, err;
				
				// Copy the specific control parameters into generic parameters.
				if(dat) {
					err = uipc->err_dat;
					size = uipc->err_dat_size;
					pos = malloc(sizeof(int)*size);
					
					for(i=0; i<size; i++)
						pos[i] = uipc->err_dat_pos[i];
				} else {
					err = uipc->err_del;
					size = uipc->err_del_size;
					pos = malloc(sizeof(int)*size);
					
					for(i=0; i<size; i++)
						pos[i] = uipc->err_del_pos[i];
				}
				
				// Get the error message
				len = get_update_error(err, NULL);
				err_message = malloc(len);
				get_update_error(err, err_message);

				char *output = generate_expression_error_message(err_message, pos, size);
				
				MessagePopup("Error in Expression", output);
				
				free(err_message);
				free(pos);
				free(output);
			}
			break;
	}
	return 0;
}

int CVICALLBACK SetupSkipCondition (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int len;
	switch (event)
	{
		case EVENT_COMMIT:
			
			break;
	}
	return 0;
}

int CVICALLBACK EditSkipCondition (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{   
		case EVENT_GOT_FOCUS:
		case EVENT_VAL_CHANGED:
			// If we start messing around with the box, clear the evaluation result
			SetCtrlAttribute(panel, control, ATTR_TEXT_BGCOLOR, VAL_WHITE);
			SetCtrlAttribute(panel, control, ATTR_TEXT_BOLD, 0);
			SetCtrlAttribute(panel, control, ATTR_TEXT_COLOR, VAL_BLACK);
			break;
		case EVENT_LOST_FOCUS:
		case EVENT_COMMIT:
			update_skip_condition();
			break;
		case EVENT_RIGHT_CLICK:
			// If there's an error, right click here to get a popup with details.
			if(uipc->skip_err) {
				int len = get_parse_error(uipc->skip_err, NULL);	// Length of the message
				char *err_message = malloc(len+1);					// Allocate space for the message
				
				get_parse_error(uipc->skip_err, err_message); 		// Length of the message
				
				char *output = generate_expression_error_message(err_message, uipc->skip_err_pos, uipc->skip_err_size);
				
				MessagePopup("Error in Expression", output);
				
				free(err_message);
				free(output);
			}
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
