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
		Implement phase cycling			  				Progress: 20%
		

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


***************************************************************************/

#include <userint.h>
#include "Magnetometer Controller.h"
#include "pathctrl.h"
#include <cvirte.h>		
#include "spinapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <analysis.h>
#include "toolbox.h"
#include <utility.h>
#include <ansi_c.h>
#include "asynctmr.h"
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <NIDAQmx.h>
#include <PulseProgramTypes.h>
#include <UIControls.h>
#include <PulseProgramLib.h>
#include "MC.h"
#include <stdarg.h>

static int panelHandle, Pulse_Prog_Tab, Pulse_Prog_Panel, FID, FFTSpectrum, HiddenPanel, scan = 0, Pulse_Prog_Config, PPConfigSubPanel;
static int panelHandleMenu, RC_menu;
static TaskHandle acquireSignal, counterTask;    
int get_status = 0, started, timer, nPoints, Transients, ttl_trigger, update_thread = NULL, idle_thread = NULL;
int ninstructions = 1, n_inst_disp = 1, inst[1000], cinst[1000], initialized = 0, cont_mode = 0;
char defaultsloc[50]= "DefaultConfig.bin", savedsessionloc[50] = "PreviousState.bin", errorstring[200];
PPROGRAM *running_prog; // Current running program

// Structures for containing UI information (if this were C++, we'd make some classes for this sort of thing..)
static ppcontrols *pc; 		// The UI controls structure
static datacontrols *dc; 	// The Data controls structure
static maincontrols *mc;	// The Main controls structure
static ui_ppconfig *uipc;	// The current program configuration structure
static all_funcs *af;		// Holder for all current functions.

// Multithreading variable declarations
DefineThreadSafeScalarVar(int, QuitUpdateStatus, 0); 
DefineThreadSafeScalarVar(int, QuitIdle, 0);
DefineThreadSafeScalarVar(int, DoubleQuitIdle, 0);
DefineThreadSafeScalarVar(int, Status, 0);
int lock_pb, lock_pp, lock_DAQ, lock_plot; // Thread locks

static double ns = 1.0; // Spincore library is dumb, temporary

//////////////////////////////////////////////////////
//                                                  //
//              Main Program Functions              //
//                                                  //
//////////////////////////////////////////////////////

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
	
	
	// First get the tab panels.
	GetPanelHandleFromTabPage(panelHandle, MainPanel_MainTabs, 0, &dc->fid);
	GetPanelHandleFromTabPage(panelHandle, MainPanel_MainTabs, 1, &dc->spec);
	GetPanelHandleFromTabPage(panelHandle, MainPanel_MainTabs, 2, &pc->PProgPan);
	GetPanelHandleFromTabPage(panelHandle, MainPanel_MainTabs, 3, &pc->PPConfigPan);
	
	// Temporary
	FID = dc->fid;
	FFTSpectrum = dc->spec;
	Pulse_Prog_Tab = pc->PProgPan;
	Pulse_Prog_Config = pc->PPConfigPan;
	
	// Then the menu bars
	mc->mainmenu = GetPanelMenuBar(panelHandle); 	// Main menu
	mc->rcmenu = LoadMenuBar(0, uifname, RCMenus);	// Right click menu
	

	// Then the container panels								 
	pc->PProgCPan = LoadPanel(pc->PProgPan, uifname, PPPanel); // Pulse program instr container
	pc->PPConfigCPan = LoadPanel(pc->PPConfigPan, uifname, PPConfigP); // ND instr container
	
	//	Now allocate memory for the instruction arrays, then create one of each
	pc->inst = malloc(sizeof(int));
	pc->cinst = malloc(sizeof(int));
	
	pc->inst[0] = LoadPanel(pc->PProgCPan, uifname, PulseInstP);
	pc->cinst[0] = LoadPanel(pc->PPConfigCPan, uifname, MDInstr);
	
	// Create two panels for containing the current location in acquisition space.
	dc->cloc[0] = LoadPanel(dc->fid, uifname, CurrentLoc);
	dc->cloc[1] = LoadPanel(dc->spec, uifname, CurrentLoc);
	
	// Move everything to where it belongs
	SetPanelPos(pc->PProgCPan, 5, 2); 	// Move the instruction container where it should go
	DisplayPanel(pc->PProgCPan); 		// Display the instruction container
	SetPanelPos(pc->inst[0], 40, 7);	// Move the first instruction to where it belongs
	DisplayPanel(pc->inst[0]);			// Display the first instruction
	
	SetPanelPos(pc->PPConfigCPan, 32, 8); 	// Move MD instruction container to where it belongs
	DisplayPanel(pc->PPConfigCPan);		// Display the MD instruction container
	SetPanelPos(pc->cinst[0], 28, 7); 		// Move the first MD instruction to where it belongs
	DisplayPanel(pc->cinst[0]);			// Display the first MD instruction
	
	// We've made an instruction, so we need to increment this.
	uipc->ni = 1;
	uipc->max_ni = 1;
	
	int i;
	for(i = 0; i<2; i++) { // Place the current location panels and display them
		SetPanelPos(dc->cloc[i], 250, 1200);
		DisplayPanel(dc->cloc[i]);
	}
	
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
	mc->pathb[1] = panelHandle;
	mc->path[1] = panelHandle;
	mc->basefname[1] = panelHandle;
	mc->pbrun[1] = panelHandle;
	mc->pbwait[1] = panelHandle;
	mc->pbstop[1] = panelHandle;
	mc->mainstatus[1] = panelHandle;
	mc->startbut[1] = panelHandle;
	mc->stopbut[1] = panelHandle;

	
	// Populate the data tab controls
	dc->fgraph = FID_Graph;		//FID controls first
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
	pc->pctable = PulseInstP_PhaseCycleInstrs;     
	pc->uparrow = PulseInstP_UpButton;
	pc->downarrow = PulseInstP_DownButton;
	
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
	
	pc->tlist = MDInstr_DelayList;
	pc->tlistu = MDInstr_ListUnits;
	pc->idlist = MDInstr_InstrDataList;
	
	//pc->cexpr = MDInstr_IncExpression;
	
	pc->nsteps = MDInstr_NumSteps;
	pc->dim = MDInstr_Dimension;
	pc->vary = MDInstr_VaryInstr;

	return 0;
}

int get_number_of_dimension_points(int dim)
{
	int i, on, nd, np;
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NDimensionalOn, &on);
	if(!on)
		return 0;
	
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NumDimensions, &nd);
	if(dim >= nd)
		return -1;
	
	np = 0;
	
	for(i = 0; i < n_inst_disp; i++)
	{
		GetCtrlVal(cinst[i], MDInstr_VaryInstr, &on);
		if(!on)
			continue;
		
		GetCtrlVal(cinst[i], MDInstr_Dimension, &on);
		if(on == dim)
		{
			GetCtrlVal(cinst[i], MDInstr_NumSteps, &np);
			break;
		}
	}
	
	return np;
}
			

int populate_dim_points ()
{
	int dim, np, i, nl;
	
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NDimensionalOn, &dim);
	if(!dim)
		return 0;
	
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NumDimensions, &dim);
	
	//Insert a random item into each ring so we always know that number of list items is at least 1, then clear the rings.
	char c[10];  
	GetNumListItems(Pulse_Prog_Config, PPConfig_DimensionPoint, &nl);

	if(nl < dim-1)
	{
		for(i = nl; i< dim; i++)
		{
			sprintf(c, "%d", i);
			InsertListItem(Pulse_Prog_Config, PPConfig_DimensionPoint, -1, c, 0);
		}
	} else if (nl > dim-1)
		DeleteListItem(Pulse_Prog_Config, PPConfig_DimensionPoint, dim-1, -1);
	
	GetNumListItems(Pulse_Prog_Config, PPConfig_DimensionPoints, &nl);
	if(nl < dim-1)
	{
		for(i = nl; i< dim; i++)
		{
			sprintf(c, "%d", i);
			np = get_number_of_dimension_points(i);
			InsertListItem(Pulse_Prog_Config, PPConfig_DimensionPoints, -1, c, 0);
		}
	} else if (nl > dim-1)
		DeleteListItem(Pulse_Prog_Config, PPConfig_DimensionPoints, dim-1, -1);
	
	return 0;
}

int switch_transient_index(int t)
{
	/*//Switches FID and FFT indices. Its own function for convenience.
	int num, t1 = t, t2 = t;
	GetNumListItems(FID, FID_TransientNum, &num);
	if(--num < t1)
		t1 = num;
	GetNumListItems(FFTSpectrum, Spectrum_TransientNum, &num);
	if(--num < t2)
		t2 = num;
	
	SetCtrlIndex(FID, FID_TransientNum, t1);
	SetCtrlIndex(FFTSpectrum, Spectrum_TransientNum, t2);  */
	
	return 0;
}

int get_t_units_pair (int panel, int control)
{
	int c_or_i = -1, i;
	
	for(i = 0; i<ninstructions; i++)
	{
		if(panel == cinst[i])
		{
			c_or_i = 0;
			break;
		}
		
		if(panel == inst[i])
		{
			c_or_i = 1;
			break;
		}
	}
	
	if(c_or_i < 0)
		return -1;
	
	if(c_or_i && control == PulseInstP_TimeUnits)
		return PulseInstP_InstDelay;
	else if(c_or_i && control != PulseInstP_TimeUnits)
		return -2;
	
	/*switch(control)
	{
		case MDInstr_FinalDelayUnits:
			return MDInstr_FinalDelay;
			break;
		case MDInstr_InitDelayUnits:
			return MDInstr_InitDelay;
			break;
	}*/
	
	return -3;
}

char *itoa (int in)
{
	int nl = (int)(log10(in)+1);
	char *out = malloc(nl+3);
	
	sprintf(out, "%d", in);
	return out;
}

int get_view_mode()
{
	//Returns an integer which determines how the user wants the plots to be updated.
	//0 = Show newest transient
	//1 = Show average over all transients
	//2 = Do not update plots
	
	int i;
	
	GetCtrlIndex(HiddenPanel, HiddenVals_TransientView, &i);
	
	return i;
}

int polynomial_fit (double *y, int np, int order, double *output)
{
	//Feed this function a data set of size np and it will calculate a polynomial fit of order "order".
	double *x = malloc(sizeof(double)*np);
	int i;
	
	for(i = 0; i< np; i++)
		x[i] = (double)i;
	
	double *coef = malloc((order+1)*sizeof(double)), error;
	if(PolyFit(x, y, np, order, output, coef, &error) < 0)
		return -1;
	
	return 0;
}

int copy_file(char *from, char *to)
{
	//Duplicates "*from" with filename "*to"
	FILE *filefrom, *fileto;
	
	if(!file_exists(from))
		return -1;
	
	filefrom = fopen(from, "rb");
	
	if(filefrom == NULL)
		return -2;
	
	fileto = fopen(to, "wb+");
	if(fileto == NULL)
	{
		fclose(filefrom);
		return -3;
	}
	
	//Get file size.
	fseek(filefrom, 0, SEEK_END);
	long filesize = ftell(filefrom);
	rewind(filefrom);
	
	char *buffer = malloc(sizeof(char)*filesize);
	int err = 0;
	
	if(buffer == NULL)
	{
		err = -4;
		goto Error;
	}
	
	size_t result = fread(buffer, 1, filesize, filefrom);
	if(result != filesize)
	{
		err = -5;
		goto Error;
	}
	
	result = fwrite(buffer, 1, filesize, fileto);
	if(result != filesize)
	{
		err = -6;
		goto Error;
	}
	
	Error:
	fclose(filefrom);
	fclose(fileto);
	if(err != 0)
		remove(to);
	return err;
}

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
	
	//Main panel
	char *uifname = "Magnetometer Controller.uir"; // So it's easy to change this if we need to.

	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, uifname, MainPanel)) < 0)
		return -1;
	DisplayPanel (panelHandle);
	
	load_ui(uifname); // This function loads the UI and creates the structs.

	if(file_exists(savedsessionloc) != 1)
	{
		if(file_exists(defaultsloc) == 1)
			copy_file(defaultsloc, savedsessionloc);
	}
	
	if(file_exists(savedsessionloc) == 1)
	{				 
		load_configuration_from_file(savedsessionloc);
	} else
		load_DAQ_info();
	
	RunUserInterface ();
	DiscardPanel (panelHandle);
	
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

int CVICALLBACK StartProgram (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		
		double *totalTime = malloc(sizeof(double)), delaytime;
		int timer, i, ntransients, trans[2] = {0, 0};
		
		//Initialize board, set clock.  
		if(!initialized)
			pb_init_safe(1);
		
		if(started == 1)
		{
			pb_start_safe(1);
			break;
		}
		
		int status = update_status(0);
		
		if((status & 0x04))
		{
			if(pb_stop_safe(0) < 0)
				break;
		}   

		GetCtrlVal(Pulse_Prog_Tab, PulseProg_ContinuousRun, &cont_mode);
		/*
		//Program the board
		running_prog = malloc(sizeof(PPROGRAM));
		running_prog = get_current_program();
		
		//Start the program.
		GetCtrlVal(Pulse_Prog_Config, PPConfig_NPoints, &nPoints);
		GetCtrlVal(Pulse_Prog_Config, PPConfig_SampleRate, &running_prog->samplingrate);
		
		running_prog->transient = 1;
		running_prog->nPoints = nPoints;
		totalTime[0] = running_prog->total_time/ms;
		
		SetQuitIdle(0);
		SetDoubleQuitIdle(0);
		
		CmtScheduleThreadPoolFunctionAdv(DEFAULT_THREAD_POOL_HANDLE, IdleAndGetData, NULL, 0, discardIdleAndGetData, EVENT_TP_THREAD_FUNCTION_END, NULL, RUN_IN_SCHEDULED_THREAD, NULL);
		*/
		break;
	}
	return 0;
}


int CVICALLBACK StopProgram (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			int contrun;
			GetCtrlVal(Pulse_Prog_Tab, PulseProg_ContinuousRun, &contrun);
			SetCtrlVal(Pulse_Prog_Tab, PulseProg_ContinuousRun, 0);
	
			if(GetQuitIdle())
				SetDoubleQuitIdle(1);
			
			started = 0;
			SetQuitIdle(1);
			SetQuitUpdateStatus(1);
			
			if(idle_thread != NULL)
				CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE, idle_thread, NULL);
			
			if(update_thread != NULL)
				CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE, update_thread, NULL);
			
			pb_stop_safe(0);
			pb_close_safe(0);
            
			update_status(0);
			
			SetCtrlVal(Pulse_Prog_Tab, PulseProg_ContinuousRun, contrun);
			
			break;
		case EVENT_RIGHT_CLICK:

			break;
	}
	return 0;
}

int CVICALLBACK UpdateStatus (void *functiondata)
{
	int rv = 0;
	int quit = GetQuitUpdateStatus();
	Timer();
	CmtGetLock(lock_pp);
	while(!quit)
	{
		get_status = update_status(0);
		SetStatus(get_status);
		if(get_status & 0x04)
			break;
		if(Timer() > 0.5)
			break;
		Delay(0.001);
		quit = GetQuitUpdateStatus();
	}
	
	while(!quit)
	{
		
		get_status = update_status(0);
		SetStatus(get_status);
		if(get_status < 0 || get_status > 32)
		{
			rv = -1;
			break;
		} else if (get_status & 0x01) {
			rv = 0;
			break;
		}

		Delay(0.001);
		
		quit = GetQuitUpdateStatus();
		
	}
	CmtReleaseLock(lock_pp);
	
	if(!(get_status & 0x04) && started)
		started = 0;
	
	CmtExitThreadPoolThread(0);
	
	return 0;
	
}

int Load_ND_Labels (int d)
{
	/*
	if(d < 1 || d > 9)
		return -1;
	
	if(d == 1)
		return MainPanel_ID1;
	if(d == 2)
		return MainPanel_ID2;
	if(d == 3)
		return MainPanel_ID3;
	if(d == 4)
		return MainPanel_ID4;
	if(d == 5)
		return MainPanel_ID5;
	if(d == 6)
		return MainPanel_ID6;
	if(d == 7)
		return MainPanel_ID7;
	if(d == 8)
		return MainPanel_ID8;
	if(d == 9)
		return MainPanel_ID9;
			   */
	return -2; 
}

int Load_ND_Val (int d)
{
	/*if(d < 1 || d > 9)
		return -1;
	if(d == 1)
		return MainPanel_ID1Val;
	if(d == 2)
		return MainPanel_ID2Val;
	if(d == 3)
		return MainPanel_ID3Val;
	if(d == 4)
		return MainPanel_ID4Val;
	if(d == 5)
		return MainPanel_ID5Val;
	if(d == 6)
		return MainPanel_ID6Val;
	if(d == 7)
		return MainPanel_ID7Val;
	if(d == 8)
		return MainPanel_ID8Val;
	if(d == 9)
		return MainPanel_ID9Val;*/
	
	return -2;
}

int CVICALLBACK IdleAndGetData(void *functionData)
{
		
		int rv = 0, i, j;
		int get_status = GetStatus();
		char *err_string = malloc(100);
		int threadstat = 0;
		int l = 0;
		PPROGRAM *p = malloc(sizeof(PPROGRAM));
		
		SetCtrlVal(panelHandle, MainPanel_IsRunning, 1);
		SetCtrlAttribute(panelHandle, MainPanel_IsRunning, ATTR_LABEL_TEXT, "Running");
		
		for(i = 1; i <= 9; i++)
		{
			SetCtrlAttribute(panelHandle, Load_ND_Labels(i), ATTR_VISIBLE, 0);
			SetCtrlAttribute(panelHandle, Load_ND_Val(i), ATTR_VISIBLE, 0);
		}
		
		for(i = 1; i < running_prog->nDimensions; i++)
		{
			SetCtrlAttribute(panelHandle, Load_ND_Labels(i), ATTR_VISIBLE, 1);
			SetCtrlAttribute(panelHandle, Load_ND_Val(i), ATTR_VISIBLE, 1);
		}
			
		
		while(!GetQuitIdle())
		{
			while(!GetQuitIdle() && running_prog->transient <= running_prog->ntransients)
			{
				if(cont_mode && running_prog->nDimensions > 1)
				{
					SetCtrlVal(Pulse_Prog_Tab, PulseProg_ContinuousRun, 0);
					cont_mode = 0;
				}
				
				if(cont_mode)
				{
				
					GetCtrlVal(Pulse_Prog_Tab, PulseProg_ContinuousRun, &cont_mode);
				
					if(!cont_mode)
						break;
				
					p = get_current_program();
					if(p == NULL)
						rv = -2;
				
					p->delaytime = 0;
					p->transient = running_prog->transient;
					p->ntransients = running_prog->ntransients;
					if(running_prog->transient > 1)
						p->filename = running_prog->filename;
					if(rv >= 0)
						pprogramcpy(p, running_prog);
					else
						MessagePopup("Error", "Error with program, using last working program.\n");
					
					SetCtrlAttribute(Pulse_Prog_Config, PPConfig_NPoints, ATTR_DIMMED, 1);
					SetCtrlAttribute(Pulse_Prog_Config, PPConfig_SampleRate, ATTR_DIMMED, 1);
					
				}
			
				pprogramcpy(running_prog, p);
			
				CmtGetLock(lock_pp);
				if(pulse_program(p) < 0)
				{
					rv = -1;
					CmtReleaseLock(lock_pp);
					return rv; 
				}
				CmtReleaseLock(lock_pp);	
	
				//Declare local variables.
		
				//SetCtrlVal(panelHandle, MainPanel_ScanNum, running_prog->transient);
				int errv;
				if(running_prog->scan)
				{
					if(!l)
					{
						setup_DAQ_task(&acquireSignal, &counterTask);
						l = 1;
					}
					
					
					CmtGetLock(lock_DAQ);
					errv = DAQmxStartTask(counterTask);
					errv = DAQmxStartTask(acquireSignal);
					CmtReleaseLock(lock_DAQ);

				}
		
				//Start the scan.
				if(started == 0)
				{
					pb_start_safe(0);
					started = 1;
				}
		
				if(update_thread == NULL)
					rv = CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE, UpdateStatus, NULL, &update_thread);
				else
				{
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE, update_thread, ATTR_TP_FUNCTION_EXECUTION_STATUS, &threadstat);
					if(threadstat > 1)
					{
						CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE, update_thread, 0);
						rv = CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE, UpdateStatus, NULL, &update_thread);
					}
				} 

				if(rv < 0)
				{
					CmtGetErrorMessage(rv, err_string);
					MessagePopup("Multithreading Error", err_string);
					return rv;
				} 
			
				if(!running_prog->scan && cont_mode)
					continue;
			
				if(!running_prog->scan)
					return rv;
			
				if(running_prog->transient == running_prog->ntransients && cont_mode)
					running_prog->ntransients++;
			
				int err = get_data(p);
			

				while(!(GetStatus() & 0x01) && !GetQuitIdle())
					Delay(0.01);
					
				if(err < 0)
				{
					switch(err)
					{
						case -1:
							sprintf(err_string, "Error Getting Data %d: Transient number is greater than number of transients.", err);
							break;
						case -2:
							sprintf(err_string, "Error Getting Data %d: Failed to read the correct number of samples from the DAQ.", err);
							break;
						case -3:
							sprintf(err_string, "Error Writing Data %d: Transient number less than 1.", err);
							break;
						case -4:
							sprintf(err_string, "Error Writing Data %d: Unable to read average file.", err);
							break;
						case -5:
							sprintf(err_string, "Error Writing Data %d: Average file is invalid.", err);
							break;
						case -6:
							sprintf(err_string, "Error Writing Data %d: Number of points in average file does not match.", err);
							break;
						case -7:
							sprintf(err_string, "Error Writing Data %d: Number of transients in average file is invalid.", err);
							break;
						case -8:
							sprintf(err_string, "Error Writing Data %d: Invalid entry in average file.", err);
							break;
						case -9:
							sprintf(err_string, "Error Writing Data %d: Average file prematurely truncated.", err);
							break;
					}
					
					MessagePopup("Error", err_string);
					CmtExitThreadPoolThread(err);
				}
				
				running_prog->transient++;
			}
			
			if(running_prog->transient >= running_prog->ntransients)
			{
				for(i = 0; i < running_prog->nDimensions-1; i++)
				{
					if(running_prog->step[i] < running_prog->nSteps[i])
					{	
						running_prog->step[i]++;
						for(j = 0; j < i; j++)
							running_prog->step[j] = 1;
						running_prog->firststep = 0;
						running_prog->transient = 1;
						break;
					}
				}
				if(i == running_prog->nDimensions-1)
					break;
			 }
			
			if(running_prog->nDimensions < 2)
				break;
			
		}
	return rv;
	*/
	
	return 0;
			
}

void CVICALLBACK discardIdleAndGetData (int poolHandle, int functionID, unsigned int event, int value, void *callbackData)
{
	if(running_prog->scan)
	{
		CmtGetLock(lock_DAQ);
		DAQmxClearTask(acquireSignal);
		DAQmxClearTask(counterTask);
		CmtReleaseLock(lock_DAQ);
	}
		
	update_status(0);
			    
	pb_close_safe(0);
	SetCtrlVal(panelHandle, MainPanel_IsRunning, 0);
	SetCtrlAttribute(panelHandle, MainPanel_IsRunning, ATTR_LABEL_TEXT, "Stopped");
	
	SetCtrlAttribute(Pulse_Prog_Config, PPConfig_NPoints, ATTR_DIMMED, 0);
	SetCtrlAttribute(Pulse_Prog_Config, PPConfig_SampleRate, ATTR_DIMMED, 0);
}

int CVICALLBACK QuitCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			SetCtrlVal(panelHandle, MainPanel_IsRunning, 0);
			
			save_configuration_to_file(savedsessionloc);
			if(!file_exists(defaultsloc))
			{
				int val = ConfirmPopup("Default Configuration Missing", "The default configuration file is missing. Would you like to use the current configuration as the default?");
				if(val)
					save_configuration_to_file(defaultsloc);
			}
			
			if(idle_thread != NULL)
				CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE, idle_thread, 0);
			if(update_thread != NULL)
			{
				SetQuitUpdateStatus(1);
				CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE, update_thread, 0);
			}
			
			QuitUserInterface(0);
			break;
	}
	return 0;
}


//////////////////////////////////////////////////////
//                                                  //
//    Data Acquisition and Processing Functions     //
//                                                  //
//////////////////////////////////////////////////////

int load_fid_data(char *filename, int np, double *fid)
{
	//Loads FID data from filename into a plot.
	if(!file_exists(filename))
		return -1;
	
	FILE *fileid = fopen(filename, "r");
	if(fileid == NULL)
		return -2;
	
	if(np < 1)
	{
		fclose(fileid);
		return -3;
	}
	
	char *string_buffer = malloc(500), *string_match = malloc(500);
	int i;
	
	while(!feof(fileid))
	{
		fgets(string_buffer, 500, fileid);
		if(strstr(string_buffer, "[FID]") != NULL || strstr(string_buffer, "[TransientData]") != NULL)
			break;
	}
	
	if(feof(fileid))
	{
		fclose(fileid);
		return -4;
	}

	double val;
	int *fiderr = malloc(sizeof(int)*np);
	
	for(i = 0; i<np; i++)
	{
		fgets(string_buffer, 500, fileid);
		if(sscanf(string_buffer, "%lf", &val) != 1)
		{
			
			fiderr[i] = 1;
			fid[i] = 0.0;
		}
		else
		{
			fiderr[i] = 0;
			fid[i] = (double)val;
		}
	}
	
	fclose(fileid);	
	return 0;
}

int load_fft_data(char *filename, int np, double *realchan, double *imagchan)
{
	//Loads FID data from filename into a plot.
	if(!file_exists(filename))
		return -1;
	
	FILE *fileid = fopen(filename, "r");
	if(fileid == NULL)
		return -2;
	
	if(np < 1)
	{
		fclose(fileid);
		return -3;
	}
	
	char *string_buffer = malloc(500), *string_match = malloc(500);
	int i;
	
	while(!feof(fileid))
	{
		fgets(string_buffer, 500, fileid);
		if(strstr(string_buffer, "[RealChannel]\t[ImagChannel]") != NULL)
			break;
	}
	
	if(feof(fileid))
	{
		fclose(fileid);
		return -4;
	}

	double rc, ic;
	int n, *ffterr = malloc(sizeof(int)*np);
	
	for(i = 0; i<np; i++)
	{
		fgets(string_buffer, 500, fileid);
		if(sscanf(string_buffer, "%lf %lf", &rc, &ic) < 2)
		{
			
			ffterr[i] = 1;
			realchan[i] = 0.0;
			imagchan[i] = 0.0;
		}
		else
		{
			ffterr[i] = 0;
			realchan[i] = rc;
			imagchan[i] = ic;
		}
	}
	
	fclose(fileid);	
	return 0;
}

int write_data(double *fid, int np, int chan, int nc, int t, int nt, PPROGRAM *p, char *filename, char *pathname)
{
	/*
	srand(time(NULL));
	//Writes the average FID to file.
	
	char *averagefilename = malloc(MAX_FILENAME_LEN), *fidname = malloc(MAX_PATHNAME_LEN), *bufferfile1 = malloc(MAX_PATHNAME_LEN), *bufferfile2 = malloc(MAX_PATHNAME_LEN);
	char extension[5] = ".txt", c;
	double *average = malloc(sizeof(double)*np);
	time_t seconds = time(NULL);


	int i;
	FILE *fileout, *buffer;
	
	if(filename == NULL)
		sprintf(filename, "No name supplied.");
	
	if(p->nDimensions > 1)
	{
		sprintf(averagefilename, "AverageFID%s", extension);
		sprintf(fidname, "%sFIDTransient%04d%s", pathname, t, extension);
	} else {
		sprintf(averagefilename, "%s%s", filename, extension);
		sprintf(fidname, "%s%s-FIDTransient%04d%s", pathname, filename, t, extension);
	}
	
	strcat(pathname, averagefilename);
	free(averagefilename);
	
	for(i = 0; i<np; i++)
		average[i] = fid[i];
	
	if(t == 1)
		fileout = fopen(pathname, "w+");
	else if(t > 1)
	{
		//Generate the average if t > 1.
		int err = average_from_file(pathname, average, np);
		if(err < 0)
			return -(1-err);
		
		bufferfile2 = temp_file(extension);
		fileout = fopen(bufferfile2, "w+");
	} else 
		return -1;
	
	//Starting with the average, make a header with experiment, date, time, number of points, sampling rate and number of dimensions if nDimensions=1.
	fprintf(fileout, "Experiment: %s\n", filename);
	fprintf(fileout, "Channel: %d of %d\n", chan, nc);
	fprintf(fileout, "Completed: %s\n", ctime(&seconds));
	fprintf(fileout, "RawTime: %lu\n\n", (unsigned long int)seconds);
	fprintf(fileout, "TransientsCompleted= %d of %d\n\n", t, nt);
	
	if(p->nDimensions == 1)
		fprintf(fileout, "NPoints= %d\nSamplingRate= %lf\nnDimensions= %d\n\n", p->nPoints, p->samplingrate, 1);
	else
		fprintf(fileout, "NPoints= %d\nSamplingRate= %lf\n\n", p->nPoints, p->samplingrate);
	
	//Print the average.
	fprintf(fileout, "[FID]\n");
	for(i = 0; i<np; i++)
		fprintf(fileout, "%lf\n", average[i]);
	fprintf(fileout, "[ENDFID]\n\n");
	fclose(fileout);
	
	if(t>1) {
		remove(pathname);															  
		rename(bufferfile2, pathname);
	}
	
	//Generate a pulse program file
	bufferfile1 = temp_file(extension);
	SaveProgram_PPROGRAM(p, bufferfile1);

	//Print individual transient data.
	fileout = fopen(fidname, "w");
	fprintf(fileout, "Transient Data - Experiment: %s\n", filename);
	fprintf(fileout, "Channel: %d of %d\n", chan, nc); 
	fprintf(fileout, "Completed: %s\n", ctime(&seconds));
	fprintf(fileout, "RawTime: %lu\n\n", (unsigned long int)seconds);
	fprintf(fileout, "Transient %d of %d\n\n", t, nt);
	
	buffer = fopen(bufferfile1, "r+");  
	while(!feof(buffer))
	{
		c = getc(buffer);
		if(feof(buffer))
			break;
		putc(c, fileout);
	}
	
	rewind(buffer);
	
	fprintf(fileout, "\n\n[TransientData]\n");
	for(i = 0; i<np; i++)
		fprintf(fileout, "%lf\n", fid[i]);
	fprintf(fileout, "[EndTransientData]\n\n");
	fclose(fileout);
	fclose(buffer);
	remove(bufferfile1);
	*/
	return 0;
}

int update_current_dim_point(int *point, int nd)
{
	/*int nl, dim1, dim2, i;
	GetNumListItems(FID, FID_CurrentDimPoint, &nl);
	if(nl != 0)
		DeleteListItem(FID, FID_CurrentDimPoint, 0, -1);
	
	for(i = 0; i<nd; i++)
		InsertListItem(FID, FID_CurrentDimPoint, -1, "", point[i]);
	
	for(i = 1; i<nd+1; i++)
		SetCtrlVal(panelHandle, Load_ND_Val(i), itoa(point[i-1]));
	
	GetCtrlIndex(FID, FID_Dimension, &dim1);
	GetCtrlIndex(FFTSpectrum, Spectrum_Dimension, &dim2);
	if(dim1 != dim2)
		return -1;
	
	
	
	GetNumListItems(FID, FID_DimPoint, &nl);
	GetNumListItems(FFTSpectrum, Spectrum_DimPoint, &dim2);
	
	if(nl != dim2)
		return -2;
	
	if(point[dim1] > nl)
		return -3;
	
	SetCtrlVal(FID, FID_DimPoint, point[dim1]);
	SetCtrlVal(FFTSpectrum, Spectrum_DimPoint, point[dim1]); */
	return 0;
}

int update_dim_point_control(int *steps, int nd)
{
	/*int nl, i;
	GetNumListItems(FID, FID_DimPointsCompleted, &nl);
	
	if(nl > 0)
		DeleteListItem(FID, FID_DimPointsCompleted, 0, -1);
	
	for(i = 0; i<nd; i++)
		InsertListItem(FID, FID_DimPointsCompleted, -1, "", steps[i]);
	
	int d, np, nl1, nl2;
	char c[10];  
	
	GetNumListItems(FID, FID_Dimension, &nl1);
	GetNumListItems(FFTSpectrum, Spectrum_Dimension, &nl2);
	if(nl1 != nd || nl2 != nd)
	{
		if(nl1 != 0)
		{
			GetCtrlIndex(FID, FID_Dimension, &d);
			DeleteListItem(FID, FID_Dimension, 0, -1);
		} else
			d = 0;
		
		if(nl2 != 0)
			DeleteListItem(FFTSpectrum, Spectrum_Dimension, 0, -1);
		
		for(i = 0; i<nd; i++)
		{
			sprintf(c, "D: %d", i+1);
			InsertListItem(FID, FID_Dimension, -1, c, i);
			InsertListItem(FFTSpectrum, Spectrum_Dimension, -1, c, i);
		}
		
		if(d < nd && d>=0)
		{
			SetCtrlIndex(FID, FID_Dimension, d);
			SetCtrlIndex(FFTSpectrum, Spectrum_Dimension, d);
		}
	}
		
	GetCtrlIndex(FID, FID_Dimension, &d);
	GetNumListItems(FID, FID_DimPoint, &nl1);
	GetNumListItems(FFTSpectrum, Spectrum_DimPoint, &nl2);


	if(nl1 != steps[d] || nl2 != steps[d])
	{
		if(nl1 != 0 || nl2 != 0)
		{
			DeleteListItem(FID, FID_DimPoint, 0, -1);
			DeleteListItem(FFTSpectrum, Spectrum_DimPoint, 0, -1);
		}
		
		for(i = 1; i<=steps[d]; i++)
		{
			sprintf(c, "%d", i); 
			InsertListItem(FID, FID_DimPoint, -1, c, i);
			InsertListItem(FFTSpectrum, Spectrum_DimPoint, -1, c, i);
		}
	}  */

	return 0;
}
	

int get_data(PPROGRAM *p)
{
	
	int i, j, nc, np = p->nPoints, t = p->transient, nt = p->ntransients, err;
	int32 nsamplesread;
	double sr = p->samplingrate;
	fpos_t *position = malloc(sizeof(fpos_t));
	char *pathname = malloc(MAX_PATHNAME_LEN), *string_buffer = malloc(500), *string_match = malloc(500);
	char *filename = malloc(MAX_FILENAME_LEN), c;
	time_t seconds = time(NULL);
	float64 **DAQsignal;
	
	GetCtrlVal(panelHandle, MainPanel_Path, pathname);
	GetCtrlVal(panelHandle, MainPanel_Filename, filename);
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NumChans, &nc);
	
	
	if(t > nt || nc < 1 ||  nc > 8)
		return -1;
	
	DAQsignal = malloc(sizeof(float64 *)*nc);
	for(i = 0; i<nc; i++)
		DAQsignal[i] = malloc(sizeof(float64)*np);
	
	float64 *samplearray = malloc(sizeof(float64)*np*nc);
	
	unsigned int bc;
	double time = Timer();
	char *errmess = malloc(300);
	i = 0;
	
	while(i < 400 && !GetDoubleQuitIdle())
	{
		CmtGetLock(lock_DAQ);
		DAQmxGetReadAttribute(acquireSignal, DAQmx_Read_AvailSampPerChan, &bc);
		CmtReleaseLock(lock_DAQ);
		if(bc >= np)
			break;
		
		i++;
		Delay(0.05);
		if(i == 400)
		{
			sprintf(errmess, "The readout task is about to time out. It has been occuring for %lf seconds and has read %d samples. Allow it to wait another 2 seconds?", Timer() - time, bc);
			j = ConfirmPopup("Timeout Warning", errmess);
			
			if(j)
				i -= 40;
			else
				return -2;
		}  
		
	}

	//Read out task
	CmtGetLock(lock_DAQ);
	DAQmxReadAnalogF64(acquireSignal, -1, 0.0, DAQmx_Val_GroupByChannel, samplearray, np*nc, &nsamplesread, NULL);
	CmtReleaseLock(lock_DAQ);
	if(nsamplesread != np)
		return -2;
	
	//Read the samples from the array so that they are in a convienient 2D array.
	for(j = 0; j<nc; j++)
	{
		for(i = 0; i<np; i++)
			DAQsignal[j][i] = samplearray[i+j*np];
	}
	
	free(samplearray);
	
	
	//Plot the data
	double *rc = malloc(sizeof(double)*np), *ic = malloc(sizeof(double)*np);
	int fidplot, fftplot;
	
	//Generate a unique filename.
	if(t == 1 && running_prog->firststep)
	{
		char *newfilename = malloc(MAX_FILENAME_LEN);
		running_prog->filename = malloc(MAX_FILENAME_LEN);
		
		char *checkfilename = malloc(MAX_FILENAME_LEN);
		for(i = 0; i<10000; i++)
		{
			sprintf(newfilename, "%s%04d", filename, i);
			sprintf(checkfilename, "%s%s\\", pathname, newfilename);
			if(!file_exists(checkfilename))
				break;
		}
		strcpy(running_prog->filename, newfilename);   
		free(checkfilename);
		free(newfilename);
	}
	
	filename = running_prog->filename;
	strcat(pathname, filename);
	
	//SetCtrlVal(FID, FID_CurrentPathname, pathname);
	char *re = malloc(MAX_PATHNAME_LEN);
	sprintf(re, "%s\\%s.txt", pathname, filename);
	add_recent_experiment(re);
	
	strcat(pathname, "\\");
	
	if(!file_exists(pathname))
		MakeDir(pathname);
	
	char **chanpathname = malloc(sizeof(char *)*nc);
	
	for(i = 0; i<nc; i++)
	{
		chanpathname[i] = malloc(MAX_PATHNAME_LEN);
		sprintf(chanpathname[i], "%sC%d\\", pathname, i);
			
		if(!file_exists(chanpathname[i]) && nc > 1)
			MakeDir(chanpathname[i]);
	}
	
	if(nc == 1)
		strcpy(chanpathname[0], pathname);

	//Make the plot controls available.
	
	if(t == 1)
	{
		clear_plots();
		if(!running_prog->firststep && running_prog->nDimensions > 1)
		{
			SetCtrlAttribute(FID, FID_Dimension, ATTR_VISIBLE, 1);
			SetCtrlAttribute(FFTSpectrum, Spectrum_Dimension, ATTR_VISIBLE, 1);
			SetCtrlAttribute(FID, FID_DimPoint, ATTR_VISIBLE, 1);
			SetCtrlAttribute(FFTSpectrum, Spectrum_DimPoint, ATTR_VISIBLE, 1);
		}
	}   
	
	if(p->nDimensions > 1)
	{
		char *ndpathname = malloc(MAX_PATHNAME_LEN);
		
		for(i = 0; i<nc; i++)
		{
			strcpy(ndpathname, chanpathname[i]);
			for(j = 0; j<p->nDimensions-1; j++)
			{
				sprintf(string_buffer, "D%02d-%04d\\", j+1, p->step[j]);
				strcat(ndpathname, string_buffer);
				if(!file_exists(ndpathname))
					MakeDir(ndpathname);
			}
			
			err = write_data(DAQsignal[i], np, i, nc, t, nt, p, filename, ndpathname);
			if(err < 0)
				return -(2-err);
		}
	
		
		//Print information about how many have been completed
		FILE *fileout;
		char *bufferfile = malloc(MAX_PATHNAME_LEN);
		strcat(pathname, filename);
		strcat(pathname, ".txt");
		if(running_prog->firststep && t == 1)
			fileout = fopen(pathname, "w+");
		else
		{
			bufferfile = temp_file(".txt");
			fileout = fopen(bufferfile, "w+");
		}
		
		//Starting with the average, make a header with experiment, date, time, number of points, sampling rate and number of dimensions if nDimensions=1.
		fprintf(fileout, "Experiment: %s\n", filename);
		fprintf(fileout, "Channels: %d\n", nc);
		fprintf(fileout, "Completed: %s\n", ctime(&seconds));
		fprintf(fileout, "RawTime: %lu\n\n", (unsigned long int)seconds);
		fprintf(fileout, "NPoints= %d\nSamplingRate= %lf\nnDimensions= %d\n\n", running_prog->nPoints, running_prog->samplingrate, running_prog->nDimensions);
		fprintf(fileout, "[Point]\n");
		for(i = 0; i<running_prog->nDimensions-1; i++)
			fprintf(fileout, "IndirectDim %d - %d of %d\n", i+1, running_prog->step[i], running_prog->nSteps[i]);
		
		fprintf(fileout, "\nTransientsCompleted= %d of %d\n", t, nt);
		fclose(fileout);
		
		if(!running_prog->firststep || t != 1)
		{
			remove(pathname);
			rename(bufferfile, pathname);
		}
		 
		update_dim_point_control(p->step, p->nDimensions-1);
		update_current_dim_point(p->step, p->nDimensions-1);

		if(t == 1 && running_prog->firststep)
		{
			SetCtrlAttribute(FID, FID_Dimension, ATTR_VISIBLE, 1);
			SetCtrlAttribute(FFTSpectrum, Spectrum_Dimension, ATTR_VISIBLE, 1);
			SetCtrlAttribute(FID, FID_DimPoint, ATTR_VISIBLE, 1);
			SetCtrlAttribute(FFTSpectrum, Spectrum_DimPoint, ATTR_VISIBLE, 1);
		} 
	} else {
		for(i = 0; i<nc; i++)
		{
			err = write_data(DAQsignal[i], np, i, nc, t, nt, p, filename, chanpathname[i]);           
			if(err < 0)
				return err-2;
		}
		
		if(nc > 1)
		{
			//If there are multiple channels, put together a generic header file.
			FILE *fileout;
			char *bufferfile = malloc(MAX_PATHNAME_LEN);
			strcat(pathname, filename);
			strcat(pathname, ".txt");
			if(t == 1)
				fileout = fopen(pathname, "w+");
			else
			{
				bufferfile = temp_file(".txt");
				fileout = fopen(bufferfile, "w+");
			}
		
			//Starting with the average, make a header with experiment, date, time, number of points, sampling rate and number of dimensions if nDimensions=1.
			fprintf(fileout, "Experiment: %s\n", filename);
			fprintf(fileout, "Channels: %d\n", nc);
			fprintf(fileout, "Completed: %s\n", ctime(&seconds));
			fprintf(fileout, "RawTime: %lu\n\n", (unsigned long int)seconds);
			fprintf(fileout, "NPoints= %d\nSamplingRate= %lf\nnDimensions= %d\n\n", running_prog->nPoints, running_prog->samplingrate, running_prog->nDimensions);

			fprintf(fileout, "\nTransientsCompleted= %d of %d\n", t, nt);
			fclose(fileout);
		
			if(t != 1)
			{
				remove(pathname);
				rename(bufferfile, pathname);
			}
		}
	}
		
	insert_transient(t); 
	if(t == 2)
		insert_transient(0);

	int display_view = get_view_mode();
		
	if(t == 1 || display_view == 0)
		change_fid_and_fft(t);
	else if (display_view == 1)
		change_fid_and_fft(0);
	
	SetCtrlAttribute(FID, FID_TransientNum, ATTR_DIMMED, 0);
	SetCtrlAttribute(FFTSpectrum, Spectrum_TransientNum, ATTR_DIMMED, 0);
	SetCtrlAttribute(FFTSpectrum, Spectrum_Channel, ATTR_DIMMED, 0);
	
	CmtGetLock(lock_DAQ);
	DAQmxStopTask(acquireSignal);
	DAQmxStopTask(counterTask);
	CmtReleaseLock(lock_DAQ);
	
	return 0;
}

int average_from_file(char *filename, double *input, int np)
{
	double inbuffer;
	int nt = 0, check, i = 0, j;
	char *string_buffer = malloc(100), *string_match = malloc(100);

	FILE *fileid; 
	
	if(nt == 1)
		return 0;

	fileid = fopen(filename, "r");
	
	if(fileid == NULL)
		return -1;
	
	while(i<2)
	{
		fgets(string_buffer, 100, fileid);
		if(feof(fileid))
		{
			fclose(fileid);
			return -2;
		}
			
		if(sscanf(string_buffer, "%s %d", string_match, &check) > 1 && strstr(string_match, "TransientsCompleted=") != NULL)
		{
			nt = check;
			i++;
		} else if (strstr(string_match, "NPoints=") != NULL){
			i++;
			if(np != check)
			{
				fclose(fileid);
				return -3;
			}
		}
		
		if(i == 2 && nt < 1)
		{
			fclose(fileid);
			return -4;
		}
	}
	
	while(i)
	{
		fgets(string_buffer, 100, fileid);
		if(feof(fileid))
		{
			fclose(fileid);
			return -2;
		}
		if(sscanf(string_buffer, "%s", string_match) && strstr(string_match, "[FID]") != NULL)
			i = 0;
	}
	
	for(i = 0; i<np; i++)
	{
		fgets(string_buffer, 100, fileid);
		if(sscanf(string_buffer, "%lf", &inbuffer))
			input[i] = ((inbuffer*nt)+input[i])/(nt+1);
		else
		{
			fclose(fileid);
			return -5;
		}
	}
	
	fgets(string_buffer, 100, fileid);
	if(!sscanf(string_buffer, "%s", string_match) || (strstr(string_match, "[ENDFID]") == NULL))
	{
		fclose(fileid);
		return -6;
	}
	
	fclose(fileid);
	return 0;
}


int insert_transient(int t)
{
	//Inserts a value for FID and FFT.
	/*if(t == 0)
	{
		InsertListItem(FID, FID_TransientNum, 0, "Average", 0);
		InsertListItem(FFTSpectrum, Spectrum_TransientNum, 0, "Average", 0);
	} else {
		char *c = malloc(50);
		sprintf(c, "Transient %d", t);
		InsertListItem(FID, FID_TransientNum, -1, c, t);
		InsertListItem(FFTSpectrum, Spectrum_TransientNum, -1, c, t);
	}   */
	return 0;
}



//////////////////////////////////////////////////////
//                                                   //
//            Configuration Functions                //
//                                                   //
///////////////////////////////////////////////////////

int save_configuration_to_file (char *fname)
{
	int n = all_panels(-1), cih, cid, pih = 0, pid = 0, i, j, val, p, hid, dim;
	char *filename = malloc(MAX_PATHNAME_LEN);
	
	strcpy(filename, fname);
	
	//Ensure that the filename is a .bin file
	if(strcmp(filename + strlen(filename) - 4, ".bin") != 0)
		strcat(filename, ".bin");
	
	ClearListCtrl(HiddenPanel, HiddenVals_ControlHidden);
	ClearListCtrl(HiddenPanel, HiddenVals_ControlDimmed);
	
	for(i = 0; i<n; i++)
	{
		p = all_panels(i);
		pih = InsertTreeItem(HiddenPanel, HiddenVals_ControlHidden, VAL_SIBLING, pih, VAL_NEXT, NULL, NULL, NULL, p);
		pid = InsertTreeItem(HiddenPanel, HiddenVals_ControlDimmed, VAL_SIBLING, pid, VAL_NEXT, NULL, NULL, NULL, p);

		val= all_controls(p, -1);
		
		for(j = 0; j<val; j++)
		{
			cih = InsertTreeItem(HiddenPanel, HiddenVals_ControlHidden, VAL_CHILD, pih, VAL_LAST, NULL, NULL, NULL, all_controls(p, j)); 
			cid = InsertTreeItem(HiddenPanel, HiddenVals_ControlDimmed, VAL_CHILD, pid, VAL_LAST, NULL, NULL, NULL, all_controls(p, j)); 
			
			GetCtrlAttribute(p, all_controls(p, j), ATTR_VISIBLE, &hid);
			GetCtrlAttribute(p, all_controls(p, j), ATTR_DIMMED, &dim);
			
			if(hid)
				SetTreeItemAttribute(HiddenPanel, HiddenVals_ControlHidden, cih, ATTR_MARK_STATE, VAL_MARK_ON);
			else
				SetTreeItemAttribute(HiddenPanel, HiddenVals_ControlHidden, cih, ATTR_MARK_STATE, VAL_MARK_OFF);
			
			if(dim)
				SetTreeItemAttribute(HiddenPanel, HiddenVals_ControlDimmed, cid, ATTR_MARK_STATE, VAL_MARK_ON);
			else
				SetTreeItemAttribute(HiddenPanel, HiddenVals_ControlDimmed, cid, ATTR_MARK_STATE, VAL_MARK_OFF);
			
		}
	}

	//Save the current program.
	char *progloc = malloc(MAX_PATHNAME_LEN);
	
	strcpy(progloc, filename);
	progloc[strlen(filename)-4] = '\0';
	strcat(progloc, "-Prog.txt");	

//	SaveProgram_IO(progloc);
//	SetCtrlVal(HiddenPanel, HiddenVals_LastProgramLoc, progloc);
	
	//Save the panel states.
	for(i = 0; i<n; i++)
		if(SavePanelState(all_panels(i), filename, i) < 0)
			return -1;
	
	return 0;
}

int load_configuration_from_file (char *filename)
{

	if(!file_exists(filename))
		return -1;

	int i, j, hid, dim, pih = 0, pid = 0, val, n = all_panels(-1), p, err = 0;

	//Load the panel states.
	for(i = 0; i<n; i++)
		RecallPanelState(all_panels(i), filename, i);
	
	for(i = 0; i<n; i++)
	{
		p = all_panels(i);
		val = all_controls(p, -1);
		
		GetTreeItemParent(HiddenPanel, HiddenVals_ControlHidden, pih, &j);

		if(j != -1)
			break;

		GetTreeItemNumChildren(HiddenPanel, HiddenVals_ControlHidden, pih++, &j);
		
		if(j != val)
			break;
		
		GetTreeItemParent(HiddenPanel, HiddenVals_ControlDimmed, pid, &j);
		if(j != -1)
			break;
		
		GetTreeItemNumChildren(HiddenPanel, HiddenVals_ControlDimmed, pid++, &j);
		if(j != val)
			break;
		
		for(j = 0; j<val; j++)
		{
			GetTreeItemAttribute(HiddenPanel, HiddenVals_ControlHidden, pih++, ATTR_MARK_STATE, &hid);
			GetTreeItemAttribute(HiddenPanel, HiddenVals_ControlDimmed, pid++, ATTR_MARK_STATE, &dim);
			
			if(j == 18 && (i == 1 || i == 2))
				j = j;

			SetCtrlAttribute(p, all_controls(p, j), ATTR_VISIBLE, hid);
			SetCtrlAttribute(p, all_controls(p, j), ATTR_DIMMED, dim);
			
		}
	}

	//Update the value of the Transient View
	int menuitems[3] = {MainMenu_View_TransView_ViewLatestTrans, MainMenu_View_TransView_ViewAverage, MainMenu_View_TransView_ViewNoUpdate};
	
	for(i = 0; i<3; i++)
	{
		
		GetCtrlIndex(HiddenPanel, HiddenVals_TransientView, &j);
		SetMenuBarAttribute(panelHandleMenu, menuitems[i], ATTR_CHECKED, (i == j));
	}
	
	//Load the program
	GetCtrlVal(Pulse_Prog_Tab, PulseProg_Trigger_TTL, &ttl_trigger);
	
	char *progloc = malloc(MAX_PATHNAME_LEN);
	
	GetCtrlVal(HiddenPanel, HiddenVals_LastProgramLoc, progloc);
//	LoadProgram_IO(progloc);
	
	
	/*
	Change_Trigger(Pulse_Prog_Tab, PulseProg_Trigger_TTL, EVENT_COMMIT, NULL, NULL, NULL);
	PhaseCycleCallback(Pulse_Prog_Config, PPConfig_PhaseCycle, EVENT_COMMIT, NULL, NULL, NULL);
	ContinuousRunCallback(Pulse_Prog_Tab, PulseProg_ContinuousRun, EVENT_COMMIT, NULL, NULL, NULL);
	  */
	//Run clear_plots() to clean up any artifacts from a default state being saved when a plot was active.
	clear_plots();
	
	
	generate_recent_experiments_menu();
	generate_recent_programs_menu();
	
	//Make sure we get the DAQ info right.
	load_DAQ_info();
	
	setup_broken_ttls();
	
	return err;
}


//////////////////////////////////////////////////////
//                                                   //
//                 DAQ Functions                     //
//                                                   //
///////////////////////////////////////////////////////

int get_devices()
{
	int buffersize, i, j=0;
	
	CmtGetLock(lock_DAQ);
	buffersize = DAQmxGetSystemInfoAttribute(DAQmx_Sys_DevNames, "", NULL);
	CmtReleaseLock(lock_DAQ);
	
	if(buffersize <= 0)
	{
	//	printf("No devices found.\n");
		return -1;
	}
	
	char *devices = malloc(buffersize), *device_name = malloc(buffersize);
	CmtGetLock(lock_DAQ);
	int k = DAQmxGetSystemInfoAttribute(DAQmx_Sys_DevNames, devices, buffersize);
	
	//Determine how many characters there are in the array containing the names of physical  analogue channels
	buffersize = DAQmxGetDeviceAttribute("Dev1", DAQmx_Dev_AI_PhysicalChans, "", 0);
	CmtReleaseLock(lock_DAQ);
	
	DeleteListItem(Pulse_Prog_Config, PPConfig_Device, 0, -1);
	
	for(i = 0; i<=strlen(devices); i++)
	{
		if(devices[i] == ',' || devices[i] == '\0' || i == strlen(devices))
		{
			device_name[j] = '\0';
			InsertListItem(Pulse_Prog_Config, PPConfig_Device, -1, device_name, device_name);
			j = 0;
		} 
		else
			device_name[j++] = devices[i];
	}
	
	if(i == 0)
	{
		MessagePopup("Error", "No devices available!\n");
		return -1;
	}
	
	return 0;
}

int load_DAQ_info_locked ()
{
	
	//This function queries any available DAQ boards and populates the various channels.
	
	int nl, buffersize, i, j=0;
	int *a_indices = malloc(sizeof(int)), a_index = -1, c_index = -1, t_index = -1, cg_index = -1;      
	
	CmtReleaseLock(lock_DAQ);
	int gd = get_devices();
	CmtGetLock(lock_DAQ);

	
	
	if(gd < 0)
	{
		GetNumListItems(Pulse_Prog_Config, PPConfig_AcquisitionChannel, &nl);
		if(nl > 0)
			DeleteListItem(Pulse_Prog_Config, PPConfig_AcquisitionChannel, 0, -1);
		
		GetNumListItems(Pulse_Prog_Config, PPConfig_Trigger_Channel, &nl);
		if(nl > 0)
			DeleteListItem(Pulse_Prog_Config, PPConfig_Trigger_Channel, 0, -1);
		
		GetNumListItems(Pulse_Prog_Config, PPConfig_CounterChan, &nl);
		if(nl > 0)
			DeleteListItem(Pulse_Prog_Config, PPConfig_CounterChan, 0, -1);
		
		GetNumListItems(Pulse_Prog_Config, PPConfig_ChannelGain, &nl);
		if(nl > 0)
			DeleteListItem(Pulse_Prog_Config, PPConfig_ChannelGain, 0, -1);
		
		return -1;
	}

	GetNumListItems(Pulse_Prog_Config, PPConfig_Device, &i);
	if(i<1)
		return -1;
	
	char *device = malloc(50);
	GetCtrlVal(Pulse_Prog_Config, PPConfig_Device, device);
	
	buffersize = DAQmxGetDeviceAttribute(device, DAQmx_Dev_AI_PhysicalChans, "", NULL);
	
	//Read out the array of the names of physical channels
	char *input_channels = malloc(buffersize), *channel_name = malloc(buffersize), *chanwithmark = malloc(buffersize); 
	
	DAQmxGetDeviceAttribute(device, DAQmx_Dev_AI_PhysicalChans, input_channels, buffersize);
	
	//Clear the ring control and replace it with the information acquired from the DAQ.
	GetNumListItems(Pulse_Prog_Config, PPConfig_AcquisitionChannel, &nl);
	int numchans = 0;
	
	if(nl > 0)
	{
		//Save the indexes it was on first.
		for(i = 0; i<nl; i++)
		{
			GetCtrlIndex(Pulse_Prog_Config, PPConfig_AcquisitionChannel, &j);
			GetLabelFromIndex(Pulse_Prog_Config, PPConfig_AcquisitionChannel, i, chanwithmark);
			if(chanwithmark[0] != ' ')
			{
				a_indices = realloc(a_indices, ++numchans*sizeof(int));
				a_indices[numchans-1] = i;
				if(a_index < 0)
					a_index = i;
			}
		}
		
		if(isin(j, a_indices, numchans) >= 0)
			a_index = j;
			
		DeleteListItem(Pulse_Prog_Config, PPConfig_AcquisitionChannel, 0, -1);
	}
	
	int k = 0;
	j = 0;
	
	for(i = 0; i<=strlen(input_channels); i++)
	{
		if(input_channels[i] == ',' || &input_channels[i] == '\0' || i == strlen(input_channels))
		{
			channel_name[j] = '\0';
			
			//Leave a space for a check mark, erase the device name from the label
			sprintf(chanwithmark, "   %s", &channel_name[strlen(device) + 1]);
			
			//If it was on before, leave it on.
			if(isin(k++, a_indices, numchans) >= 0)
				chanwithmark[0] = 149;
	
			//Populate the ring control
			InsertListItem(Pulse_Prog_Config, PPConfig_AcquisitionChannel, -1, chanwithmark, channel_name);
			j = 0;
		}	
		else
		{
			//Eliminate spaces
			if(j == 0 && input_channels[i] == ' ')
				continue;

			//Next Letter
			channel_name[j++] = input_channels[i];
		}
	}
	
	GetNumListItems(Pulse_Prog_Config, PPConfig_AcquisitionChannel, &nl);
	if(a_index < nl && a_index >= 0)
		SetCtrlIndex(Pulse_Prog_Config, PPConfig_AcquisitionChannel, a_index); 
	
	//Set up the channel gains
	int nl2;
	
	//Generate the list of channels in use (hidden for nc == 1)
	GetCtrlIndex(Pulse_Prog_Config, PPConfig_AcquisitionChannel, &a_index);
	GetValueFromIndex(Pulse_Prog_Config, PPConfig_AcquisitionChannel, a_index, channel_name);
	GetLabelFromIndex(Pulse_Prog_Config, PPConfig_AcquisitionChannel, a_index, chanwithmark);
	chanwithmark[0] = 149;
	ReplaceListItem(Pulse_Prog_Config, PPConfig_AcquisitionChannel, a_index, chanwithmark, channel_name); 
	
	//Generate a hidden list of the maximum values and minimum values for each channel.
	//This list will allow the user to have independent control on the gain of each channel in a simple manner.

	GetNumListItems(Pulse_Prog_Config, PPConfig_AcquisitionChannel, &numchans);
	
	GetNumListItems(Pulse_Prog_Config, PPConfig_MaxVals, &nl);
	GetNumListItems(Pulse_Prog_Config, PPConfig_MinVals, &nl2);
	
	if(numchans != nl || nl != nl2)
	{
		if(nl > 0)
			DeleteListItem(Pulse_Prog_Config, PPConfig_MaxVals, 0, -1);
		
		if(nl2 > 0)
			DeleteListItem(Pulse_Prog_Config, PPConfig_MinVals, 0, -1);

		//Initialize all values at 10.0 and -10.0V if values don't already exist.
		for(i = 0; i<numchans; i++)
		{
			GetValueFromIndex(Pulse_Prog_Config, PPConfig_AcquisitionChannel, i, channel_name);
			InsertListItem(Pulse_Prog_Config, PPConfig_MaxVals, -1, channel_name, 10.0);
			InsertListItem(Pulse_Prog_Config, PPConfig_MinVals, -1, channel_name, -10.0);
		}

	}
	
	//Get the buffer size necessary to read out the digital input lines.
	buffersize = DAQmxGetDeviceAttribute(device, DAQmx_Dev_Terminals, "", 0);
	
	//Free up the old variables and repurpose them.
	free(input_channels);
	free(channel_name);
	free(chanwithmark);
	input_channels = malloc(buffersize);
	channel_name = malloc(buffersize);
	chanwithmark = malloc(buffersize);
	
	DAQmxGetDeviceAttribute(device, DAQmx_Dev_Terminals, input_channels, buffersize);
	
	//Again clear the ring control and replace it with information from the DAQ.
	
	GetNumListItems(Pulse_Prog_Config, PPConfig_Trigger_Channel, &nl);
	if(nl > 0)
	{
		GetCtrlIndex(Pulse_Prog_Config, PPConfig_Trigger_Channel, &t_index);
		DeleteListItem(Pulse_Prog_Config, PPConfig_Trigger_Channel, 0, -1);
	}
	
	for(i = 0; i<=strlen(input_channels); i++)
	{
		if(input_channels[i] == ',' || input_channels[i] == '\0' || i == strlen(input_channels))
		{
			channel_name[j] = '\0';
			
			//Erase the device name
			chanwithmark = &channel_name[strlen(device)+2];
			
			//Populate the ring controls
			InsertListItem(Pulse_Prog_Config, PPConfig_Trigger_Channel, -1, chanwithmark, channel_name);
			j= 0;
		}	
		else
		{
			//Erase space before the name.
			if(j == 0 && input_channels[i] == ' ')
				continue;
			
			channel_name[j++] = input_channels[i];
		}
	}
	
	GetNumListItems(Pulse_Prog_Config, PPConfig_Trigger_Channel, &nl);
	if(t_index < nl && t_index >= 0)
		SetCtrlIndex(Pulse_Prog_Config, PPConfig_Trigger_Channel, t_index);
		
	
	//Now the counter channel - this will be used as a clock for our acquisition
	GetNumListItems(Pulse_Prog_Config, PPConfig_CounterChan, &nl);
	if(nl > 0)
	{
		GetCtrlIndex(Pulse_Prog_Config, PPConfig_CounterChan, &c_index);
		DeleteListItem(Pulse_Prog_Config, PPConfig_CounterChan, 0, -1);
	}
	
	//Free up old variables again
	free(input_channels);
	free(channel_name);
	
	buffersize = DAQmxGetDeviceAttribute(device, DAQmx_Dev_CO_PhysicalChans, "", 0);
	
	input_channels = malloc(buffersize);
	channel_name = malloc(buffersize);

	DAQmxGetDeviceAttribute(device, DAQmx_Dev_CO_PhysicalChans, input_channels, buffersize);
	
	for(i = 0; i<= strlen(input_channels); i++)
	{
		if(input_channels[i] == ',' || input_channels[i] == '\0' || i == strlen(input_channels))
		{
			//Same procedure as above.
			channel_name[j] = '\0';
			chanwithmark = &channel_name[strlen(device)+1];
			
			InsertListItem(Pulse_Prog_Config, PPConfig_CounterChan, -1, chanwithmark, channel_name);
			j = 0;
		}
		else
		{
			if(j == 0 && input_channels[i] == ' ')
				continue;
			
			channel_name[j++] = input_channels[i];
		}
	}
	
	GetNumListItems(Pulse_Prog_Config, PPConfig_CounterChan, &nl);
	if(c_index < nl && c_index >=0)
		SetCtrlIndex(Pulse_Prog_Config, PPConfig_CounterChan, c_index);
	
	//Update the channel gain control.
	update_channel_gain();
	
	//Make sure the trigger attributes are set up correctly.
	ReplaceListItem(Pulse_Prog_Config, PPConfig_TriggerEdge, 0, "Rising", DAQmx_Val_Rising);
	ReplaceListItem(Pulse_Prog_Config, PPConfig_TriggerEdge, 1, "Falling", DAQmx_Val_Falling);
	
	
	return 0;
}

int load_DAQ_info()
{
	CmtGetLock(lock_DAQ);
	int rv = load_DAQ_info_locked();
	CmtReleaseLock(lock_DAQ);
	
	return rv;
}


int setup_DAQ_task_locked (TaskHandle *acquiretask, TaskHandle *countertask)
{
	/*
	This function sets up the DAQ task using the parameters from the UI. Two channels
	are created - a counter channel and an analog input channel. The counter channel 
	generates a finite sequence of pulses upon triggering, which is used as the clock 
	for the analog input channel.
	*/
	
	double samplerate;
	int np, nc, i, ic;
	double max, min;
	char *input_channel = malloc(100), *trigger_channel = malloc(100), *counter_chan = malloc(100), *tname = malloc(30);
	int32 *err_codes = malloc(sizeof(int32)*30);
	
	DAQmxCreateTask("AcquireSignal", acquiretask);
	DAQmxCreateTask("Counter", countertask);
	

	GetCtrlVal(Pulse_Prog_Config, PPConfig_SampleRate, &samplerate);
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NPoints, &np);
	update_channel_gain();
	GetNumListItems(Pulse_Prog_Config, PPConfig_ChannelGain, &nc);
	
	nPoints = np;
	
	//Create analogue input voltage channels.
	for(i = 0; i < nc; i++)
	{
		GetValueFromIndex(Pulse_Prog_Config, PPConfig_ChannelGain, i, &ic);
		GetValueFromIndex(Pulse_Prog_Config, PPConfig_AcquisitionChannel, ic, input_channel);
		GetValueFromIndex(Pulse_Prog_Config, PPConfig_MaxVals, ic, &max);
		GetValueFromIndex(Pulse_Prog_Config, PPConfig_MinVals, ic, &min);
		
		//Name the channel
		sprintf(tname, "VoltInput%02d", i+1);
		
		//Create the channel
		DAQmxCreateAIVoltageChan(*acquiretask, input_channel, tname, DAQmx_Val_Cfg_Default, min, max, DAQmx_Val_Volts, NULL);

	}
	
	//Create the counter channel
	GetCtrlIndex(Pulse_Prog_Config, PPConfig_CounterChan, &i);
	GetLabelFromIndex(Pulse_Prog_Config, PPConfig_CounterChan, i, counter_chan);
	
	//Figure out what the output channel is - I can't find a way to programmatically detect this.
	if(strstr(counter_chan, "ctr0") != NULL)
		sprintf(input_channel, "Ctr0InternalOutput");
	else if (strstr(counter_chan, "ctr1") != NULL)
		sprintf(input_channel, "Ctr1InternalOutput");
	else
	{
		DAQmxClearTask(*acquiretask);
		DAQmxClearTask(*countertask);

		MessagePopup("Counter Error", "Unsupported counter channel - choose ctr0 or ctr1.\n If these options do not exist, you are fucked. Sorry, mate.\n");
		return -1;
	}
	
	GetCtrlVal(Pulse_Prog_Config, PPConfig_CounterChan, counter_chan);
	
	//Create the output. When this task is started, it will generate a train of pulses at frequency samplerate upon triggering, terminating after np pulses.
	DAQmxCreateCOPulseChanFreq(*countertask, counter_chan, "Counter", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, samplerate, 0.5);
	DAQmxCfgImplicitTiming(*countertask, DAQmx_Val_FiniteSamps, np);
	
	//Set the trigger for the counter and make it retriggerable.
	int edge;
 
	GetCtrlVal(Pulse_Prog_Config, PPConfig_Trigger_Channel, trigger_channel);
	GetCtrlVal(Pulse_Prog_Config, PPConfig_TriggerEdge, &edge);
	
	DAQmxCfgDigEdgeStartTrig(*countertask, trigger_channel, edge);
	
	DAQmxSetTrigAttribute(*countertask, DAQmx_StartTrig_Retriggerable, TRUE);
	
	//Generate the name of the counter output and set it as the clock for my analog input task
	GetCtrlVal(Pulse_Prog_Config, PPConfig_Device, tname);
	sprintf(counter_chan, "/%s/%s", tname, input_channel);

	DAQmxCfgSampClkTiming(*acquiretask, counter_chan, samplerate, DAQmx_Val_Rising, DAQmx_Val_ContSamps, np);

	//Set the buffer for my input. Currently there is no programmatic protection against buffer overruns.
	DAQmxSetBufferAttribute(*acquiretask, DAQmx_Buf_Input_BufSize, np*nc+1000);

	return 0; 

}

int setup_DAQ_task (TaskHandle *acquiretask, TaskHandle *countertask)
{
	CmtGetLock(lock_DAQ);
	int rv = setup_DAQ_task_locked(acquiretask, countertask);
	CmtReleaseLock(lock_DAQ);
	
	return rv;
}

//Auxiliary Acquisition Functions
int change_np_or_sr(int np_sr_at)
{
	//Tell this function which to change (number of points, sample rate or acquisition time)
	//and it will update the controls based on that.
	//0 = AT
	//1 = SR
	//2 = NP

	if(np_sr_at != 0 && np_sr_at != 1 && np_sr_at != 2)
		return -1;
	
	double samplerate, acquisitiontime;
	int np;
	
	GetCtrlVal(Pulse_Prog_Config, PPConfig_SampleRate, &samplerate);      
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NPoints, &np);
	GetCtrlVal(Pulse_Prog_Config, PPConfig_AcquisitionTime, &acquisitiontime);
	
	//Acquisition time is in milliseconds, so convert it to seconds first.
	acquisitiontime /= 1000.0;
	
	switch(np_sr_at)
	{
		case 1:
			//Determine sample rate and round to the nearest millihertz.
			samplerate = np/acquisitiontime;
			samplerate = (double)((int)(samplerate*1000.0 + 0.5));
			samplerate /= 1000.0;
			
			//Recalculate the acquisition time.
			acquisitiontime = 1000.0*(double)np/samplerate;
			
			//Set the controls
			SetCtrlVal(Pulse_Prog_Config, PPConfig_AcquisitionTime, acquisitiontime);
			SetCtrlVal(Pulse_Prog_Config, PPConfig_SampleRate, samplerate);
			break;
		
		case 2:
			//Determine the number of points and update the panel.
			
			//Round the number of points to the nearest integer.
			np = (int)(acquisitiontime*samplerate + 0.5);
			
			SetCtrlVal(Pulse_Prog_Config, PPConfig_NPoints, np);
			break;
	}
				  
	//Determine acquisition time in milliseconds, then update the panel -> this happens in any case due to rounding, so it is out of the conditional.
	acquisitiontime = 1000.0*(double)np/samplerate;
	
	SetCtrlVal(Pulse_Prog_Config, PPConfig_AcquisitionTime, acquisitiontime);

	return 0;
}


//////////////////////////////////////////////////////
//                                                   //
//             Pulse Program Functions               //
//                                                   //
///////////////////////////////////////////////////////

double pulse_program (PPROGRAM *p)
{
	/*
	//Programs the board with the entered program 
	int i, j, k, t=p->transient, nt=p->ntransients, flags, trigger_scan, instr_data, stop = 0, delay = 0, f[24];
	double time, time_units, Short = 100*ns, Long = 21464.835*ms;
	int *TTLctrls = malloc(24*sizeof(int));

	if (p->n_inst < 1)
		return -1;

	
	//If it's multi-dimensional, update the program accordingly.
	if(p->nDimensions > 1)
	{
		for(i = 0; i<p->nVaried; i++) {
			if(!p->v_instr_type[i]) {
				p->instruction_time[p->v_instr_num[i]] = p->init[i]*p->initunits[i]+p->inc[i]*p->incunits[i]*(p->step[p->dimension[i]-1]-1); // Increment if it's the time data you want to increment.
			} else {
				p->instr_data[p->v_instr_num[i]] = p->init_id[i] + p->inc_id[i]*(p->step[p->dimension[i]-1]-1); // Increment if it's the instruction data you want to increment.
			}
		}
	}
	
	int phasecycle = p->phasecycle;
	
	//Phase cycling - this divides a single period of evolution at the phase cycling
	//frequency into the relevant number of cycles and adds it to the specified step.
	if(phasecycle)
	{
		double tmod, cycletime;

		tmod = (t-1)%p->numcycles;

		if(tmod != 0)
		{
			cycletime = tmod*1000*ms/(p->cyclefreq*p->numcycles);
			PINSTR *instr = malloc(sizeof(PINSTR));
			to_PINSTR(instr, p->flags[p->phasecycleinstr], CONTINUE, 0, cycletime, ms, 0);    // Creates an instruction variable appropriately
			insert_instruction(p, instr, p->phasecycleinstr+1);   // Insert a new instruction
			free(instr);
			
		} else {
			phasecycle = 0; // Phase cycling instruction is not added in the case when the instruction length would be 0.
		}
		
		
	}

	// If there's a delay and you aren't on the last step, add in the delay.
	// I personally never use this.
	if ((t != nt || cont_mode) && p->delaytime > 0.0)
	{
		for(i = 0; i<p->n_inst; i++)
		{
			if(p->instr[i] == STOP)
				break;
		}
		
		if(i == p->n_inst)
			i--;
		
		double delaytime = p->delaytime*ms;
		PINSTR *instr = malloc(sizeof(PINSTR));
		
		if(delaytime < Short)
			delaytime = Short;

		if(delaytime > Long)
		{
			j = ((int)p->delaytime/(Long-100*ms))+1;
			delaytime /= j;
			to_PINSTR(instr, 0, LONG_DELAY, j, delaytime, ms, 0);
		} else
			to_PINSTR(instr, 0, CONTINUE, 0, delaytime, ms, 0);
		
		insert_instruction(p, instr, i);
		
	}
	
	
	// This is where we start the main body of the program.
	int fid[p->n_inst];
	char *fiderr[p->n_inst];
	for(i = 0; i<p->n_inst; i++)
		fiderr[i] = malloc(100);
	
	pb_start_programming_safe(1, PULSE_PROGRAM);
	
	
	for(i = 0; i < p->n_inst; i++)
	{
		instr_data = p->instr_data[i];
		// We want to take the initial instruction and make it work according to the dimension.
		// I'd also like to be able to change the number of instructions here. For now, I'm just going to
		// allow zero second evolution times, which will delete that step from the program entirely.
	
		// TODO: Implement the ability to step things like loops, and phase cycle through TTLs (+-x, +-y, etc).
		// -> Update: The first part of this has been done, and I think that I know how to do the second part without anything new:
		// -> this is done by using JSR/RTS, where you put some subroutines after the main body of the program. I think that the next thing to do
		// -> in this regard is to make it so that you can feed it a list of values, rather than stepping linearly through them.
		if(p->instruction_time[i] < 100.0*ns || (p->instr[i] == LONG_DELAY && p->instr_data[i] == 0))  {
			continue;
		}
		
		// If a loop has instr_data == 0, skip it.
		if(p->instr[i] == LOOP && p->instr_data[i] == 0) {
			int skip = -1;
			
			// Find the end of the loop we're in.
			for (j = i+1; j<ninstructions; j++) {
				if(p->instr[j] == END_LOOP && p->instr_data[j] == i) {
					skip = j;
					break;
				}
			}
			
			// If there's no end, this is a malformed program, kill it.
			if(skip < 0) {
				MessagePopup("Programming Error", "LOOP Instruction is not terminated by an appropriate END_LOOP instruction.");
				return -1;
			}
			
			// If we've found skip, set the instruction to the END_LOOP, don't add it, and continue moving on.
			i = skip;
			continue;
		}

	
		if(p->instr[i] == END_LOOP || p->instr[i] == JSR || p->instr[i] == BRANCH)
		{
			if(phasecycle && instr_data >= p->phasecycleinstr)
				instr_data++; // This is done because an additional instruction is added for phase cycling.
			
			// This is just a check to see if you've moved the instructions around.
			// TODO: Make END_LOOP statements respond to moving an instruction  around appropriately.
			if(p->instr[i] == END_LOOP && p->instr[instr_data] != LOOP)
			{
				MessagePopup("Programming Error", "END_LOOP Instruction does not specify beginning of loop.\n");
				return -1;
			}
			
			
			if(instr_data >= i)
				instr_data += fid[0];
			else
				instr_data = fid[instr_data];
			
			p->instr_data[i] = instr_data;
		}
		
		// This is where you actually insert the instruction
		fid[i] = pb_inst_pbonly_safe(0, p->flags[i], p->instr[i], p->instr_data[i], p->instruction_time[i]);
		
		// Uncomment this part if you want to read out what the actual instructions are.
		/*char *s = malloc(100);
		if (p->instr[i] == END_LOOP)
			sprintf(s, "END_LOOP");
		else if (p->instr[i] == LOOP)
			sprintf(s, "LOOP");
		else if (p->instr[i] == WAIT)
			sprintf(s, "WAIT");
		else if (p->instr[i] == STOP)
			sprintf(s, "STOP");
		else if (p->instr[i] == CONTINUE)
			sprintf(s, "CONTINUE");
		else
			sprintf(s, "OTHER");
		
		
		printf("Instr #: %d - Instr: %s, Instr_data: %d, Time: %f\n", fid[i], s, p->instr_data[i], p->instruction_time[i]); 
		*/
		 /*
		if(fid[i] < 0)
			sprintf(fiderr[i], pb_get_error());
	}

	// You have now finished programming the board.
	pb_stop_programming_safe(1);
	
	// The fid numbers will be negative if there was an error during programming, read them out here.
	for (i = 0; i<(p->n_inst); i++)
	{
		if(fid[i] < 0)
		{
			sprintf(errorstring, "%d . Error number %s\n", i, fiderr[i]);
			MessagePopup("Programming Error", errorstring);
		}
	}
	
	// This is the total experiment time.
	// TODO: Make this actually useful, especially in 2D experiments.
	return p->total_time/ms;
	*/
	
	return 0.0;
	
}


		



int fix_number_of_dimensions ()
{
	//If the number of dimensions is set in the UI as a number larger than the number of dimensions
	//actually used in the program, this updates the UI appropriately.
	
	int nd;
	
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NDimensionalOn, &nd);
	if(!nd)
		return 0;
	
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NumDimensions, &nd);
	
	int instr_per_dim[--nd], i, j=0, k=0, dim;
	for(i = 0; i<nd; i++)
		instr_per_dim[i] = 0;
	
	j = 0;
	
	double inc = 0.0;
	int inc_id = 0;
	for(i = 0; i<n_inst_disp; i++)
	{
		/*
		// This checks if the step is off.
		dim = get_nd_state(cinst[i], MDInstr_VaryInstr);
		if(dim == 0)
			continue;
		else if (dim == 1) {
			GetCtrlVal(cinst[i], MDInstr_Increment, &inc);
		} else if (dim == 2) {
			GetCtrlVal(cinst[i], MDInstr_IncInstrData, &inc_id);
		}
		
		// If the step is on, but it shouldn't be, turn it off.
		if(dim == 1 || dim == 2) {
			if(inc == 0 && inc_id == 0) {
				update_nd_state(cinst[i], MDInstr_VaryInstr, 0);
				continue;
			}
		}

		GetCtrlVal(cinst[i], MDInstr_Dimension, &dim);
		if(instr_per_dim[dim-1]++ == 0) // I think I do this check because sometimes dim is the same thing, and I don't want double-counting.
			j++;
		*/
	}
		

	if(j == nd+1)
		return 0;
	
	if(j == 0)
	{	
		SetCtrlVal(Pulse_Prog_Config, PPConfig_NDimensionalOn, 0);
		SetCtrlVal(Pulse_Prog_Config, PPConfig_NumDimensions, 2);
		NumDimensionCallback(Pulse_Prog_Config, PPConfig_NumDimensions, EVENT_COMMIT, NULL, NULL, NULL);
		ToggleND(Pulse_Prog_Config, PPConfig_NDimensionalOn, EVENT_COMMIT, NULL, NULL, NULL);
		return 0;
	}
	
	SetCtrlVal(Pulse_Prog_Config, PPConfig_NumDimensions, j+1);
	
	for(i = 0; i<nd; i++)
	{
		if(instr_per_dim[i] != 0)
			continue;
		
		for(j = i; j<nd; j++)
			if(instr_per_dim[j] != 0)
				break;
		
		if(j == nd)
			continue;
		
		for(k = 0; k<n_inst_disp; k++)
		{
			GetCtrlVal(cinst[k], MDInstr_VaryInstr, &dim);
			if(!dim)
				continue;
			
			GetCtrlVal(cinst[k], MDInstr_Dimension, &dim);
			if(dim == j+1)
				SetCtrlVal(cinst[k], MDInstr_Dimension, i+1);
		}
	}
	
	NumDimensionCallback(Pulse_Prog_Config, PPConfig_NumDimensions, EVENT_COMMIT, NULL, NULL, NULL); 
	
	return 0;
}

int get_num_varying_instr ()
{
	int i, on, total = 0;
	for(i = 0; i<n_inst_disp; i++)
	{
		GetCtrlVal(cinst[i], MDInstr_VaryInstr, &on);
		if(on)
			total++;
	}
	
	return total;
}


//Auxiliary Pulse Programming Functions


int CVICALLBACK CalculateAverage (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:


			
			/*int i, j = 1, nl = 0, val, hid;

			GetCtrlAttribute(FID, FID_Graph, ATTR_NUM_PLOTS, &nl);
			GetCtrlAttribute(FID, FID_Graph, ATTR_FIRST_PLOT, &val);
			for(i = 0; i<nl; i++)
			{
				GetPlotAttribute(FID, FID_Graph, val, ATTR_TRACE_VISIBLE, &hid);
				printf("%d. Plot %d: %d\n", i, val, hid);
				GetPlotAttribute(FID, FID_Graph, val, ATTR_NEXT_PLOT, &val);
			}*/
				
			
			/*for(j = 0; j<2; j++)
			{
				for(i = 2; i<= 4; i++)
				{
					GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(i, j+1), &val);
					if(val >= 0)
					{
						GetPlotAttribute(FFTSpectrum, Spectrum_Graph, val, ATTR_TRACE_VISIBLE, &hid);
						printf("Channel %d, Plot %d: %d\n", j+1, i, hid);
					} else
						printf("Channel %d, Ploy %d: %d\n", j+1, i, val);
				}
			} */
			
			/*int i, plot, numpoints, data_type;
			double average =0.0;
			
			GetCtrlAttribute(panel, FID_Graph, ATTR_FIRST_PLOT, &plot);
			
			if(plot < 1)
				break;
			
			if(GetPlotAttribute(panel, FID_Graph, plot, ATTR_NUM_POINTS, &numpoints) != 0)
				break;
			
			GetPlotAttribute(panel, FID_Graph, plot, ATTR_PLOT_YDATA_TYPE, &data_type);
			
			if(data_type == VAL_DOUBLE)
			{
				double *data = malloc(sizeof(double)*numpoints);
				GetPlotAttribute(panel, FID_Graph, plot, ATTR_PLOT_YDATA, data);
				for(i = 0; i<numpoints; i++)
					average += data[i];
			}
			else
			{
				SetCtrlVal(panel, FID_Average, -1*(double)data_type);
				break;
			}
			

			average /= numpoints;
			SetCtrlVal(panel, FID_Average, average);
			*/
			break;
	}
	return 0;
}



///////////////////////////////////////////////////////
//                                                   //
//        		Plotting Functions                   //
//                                                   //
///////////////////////////////////////////////////////
int clear_plots()
{
	/*int num, i, j;
	GetNumListItems(FID, FID_TransientNum, &num);
	if(num > 0)
		DeleteListItem(FID, FID_TransientNum, 0, -1);
	
	SetCtrlAttribute(FID, FID_TransientNum, ATTR_DIMMED, 1);
	
	GetNumListItems(FFTSpectrum, Spectrum_TransientNum, &num);
	if(num > 0)
		DeleteListItem(FFTSpectrum, Spectrum_TransientNum, 0, -1);
	
	SetCtrlAttribute(FFTSpectrum, Spectrum_TransientNum, ATTR_DIMMED, 1);
	SetCtrlIndex(FFTSpectrum, Spectrum_Channel, 0);
	SetCtrlAttribute(FFTSpectrum, Spectrum_Channel, ATTR_DIMMED, 1);
	

	DeleteGraphPlot(FID, FID_Graph, -1, VAL_IMMEDIATE_DRAW);
	DeleteGraphPlot(FFTSpectrum, Spectrum_Graph, -1, VAL_IMMEDIATE_DRAW);
	DeleteGraphAnnotation(FFTSpectrum, Spectrum_Graph, -1);
	
	SetCtrlAttribute(FID, FID_Dimension, ATTR_VISIBLE, 0);
	SetCtrlAttribute(FFTSpectrum, Spectrum_Dimension, ATTR_VISIBLE, 0);
	SetCtrlAttribute(FID, FID_DimPoint, ATTR_VISIBLE, 0);
	SetCtrlAttribute(FFTSpectrum, Spectrum_DimPoint, ATTR_VISIBLE, 0);
	
	for(i = 1; i<=8; i++)
	{
		for(j = 1; j <=4; j++)
			SetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(j, i), -1);
	} */
			

	return 0;
}

int get_num_points(char *filename)
{
	if(!file_exists(filename))
		return -1;
	
	FILE *fileid = fopen(filename, "r");
	
	if(fileid == NULL)
		return -2;
	
	char *string_buffer = malloc(500);
	char *string_match = malloc(500);
	int np;
	
	while(!feof(fileid))
	{
		fgets(string_buffer, 500, fileid);
		if(sscanf(string_buffer, "%s %d", string_match, &np) == 2 && strstr(string_match, "NPoints="))
			break;
	}
	
	if(feof(fileid) || np < 1)
	{
		fclose(fileid);
		return -3;
	}
	
	fclose(fileid);
	return np;
}

double get_sampling_rate(char *filename)
{
	if(!file_exists(filename))
		return -1;
	
	FILE *fileid = fopen(filename, "r");
	
	if(fileid == NULL)
		return -2;
	
	char *string_buffer = malloc(500);
	char *string_match = malloc(500);
	double sr;
	
	while(!feof(fileid))
	{
		fgets(string_buffer, 500, fileid);
		if(sscanf(string_buffer, "%s %lf", string_match, &sr) == 2 && strstr(string_match, "SamplingRate="))
			break;
	}
	
	if(feof(fileid) || sr < 1)
	{
		fclose(fileid);
		return -3;
	}
	
	fclose(fileid);
	return (double)sr;
}

int get_num_transients_completed(char *filename)
{
	if(!file_exists(filename))
		return -1;
	
	FILE *fileid = fopen(filename, "r");
	
	if(fileid == NULL)
		return -2;
	
	char *string_buffer = malloc(500);
	char *string_match = malloc(500);
	int nt;
	
	while(!feof(fileid))
	{
		fgets(string_buffer, 500, fileid);
		if(sscanf(string_buffer, "%s %d %*s %*d", string_match, &nt) == 2 && strstr(string_match, "TransientsCompleted="))
			break;
	}
	
	if(feof(fileid) || nt < 1)
	{
		fclose(fileid);
		return -3;
	}
	
	fclose(fileid);
	return nt;
}

int get_folder_name (char *pathname, char *pathout)
{
	int l = strlen(pathname);
	int i = l-1, j = 0;
	char *buff = malloc(l);
	while(i > 0)
	{
		if(pathname[i] == '\\')
			break;
		buff[j++] = pathname[i--];
	}
	if(i == 0)
		return -2;
	
	buff[j] = '\0';
	
	for(i = strlen(buff)-1; i>=0; i--)
		pathout[strlen(buff)-i-1] = buff[i];
	
	pathout[strlen(buff)] = '\0';
	
	return 0;
}

int get_num_dimensions (char *filename)
{
	if(!file_exists(filename))
		return -1;
	
	FILE *fileid = fopen(filename, "r");
	
	if(fileid == NULL)
		return -2;
	
	char *string_buffer = malloc(500), *string_match = malloc(500);
	int nd;
	
	while(!feof(fileid))
	{
		fgets(string_buffer, 500, fileid);
		if(sscanf(string_buffer, "%s %d", string_match, &nd) == 2 && strstr(string_match, "nDimensions="))
			break;
	}
	
	if(feof(fileid) || nd < 1)
	{
		fclose(fileid);
		return -3;
	}
	
	fclose(fileid);
	return nd-1;
}

int get_num_channels (char *filename)
{
	if(!file_exists(filename))
		return -1;
	
	FILE *fileid = fopen(filename, "r");
	
	if(fileid == NULL)
		return -2;
	
	char *string_buffer = malloc(500), *string_match = malloc(500);
	int nc;
	
	while(!feof(fileid))
	{
		fgets(string_buffer, 500, fileid);
		if(sscanf(string_buffer, "%s %d", string_match, &nc) == 2 && strstr(string_match, "Channels:") != NULL)
			break;
		
		if(sscanf(string_buffer, "%s %*d of %d", string_match, &nc) == 2 && strstr(string_match, "Channel:") !=  NULL)
			break;
	}
	
	if(feof(fileid))
	{
		fclose(fileid);
		return 1;
	}
	else if (nc < 1)
	{
		fclose(fileid);
		return -1;
	}
	
	fclose(fileid);
	return nc;
}

int get_points_completed (char *fname, int nd, int *points)
{
	FILE *fileid = fopen(fname, "r");
	if(fileid == NULL)
		return -1;
	
	int i, dim, point;
	char *string_buff = malloc(500), *string_match = malloc(500);

	i = 0;

	while(!feof(fileid))
	{
		fgets(string_buff, 500, fileid);
		if(sscanf(string_buff, "%s %d %*s %d %*s %*d", string_match, &dim, &point) == 3 && strstr(string_match, "IndirectDim") != NULL)
		{
			points[dim-1] = point;
			i++;
		}
		if(i == nd)
			break;
	}
	
	if(feof(fileid))
	{
		fclose(fileid);
		return -2;
	}
	
	fclose(fileid);
	return 0;
}
	

int get_fid_or_fft_fname(char *fname, int t, int c, int fid_or_fft)
{
	//Give this file a blank array of size MAX_PATHNAME_LEN and it will return the filename for the t-th transient.
	//On 0 it returns the FID filename, on 1 it returns FFT filename, on 2 it returns the base + .txt.
	/*char *filename = malloc(MAX_PATHNAME_LEN);
	char *buffer_string = malloc(MAX_PATHNAME_LEN);
	int nl, nd, nc;
	if(c < 0)
		c = 0;
	
	GetCtrlVal(FID, FID_CurrentPathname, filename);
    GetNumListItems(FID, FID_CurrentDimPoint, &nl);
	
	if(strlen(filename) == 0)
		return -3;
	
	if(filename[strlen(filename)-1] == '\\')
		filename[strlen(filename)-1] = '\0';

	//Give it the full filename, get just the folder name (fname)
	get_folder_name(filename, fname);

	//Use the folder name to get at the header file, which is located in pathname\foldername.txt, so if the pathname is C:\Documents\Experiments\Experiment0001, the header will be C:\Documents\Experiemnts\Experiment0001\Experiment0001.txt
	sprintf(buffer_string, "%s\\%s.txt", filename, fname);
	
	//The filename will depend on number of channels and number of dimensions:
	nd = get_num_dimensions(buffer_string);
	nc = get_num_channels(buffer_string);
	
	if(nd > 0 && nd != nl)
		return -1;
	else if(nc < 1 || nc > 8)
		return -1;

	char *fidname = malloc(50), *fftname = malloc(50);
	char *channame = malloc(10);
	sprintf(fidname, "FIDTransient%04d.txt", t);
	sprintf(fftname, "FFTTransient%04d.txt", t);
	
	if(nc == 1)
		strcpy(channame, "\\");
	else
		sprintf(channame, "\\C%d\\", c);
	
	if(t == 0)
		fid_or_fft = 2;
	
	if(nd == 0)
	{
		strcat(filename, channame);
		strcat(filename, fname);

		if(fid_or_fft == 0)
			sprintf(fname, "%s-%s", filename, fidname);
		else if(fid_or_fft == 1)
			sprintf(fname, "%s-%s", filename, fftname);
		else if(fid_or_fft == 2)
			sprintf(fname, "%s.txt", filename);
		else
			return -1;
	} else {
		int i, point[nd];
		for(i = 0; i<nd; i++)
			GetValueFromIndex(FID, FID_CurrentDimPoint, i, &point[i]);
		
		strcpy(fname, filename);
		strcat(fname, channame);
		for(i = 0; i<nd; i++)
		{
			sprintf(buffer_string, "D%02d-%04d\\", i+1, point[i]);
			strcat(fname, buffer_string);
		}
		
		if(fid_or_fft == 0)
			strcat(fname, fidname);
		else if(fid_or_fft == 1)
			strcat(fname, fftname);
		else if(fid_or_fft == 2)
			strcat(fname, "AverageFID.txt");
	}
	
	if(!file_exists(fname))
		return -2;
	*/
	return 0;
}

int channel_control (int channel, int fid_or_fft, int *out)
{
	//Feed this the channel you want and whether you are asking about the FID or the FFT and it returns whether or not the control is on
	//*out gives the channel control.
	//0 = FID, 1 = FFT
	int chans[8];
	int panel;
	int on;
	if(!fid_or_fft)
	{
		panel = FID;    
		chans[0] = FID_Chan1;
		chans[1] = FID_Chan2;
		chans[2] = FID_Chan3;
		chans[3] = FID_Chan4;
		chans[4] = FID_Chan5;
		chans[5] = FID_Chan6;
		chans[6] = FID_Chan7;
		chans[7] = FID_Chan8;
	} else {
		panel = FFTSpectrum;  
		chans[0] = Spectrum_Chan1;
		chans[1] = Spectrum_Chan2;
		chans[2] = Spectrum_Chan3;
		chans[3] = Spectrum_Chan4;
		chans[4] = Spectrum_Chan5;
		chans[5] = Spectrum_Chan6;
		chans[6] = Spectrum_Chan7;
		chans[7] = Spectrum_Chan8;
	}

	GetCtrlVal(panel, chans[channel], &on);

	if(out != NULL)
		out[0] = chans[channel];
	
	return on;
}

int channel_on (int channel, int fid_or_fft)
{
	//Feed this the channel (0-based index) you want and whether you are asking about the FID or the FFT and it returns 1 if the channel is on, 0 if it is off.
	//0 = FID, 1 = FFT
	
	int on = channel_control(channel, fid_or_fft, NULL);
	
	/*int panel, control, nl, i, on;
	
	if(!fid_or_fft)
	{	
		panel = FID;
		control = FID_ChanPrefs;
	}
	else
	{
		panel = FFTSpectrum;
		control = Spectrum_ChanPrefs;
	}

	GetNumListItems(panel, control, &nl);
	for(i = 0; i<nl; i++)
	{
		GetValueFromIndex(panel, control, i, &on);
		if(on == channel)
		{
			on = 1;
			break;
		}
		else
			on = 0;
	}*/
	
	return on;
}

int plot_fid_data(double *data, int chan, int np, int t, double sr)
{
	//This function will plot your data on the relevant channel.
	if(t < 0 || np < 1 || chan >= 8 || chan < 0)
		return -3;
	
	//See if the channel is even supposed to be displayed
	if(!channel_on(chan, 0))
		return -3;
	
	int as, i, err, plot, nl;
	double gain, offset;
	
	//Determine whether or not to autoscale
	GetCtrlVal(FID, FID_Autoscale, &as);
	
	//If autoscaling should be on, turn it on.
	if(as)
	{
		SetAxisScalingMode(FID, FID_Graph, VAL_BOTTOM_XAXIS, VAL_AUTOSCALE, NULL, NULL);
		SetAxisScalingMode(FID, FID_Graph, VAL_LEFT_YAXIS, VAL_AUTOSCALE, NULL, NULL);
	}
	
	//Determine gains and offsets.
	GetTableCellVal(HiddenPanel, HiddenVals_ChannelGains, MakePoint(1, chan+1), &gain);
	GetTableCellVal(HiddenPanel, HiddenVals_ChannelOffsets, MakePoint(1, chan+1), &offset);
	
	GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(1, chan+1), &plot);

	if(plot >= 0)
		DeleteGraphPlot(FID, FID_Graph, plot, VAL_IMMEDIATE_DRAW);
	
	//Apply the gain and offset programmatically
	for(i = 0; i<np; i++)
		data[i] = data[i]*gain + offset;
	
	unsigned long int color;
	GetValueFromIndex(HiddenPanel, HiddenVals_FIDChanColor, chan, &color);
	plot = PlotY(FID, FID_Graph, data, np, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, color);
	
	
	SetCtrlAttribute(FID, FID_Graph, ATTR_XAXIS_GAIN, 1000/sr);
	
	//Update the location of the plot ID.
	SetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(1, chan+1), plot);
	//calculate_RMS(FID, FID_Graph);
	
	return plot;
}

int plot_fft_data(double *realchan, double *imagchan, int chan, int np, double sr, int t)
{
	//This function will plot the FFT.
	if(t < 0 || np < 1 || chan >= 8 || chan < 0)
		return -1;
	
	//See if the channel is even supposed to be displayed
	if(!channel_on(chan, 1))
		return -3;
	
	int nl, as, magplot, realplot, imagplot, i;
	double ph0, ph1, ph2, gain, offset;
	double *realcorr = malloc(sizeof(double)*np), *imagcorr = malloc(sizeof(double)*np);
	double *fftmag = malloc(sizeof(double)*np);
	
	//Determine whether or not to autoscale
	GetCtrlVal(FFTSpectrum, Spectrum_Autoscale, &as);
	
	//If autoscaling should be on, turn it on.
	if(as)
	{
		SetAxisScalingMode(FFTSpectrum, Spectrum_Graph, VAL_BOTTOM_XAXIS, VAL_AUTOSCALE, NULL, NULL);
		SetAxisScalingMode(FFTSpectrum, Spectrum_Graph, VAL_LEFT_YAXIS, VAL_AUTOSCALE, NULL, NULL);
	}
	
	//Get the gain and offset values
	GetTableCellVal(HiddenPanel, HiddenVals_ChannelGains, MakePoint(2, chan+1), &gain);
	GetTableCellVal(HiddenPanel, HiddenVals_ChannelOffsets, MakePoint(2, chan+1), &offset);
	
	
	//Determine the phase correction values.
	GetTableCellVal(HiddenPanel, HiddenVals_PhaseCorrectionValues, MakePoint(1, chan+1), &ph0);
	GetTableCellVal(HiddenPanel, HiddenVals_PhaseCorrectionValues, MakePoint(2, chan+1), &ph1);
	GetTableCellVal(HiddenPanel, HiddenVals_PhaseCorrectionValues, MakePoint(3, chan+1), &ph2);
	
	//Convert to radians.
	ph0 /= 360/(2*Pi());
	ph1 /= 360/(2*Pi());
	ph2 /= 360/(2*Pi());
	double om, om2, ph;
	
	//Apply phase correction e^(iph0 + iph1 + i^2ph2), and the gain and offset.
	for(i = 0; i < np; i++)
	{
		om = (2*i/np -1)*10;
		om2 = (2*(i*i)/(np*np) - 1)*5;
		ph = ph0 + ph1*om + ph2*om2;
		
		realcorr[i] = realchan[i]*cos(ph) - imagchan[i]*sin(ph);
		imagcorr[i] = imagchan[i]*cos(ph) + realchan[i]*sin(ph);
		fftmag[i] = sqrt(realchan[i]*realchan[i] + imagchan[i]*imagchan[i]);
		
		realchan[i] = gain*realcorr[i] + offset;
		imagchan[i] = gain*imagcorr[i] + offset;
		fftmag[i] = gain*fftmag[i] + offset;
	}
	
	free(realcorr);
	free(imagcorr);
	
	
	//Determine the plot IDs of the real, imaginary and magnitude plots
	GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(2, chan+1), &realplot);
	GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(3, chan+1), &imagplot);
	GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(4, chan+1), &magplot);
	
	
	//Delete the plots if there are any for this channel.
	if(realplot >= 0)
		DeleteGraphPlot(FFTSpectrum, Spectrum_Graph, realplot, VAL_IMMEDIATE_DRAW);
	if(imagplot >= 0)
		DeleteGraphPlot(FFTSpectrum, Spectrum_Graph, imagplot, VAL_IMMEDIATE_DRAW);
	if(magplot >= 0)
		DeleteGraphPlot(FFTSpectrum, Spectrum_Graph, magplot, VAL_IMMEDIATE_DRAW);
	
	DeleteGraphAnnotation(FFTSpectrum, Spectrum_Graph, -1);
	
	//Plot each of the spectra
	unsigned long int color;
	GetValueFromIndex(HiddenPanel, HiddenVals_SpectrumChanColor, chan, &color);

	realplot = PlotY(FFTSpectrum, Spectrum_Graph, realchan, np, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, color);
	imagplot = PlotY(FFTSpectrum, Spectrum_Graph, imagchan, np, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, color);
	magplot = PlotY(FFTSpectrum, Spectrum_Graph, fftmag, np, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, color);
	
	
	//If there was a problem with any of the plots, erase them all.
	if(realplot < 0 || imagplot < 0 || magplot < 0)
	{
		if(realplot >= 0)
			DeleteGraphPlot(FFTSpectrum, Spectrum_Graph, realplot, VAL_IMMEDIATE_DRAW);
		if(imagplot >= 0)
			DeleteGraphPlot(FFTSpectrum, Spectrum_Graph, imagplot, VAL_IMMEDIATE_DRAW);
		if(magplot >= 0)
			DeleteGraphPlot(FFTSpectrum, Spectrum_Graph, magplot, VAL_IMMEDIATE_DRAW);
		
		SetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(2, chan+1), -1);
		SetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(3, chan+1), -1);
		SetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(4, chan+1), -1);
		
		return -2;
	}

	//Set the frequency axis correctly.
	SetCtrlAttribute(FFTSpectrum, Spectrum_Graph, ATTR_XAXIS_GAIN, sr/(2*(double)np));

	int v[3] = {0, 0, 0};
	
	//Determine if they want to see the real, imaginary or magnitude channel.
	GetCtrlIndex(FFTSpectrum, Spectrum_Channel, &i);
	if(i <= 0 || i > 2)
		v[0] = 1;
	else
		v[i] = 1;
	
	//Hide whichever plot they don't want to see.
	SetPlotAttribute(FFTSpectrum, Spectrum_Graph, realplot, ATTR_TRACE_VISIBLE, v[0]);
	SetPlotAttribute(FFTSpectrum, Spectrum_Graph, imagplot, ATTR_TRACE_VISIBLE, v[1]);
	SetPlotAttribute(FFTSpectrum, Spectrum_Graph, magplot, ATTR_TRACE_VISIBLE, v[2]);

	//Update the plot IDs.
	SetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(2, chan+1), realplot);
	SetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(3, chan+1), imagplot);
	SetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(4, chan+1), magplot);

	//If they are currently editing the channel you've just updated, update the ring control with the proper plots.
	int multichan;
	GetCtrlAttribute(FFTSpectrum, Spectrum_ChanPrefs, ATTR_VISIBLE, &multichan);
	GetCtrlVal(FFTSpectrum, Spectrum_ChanPrefs, &i);
	
	if(!multichan || i == chan)
	{
		ReplaceListItem(FFTSpectrum, Spectrum_Channel, 0, "Real Channel", realplot);
		ReplaceListItem(FFTSpectrum, Spectrum_Channel, 1, "Imag Channel", imagplot);
		ReplaceListItem(FFTSpectrum, Spectrum_Channel, 2, "Magnitude", magplot);
	}
	
	calculate_RMS(FFTSpectrum, Spectrum_Graph);
	
	return 0;
}

int exp_is_1d (char *loadfile)
{
	FILE *file;
	char *string_buffer = malloc(50);
	char *string_match = malloc(50);
	file = fopen(loadfile, "r");
	int nd;
	
	while(!feof(file))
	{
		fgets(string_buffer, 50, file);
		if(sscanf(string_buffer, "%s %d", string_match, &nd) == 2 && strstr(string_match, "nDimensions=") != NULL)
			break;
	}
	int i = feof(file);
	fclose(file);
	
	if(i)
		return -1;
	if(nd > 1)
		return 0;
	else if (nd < 1)
		return -2;
	else
		return 1;
}

int load_data_1d(char *loadfile)
{
	/*int i, j = 1;
	int nt = get_num_transients_completed(loadfile);

	clear_plots();

	if(nt > 1)
	{
		SetCtrlAttribute(FID, FID_TransientNum, ATTR_DIMMED, 0);
		SetCtrlAttribute(FFTSpectrum, Spectrum_TransientNum, ATTR_DIMMED, 0);
		j = 0;
	}

	for(i = j; i <=nt; i++)
		insert_transient(i);

	change_fid_and_fft(j);

	SetCtrlAttribute(FFTSpectrum, Spectrum_Channel, ATTR_DIMMED, 0);
	*/
	return 0;
}

int load_data_nd(int nd, char *loadfile)
{
	/*
	if(nd < 1)
		return -1;
	
	clear_plots();
	int *points = malloc(sizeof(int)*nd), nl, i;
	get_points_completed(loadfile, nd, points);
	
	update_dim_point_control(points, nd);
	
	for(i = 0; i<nd; i++)
		points[i] = 1;
	
	update_current_dim_point(points, nd);
	
	SetCtrlIndex(FID, FID_Dimension, 0);
	ChangeDim(FID, FID_Dimension, EVENT_COMMIT, NULL, NULL, NULL);
	
	SetCtrlIndex(FID, FID_DimPoint, 0);
	
	for(i = 1; i<9; i++)
	{
		SetCtrlAttribute(panelHandle, Load_ND_Labels(i), ATTR_VISIBLE, 0);
		SetCtrlAttribute(panelHandle, Load_ND_Val(i), ATTR_VISIBLE, 0);
	}

	for(i = 1; i<nd+1; i++)
	{
		SetCtrlAttribute(panelHandle, Load_ND_Labels(i), ATTR_VISIBLE, 1);
		SetCtrlAttribute(panelHandle, Load_ND_Val(i), ATTR_VISIBLE, 1);
	}
	
	char *fname = malloc(MAX_PATHNAME_LEN);
	int nt, np, nc, j=1;
	double sr;
	
	get_fid_or_fft_fname(fname, 1, -1, 2);
	
	np = get_num_points(fname);
	sr = get_sampling_rate(fname);
	nt = get_num_transients_completed(fname);
	nc = get_num_channels(fname);
	
	if(np < 1 || sr <= 0.0 || nt < 1 || nc < 1)
		return -2;
	
	if(nt > 1)
	{
		SetCtrlAttribute(FID, FID_TransientNum, ATTR_DIMMED, 0);
		SetCtrlAttribute(FFTSpectrum, Spectrum_TransientNum, ATTR_DIMMED, 0);
		j = 0;
	}
	
	for(i = j; i<=nt; i++)
		insert_transient(i);
	
	change_fid_and_fft(j);
	
	SetCtrlAttribute(FFTSpectrum, Spectrum_Channel, ATTR_DIMMED, 0);
	SetCtrlAttribute(FID, FID_DimPoint, ATTR_VISIBLE, 1);
	SetCtrlAttribute(FFTSpectrum, Spectrum_DimPoint, ATTR_VISIBLE, 1);
	SetCtrlAttribute(FID, FID_Dimension, ATTR_VISIBLE, 1);
	SetCtrlAttribute(FFTSpectrum, Spectrum_Dimension, ATTR_VISIBLE, 1);
	*/
	return 0;
}

void CVICALLBACK SelectRecentData (int menuBarHandle, int menuItemID, void *callbackData, int panelHandle)
{
	int sid, err, nl, nm, m;
	
	GetMenuBarAttribute(menuBarHandle, MainMenu_File_Load_LoadRecentData, ATTR_SUBMENU_ID, &sid);
	if(sid == 0)
	{
		err = -1;
		goto Exit;
	}

	GetMenuBarAttribute(menuBarHandle, sid, ATTR_NUM_MENU_ITEMS, &nm);
	GetNumListItems(HiddenPanel, HiddenVals_RecentData, &nl);
	
	if(nl != nm)
	{
		err = -2;
		goto Exit;
	}
	
	GetMenuBarAttribute(menuBarHandle, sid, ATTR_FIRST_ITEM_ID, &m);
	int i = 0;
	while(m != menuItemID && i < nm)
	{
		GetMenuBarAttribute(menuBarHandle, m, ATTR_NEXT_ITEM_ID, &m);
		i++;
	}
	
	if(m == 0)
	{
		err = -3;
		goto Exit;
	}
	
	char *filename = malloc(MAX_PATHNAME_LEN);
	GetValueFromIndex(HiddenPanel, HiddenVals_RecentData, i, filename);
	
	if(!file_exists(filename))
	{
		err = -4;
		goto Exit;
	} else
		err = load_data_file(filename);
	
	if(err < 0)
		err = -4;
	
	Exit:
	switch(err) {
		case -1:
			MessagePopup("Error Loading Data", "No submenu detected. That's odd.");
			break;
		case -2:
		case -3:
			MessagePopup("Error Loading Data", "Doesn't seem that the menu was reloaded. Try again, and if it still doesn't work, load the old fashioned way.");
			break;
		case -4:
			DeleteListItem(HiddenPanel, HiddenVals_RecentData, i, 1); 
			MessagePopup("Error Loading Data", "File not found.");
			break;
	}
	
	if(err < 0)
		generate_recent_experiments_menu();
			
}

int generate_recent_experiments_menu ()
{
	char *label = malloc(MAX_FILENAME_LEN);
	int i, nl;
	
	GetNumListItems(HiddenPanel, HiddenVals_RecentData, &nl);
	
	int rid;
	GetMenuBarAttribute(panelHandleMenu, MainMenu_File_Load_LoadRecentData, ATTR_SUBMENU_ID, &rid);
	
	if(nl > 0)
	{
		if(rid == 0)
			rid = NewSubMenu(panelHandleMenu, MainMenu_File_Load_LoadRecentData);
		else
			EmptyMenu(panelHandleMenu, rid);
		
		SetMenuBarAttribute(panelHandleMenu, MainMenu_File_Load_LoadRecentData, ATTR_DIMMED, 0);   
	} else {
		if(rid != 0)
			DiscardMenu(panelHandleMenu, rid);
		
		SetMenuBarAttribute(panelHandleMenu, MainMenu_File_Load_LoadRecentData, ATTR_DIMMED, 1);
	}
	
	for(i = 0; i<nl; i++)
	{
		GetLabelFromIndex(HiddenPanel, HiddenVals_RecentData, i, label);
		NewMenuItem(panelHandleMenu, rid, label, -1, NULL, SelectRecentData, 0);
	}
	
	return 0;
}

void CVICALLBACK SelectRecentProgram (int menuBarHandle, int menuItemID, void *callbackData, int panelHandle)
{
	int sid, err, nl, nm, m;
	
	GetMenuBarAttribute(menuBarHandle, MainMenu_File_Load_LoadRecentProgram, ATTR_SUBMENU_ID, &sid);
	if(sid == 0)
	{
		err = -1;
		goto Exit;
	}

	GetMenuBarAttribute(menuBarHandle, sid, ATTR_NUM_MENU_ITEMS, &nm);
	GetNumListItems(HiddenPanel, HiddenVals_RecentPrograms, &nl);
	
	if(nl != nm)
	{
		err = -2;
		goto Exit;
	}
	
	GetMenuBarAttribute(menuBarHandle, sid, ATTR_FIRST_ITEM_ID, &m);
	int i = 0;
	while(m != menuItemID && i < nm)
	{
		GetMenuBarAttribute(menuBarHandle, m, ATTR_NEXT_ITEM_ID, &m);
		i++;
	}
	
	if(m == 0)
	{
		err = -3;
		goto Exit;
	}
	
	char *filename = malloc(MAX_PATHNAME_LEN);
	GetValueFromIndex(HiddenPanel, HiddenVals_RecentPrograms, i, filename);
	
	if(!file_exists(filename))
	{
		err = -4;
		goto Exit;
	} else
//		err = LoadProgram_IO(filename);
	
	if(err < 0)
		err = -4;
	
	Exit:
	switch(err) {
		case -1:
			MessagePopup("Error Loading Program", "No submenu detected. That's odd.");
			break;
		case -2:
		case -3:
			MessagePopup("Error Loading Program", "Doesn't seem that the menu was reloaded. Try again, and if it still doesn't work, load the old fashioned way.");
			break;
		case -4:
			DeleteListItem(HiddenPanel, HiddenVals_RecentPrograms, i, 1); 
			MessagePopup("Error Loading Program", "File not found.");
			break;
	}
	
	if(err < 0)
		generate_recent_programs_menu();
			
}

int generate_recent_programs_menu()
{
	char *label = malloc(MAX_FILENAME_LEN);
	int i, nl;
	
	GetNumListItems(HiddenPanel, HiddenVals_RecentPrograms, &nl);
	
	int rid;
	GetMenuBarAttribute(panelHandleMenu, MainMenu_File_Load_LoadRecentProgram, ATTR_SUBMENU_ID, &rid);
	
	if(nl > 0)
	{
		if(rid == 0)
			rid = NewSubMenu(panelHandleMenu, MainMenu_File_Load_LoadRecentProgram);
		else
			EmptyMenu(panelHandleMenu, rid);
		
		SetMenuBarAttribute(panelHandleMenu, MainMenu_File_Load_LoadRecentProgram, ATTR_DIMMED, 0);   
	} else {
		if(rid != 0)
			DiscardMenu(panelHandleMenu, rid);
		
		SetMenuBarAttribute(panelHandleMenu, MainMenu_File_Load_LoadRecentProgram, ATTR_DIMMED, 1);
	}
	
	for(i = 0; i<nl; i++)
	{
		GetLabelFromIndex(HiddenPanel, HiddenVals_RecentPrograms, i, label);
		NewMenuItem(panelHandleMenu, rid, label, -1, NULL, SelectRecentProgram, 0);
	}
	
	return 0;
}

int add_recent_program(char *fname)
{
	char *filename = malloc(MAX_PATHNAME_LEN);
	char *label = malloc(MAX_PATHNAME_LEN);
	strcpy(filename, fname);
	SplitPath(filename, NULL, NULL, label);
	
	if(strcmp(label + strlen(label) - 4, ".txt") == 0)
		label[strlen(label)-4] = '\0';
	
	int nl;
	GetNumListItems(HiddenPanel, HiddenVals_RecentPrograms, &nl);
	
	int loc;
	if(nl > 0)
	{
		GetIndexFromValue(HiddenPanel, HiddenVals_RecentPrograms, &loc, filename);
		if(loc >= 0)
			DeleteListItem(HiddenPanel, HiddenVals_RecentPrograms, loc, 1);
		else if (nl == 10)
			DeleteListItem(HiddenPanel, HiddenVals_RecentPrograms, 9, -1);

	} else if(nl < 0) 
		return -1;
	
	InsertListItem(HiddenPanel, HiddenVals_RecentPrograms, 0, label, filename);

	generate_recent_programs_menu();
	
	return 0;
}

int add_recent_experiment(char *fname)
{
	char *label = malloc(MAX_PATHNAME_LEN);
	char *filename = malloc(MAX_PATHNAME_LEN);
	strcpy(filename, fname);
	SplitPath(filename, NULL, NULL, label);
	
	filename[strlen(filename)-strlen(label)] = '\0';
	if(strcmp(filename + strlen(filename) - 1, "\\") == 0)
		filename[strlen(filename)-1] = '\0';
	
	if(strcmp(label + strlen(label) - 4, ".txt") == 0)
		label[strlen(label)-4] = '\0';
	
	int nl;
	GetNumListItems(HiddenPanel, HiddenVals_RecentData, &nl);
	
	int loc;
	if(nl > 0)
	{
		GetIndexFromValue(HiddenPanel, HiddenVals_RecentData, &loc, filename);
		if(loc >= 0)
			DeleteListItem(HiddenPanel, HiddenVals_RecentData, loc, 1);
		else if (nl == 10)
			DeleteListItem(HiddenPanel, HiddenVals_RecentData, 9, -1);

	} else if(nl < 0) 
		return -1;
	
	InsertListItem(HiddenPanel, HiddenVals_RecentData, 0, label, filename);

	generate_recent_experiments_menu();
	
	return 0;
}
				
int load_data()
{
	char *loadfile = malloc(MAX_PATHNAME_LEN);
	int stat;
	
	char *directory = malloc(MAX_PATHNAME_LEN);

	GetProjectDir(directory);
	if(directory[strlen(directory)-1] != '\\')
		strcat(directory, "\\");
	strcat(directory, "Experiments");
	
	if(!file_exists(directory))
		sprintf(directory, "");
	
	stat = DirSelectPopup(directory, "Select data directory.", 1, 0, loadfile);
	
	if(stat == VAL_NO_FILE_SELECTED)
		return 0;
	
	return load_data_file(loadfile);	
}

int load_data_file(char *loadfile)
{
	/*
	if(!file_exists(loadfile))
		return -1;
	
	char *buff = malloc(MAX_FILENAME_LEN);
	char *lfbuff = malloc(MAX_PATHNAME_LEN);  

	if(get_folder_name(loadfile, buff) < 0)
		return -2;
	
	GetCtrlVal(FID, FID_CurrentPathname, lfbuff);
	SetCtrlVal(FID, FID_CurrentPathname, loadfile);
	
	strcat(loadfile, "\\");
	strcat(loadfile, buff);
	strcat(loadfile, ".txt");
	
	int nd = get_num_dimensions(loadfile);
	
	if(nd < 0)
	{
		SetCtrlVal(FID, FID_CurrentPathname, lfbuff);
		return -3;
	}
	
	int err;
	if(nd == 0)
		err = load_data_1d(loadfile);
	else
		err = load_data_nd(nd, loadfile);
	
	if(err < 0)
	{
		SetCtrlVal(FID, FID_CurrentPathname, lfbuff);
		return -4;
	}
	
	add_recent_experiment(loadfile);
	*/
	return 0;
}

int poly_subtract(double *data, int c, int np)
{
	//Feed this the array of doubles *data[np] and the channel, and it will determine if polynomial subtraction
	//is on for this channel and if so, calculate the polynomial and return it as *polyout.
	
	//Passing NULL to the *data parameter returns whether or not it is on.
	int polyon, nl;
	
	//Start by determining if the relevant channel should have polynomial subtraction.
	GetNumListItems(HiddenPanel, HiddenVals_PolySubtractOnOffRing, &nl);
	
	if(c >= nl)
		return -1;
	
	GetValueFromIndex(HiddenPanel, HiddenVals_PolySubtractOnOffRing, c, &polyon);
	if(polyon < 0 || polyon > 1)
		return -1;
	
	if(!polyon)
		return 0;
	else if(data != NULL)
	{
		//If polynomial subtraction is on, fit the data in this channel to a polynomial and return it.
		GetNumListItems(HiddenPanel, HiddenVals_PolyOrderValues, &nl);
		
		if(c > nl)
			return -2;
		
		int order;

		GetValueFromIndex(HiddenPanel, HiddenVals_PolyOrderValues, c, &order);
		
		if(order < 0 || order > 90)
			return -3;
		
		if(order > 0)
		{
			double *polyarray = malloc(sizeof(double)*np);
			polynomial_fit(data, np, order, polyarray);
			for(int i = 0; i<np; i++)
				data[i] -= polyarray[i];
		} else {
			double mean;
			Mean(data, np, &mean);
			
			for(int i = 0; i<np; i++)
				data[i] -= mean;
		}
	}
	
	return 1;
}
	

int change_fid_and_fft_locked(int t)
{
	//Clears the current transient and loads t. If t = 0, it loads the average.
	char *fname = malloc(MAX_PATHNAME_LEN);
	
	if(get_fid_or_fft_fname(fname, t, -1, 2) < 0)
		return -1;
	
	int i, nt, np, nc, sr;

	nt = get_num_transients_completed(fname);
	nc = get_num_channels(fname);
	
	if(nt < 1 || t > nt || t < 0 || nc < 1 || nc > 8)
		return -2;
	
	//Clear the plots
	DeleteGraphPlot(FID, FID_Graph, -1, VAL_IMMEDIATE_DRAW);
	DeleteGraphPlot(FFTSpectrum, Spectrum_Graph, -1, VAL_IMMEDIATE_DRAW);
	
	
	double *data;
	double *ic, *rc;
	int polyon, polyplot, perr[4] = {0, 0, 0, 0}, fid_on, fft_on;

	for(i = 0; i < nc; i++)
	{
		
		//Check which channels are on
		fid_on = channel_on(i, 0);
		fft_on = channel_on(i, 1);
		
		//Initialize these to -2. This will tell other functions that the channel is available, but was never plotted.
		SetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(1, i+1), -2);
		SetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(2, i+1), -2);
		SetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(3, i+1), -2);
		SetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(4, i+1), -2);
	
		//Check if polynomial subtraction is on first, if at least one of the two channels is on
		if(fid_on || fft_on)
		{
			sr = get_sampling_rate(fname);
			np = get_num_points(fname);
			polyon = poly_subtract(NULL, i, np);
			
			if(np < 1 || sr < 1)
				return -3;
		}
		else
			continue;
		
		//Plot the FID first - if the FID channel is off but polynomial subtraction is on, we still need to get the data so we can fourier transform it.
		if(fid_on || polyon)
		{
			if(get_fid_or_fft_fname(fname, t, i, 0) < 0)
				return -3;

			data = malloc(sizeof(double)*np);
	
			load_fid_data(fname, np, data);
			
			//If polynomial subtraction is on, subtract off the polynomial fit.
			if(polyon == 1)
				poly_subtract(data, i, np);

			if(fid_on)
				plot_fid_data(data, i, np, t, sr);
				
		} else if (fft_on) {
			data = malloc(sizeof(double)*np);
			load_fid_data(fname, np, data);
		}
		 
		
		//Now the FFT
		if(fft_on)
		{
			
			rc = malloc(sizeof(double)*np);
			ic = malloc(sizeof(double)*np);
			//If polynomial subtraction is on or it's the average file, we need to FFT the data again.
			/*int rms;
			GetCtrlVal(FFTSpectrum, Spectrum_DisplayRMSBounds, &rms);
			if(t == 0 || polyon)
			{
				if(fft(data, rc, ic, np) < 0)
					return -4;
			} else {

				//Get FFT filename
				if(get_fid_or_fft_fname(fname, t, i, 1) < 0)
					return -5;
	
				np = get_num_points(fname);
				sr = get_sampling_rate(fname);
				
				if(np < 1 || sr < 1)
					return -6;
		
				load_fft_data(fname, np, rc, ic);
			}*/
			
			//FFT the data
			if(fft(data, rc, ic, np) < 0)
				return -4;
			
			//Plot the FFT
			plot_fft_data(rc, ic, i, np, sr, t); 
		}
	}
	
	int vt;
	
	/*GetNumListItems(FID, FID_TransientNum, &vt);
	if(vt > t)
		SetCtrlIndex(FID, FID_TransientNum, t);
	
	GetNumListItems(FFTSpectrum, Spectrum_TransientNum, &vt);
	if(vt > t)
		SetCtrlIndex(FFTSpectrum, Spectrum_TransientNum, t);
	*/
	return 0;
}

int change_fid_and_fft(int t)
{
	CmtGetLock(lock_plot);
	int rv = change_fid_and_fft_locked(t);
	CmtReleaseLock(lock_plot);
	
	return rv;
}


int change_fid(int plot)
{
	/*
	int i, to, tplot;
	
	GetNumListItems(FID, FID_TransientNum, &to);
	
	if(to < 1)
		return -1;
	
	for(i = 0; i < to; i++)
	{
		GetValueFromIndex(FID, FID_TransientNum, i, &tplot);
		SetPlotAttribute(FID, FID_Graph, tplot, ATTR_TRACE_VISIBLE, 0);
	}
	
	SetPlotAttribute(FID, FID_Graph, plot, ATTR_TRACE_VISIBLE, 1);
	
	GetIndexFromValue(FID, FID_TransientNum, &tplot, plot);
	
	SetCtrlIndex(FID, FID_TransientNum, tplot);
	*/
	return 0;
}

int change_spectrum(int plot)
{
	/*
	int i, to, tplot;
	
	GetNumListItems(FFTSpectrum, Spectrum_TransientNum, &to);
	
	if(to < 1)
		return -1;
	
	for(i = 0; i < to; i++)
	{
		GetValueFromIndex(FFTSpectrum, Spectrum_TransientNum, i, &tplot);
		SetPlotAttribute(FFTSpectrum, Spectrum_Graph, tplot, ATTR_TRACE_VISIBLE, 0);
	}
	
	
	SetPlotAttribute(FFTSpectrum, Spectrum_Graph, plot, ATTR_TRACE_VISIBLE, 1);
	
	GetIndexFromValue(FFTSpectrum, Spectrum_TransientNum, &tplot, plot);
	
	SetCtrlIndex(FFTSpectrum, Spectrum_TransientNum, tplot);
	*/
	return 0;
}

int change_phase(double phase, int order, int chan)
{
	//Changes the phase between the real and imaginary components of the data.
	
	int realplot, imagplot, np, np0;
	double ph0, ph1, ph2, nph0, nph1, nph2;
	
	GetTableCellVal(HiddenPanel, HiddenVals_PhaseCorrectionValues, MakePoint(1, chan+1), &ph0);
	GetTableCellVal(HiddenPanel, HiddenVals_PhaseCorrectionValues, MakePoint(2, chan+1), &ph1);
	GetTableCellVal(HiddenPanel, HiddenVals_PhaseCorrectionValues, MakePoint(3, chan+1), &ph2);
	
	GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(2, chan+1), &realplot);
	GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(3, chan+1), &imagplot);
	
	if(realplot < 0 || imagplot < 0)
		return -1;
	
	GetPlotAttribute(FFTSpectrum, Spectrum_Graph, realplot, ATTR_NUM_POINTS, &np);
	GetPlotAttribute(FFTSpectrum, Spectrum_Graph, imagplot, ATTR_NUM_POINTS, &np0);
	
	if(np != np0)
		return -2;
	
	double *rc = malloc(sizeof(double)*np), *ic = malloc(sizeof(double)*np), *orc = malloc(sizeof(double)*np), *oic = malloc(sizeof(double)*np);
	
	//Get the current data.
	GetPlotAttribute(FFTSpectrum, Spectrum_Graph, realplot, ATTR_PLOT_YDATA, orc);
	GetPlotAttribute(FFTSpectrum, Spectrum_Graph, imagplot, ATTR_PLOT_YDATA, oic);
	
	nph0 = ph0;
	nph1 = ph1;
	nph2 = ph2;
	
	if(order == 1)
		ph0 = phase;
	if(order == 2)
		ph1 = phase;
	if(order == 3)
		ph2 = phase;
	
	if(nph0 == ph0 && nph1 == ph1 && nph2 == ph2)
		return 0;
	
	SetTableCellVal(HiddenPanel, HiddenVals_PhaseCorrectionValues, MakePoint(1, chan+1), ph0);
	SetTableCellVal(HiddenPanel, HiddenVals_PhaseCorrectionValues, MakePoint(2, chan+1), ph1);
	SetTableCellVal(HiddenPanel, HiddenVals_PhaseCorrectionValues, MakePoint(3, chan+1), ph2);
	
	//Get only phase differences.
	ph0 -= nph0;
	ph1 -= nph1;
	ph2 -= nph2;
	
	//Convert to radians
	ph0 /= 360/(2*Pi());
	ph1 /= 360/(2*Pi());
	ph2 /= 360/(2*Pi());
	
	double om, om2, ph;
	
	for(int i = 0; i<np; i++)
	{
		//Normalize the frequencies modulation.
		om = (2*i/np - 1)*10;
		om2 = (2*(i*i)/(np*np) - 1)*5;
		ph = ph0 + ph1*om + ph2*om2;
		
		//Apply the new phase transformation -> e^i(/\ph0 + /\ph1w + /\ph2w^2)
		rc[i] = cos(ph)*orc[i] - sin(ph)*oic[i];
		ic[i] = cos(ph)*oic[i] + sin(ph)*orc[i];
	}

	//Replace the current data.
	SetPlotAttribute(FFTSpectrum, Spectrum_Graph, realplot, ATTR_PLOT_YDATA, rc);
	SetPlotAttribute(FFTSpectrum, Spectrum_Graph, imagplot, ATTR_PLOT_YDATA, ic);
	RefreshGraph(FFTSpectrum, Spectrum_Graph);
	
	return 0;
}

//////////////////////////////////////////////////////
//                                                   //
//                 General Utilities                 //
//                                                   //
///////////////////////////////////////////////////////

char *temp_file(char *extension) {
	//Convenience method, generates a temporary file with extension "extension".
	return temporary_filename(NULL, extension);
}

char *temporary_filename(char *dir, char *extension) {
	//Generates a random temporary file in the directory dir and returns the pathname.
	//If dir == NULL, it uses the directory TemporaryFiles in the project directory.

	char *filenameout = malloc(MAX_PATHNAME_LEN), *dirtok = malloc(MAX_PATHNAME_LEN);
	
	if(dir == NULL) {
		char *tempdir = malloc(MAX_PATHNAME_LEN);  
		GetProjectDir(tempdir);
		if(tempdir[strlen(tempdir)] != '\\')
		strcat(tempdir, "\\");
		strcat(tempdir, "TemporaryFiles");
		strcpy(dirtok, tempdir);
		strcpy(filenameout, tempdir);
		free(tempdir);
	} else {
		strcpy(dirtok, dir);
		strcpy(filenameout, dir);
	}
	
	if(!file_exists(dirtok))
	{
		char *createdir = malloc(MAX_PATHNAME_LEN), *tok = malloc(MAX_PATHNAME_LEN);
		tok = strtok(dirtok, "\\");
		strcpy(createdir, "");
		while(tok != NULL)
		{
			strcat(createdir, tok);
			strcat(createdir, "\\");
			if(!file_exists(createdir)) {
				MakeDir(createdir);
			}
			tok = strtok(NULL, "\\");
		}
	}
	
	srand(time(NULL));
	char randtotext[39] = "0123456789abcdefghijklmnopqrstuvwxyz_-\0";
	
	if(filenameout[strlen(filenameout)] != '\\')
		strcat(filenameout, "\\");
	
	int i, j = 0, r;
	while(j++ < 200)
	{
		r = rand()%6;
		for(i = 0; i< 18 + r; i++)
			sprintf(filenameout, "%s%c", filenameout, randtotext[rand()%strlen(randtotext)]);
		
		strcat(filenameout, extension);
		
		if(!file_exists(filenameout))
			break;
	}
	
	if(j == 1000)
		return NULL;

	FILE *fout = fopen(filenameout, "w+");
	fclose(fout);
	
	return filenameout;
	
}

int all_panels (int num)
{
	//This returns the num-th panel (not including instruction panels).
	//Passing a negative value to this returns the number of panels.
	
	if(num < 0)
		return 6;
	
	if(num > 5)
		return -1;
	
	int *panels = malloc(6*sizeof(int));
	
	panels[0] = panelHandle;
	panels[1] = FID;
	panels[2] = FFTSpectrum;
	panels[3] = Pulse_Prog_Tab;
	panels[4] = Pulse_Prog_Config;
	panels[5] = HiddenPanel;

	return panels[num];
}

int all_controls(int panel, int num)
{
	/*
	//This returns the num-th control on "panel" (0-based index).
	//Passing a negative value to this returns the number of panels.

	int *panels, number;
	//Number for each panel
	if(panel == panelHandle)
		number = 31;
	else if(panel == FID)
		number = 26;
	else if(panel == FFTSpectrum)
		number = 27;
	else if(panel == Pulse_Prog_Tab)
		number = 6;
	else if(panel == Pulse_Prog_Config)
		number = 26;
	else if(panel == HiddenPanel)
		number = 9;
	else
		return -1;
	
	if(num >= number)
		return -2;
	
	if(num < 0)
		return number;

	panels = malloc(number*sizeof(int));
	
	//panelHandle -> The Main Panel
	if(panel == panelHandle)
	{
		panels[0] = MainPanel_MainTabs;
		panels[1] = MainPanel_Start;
		panels[2] = MainPanel_Stop;
		panels[3] = MainPanel_ScanNum;
		panels[4] = MainPanel_QUITBUTTON;
		panels[5] = MainPanel_Filename;
		panels[6] = MainPanel_DirectorySelect;
		panels[7] = MainPanel_Running;
		panels[8] = MainPanel_Waiting;
		panels[9] = MainPanel_Stopped;
		panels[10] = MainPanel_Path;
		panels[11] = MainPanel_ID9;
		panels[12] = MainPanel_ID8;
		panels[13] = MainPanel_ID7;
		panels[14] = MainPanel_ID6;
		panels[15] = MainPanel_ID5;
		panels[16] = MainPanel_ID4;
		panels[17] = MainPanel_ID3;
		panels[18] = MainPanel_ID2;
		panels[19] = MainPanel_ID1;
		panels[20] = MainPanel_ID9Val;
		panels[21] = MainPanel_ID8Val;
		panels[22] = MainPanel_ID7Val;
		panels[23] = MainPanel_ID6Val;
		panels[24] = MainPanel_ID5Val;
		panels[25] = MainPanel_ID4Val;
		panels[26] = MainPanel_ID3Val;
		panels[27] = MainPanel_ID2Val;
		panels[28] = MainPanel_ID1Val;
		panels[29] = MainPanel_IsRunning;
		panels[30] = MainPanel_PBStatus;
	}
	
	//FID -> The tab containing the FID
	if(panel == FID)
	{
		panels[0] = FID_Graph;
		panels[1] = FID_Average;
		panels[2] = FID_CalculateAverage;
		panels[3] = FID_TransientNum;
		panels[4] = FID_CurrentPathname;
		panels[5] = FID_Dimension;
		panels[6] = FID_DimPoint;
		panels[7] = FID_CurrentDimPoint;
		panels[8] = FID_DimPointsCompleted;
		panels[9] = FID_Chan8;
		panels[10] = FID_Chan7;
		panels[11] = FID_Chan6;
		panels[12] = FID_Chan5;
		panels[13] = FID_Chan4;
		panels[14] = FID_Chan3;
		panels[15] = FID_Chan2;
		panels[16] = FID_Chan1;
		panels[17] = FID_ChanLabel;
		panels[18] = FID_ChanPrefs;
		panels[19] = FID_PolySubtract;
		panels[20] = FID_PolyFitOrder;
		panels[21] = FID_ChannelBox;
		panels[22] = FID_PolyFitLabel;
		panels[23] = FID_Offset;
		panels[24] = FID_Gain;
		panels[25] = FID_ChanColor;
	}
	
	//FFTSpectrum -> The Tab containing the FFT Spectrum
	if(panel == FFTSpectrum)
	{
		panels[0] = Spectrum_Graph;
		panels[1] = Spectrum_CursorX;
		panels[2] = Spectrum_CursorY;
		panels[3] = Spectrum_TransientNum;
		panels[4] = Spectrum_ChangePhase;
		panels[5] = Spectrum_Dimension;
		panels[6] = Spectrum_DimPoint;
		panels[7] = Spectrum_Channel;
		panels[8] = Spectrum_ChanLabel;
		panels[9] = Spectrum_Chan8;
		panels[10] = Spectrum_Chan7;
		panels[11] = Spectrum_Chan6;
		panels[12] = Spectrum_Chan5;
		panels[13] = Spectrum_Chan4;
		panels[14] = Spectrum_Chan3;
		panels[15] = Spectrum_Chan2;
		panels[16] = Spectrum_Chan1;
		panels[17] = Spectrum_ChannelBox;
		panels[18] = Spectrum_ChanPrefs;
		panels[19] = Spectrum_PolySubtract;
		panels[20] = Spectrum_PolyFitOrder;
		panels[21] = Spectrum_PolyFitLabel;
		panels[22] = Spectrum_PhaseCorrectionOrder;
		panels[23] = Spectrum_PhaseKnob;
		panels[24] = Spectrum_Offset;
		panels[25] = Spectrum_Gain;
		panels[26] = Spectrum_ChanColor;
	}
	
	//Pulse_Prog_Tab -> Tab containing the 1D Pulse program.
	if(panel == Pulse_Prog_Tab)
	{
		panels[0] = PulseProg_InstNum;
		panels[1] = PulseProg_SaveProgram;
		panels[2] = PulseProg_NewProgram;
		panels[3] = PulseProg_LoadProgram;
		panels[4] = PulseProg_Trigger_TTL;
		panels[5] = PulseProg_ContinuousRun;
	}
	
	//Pulse_Prog_Config -> The Pulse program configuration/2D Sequence tab
	if(panel == Pulse_Prog_Config)
	{
		panels[0] = PPConfig_SaveProgram;
		panels[1] = PPConfig_NTransients;
		panels[2] = PPConfig_LoadProgram;
		panels[3] = PPConfig_NDimensionalOn;
		panels[4] = PPConfig_PhaseCycle;
		panels[5] = PPConfig_PhaseCycleInstr;
		panels[6] = PPConfig_NumCycles;
		panels[7] = PPConfig_EstimatedTime;
		panels[8] = PPConfig_PhaseCycleFreq;
		panels[9] = PPConfig_NPoints;
		panels[10] = PPConfig_SampleRate;
		panels[11] = PPConfig_Device;
		panels[12] = PPConfig_CounterChan;
		panels[13] = PPConfig_TriggerEdge;
		panels[14] = PPConfig_Trigger_Channel;
		panels[15] = PPConfig_AcquisitionChannel;
		panels[16] = PPConfig_AcquisitionTime;
		panels[17] = PPConfig_NumDimensions;
		panels[18] = PPConfig_DimensionPoint;
		panels[19] = PPConfig_DimensionPoints;
		panels[20] = PPConfig_NumChans;
		panels[21] = PPConfig_InputMax;
		panels[22] = PPConfig_InputMin;
		panels[23] = PPConfig_ChannelGain;
		panels[24] = PPConfig_MaxVals;
		panels[25] = PPConfig_MinVals;
	}
	
	//HiddenPanel -> The Hidden Panel containing various information.
	if(panel == HiddenPanel)
	{
		panels[0] = HiddenVals_PolySubtractOnOffRing;
		panels[1] = HiddenVals_PhaseCorrectionValues;
		panels[2] = HiddenVals_PolyOrderValues;
		panels[3] = HiddenVals_ChannelOffsets;
		panels[4] = HiddenVals_ChannelGains;
		panels[5] = HiddenVals_TransientView;
		panels[6] = HiddenVals_LastProgramLoc;
		panels[7] = HiddenVals_ControlDimmed;
		panels[8] = HiddenVals_ControlHidden;
		
	}
	
	return panels[num];
	*/
	return NULL;
}
	

int Load_TTLs (int num)
{
	//Returns the control ID of TTL #num.
	int *TTLs = malloc(24*sizeof(int));
	TTLs[0] = PulseInstP_TTL0;
	TTLs[1] = PulseInstP_TTL1;
	TTLs[2] = PulseInstP_TTL2;
	TTLs[3] = PulseInstP_TTL3;
	TTLs[4] = PulseInstP_TTL4;
	TTLs[5] = PulseInstP_TTL5;
	TTLs[6] = PulseInstP_TTL6;
	TTLs[7] = PulseInstP_TTL7;
	TTLs[8] = PulseInstP_TTL8;
	TTLs[9] = PulseInstP_TTL9;		
	TTLs[10] = PulseInstP_TTL10;
	TTLs[11] = PulseInstP_TTL11;
	TTLs[12] = PulseInstP_TTL12;
	TTLs[13] = PulseInstP_TTL13;
	TTLs[14] = PulseInstP_TTL14;
	TTLs[15] = PulseInstP_TTL15;
	TTLs[16] = PulseInstP_TTL16;
	TTLs[17] = PulseInstP_TTL17;
	TTLs[18] = PulseInstP_TTL18;
	TTLs[19] = PulseInstP_TTL19;
	TTLs[20] = PulseInstP_TTL20;
	TTLs[21] = PulseInstP_TTL21;
	TTLs[22] = PulseInstP_TTL22;
	TTLs[23] = PulseInstP_TTL23;
	
	return TTLs[num];
}

int swap_ttls(int to, int from)
{
	int i, j, val, bufferval;
	for(i = 0; i<ninstructions; i++)
	{
		GetCtrlVal(inst[i], Load_TTLs(to), &bufferval);
		GetCtrlVal(inst[i], Load_TTLs(from), &val);
		
		SetCtrlVal(inst[i], Load_TTLs(to), val);
		SetCtrlVal(inst[i], Load_TTLs(from), bufferval);
	}
	
	return 0;
}
		

int isin(int source, int *array, int sizeofarray)
{
	//Check if source is in the array of integers passed to the function..
	int i;

	if(sizeofarray < 1)
		return -2;
	
	if(array == NULL)
		return -1;
	
	for(i = 0; i<sizeofarray; i++) {
		if (source == array[i])
			return i;
	}
	
	return -1;
}

file_exists(char *filename)
{
	//Check if a file exists.
	return FileExists(filename, 0);
}


get_value_from_file(char *filename, int line, char *string, void *value)
{
	
	//Reads out a value from a file. This will work with program data and defaults files.
	FILE *file;
	//fpos_t *fpos;
	int i;
	
	char *string_buffer = malloc(255), *string_pointer;
	
	//Check if the file exists.
	if(!file_exists(filename))
		return -1;
	
	//Read out the line we want.
	file = fopen(filename, "r+");
	
	if(file == NULL)
		return -2;
	
	for(i = 0; i<line; i++)
		fgets(string_buffer, 255, file);
	
	fclose(file);
	
	//If a string is too small, use our own string buffer. 
	if(sizeof(string) >= 250)
		string_pointer = string;
	else
	{
		char *string_match = malloc(255);
		string_pointer = string_match;
	}
	
	if(sizeof(string) >= 20)
		sprintf(string, "String too short.\n");
	
	
	if(sscanf(string_buffer, "%s %d", string_pointer, value) == 2)
		return 1;
	else if (sscanf(string_buffer, "%s %s", string_pointer, value) == 2)
		return 2;
	else if (sscanf(string_buffer, "%s %lf", string_pointer, value) == 2)
		return 3;
	
	free(string_buffer);
	free(string_pointer);
	return 0;
}

int update_status(int status)
{
	int i, bit, statusbin[4] = {0, 0, 0, 0};
	
	//Get the status. If this is not a valid status, ask the board.
	if(status > 32 || status < 1)
		status = pb_read_status_safe(1);
	
	//Convert from integer to binary so that I can access the individual bits. 
	for (i = 0; i<4; i++)
	{
        bit = (int)pow(2, i);
		if(bit & status)
			statusbin[i] = 1;
		else
			statusbin[i] = 0;
	}
	
	/*Toggle the LEDs.
	Bit 0 = Stopped
	Bit 1 = Reset
	Bit 2 = Running
	Bit 3 = Waiting
	Bit 4 = Scanning*/
	
	SetCtrlVal(panelHandle, MainPanel_Stopped, statusbin[0]);
	SetCtrlVal(panelHandle, MainPanel_Running, statusbin[2]);
	SetCtrlVal(panelHandle, MainPanel_Waiting, statusbin[3]);

	return status;
}

double log2(double value)
{
    return log(value)/log(2);
}

int fft (double *in, double *rc, double *ic, int np)
{
	
	NIComplexNumber *fft_out = malloc(sizeof(NIComplexNumber)*np*2);
	
	
	FFTEx(in, np, np*2, NULL, 0, fft_out);
	
	
	NIComplexNumber *out;
	for(int i=0; i<np; i++) {
		out = &fft_out[i]; 
		rc[i] = out->real;
		ic[i] = out->imaginary;
	}
	
	/*int i;
	fftw_complex *out = fftw_malloc(sizeof(fftw_complex)*np);
	double *in_fftw = malloc(sizeof(double)*np);
	
	fftw_plan p = fftw_plan_dft_r2c_1d(np, in_fftw, out, FFTW_PRESERVE_INPUT);
	
	for(i = 0; i<np; i++)
		in_fftw[i] = in[i];
	
	fftw_execute(p);
	
	for(i = 0; i<np; i++)
	{
		rc[i] = out[0][i];
		ic[i] = out[1][i];
	}
	
	fftw_destroy_plan(p);
	fftw_free(out);
	*/
	return 0;
}


	


///////////////////////////////////////////////////////
//                                                   //
//         Safe Pulseblaster Functions               //
//                                                   //
///////////////////////////////////////////////////////

int pb_init_safe (int verbose)
{
	
	CmtGetLock(lock_pb);
	int rv = pb_init();
	pb_set_clock(100.0);
    CmtReleaseLock(lock_pb);
	
	if(rv < 0)
	{
		sprintf(errorstring, "%d - %s\n", rv, pb_get_error());
		MessagePopup("Error", errorstring);
	}
	if(rv < 0 && verbose)
	{
		sprintf(errorstring, "%s\n", pb_get_error());
		MessagePopup("Error", errorstring);
	}
	
	initialized = 1;
	
	return rv;
}

int pb_start_programming_safe(int verbose, int device)
{
	CmtGetLock(lock_pb);
	if(!initialized)
	{
		CmtReleaseLock(lock_pb);
		pb_init_safe(verbose);
		CmtGetLock(lock_pb);
	}
	int rv = pb_start_programming(device);
	CmtReleaseLock(lock_pb);
	
	if(rv != 0 && verbose)
		MessagePopup("Error", pb_get_error());
	return rv;
}

int pb_stop_programming_safe(int verbose)
{
	
	CmtGetLock(lock_pb);
	if(!initialized)
	{
		CmtReleaseLock(lock_pb);
		pb_init_safe(verbose);
		CmtGetLock(lock_pb);
	}
	int rv = pb_stop_programming();
	CmtReleaseLock(lock_pb);
	
	if(rv <0 && verbose)
		MessagePopup("Error", pb_get_error());
	return rv;
}

int pb_inst_pbonly_safe (int verbose, unsigned int flags, int inst, int inst_data, double length)
{
	
	CmtGetLock(lock_pb);
	if(!initialized)
	{
		CmtReleaseLock(lock_pb);
		pb_init_safe(verbose);
		CmtGetLock(lock_pb);
	}

	int rv = pb_inst_pbonly(flags, inst, inst_data, length);
	CmtReleaseLock(lock_pb);
		
	if(rv <0 && verbose)
		MessagePopup("Error", pb_get_error());
	return rv;
}
	

int pb_close_safe (int verbose)
{
	
	CmtGetLock(lock_pb);
	int rv = pb_close();
	CmtReleaseLock(lock_pb);
	if(rv < 0 && verbose)
		MessagePopup("Error", pb_get_error());
	
	initialized = 0;
	
	return rv;
}

int pb_read_status_safe(int verbose)
{
	
	int rv;
	
	CmtGetLock(lock_pb);
	if(!initialized)
	{
		CmtReleaseLock(lock_pb);
		rv = pb_init_safe(verbose);
		CmtGetLock(lock_pb);
	}
	rv = pb_read_status();
	CmtReleaseLock(lock_pb);
		
	
	if((rv < 0 || rv > 32) && verbose)
	{
		int i, length = (int)(log2(abs(rv))+1);
		char *status = malloc(length+1);
		
		for(i = 0; i<length; i++)
			if(rv & (int)pow(2, i))
				status[length-i-1] = '1';
			else
				status[length-i-1] = '0';
		status[length] = '\0';
		sprintf(errorstring, "%s, Status: %s\n", pb_get_error(), status);
		MessagePopup("Error", errorstring);
	}
	
	return rv;
}

int pb_start_safe(int verbose)
{
	
	CmtGetLock(lock_pb);
	if(!initialized)
	{
		CmtReleaseLock(lock_pb);
		pb_init_safe(verbose);
		CmtGetLock(lock_pb);
	}
	int rv = pb_start();
	CmtReleaseLock(lock_pb);

	if(rv < 0 && verbose)
		MessagePopup("Error", pb_get_error());

	return rv;
}

int pb_stop_safe(int verbose)
{
	CmtGetLock(lock_pb);
	if(!initialized)
	{
		CmtReleaseLock(lock_pb);
		pb_init_safe(verbose);
		CmtGetLock(lock_pb);
	}
	int rv = pb_stop();
	CmtReleaseLock(lock_pb);
	
	if(rv < 0 && verbose)
		MessagePopup("Error", pb_get_error());
	return rv;
}


///////////////////////////////////////////////////////
//                                                   //
//               Main Panel Callbacks                //
//                                                   //
///////////////////////////////////////////////////////

int CVICALLBACK DirectorySelect (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			 char *directory = malloc(500);
			 int stat;
			 
			 GetCtrlVal(panelHandle, MainPanel_Path, directory);
			 
			 stat = DirSelectPopup(directory, "Select path", 1, 1, directory);
			 if(stat < 0)
			 {
				 MessagePopup("Error", "There was an error.");
				 return -1;
			 } else if (stat == VAL_NO_DIRECTORY_SELECTED)
				 return 0;
			 
			 if(strcmp(&directory[strlen(directory)-1], "\\"))
				 strcat(directory, "\\");

			 SetCtrlVal(panelHandle, MainPanel_Path, directory);
			 free(directory);
			break;
	}
	return 0;
}

int CVICALLBACK WriteToFile (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeSampleRate (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			int i, np_sr = -1;
			GetCtrlIndex(HiddenPanel, HiddenVals_SRConstant, &i);
			if(i ==  0)
				np_sr = 0; //Keep np constant, change at.
			else if(i == 1)
				np_sr = 2; //Keep at constant, change np.
			
			if(np_sr >= 0)
				change_np_or_sr(np_sr);
			
			break;
			
		case EVENT_RIGHT_CLICK:
			
			int nl;
			GetNumListItems(HiddenPanel, HiddenVals_SRConstant, &nl);
			
			//Just in case the hidden control got broken somehow, restore it to default setting.
			if(nl != 2)
			{
				if(nl > 0)
					DeleteListItem(HiddenPanel, HiddenVals_SRConstant, 0, -1);
				InsertListItem(HiddenPanel, HiddenVals_SRConstant, -1, "Number of Points", 0);
				InsertListItem(HiddenPanel, HiddenVals_SRConstant, -1, "Acquisition Time", 1);
			}
			
			int choice = RunPopupMenu(RC_menu, RCMenus_SampleRate, panel, eventData1, eventData2, 0, 0, 0, 0);
			switch(choice)
			{
				case RCMenus_SampleRate_NumPoints:
					SetCtrlIndex(HiddenPanel, HiddenVals_SRConstant, 0);
					break;
				
				case RCMenus_SampleRate_AcquisitionTime:
					SetCtrlIndex(HiddenPanel, HiddenVals_SRConstant, 1);
					break;
			}

			break;
	}
	return 0;
}

int CVICALLBACK ChangeAcquisitionTime (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			int i, np_sr = -1;
			GetCtrlIndex(HiddenPanel, HiddenVals_ATConstant, &i);
			if(i ==  0)
				np_sr = 2; //Keep sample rate constant, change np.
			else if(i == 1)
				np_sr = 1; //Keep number of points constant, change sr.
			
			if(np_sr >= 0)
				change_np_or_sr(np_sr);
			
			break;
			
		case EVENT_RIGHT_CLICK:
			
			int nl;
			GetNumListItems(HiddenPanel, HiddenVals_ATConstant, &nl);
			
			//Just in case the hidden control got broken somehow, restore it to default setting.
			if(nl != 2)
			{
				if(nl > 0)
					DeleteListItem(HiddenPanel, HiddenVals_ATConstant, 0, -1);
				InsertListItem(HiddenPanel, HiddenVals_ATConstant, -1, "Sample Rate", 0);
				InsertListItem(HiddenPanel, HiddenVals_ATConstant, -1, "Number of Points", 1);
			}
			
			int choice = RunPopupMenu(RC_menu, RCMenus_AcquisitionTime, panel, eventData1, eventData2, 0, 0, 0, 0);
			switch(choice)
			{
				case RCMenus_AcquisitionTime_SampleRate:
					SetCtrlIndex(HiddenPanel, HiddenVals_ATConstant, 0);
					break;
				
				case RCMenus_AcquisitionTime_NumPoints:
					SetCtrlIndex(HiddenPanel, HiddenVals_ATConstant, 1);
					break;
			}

			break;

	}
	return 0;
}

int CVICALLBACK ChangeNPoints (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			int i, np_sr = -1;
			GetCtrlIndex(HiddenPanel, HiddenVals_NPConstant, &i);
			if(i ==  0)
				np_sr = 1; //Keep acquisition time constant, change sr.
			else if(i == 1)
				np_sr = 0; //Keep sample rate constant, change at.
			
			if(np_sr >= 0)
				change_np_or_sr(np_sr);
			
			break;
			
		case EVENT_RIGHT_CLICK:
			
			int nl;
			GetNumListItems(HiddenPanel, HiddenVals_NPConstant, &nl);
			
			//Just in case the hidden control got broken somehow, restore it to default setting.
			if(nl != 2)
			{
				if(nl > 0)
					DeleteListItem(HiddenPanel, HiddenVals_ATConstant, 0, -1);
				InsertListItem(HiddenPanel, HiddenVals_NPConstant, -1, "Acquisition Time", 0);
				InsertListItem(HiddenPanel, HiddenVals_NPConstant, -1, "Sample Rate", 1);
			}
			
			int choice = RunPopupMenu(RC_menu, RCMenus_NumPoints, panel, eventData1, eventData2, 0, 0, 0, 0);
			switch(choice)
			{
				case RCMenus_NumPoints_AcquisitionTime:
					SetCtrlIndex(HiddenPanel, HiddenVals_NPConstant, 0);
					break;
				
				case RCMenus_NumPoints_SampleRate:
					SetCtrlIndex(HiddenPanel, HiddenVals_NPConstant, 1);
					break;
				
			}

			break;
	}
	return 0;
}

int CVICALLBACK UpdateChannels (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			int index, num;
			GetCtrlIndex(Pulse_Prog_Config, PPConfig_Device, &index);
			DeleteListItem(Pulse_Prog_Config, PPConfig_Device, 0, -1);
			
			load_DAQ_info();

			GetNumListItems(Pulse_Prog_Config, PPConfig_Device, &num);
			
			if(index > num)
			{
				SetCtrlIndex(Pulse_Prog_Config, PPConfig_Device, 0);
				break;
			}
			
			GetCtrlIndex(Pulse_Prog_Config, PPConfig_Device, &num);
			
			if(index == num)
				break;
			
			load_DAQ_info();	
			
		break;
	}
	return 0;
}

///////////////////////////////////////////////////////
//                                                   //
//           Pulse Program Callbacks                 //
//                                                   //
///////////////////////////////////////////////////////

int CVICALLBACK InstNumChange (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			//change_number_of_instructions(pc, uipc);
			break;
	}
	return 0;
}

int CVICALLBACK LoadProgram (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int stat;
			char *loadfile = malloc(MAX_PATHNAME_LEN);
			char *directory = malloc(MAX_PATHNAME_LEN);
			
			GetProjectDir(directory);
			if(directory[strlen(directory)-1] != '\\')
				strcat(directory, "\\");
			strcat(directory, "Programs");
			
			if(!file_exists(directory))
				sprintf(directory, "");
			
			stat = FileSelectPopup(directory, "*.txt", "*.txt", "Select Program to Load", VAL_LOAD_BUTTON, 0, 0, 1, 0, loadfile);
			if(stat == VAL_NO_FILE_SELECTED)
				return 0;
			 /*
			if(LoadProgram_IO(loadfile) >= 0)
				add_recent_program(loadfile);
			*/
			break;
	}
	return 0;
}

PPROGRAM *generate_test_program() {
	PPROGRAM *p = malloc(sizeof(PPROGRAM));
	// Generates a program for testing save/load -> Temporary function until get_current_program is done.
	
	// All the metadata
	p->np = 8192;
	p->sr = 819.2;
	p->trigger_ttl = 0;
	p->scan = 1;
	p->varied = 1;
	p->n_inst = 10;
	
	p->total_time = 1883.4;
	
	p->nDims = 2;
	p->nCycles = 1;
	p->nVaried = 3;
	
	p->skip = 1;
	
	int nFuncs = 1;
	int tFuncs = 2;
	
	
	
	return p;
}

int CVICALLBACK SaveProgram (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int stat;
			char *savefile = malloc(500);
			char *directory = malloc(MAX_PATHNAME_LEN);
			
			GetProjectDir(directory);
			if(directory[strlen(directory)-1] != '\\')
				strcat(directory, "\\");
			strcat(directory, "Programs");
			
			if(!file_exists(directory))
				sprintf(directory, "");
			
			stat = FileSelectPopup(directory, "*.txt", "*.txt", "Save Program As", VAL_SAVE_BUTTON, 0, 0, 1, 1, savefile);
			if (stat == VAL_NO_FILE_SELECTED)
				return 0;
			
			
			break;

	}
	return 0;
}

int CVICALLBACK InstrCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int instruction, instr;
			GetCtrlVal(panel, control, &instruction);
			GetCtrlVal(panel, PulseInstP_InstNum, &instr);
			SetCtrlVal(cinst[instr], MDInstr_Instructions, instruction);

			int state = get_nd_state(cinst[instr], MDInstr_VaryInstr); // Get the current state of the instruction. 
			
			if(instruction == LOOP || instruction == LONG_DELAY && state == 0) {
				// If the state is off, we need to know if this is off because we temporarily switched out of a State = 2 instruction or otherwise, and fix accordingly.
				int color;
				GetCtrlAttribute(cinst[instr], MDInstr_VaryInstr, ATTR_ON_COLOR, &color); // Get the on color of this state.
				
				if(color == VAL_BLUE) {
					update_nd_state(cinst[instr], MDInstr_VaryInstr, 2); //State is 0.
				}
					
			} else if (state == 2) {
					// If this was previously an instruction that takes state 2, and was in state 2, change that to state 0, but leave a trace of where we were.
					update_nd_state(cinst[instr], MDInstr_VaryInstr, 0); //State is 0.
					SetCtrlAttribute(cinst[instr], MDInstr_VaryInstr, ATTR_ON_COLOR, VAL_BLUE); // Set this to be blue - if it's blue later, we'll know we were in this state.
					
			}
			
			
			
			if(instruction == 0 || instruction == 1 || instruction == 5 || instruction == 8)
			{
				SetCtrlAttribute(panel, PulseInstP_Instr_Data, ATTR_DIMMED, 1);
			} else { 
				SetCtrlAttribute(panel, PulseInstP_Instr_Data, ATTR_DIMMED, 0);
			}
			if(instruction == 2)
			{
				SetCtrlAttribute(panel, PulseInstP_Instr_Data, ATTR_MIN_VALUE, 1);
				SetCtrlVal(panel, PulseInstP_Instr_Data, 1);
			}
			else if (instruction == 7)
			{
				SetCtrlAttribute(panel, PulseInstP_Instr_Data, ATTR_MIN_VALUE, 2);
				SetCtrlVal(panel, PulseInstP_Instr_Data, 2);
			}
			else
			{
				SetCtrlAttribute(panel, PulseInstP_Instr_Data, ATTR_MIN_VALUE, 0);
				SetCtrlVal(panel, PulseInstP_Instr_Data, 0);
			}
			
		
			
			InstrDataCallback(panel, PulseInstP_Instr_Data, EVENT_COMMIT, NULL, NULL, NULL);
			break;

	}
	return 0;
}

int CVICALLBACK ChangeTUnits (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			double tunits, mintime = 100, maxtime = 21474.835, current_mintime;
			int inst, ctrl = PulseInstP_InstDelay;
			GetCtrlVal(panel, PulseInstP_InstNum, &inst);
			
			if(ctrl < 0)
				return -1;
			
			GetCtrlVal(panel, control, &tunits);
			GetCtrlAttribute(panel, ctrl, ATTR_MIN_VALUE, &current_mintime);
			
			if(current_mintime == 0) //If this is a multidimensional experiment, the minimum time is 0 to allow you to skip a step.
				mintime = 0;
			
			maxtime *= (ms/tunits);
			mintime *= (ns/tunits);
			
			SetCtrlAttribute(panel, ctrl, ATTR_MAX_VALUE, maxtime);
			SetCtrlAttribute(panel, ctrl, ATTR_MIN_VALUE, mintime);
			
			ChangeInstDelay(panel, ctrl, EVENT_COMMIT, NULL, NULL, NULL);
			
			break;

	}
	return 0;
}

int CVICALLBACK NewProgram (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			/*
			int i;

			for(i = 0; i < ninstructions; i++)
				clear_instruction(i);
			
			SetCtrlVal(Pulse_Prog_Config, PPConfig_NumDimensions, 2);
			SetCtrlVal(Pulse_Prog_Config, PPConfig_NDimensionalOn, 0);
			ToggleND(Pulse_Prog_Config, PPConfig_NDimensionalOn, EVENT_COMMIT, NULL, NULL, NULL);
			
			SetCtrlVal(Pulse_Prog_Tab, PulseProg_InstNum, 1);
			SetCtrlVal(Pulse_Prog_Config, PPConfig_PhaseCycle, 0);
			SetCtrlVal(Pulse_Prog_Config, PPConfig_PhaseCycleInstr, 1);
			SetCtrlVal(Pulse_Prog_Config, PPConfig_PhaseCycleFreq, 60.0);
			SetCtrlVal(Pulse_Prog_Config, PPConfig_NumCycles, 4);
			SetCtrlVal(Pulse_Prog_Config, PPConfig_NTransients, 1);
			PhaseCycleCallback(Pulse_Prog_Config, PPConfig_PhaseCycle, EVENT_COMMIT, NULL, NULL, NULL);
			change_number_of_instructions();
			*/
		break;
	}
	
	return 0;
}

int CVICALLBACK MoveInst (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			int i, to, from;
			
			from = -1;
			
			for (i=0; i<n_inst_disp; i++)
			{
				if(inst[i] == panel)
					from = i;
			}
			
			if (from < 0)
			{
				MessagePopup("Error", "Invalid starting panel");
				return -1;
			}
				
			GetCtrlVal(panel, control, &to);
			
			if(to >= n_inst_disp)
				to = n_inst_disp - 1;
			
			SetCtrlVal(panel, control, to);
			
			//move_instruction(to, from);
			
			break;

	}
	return 0;
}

int phase_cycle_n_transients (int panel)
{
	/*
	int nt, nc, r;
			
	GetCtrlVal(panel, PPConfig_PhaseCycle, &nc);
	if(!nc)
		return 1;

	GetCtrlVal(panel, PPConfig_NTransients, &nt);
	GetCtrlVal(panel, PPConfig_NumCycles, &nc);
			
	if(nt%nc != 0)
	{
		r = (int)((double)(nt/nc + 0.5));
		if(r == 0)
			r = nc;
		else
			r *= nc;
		SetCtrlVal(panel, PPConfig_NTransients, r);
	}
	
	return nc;
	*/
	return NULL;
}

int CVICALLBACK ChangeTransients (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			phase_cycle_n_transients(panel);
			
			break;
	}
	return 0;
}

int CVICALLBACK Change_Scan (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			int scan;
			
			GetCtrlVal(panel, control, &scan);
			SetCtrlVal(panel, Load_TTLs(ttl_trigger), scan);
			
			break;

	}
	return 0;
}

int CVICALLBACK Change_Trigger (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			int scan, previous_ttl, i, ttl_val;
			
			previous_ttl = ttl_trigger;   
			GetCtrlVal(panel, control, &ttl_trigger);

			int broken;
			GetValueFromIndex(HiddenPanel, HiddenVals_TTLBroken, ttl_trigger, &broken);
			if(broken)
			{
				SetCtrlVal(panel, control, previous_ttl);
				ttl_trigger = previous_ttl;
				MessagePopup("TTL Broken", "Sorry, the TTL you've chosen for a trigger is currently broken. Please choose a different TTL");
				return 0;
			}
			
			for(i = 0; i<ninstructions; i++)
			{
				SetCtrlAttribute(inst[i], Load_TTLs(previous_ttl), ATTR_ON_COLOR, VAL_RED);
				SetCtrlAttribute(inst[i], Load_TTLs(previous_ttl), ATTR_OFF_COLOR, VAL_BLACK);  
				SetCtrlAttribute(inst[i], Load_TTLs(previous_ttl), ATTR_CTRL_MODE, VAL_HOT);
				SetCtrlAttribute(inst[i], Load_TTLs(ttl_trigger), ATTR_ON_COLOR, VAL_BLUE);
				SetCtrlAttribute(inst[i], Load_TTLs(ttl_trigger), ATTR_OFF_COLOR, VAL_DK_GRAY);
				SetCtrlAttribute(inst[i], Load_TTLs(ttl_trigger), ATTR_CTRL_MODE, VAL_INDICATOR);
				//move_ttl(pc->inst[i], pc->TTLs, ttl_trigger, previous_ttl);
			}
			
			
			break;

	}
	return 0;
}

int CVICALLBACK ChangeDelay (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
		case EVENT_RIGHT_CLICK:

			break;
	}
	return 0;
}




int CVICALLBACK ChangePhaseKnob (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
			double phase;
			int order, chan;
			
			GetCtrlVal(panel, control, &phase);
			GetCtrlVal(panel, Spectrum_PhaseCorrectionOrder, &order);
			GetCtrlVal(panel, Spectrum_ChanPrefs, &chan);
			
			change_phase(phase, order, chan);
			break;
	}
	return 0;
}


void CVICALLBACK QuitCallbackMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	QuitCallback(panelHandle, 0, EVENT_COMMIT, NULL, NULL, NULL);
}


int change_ND_point ()
{
	/*
	int nl;
	GetNumListItems(FID, FID_CurrentDimPoint, &nl);
	
	if(nl == 0)
		return 0;
	
	int nd;
	GetNumListItems(FID, FID_Dimension, &nd);
	if(nl != nd || nd == 0)
		return -1;
	
	int dim, p;
	GetCtrlIndex(FID, FID_Dimension, &dim);
	GetCtrlVal(FID, FID_DimPoint, &p);
	
	ReplaceListItem(FID, FID_CurrentDimPoint, dim, "", p);
	SetCtrlVal(panelHandle, Load_ND_Val(dim+1), itoa(p));
	
	change_viewing_transient(FID, FID_TransientNum);
	*/
	return 0;
}

int change_viewing_transient (int panel, int control)
{
	/*
	int fidindex, fftindex;
	int fidnum, fftnum;   
	
	GetCtrlIndex(FFTSpectrum, Spectrum_TransientNum, &fftindex);
	GetCtrlIndex(FID, FID_TransientNum, &fidindex);

	GetNumListItems(FFTSpectrum, Spectrum_TransientNum, &fftnum);
	GetNumListItems(FID, FID_TransientNum, &fidnum);
			
	if(fidnum != fftnum)
		return -2;
			
	int ctrlindex;
	if(panel == FFTSpectrum && control == Spectrum_TransientNum)
	{
		ctrlindex = fftindex;
		SetCtrlIndex(FID, FID_TransientNum, fftindex);
	}
	else if(panel == FID && control == FID_TransientNum)
	{
		ctrlindex = fidindex;
		SetCtrlIndex(FFTSpectrum, Spectrum_TransientNum, fidindex);
	}
	else
		return -3;
			
	int t;
	
	GetValueFromIndex(panel, control, ctrlindex, &t);
	
	if(t < 0)
		return -4;
			
	change_fid_and_fft(t);
	*/
	return 0;
	
}

int CVICALLBACK ChangeViewingTransientNumSpec (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			change_viewing_transient(panel, control);
			break;

	}
	return 0;
}

int CVICALLBACK ChangeViewingTransientFID (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			change_viewing_transient(panel, control);
			
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
			
			load_DAQ_info();
			
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
			/*int num;
			GetCtrlVal(panel, PulseInstP_InstNum, &num);
			clear_instruction(num);
			move_instruction(ninstructions-1, num);
			GetCtrlVal(Pulse_Prog_Tab, PulseProg_InstNum, &num);
			SetCtrlVal(Pulse_Prog_Tab, PulseProg_InstNum, num-1);
			change_number_of_instructions(); */
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
			int val;
			GetCtrlVal(panel, control, &val);
			if(val)
				SetCtrlAttribute(Pulse_Prog_Config, PPConfig_NTransients, ATTR_DIMMED, 1);
			else
				SetCtrlAttribute(Pulse_Prog_Config, PPConfig_NTransients, ATTR_DIMMED, 0);
			break;

	}
	return 0;
}


int CVICALLBACK PhaseCycleCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int val, nc;
			GetCtrlVal(panel, control, &val);
			/*
			if(val)
			{
				SetCtrlAttribute(panel, PPConfig_PhaseCycleInstr, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, PPConfig_NumCycles, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, PPConfig_PhaseCycleFreq, ATTR_DIMMED, 0);
				
				nc = phase_cycle_n_transients(panel);

				SetCtrlAttribute(panel, PPConfig_NTransients, ATTR_INCR_VALUE, nc);
				
			} else {
				SetCtrlAttribute(panel, PPConfig_PhaseCycleInstr, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PPConfig_NumCycles, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PPConfig_PhaseCycleFreq, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PPConfig_NTransients, ATTR_INCR_VALUE, 1);
			}
	
			*/
			break;
	}
	return 0;
}

int CVICALLBACK ChangePhaseCycleInstr (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int val;
			GetCtrlVal(panel, control, &val);
			
			if(val > n_inst_disp-1)
				SetCtrlVal(panel, control, n_inst_disp-1);
			
			break;
	}
	return 0;
}



void CVICALLBACK LoadDataMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	load_data();
	
}

int CVICALLBACK ChangeSpectrumChannel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int num;
			
			GetCtrlAttribute(FFTSpectrum, Spectrum_Graph, ATTR_NUM_PLOTS, &num);
			
			if(num < 1)
				return -1;
											  
			int index;
			GetNumListItems(FFTSpectrum, Spectrum_Channel, &index);

			if(index == 3)
				GetCtrlIndex(FFTSpectrum, Spectrum_Channel, &index);
			else
				return -2;
						  
			int *val = malloc(sizeof(int)*3), i;
			int *on = malloc(sizeof(int)*3);
			on[0] = 0;
			on[1] = 0;
			on[2] = 0;
			on[index] = 1;
			
			for(i = 0; i<3; i++)
			{
				GetValueFromIndex(FFTSpectrum, Spectrum_Channel, i, &val[i]);
				if(val[i] < 0)
					return -3;
			}
			
			for(i = 0; i< 3; i++)
				SetPlotAttribute(FFTSpectrum, Spectrum_Graph, val[i], ATTR_TRACE_VISIBLE, on[i]);

			break;
	}
	return 0;
}
																					   

	

int CVICALLBACK ChangeInstrVary (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{		
			case EVENT_RIGHT_CLICK:
			case EVENT_LEFT_CLICK:
				// We want this to to toggle in one direction on a left click, toggle in another direction on the right click.
				// Left-click goes - Off->Red (Time increase)->Blue (Step Increase)->Off
				// Right click goes Off->Blue->Red->Off

				// Get the current state
				int state = get_nd_state(panel, control);
				
				// Get the instruction - we only want blue to be available for LOOP, JSR and LONG_DELAY
				int instr;
				GetCtrlVal(panel, MDInstr_Instructions, &instr);
				if(instr == LOOP || instr == LONG_DELAY) {
			
					// Figure out how we need to get there
					if (event == EVENT_RIGHT_CLICK) {
						state--;
					} else if(event == EVENT_LEFT_CLICK) {
						state++;
					}
				
					state = state+3;
				
					state = state%3; // Only 3 states, so it's modulo.
				
				} else { 
					// If it's normal, it's just a red/off toggle switch
					if(state == 0) {
						state = 1;
					} else {
						state = 0;
					}
				}
				
				update_nd_state(panel, control, state); 
				
				// Set the value of the control - it should be whatever we want it to be (state 0 = off, 1/2 = on)
				int val;
				if(state == 0) {
					val = 0;
				} else {
					val = 1;
				}
			
				SetCtrlVal(panel, control, val);
			
				break;
				
	}
	return 0;
}



int CVICALLBACK ToggleND (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int val, i, ib;
			GetCtrlVal(panel, control, &val);
			
			if(val)
			{
				for(i = 0; i < ninstructions; i++)
				{
					SetPanelAttribute(cinst[i], ATTR_DIMMED, 0);
					update_nd_state(cinst[i], MDInstr_VaryInstr, -1);
				}
				SetPanelAttribute(PPConfigSubPanel, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, PPConfig_NumDimensions, ATTR_DIMMED, 0);
			}
			else
			{
				SetPanelAttribute(PPConfigSubPanel, ATTR_DIMMED, 1);
				for(i = 0; i < ninstructions; i++)
				{
					SetPanelAttribute(cinst[i], ATTR_DIMMED, 1);
					GetCtrlVal(cinst[i], MDInstr_VaryInstr, &ib);
					SetCtrlVal(cinst[i], MDInstr_VaryInstr, 0);
					update_nd_state(cinst[i], MDInstr_VaryInstr, -1);  
					SetCtrlVal(cinst[i], MDInstr_VaryInstr, ib);
				}
				SetCtrlAttribute(panel, PPConfig_NumDimensions, ATTR_DIMMED, 1);
			}
			break;
	}
	return 0;
}

int change_precision (int panel, int control)
{
	double val;
	GetCtrlVal(panel, control, &val);

	control = get_t_units_pair(panel, control);
	
	if(control < 0)
		return -1;
	
	if(val == ns)
		SetCtrlAttribute(panel, control, ATTR_PRECISION, 0);
	else
		SetCtrlAttribute(panel, control, ATTR_PRECISION, 3);
	
	return 0;
}

double calculate_units (double val)
{
	if(val == 0)
		return ns;
	
	int a = log10(fabs(val));
	if(a < 3)
		return ns;
	if(a < 6)
		return us;
	if(a < 9)
		return ms;
	else
		return 1000*ms;
	
}

int get_precision (double val, int total_num) {
	// Feed this a value, then tell you how many numbers you want to display total.
	// This will give you the number of numbers to pad out after the decimal place.
	// If the number to be displayed has more digits after the decimal point than 
	// total_num, it returns 0.
	
	total_num = total_num-1; // Always need one thing before the decimal place.
	
	if(total_num <= 1)
		return 0;
	
	if(val == 0)
		return total_num;

	int a = log10(fabs(val)); // The number of digits is the base-10 log, plus 1 - since we subtracted off the 1 from total_num, we're good..
	
	if(a >= total_num) // If we're already at capacity, we need nothing after the 0.
		return 0;
	
	return total_num-a; // Subtract off the number being used by the value, that's your answer.
		
}
	

int update_final_val (int panel)
{
	/*
	double init, initu, fval, fvalu, inc, incu;
	int np;
	
	GetCtrlVal(panel, MDInstr_InitDelay, &init);
	GetCtrlVal(panel, MDInstr_InitDelayUnits, &initu);
	GetCtrlVal(panel, MDInstr_Increment, &inc);
	GetCtrlVal(panel, MDInstr_IncrementUnits, &incu);
	GetCtrlVal(panel, MDInstr_FinalDelayUnits, &fvalu);
	GetCtrlVal(panel, MDInstr_NumSteps, &np);
	
	fval = init*initu + (np-1)*(inc*incu);
	
	// fvalu = calculate_units(fval);
	fval /= fvalu;

	SetCtrlVal(panel, MDInstr_FinalDelayUnits, fvalu);
	change_precision(panel, MDInstr_FinalDelayUnits);
	SetCtrlVal(panel, MDInstr_FinalDelay, fval);
	*/
	return 0;
}

int change_increment (int panel)
{
	// Changes the increment.
	// This is always called after the final and initial units have been set appropriately.
	/*
	double inc, incu;
	
	GetCtrlVal(panel, MDInstr_IncrementUnits, &incu);
	GetCtrlVal(panel, MDInstr_Increment, &inc);
	
	inc *= incu;
//	inc = 10*((double)((int)(inc/10 + 0.5)));
	
	inc /= incu;
	
	SetCtrlVal(panel, MDInstr_IncrementUnits, incu);
	SetCtrlVal(panel, MDInstr_Increment, inc);

	if(update_final_val(panel) < 0)
	{
		double fval, fvalu, init, initu;
		int np;
		GetCtrlVal(panel, MDInstr_InitDelay, &init);
		GetCtrlVal(panel, MDInstr_InitDelayUnits, &initu);
		GetCtrlVal(panel, MDInstr_FinalDelay, &fval);
		GetCtrlVal(panel, MDInstr_FinalDelayUnits, &fvalu);
		GetCtrlVal(panel, MDInstr_NumSteps, &np);
		
		inc = (fval*fvalu - init*initu)/(np-1);
		incu = calculate_units(inc);
		inc /= incu;
		
		SetCtrlVal(panel, MDInstr_IncrementUnits, incu);
		SetCtrlVal(panel, MDInstr_Increment, inc);
	}
		
	*/
	return 0;

}

int update_dim_point (int dim, int np)
{
	int dim_buff;
	
	GetNumListItems(Pulse_Prog_Config, PPConfig_DimensionPoints, &dim_buff);
	if(dim_buff < dim)
		populate_dim_points();
	else
	{
		char c[10];
		sprintf(c, "%d", dim);
		ReplaceListItem(Pulse_Prog_Config, PPConfig_DimensionPoints, dim-1, c, np);
		GetValueFromIndex(Pulse_Prog_Config, PPConfig_DimensionPoint, dim-1, &dim_buff);
		if(dim_buff > np || (np >= 1 && dim_buff == 0))
			ReplaceListItem(Pulse_Prog_Config, PPConfig_DimensionPoint, dim-1, c, 1);
	}
	SetCtrlVal(Pulse_Prog_Config, PPConfig_DimensionPoint, np);
	
	return 0;
}

int change_num_steps (int panel)
{
	//For the specififed panel, changes the step size such that it takes np steps to get to final value.
	double init, initu, fval, fvalu, inc, incu;
	int np, dim, dim_buff, i;
	/*
	GetCtrlVal(panel, MDInstr_NumSteps, &np);
	GetCtrlVal(panel, MDInstr_Dimension, &dim);
	
	populate_dim_points();
	
	SetCtrlVal(Pulse_Prog_Config, PPConfig_DimensionPoints, np); 

	//Number of points in each covarying instruction must be the same.
	for(i = 0; i< n_inst_disp; i++)
	{
		GetNumListItems(cinst[i], MDInstr_Dimension, &dim_buff);
		if(dim_buff == 0)
			continue;
	
		GetCtrlVal(cinst[i], MDInstr_Dimension, &dim_buff);
	
		if(dim_buff == dim)
		{
			GetCtrlVal(cinst[i], MDInstr_FinalDelay, &fval);
			GetCtrlVal(cinst[i], MDInstr_FinalDelayUnits, &fvalu);
			GetCtrlVal(cinst[i], MDInstr_InitDelay, &init);
			GetCtrlVal(cinst[i], MDInstr_InitDelayUnits, &initu);
			GetCtrlVal(cinst[i], MDInstr_IncrementUnits, &incu);

			inc = (int)((fval*fvalu - init*initu)/(np-1) + 0.5);

			inc = 10*((double)((int)(inc/10 + 0.5)));
			inc /= incu;
	
			SetCtrlVal(cinst[i], MDInstr_Increment, inc);
			SetCtrlVal(cinst[i], MDInstr_NumSteps, np);
			int state = get_nd_state(cinst[i], MDInstr_VaryInstr);
			if(state == 1) {
				change_increment(cinst[i]);
			} else if (state == 2) {
				update_instr_data_nd(cinst[i], 2);
			}
		}
	}
	
	update_dim_point(dim, np);
	*/
	return 0;
}

int CVICALLBACK ChangeInstDelay (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			/*
			double val, ival, t_units;
			int instr;
			
			GetCtrlVal(panel, control, &val);
			GetCtrlVal(panel, PulseInstP_InstNum, &instr);
			GetCtrlVal(panel, PulseInstP_TimeUnits, &t_units);
			GetCtrlVal(cinst[instr], MDInstr_InitDelay, &ival);			 
			
			SetCtrlVal(cinst[instr], MDInstr_InitDelayUnits, t_units);

			change_precision(cinst[instr], MDInstr_InitDelayUnits);
			
			SetCtrlVal(cinst[instr], MDInstr_InitDelay, val);
		
			update_final_val(cinst[instr]);
			
			// If this guy's in a loop, or in a loop in a loop, and 2D is on and it's in the right state, we need to update the times appropriately.
			int loop = instr;
			for(int failsafe = 0; failsafe < ninstructions; failsafe++) {
//				loop = in_loop(loop, 1);
				if(loop >= 0) {
					int state = get_nd_state(cinst[loop], MDInstr_VaryInstr);
					if(state == 2) {
						update_instr_data_nd(cinst[loop], 2); // Updates the time calculation as well.
					}
				
				}
			}
			 */
			break;
	}
	return 0;
}

int CVICALLBACK InstrDataCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int val;
			int instr;
			
			GetCtrlVal(panel, control, &val);
			GetCtrlVal(panel, PulseInstP_InstNum, &instr);
			
			//SetCtrlVal(cinst[instr], MDInstr_Instr_Data, val);
			
			break;

	}
	return 0;
}

int CVICALLBACK ChangeITUnits (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			change_precision(panel, control);
			//ChangeInitDelay(panel, MDInstr_InitDelay, EVENT_COMMIT, NULL, NULL, NULL);
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

			change_num_steps(panel);
			
			break;
	}
	return 0;
}

int CVICALLBACK ChangeFDelay (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			 /*
			// Changes the final delay value.
			// Called by: Final Delay Callback, Final Delay Units, LoadProgram_To_Controls, 
			double diff, init, initu, fval, fvalu, inc, incu;
			int np;
			
			GetCtrlVal(panel, MDInstr_FinalDelay, &fval); // Current final delay value
			GetCtrlVal(panel, MDInstr_FinalDelayUnits, &fvalu); // Current final delay units
			GetCtrlVal(panel, MDInstr_InitDelay, &init); // Current initial delay value
			GetCtrlVal(panel, MDInstr_InitDelayUnits, &initu); // Current initial delay units
			GetCtrlVal(panel, MDInstr_NumSteps, &np); // Current number of points
			
			diff = fval*fvalu - init*initu; // What's the difference?
			if(np >= 2)
				inc = diff/(np-1);
			else
				inc = 0;
			int a;
			if(inc != 0)
				a = (int)log10(abs(inc));
			else
				a = 0;
			
			if(a < 3)
				incu = ns;
			else if(a < 6)
				incu = us;
			else if(a < 9)
				incu = ms;
			else
				incu = 1000*ms;
			
			inc /= incu; 
			
			SetCtrlVal(panel, MDInstr_Increment, inc);
			SetCtrlVal(panel, MDInstr_IncrementUnits, incu);  // Changes to the current units. Must fix this.
			change_increment(panel);
			*/
			break;
	}
	return 0;
}

int CVICALLBACK ChangeIncrement (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			change_increment(panel);
			
			break;
	}
	return 0;
}

int CVICALLBACK ChangeInitDelay (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			double val;
			double tunits;
			int instr;
			/*
			GetCtrlVal(panel, control, &val);
			GetCtrlVal(panel, MDInstr_InstrNum, &instr);
			GetCtrlVal(panel, MDInstr_InitDelayUnits, &tunits);
			
			SetCtrlVal(inst[instr], PulseInstP_TimeUnits, tunits);
			SetCtrlVal(inst[instr], PulseInstP_InstDelay, val);
			ChangeTUnits(inst[instr], PulseInstP_TimeUnits, EVENT_COMMIT, NULL, NULL, NULL);
			*/
			break;
	}
	return 0;
}

int change_nc_or_cf(int panel)
{
	int nc;
	double cf;
	/*
	GetCtrlVal(panel, PPConfig_NumCycles, &nc);
	GetCtrlVal(panel, PPConfig_PhaseCycleFreq, &cf);

	if(1000*ms/(nc*cf) < 100*ns)
	{
		nc = (int)((10*ms)/(cf*ns));
		cf = (10*ms)/(nc*ns);
		SetCtrlVal(panel, PPConfig_NumCycles, nc);
		SetCtrlVal(panel, PPConfig_PhaseCycleFreq, cf);
	}
	*/
	return 0;
}

int CVICALLBACK ChangeNumCycles (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			change_nc_or_cf(panel);
			
			int nc = phase_cycle_n_transients(panel);
			
			SetCtrlAttribute(panel, PPConfig_NTransients, ATTR_INCR_VALUE, nc);
			
			break;
	}
	return 0;
}

int CVICALLBACK ChangePhaseCycleFreq (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			change_nc_or_cf(panel);
			
			break;
	}
	return 0;
}

int CVICALLBACK NumDimensionCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			int i;
			
			for(i = 0; i<n_inst_disp; i++)
				change_dimension(cinst[i]);
			
			break;
	}
	return 0;
}

void CVICALLBACK LoadProgramMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	LoadProgram(NULL, NULL, EVENT_COMMIT, NULL,  NULL, NULL);
}

void CVICALLBACK SaveProgramMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	SaveProgram(NULL, NULL, EVENT_COMMIT, NULL, NULL, NULL);
}

void CVICALLBACK NewAcquisitionMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	/*clear_plots();
	SetCtrlAttribute(FID, FID_TransientNum, ATTR_DIMMED, 1);
	SetCtrlAttribute(FFTSpectrum, Spectrum_TransientNum, ATTR_DIMMED, 1);
	SetCtrlAttribute(FFTSpectrum, Spectrum_Channel, ATTR_DIMMED, 1);
	
	int i;
	for(i = 1; i <= 9; i++)
	{
		SetCtrlAttribute(panelHandle, Load_ND_Labels(i), ATTR_VISIBLE, 0);
		SetCtrlAttribute(panelHandle, Load_ND_Val(i), ATTR_VISIBLE, 0);
	}
	*/
			
}

void CVICALLBACK NewProgramMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	NewProgram(NULL, NULL, EVENT_COMMIT, NULL, NULL, NULL);
}

void CVICALLBACK UpdateDAQMenuCallback (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	UpdateChannels(NULL, NULL, EVENT_COMMIT, NULL, NULL, NULL);
}

int CVICALLBACK ChangeDimension (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int val, np, nl;
			GetCtrlVal(panel, control, &val);
			GetNumListItems(Pulse_Prog_Config, PPConfig_DimensionPoints, &nl);
			if(nl+1 <= --val)
				return 0;
			GetValueFromIndex(Pulse_Prog_Config, PPConfig_DimensionPoints, val, &np);
			SetCtrlVal(panel, MDInstr_NumSteps, np);
			ChangeNumSteps(panel, MDInstr_NumSteps, EVENT_COMMIT, NULL, NULL, NULL);
			break;
	}
	return 0;
}

int CVICALLBACK TestButtonCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int nl1, nl2, val1, val2, i;
			/*
			GetNumListItems(FID, FID_CurrentDimPoint, &nl1);
			GetNumListItems(FID, FID_DimPointsCompleted, &nl2);
			
			if(nl1 != 0)
				DeleteListItem(FID, FID_CurrentDimPoint, 0, -1);
			if(nl2 != 0)
				DeleteListItem(FID, FID_DimPointsCompleted, 0, -1);
			
			char *loadfile = malloc(MAX_PATHNAME_LEN);
			
			int stat = FileSelectPopup("", "*.txt", "*.txt", "Select file", VAL_LOAD_BUTTON, 0, 0, 1, 0, loadfile);
			if(stat == VAL_NO_FILE_SELECTED)
				return 0;
			
			int nd = get_num_dimensions(loadfile);
			
			if(nd < 1)
				return 0;
			
			int *points = malloc(sizeof(int)*nd);
			get_points_completed(loadfile, nd, points);
			
			for(i = 0; i < nd; i++)
			{
				InsertListItem(FID, FID_CurrentDimPoint, -1, "", 1);
				InsertListItem(FID, FID_DimPointsCompleted, -1, "", points[i]);
				printf("D %d: Point 1 of %d\n", i, points[i]); 
			}
			
			char *pathname = malloc(MAX_PATHNAME_LEN), *aname = malloc(MAX_PATHNAME_LEN);
			SplitPath(loadfile, pathname, aname, NULL);
			strcat(pathname, aname);
			printf("%s\n", pathname);
			
			SetCtrlVal(FID, FID_CurrentPathname, pathname);
			
			
			GetNumListItems(Pulse_Prog_Config, PPConfig_DimensionPoint, &nl1);
			GetNumListItems(Pulse_Prog_Config, PPConfig_DimensionPoints, &nl2);
			
			printf(errorstring, "Point: %d, Points: %d\n", nl1, nl2);
			
			if(nl1 != nl2)
				return 0;
			
			for(i = 0; i < nl2; i++)
			{
				GetValueFromIndex(Pulse_Prog_Config, PPConfig_DimensionPoint, i, &val1);
				GetValueFromIndex(Pulse_Prog_Config, PPConfig_DimensionPoints, i, &val2);
				printf("IndirectDim %d - %d of %d\n", i+1, val1, val2);
			}
				  */
			break;
			
			

	}
	return 0;
}

int getCursorValues()
{
	double x, y, x_gain, y_gain, x_off, y_off;
	
	GetGraphCursor(FFTSpectrum, Spectrum_Graph, 1, &x, &y);
	GetCtrlAttribute(FFTSpectrum, Spectrum_Graph, ATTR_XAXIS_GAIN, &x_gain);
	GetCtrlAttribute(FFTSpectrum, Spectrum_Graph, ATTR_XAXIS_OFFSET, &x_off);
	GetCtrlAttribute(FFTSpectrum, Spectrum_Graph, ATTR_YAXIS_GAIN, &y_gain);
	GetCtrlAttribute(FFTSpectrum, Spectrum_Graph, ATTR_YAXIS_OFFSET, &y_off);
	
	x = x*x_gain + x_off;
	y = y*y_gain + y_off;
	SetCtrlVal(FFTSpectrum, Spectrum_CursorX, x);
	SetCtrlVal(FFTSpectrum, Spectrum_CursorY, y);
	return 0;
}

int CVICALLBACK SpectrumCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int CursorMode;
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			/*getCursorValues();
			GetCtrlVal(FFTSpectrum, Spectrum_CursorMode, &CursorMode);
			switch(CursorMode) {
				case CM_POSITION:
				case CM_RMS:
				case CM_AVG:
					RMS_click(panel, control, eventData2, eventData1);
					break;
			}
			break;
		case EVENT_LEFT_CLICK_UP:
			getCursorValues();
			GetCtrlVal(FFTSpectrum, Spectrum_CursorMode, &CursorMode);
			if(CursorMode != CM_NO_CUR)
				RMS_click_up(panel, control, eventData2, eventData1);
			break;
		case EVENT_MOUSE_MOVE:
			GetCtrlVal(panel, Spectrum_CursorMode, &CursorMode);
			switch(CursorMode) {
				case CM_POSITION:
				case CM_RMS:
				case CM_AVG:
					RMS_mouse_move(panel, control, eventData2, eventData1);
					break;
			}
			break;
		case EVENT_COMMIT:
			getCursorValues();
			break;
		case EVENT_LEFT_DOUBLE_CLICK:
			double x, y, x_gain, y_gain, x_off, y_off;
			char *xchar = malloc(40);
	
			GetGraphCursor(FFTSpectrum, Spectrum_Graph, 1, &x, &y);
			GetCtrlAttribute(FFTSpectrum, Spectrum_Graph, ATTR_XAXIS_GAIN, &x_gain);
			GetCtrlAttribute(FFTSpectrum, Spectrum_Graph, ATTR_XAXIS_OFFSET, &x_off);
			GetCtrlAttribute(FFTSpectrum, Spectrum_Graph, ATTR_YAXIS_GAIN, &y_gain);
			GetCtrlAttribute(FFTSpectrum, Spectrum_Graph, ATTR_YAXIS_OFFSET, &y_off);
	
			sprintf(xchar, "(%d, %d)", (int)((x*x_gain + x_off)+0.5), (int)((y*y_gain + y_off)+0.5));   
			
			AddGraphAnnotation(FFTSpectrum, Spectrum_Graph, x, y, xchar, 0, -100);
			free(xchar);

			getCursorValues();
			break;
		case EVENT_RIGHT_CLICK:
			RunPopupMenu(RC_menu, RCMenus_GraphMenu, panel, eventData1, eventData2, 0, 0, 0, 0);
			break;
		
		case EVENT_KEYPRESS:
			run_hotkey(panel, control, eventData1, eventData2);*/
			break;
			
	}
	return 0;
}

void CVICALLBACK ChangeNDPointMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	
}

int RMS_click (int panel, int control, double x, double y)
{
	/*if(getMousePositionOnGraph(panel, control, &x, &y) < 0)
		return -1;

	SetCursorAttribute(panel, control, 3, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);
	SetCursorAttribute(panel, control, 4, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);

	SetGraphCursor(panel, control, 2, x, y);
	SetGraphCursor(panel, control, 3, x, y);
	SetCursorAttribute(panel, control, 2, ATTR_CURSOR_COLOR, VAL_GREEN);

	if(panel == FID)
		SetCtrlVal(panel, FID_RMS1, x);
	else if(panel == FFTSpectrum)
		SetCtrlVal(panel, Spectrum_RMS1, x);
	else
		return -2;
	*/
	return 0;
}

int cursor_click_up (int panel, int control, int mode, double x, double y) {
	/*
	if(getMousePositionOnGraph(panel, control, &x, &y) < 0)
		return -1;
	
	int RMS1 = FID_RMS1, RMS2 = FID_RMS2, bounds = FID_DisplayRMSBounds, mode_id = FID_RMSBorders;
	
	if(panel == FFTSpectrum) {
		RMS1 = Spectrum_RMS1;
		RMS2 = Spectrum_RMS2;
		bounds = Spectrum_DisplayRMSBounds;
		mode_id = Spectrum_CursorMode;
	} else if (panel != FID)
		return -2;
	
	SetGraphCursor(panel, control, 1, x, y);
	double max = x, min;
	GetGraphCursor(panel, control, 3, &min, &y);
	setup_RMS(panel, min, max);
	*/
	
	return 0;
}

int RMS_click_up (int panel, int control, double x, double y)
{
	/*
	if(getMousePositionOnGraph(panel, control, &x, &y) < 0)
		return -1;

	int RMS1 = FID_RMS1, RMS2 = FID_RMS2, bounds = FID_DisplayRMSBounds, borders = FID_RMSBorders;
	
	if(panel == FFTSpectrum) {
		RMS1 = Spectrum_RMS1;
		RMS2 = Spectrum_RMS2;
		bounds = Spectrum_DisplayRMSBounds;
		borders = Spectrum_CursorMode;
	} else if (panel != FID)
		return -2;
	
	
	SetGraphCursor(panel, control, 1, x, y);
	double max = x, min;
	GetGraphCursor(panel, control, 3, &min, &y);
	
	setup_RMS(panel, min, max);

	SetCursorAttribute(panel, control, 2, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);

	int disp;
	GetCtrlVal(panel, bounds, &disp);
	if(disp)
	{
		SetCursorAttribute(panel, control, 3, ATTR_CURSOR_COLOR, RMS_BORDER_COLOR);
		SetCursorAttribute(panel, control, 4, ATTR_CURSOR_COLOR, RMS_BORDER_COLOR);
	}

	int border_bold;
	GetCtrlAttribute(panel, borders, ATTR_LABEL_BOLD, &border_bold);
	if(border_bold)
	{
		SetCtrlVal(panel, borders, 0);
		SetCtrlAttribute(panel, borders, ATTR_LABEL_BOLD, 0);
		toggle_RMS_borders(panel, 0);
	}
	
	calculate_RMS(panel, control);
	 */
	return 0;
}

int RMS_mouse_move (int panel, int control, double x, double y)
{
	if(getMousePositionOnGraph(panel, control, &x, &y) < 0)
		return -1;

	SetGraphCursor(panel, control, 1, x, y);
	return 0;
}

int calculate_RMS (int panel, int control) {
	 /*
	double min, max;
	int plot, channel, np, RMS1, RMS2, RMSOut;

	if(panel == FID) {
		GetCtrlVal(panel, FID_ChanPrefs, &channel);
		GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(1, channel+1), &plot);
		RMS1 = FID_RMS1;
		RMS2 = FID_RMS2;
		RMSOut = FID_RMSControl;
	} else if (panel == FFTSpectrum) {
		GetCtrlVal(panel, Spectrum_ChanPrefs, &channel);
		GetCtrlIndex(panel, Spectrum_Channel, &np);
		GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(2+np, channel+1), &plot);
		RMS1 = Spectrum_RMS1;
		RMS2 = Spectrum_RMS2;
		RMSOut = Spectrum_RMSControl;
	} else 
		return -1;
	
	if(plot == -1)
		return 0;
	
	GetPlotAttribute(panel, control, plot, ATTR_NUM_POINTS, &np);
	
	if(np < 1)
		return 0;
	
	double *data = malloc(sizeof(double)*np);
	
	GetPlotAttribute(panel, control, plot, ATTR_PLOT_YDATA, data);
	
	GetCtrlVal(panel, RMS1, &min);
	GetCtrlVal(panel, RMS2, &max);
	
	double xgain, xoff;
	GetCtrlAttribute(panel, control, ATTR_XAXIS_GAIN, &xgain);
	GetCtrlAttribute(panel, control, ATTR_XAXIS_OFFSET, &xoff);
	
	min = (min - xoff)/xgain;
	max = (max - xoff)/xgain;
	
	int p1 = (int)(min + 0.5);
	int p2 = (int)(max + 0.5);
	
	//setup_RMS(panel, (double)p1, (double)p2);

	np = p2 - p1 + 1;
	double *rmsdata = malloc(sizeof(double)*np);
	for(int i = p1; i<=p2; i++)
		rmsdata[i-p1] = data[i];
	
	double rms;
	RMS(rmsdata, np, &rms);
	
	SetCtrlVal(panel, RMSOut, rms);
	*/
	return 0;
}

int setup_RMS (int panel, double rms1, double rms2)
{
	/*int control = FID_Graph, RMS1 = FID_RMS1, RMS2 = FID_RMS2;
	if(panel == FFTSpectrum)
	{
		control = Spectrum_Graph;
		RMS1 = Spectrum_RMS1;
		RMS2 = Spectrum_RMS2;
	} else if (panel != FID)
		return -1;
	
	double min, max;
	double x, y1, y2;
	GetGraphCursor(panel, control, 1, &x, &y1);
	GetGraphCursor(panel, control, 2, &x, &y2);

	if(rms1>rms2)
	{
		min = rms2;
		max = rms1;
	} else {
		min = rms1;
		max = rms2;
		x = y1;
		y1 = y2;
		y2 = x;
	}
	
	SetCursorAttribute(panel, control, 3, ATTR_CURSOR_MODE, VAL_FREE_FORM);
	SetCursorAttribute(panel, control, 4, ATTR_CURSOR_MODE, VAL_FREE_FORM);
	
	SetGraphCursor(panel, control, 3, min, y1);
	SetGraphCursor(panel, control, 4, max, y2);
	
	SetCursorAttribute(panel, control, 3, ATTR_CURSOR_MODE, VAL_SNAP_TO_POINT);
	SetCursorAttribute(panel, control, 4, ATTR_CURSOR_MODE, VAL_SNAP_TO_POINT);
	
	double xoff, xgain;
	GetCtrlAttribute(panel, control, ATTR_XAXIS_OFFSET, &xoff);
	GetCtrlAttribute(panel, control, ATTR_XAXIS_GAIN, &xgain);
	
	min = min*xgain+xoff;
	max = max*xgain+xoff;
	
	SetCtrlVal(panel, RMS1, min);
	SetCtrlVal(panel, RMS2, max);

	 */
	return 0;
}
	
		
		

int CVICALLBACK ChangeDimPoint (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int i1, i2;
			/*GetCtrlVal(FID, FID_DimPoint, &i1);
			GetCtrlVal(FFTSpectrum, Spectrum_DimPoint, &i2);
			
			if(i1 == i2)
				return 0;
			
			int val;
			GetCtrlVal(panel, control, &val);
			
			SetCtrlVal(FID, FID_DimPoint, val);
			SetCtrlVal(FFTSpectrum, Spectrum_DimPoint, val);
			
			int dim, nl;
			GetCtrlVal(FID, FID_Dimension, &dim);
			
			GetNumListItems(FID, FID_CurrentDimPoint, &nl);
			if(dim > nl-1)
				return -1;
			
			ReplaceListItem(FID, FID_CurrentDimPoint, dim, "", val);
			
			change_ND_point();
			 */
			break;
	}
	return 0;
}



int CVICALLBACK ChangeDim (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int val;
			/*
			//Make sure both controls are the same.
			GetCtrlIndex(panel, control, &val);
			SetCtrlIndex(FID, FID_Dimension, val);
			SetCtrlIndex(FFTSpectrum, Spectrum_Dimension, val);
			
			int nl1, nl2, np, nd, i;
			
			//Make sure that there isn't some crazy error.
			GetNumListItems(FID, FID_DimPoint, &nl1);
			GetNumListItems(FFTSpectrum, Spectrum_DimPoint, &nl2);
			GetNumListItems(FID, FID_DimPointsCompleted, &nd);
			if(nd <= val)
				return -1;
			
			GetValueFromIndex(FID, FID_DimPointsCompleted, val, &np);
			
			//Populate the correct number of points for the dimension
			if(nl1 > np)
				DeleteListItem(FID, FID_DimPoint, np, -1);
			else if(nl1 < np)
				for(i = nl1; i<np; i++)
					InsertListItem(FID, FID_DimPoint, -1, itoa(i+1), i+1);

			if(nl2 > np)
				DeleteListItem(FFTSpectrum, Spectrum_DimPoint, np, -1);
			else if(nl2 < np)
			{
				for(i = nl2; i<np; i++)
					InsertListItem(FFTSpectrum, Spectrum_DimPoint, -1, itoa(i+1), i+1);
			}
			
			GetNumListItems(FID, FID_CurrentDimPoint, &nl1);
			if(nl1 < nd)
				return -2;
			
			GetValueFromIndex(FID, FID_CurrentDimPoint, val, &nl1);
			SetCtrlVal(FID, FID_DimPoint, nl1);
			SetCtrlVal(FFTSpectrum, Spectrum_DimPoint, nl1);
			*/
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
		case EVENT_RIGHT_CLICK:

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

int CVICALLBACK ChangeInputMinMax (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			//Changes the hidden ring controls to the values in these controls.
			//If the hidden ring controls have somehow been changed in some non-canonical way, this will cause issues.
			
			int index;
			
			double max;
			double min;
			
			GetCtrlIndex(panel, PPConfig_MaxVals, &index);
			GetCtrlVal(panel, PPConfig_InputMin, &min);
			GetCtrlVal(panel, PPConfig_InputMax, &max);

			ReplaceListItem(panel, PPConfig_MaxVals, index, 0, max);  
			ReplaceListItem(panel, PPConfig_MinVals, index, 0, min);  
			
			break;
	}
	return 0;
}

int CVICALLBACK ChangeChannelGain (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			int index, nl;
			GetNumListItems(panel, control, &nl);
			
			if(nl < 1)
				return -1;

			GetCtrlVal(panel, control, &index);
			
			GetNumListItems(panel, PPConfig_MaxVals, &nl);
			if(index < 0 || index > nl)
				return -2;
			
			GetNumListItems(panel, PPConfig_MinVals, &nl);
			if(index < 0 || index > nl)
				return -2;
			
			double max, min;
			
			GetValueFromIndex(panel, PPConfig_MaxVals, index, &max);
			GetValueFromIndex(panel, PPConfig_MinVals, index, &min);
			
			
			SetCtrlIndex(panel, PPConfig_MaxVals, index);
			SetCtrlIndex(panel, PPConfig_MinVals, index);
			SetCtrlIndex(panel, PPConfig_AcquisitionChannel, index);
			
			SetCtrlVal(panel, PPConfig_InputMax, max);
			SetCtrlVal(panel, PPConfig_InputMin, min);
			
			break;
	}
	return 0;
}

int update_channel_gain ()
{
	//Updates the values of the PPConfig_ChannelGain ring control based on the Acquisition Channel ring control.
	int i, index = -1, nl, panel = Pulse_Prog_Config, ac = PPConfig_AcquisitionChannel, cg = PPConfig_ChannelGain;
	char *label = malloc(150);
	
	GetNumListItems(panel, ac, &nl);
	
	GetNumListItems(panel, cg, &i);

	if(i > 0)
	{
		GetCtrlIndex(panel, cg, &index);
		DeleteListItem(panel, cg, 0, -1);
	}
	
	for(i = 0; i<nl; i++)
	{
	   GetLabelFromIndex(panel, ac, i, label);
	   if(label[0] == ' ')
		   continue;
	   										   
	   InsertListItem(panel, cg, -1, &label[3], i);
	}
	
	GetNumListItems(panel, cg, &nl);    
	
	if(index > 0 && index < nl)
		SetCtrlIndex(panel, cg, index);
	else if (index < 0 && index >= nl)
		SetCtrlIndex(panel, cg, nl-1);
	
	if(nl > 1)
		SetCtrlAttribute(panel, cg, ATTR_VISIBLE, 1);
	else
		SetCtrlAttribute(panel, cg, ATTR_VISIBLE, 0);
	
	SetCtrlVal(panel, PPConfig_NumChans, nl);
	
	ChangeChannelGain(panel, cg, EVENT_COMMIT, NULL, NULL, NULL);

	return 0;
}



int CVICALLBACK ChangeAcquisitionChannel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int index, nl, nc, i;
			char *label = malloc(150), *value = malloc(150);
			
			GetNumListItems(panel, control, &nl);
			
			if(nl < 1)
				return 0;
			
			GetCtrlIndex(panel, control, &index);
			
			GetLabelFromIndex(panel, control, index, label);
			GetValueFromIndex(panel, control, index, value);
			
			GetCtrlVal(panel, PPConfig_NumChans, &nc);
			
			if(label[0] != ' ' && nc == 1)
				return 0;
			else if(label[0] == ' ')
			{
				label[0] = 149;
				ReplaceListItem(panel, control, index, label, value);
			}
			else if(label[0] != ' ' && nc > 1)
			{
				label[0] = ' ';
				ReplaceListItem(panel, control, index, label, value);
				
				for(i = 0; i<nl; i++)
				{
					GetLabelFromIndex(panel, control, i, label);
					if(label[0] !=  ' ')
					{
						SetCtrlIndex(panel, control, i);
						break;
					}
				}
				if(i >= nl)
				{
					GetLabelFromIndex(panel, control, index, label);
					label[0] = 149;
					ReplaceListItem(panel, control, index, label, value);
					SetCtrlIndex(panel, control, index);
				}
			}
			
			update_channel_gain();
			
			break;
	}
	return 0;
}

void CVICALLBACK ChangeTransientView (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int i, menuitems[3] = {MainMenu_View_TransView_ViewLatestTrans, MainMenu_View_TransView_ViewAverage, MainMenu_View_TransView_ViewNoUpdate};
	
	for(i = 0; i<3; i++)
	{
		SetMenuBarAttribute(menuBar, menuitems[i], ATTR_CHECKED, (menuitems[i] == menuItem));
		if(menuitems[i] == menuItem)
			SetCtrlIndex(HiddenPanel, HiddenVals_TransientView, i);
	}

}

int CVICALLBACK ChangeSpecChanPrefs (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			unsigned long int color;
			int i, nl, polyon, polyorder, phaseorder, chan, magplot, realplot, imagplot;
			double gain, offset, phase;
			
			GetNumListItems(panel, control, &nl);
			if(nl < 1)
			{
				SetCtrlVal(panel, Spectrum_PolySubtract, 0);
				SetCtrlVal(panel, Spectrum_PolyFitOrder, 3);
				SetCtrlVal(panel, Spectrum_PhaseKnob, 0.0);
				SetCtrlVal(panel, Spectrum_Gain, 1.0);
				SetCtrlVal(panel, Spectrum_Offset, 0.0);
				
				SetCtrlAttribute(panel, Spectrum_PolySubtract, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, Spectrum_PolyFitOrder, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, Spectrum_PhaseKnob, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, Spectrum_Gain, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, Spectrum_Offset, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, Spectrum_Channel, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, Spectrum_ChanColor, ATTR_DIMMED, 1);   
				break;
				
			}
			
			//Which channel? What is the current phase order?
			GetCtrlVal(panel, control, &chan);
			GetCtrlVal(panel, Spectrum_PhaseCorrectionOrder, &phaseorder);
			
			//Get all the values from the super secret hidden panel
			GetValueFromIndex(HiddenPanel, HiddenVals_PolySubtractOnOffRing, chan, &polyon);
			GetValueFromIndex(HiddenPanel, HiddenVals_PolyOrderValues, chan, &polyorder);
			GetValueFromIndex(HiddenPanel, HiddenVals_SpectrumChanColor, chan, &color);
			GetTableCellVal(HiddenPanel, HiddenVals_PhaseCorrectionValues, MakePoint(phaseorder, chan+1), &phase);
			GetTableCellVal(HiddenPanel, HiddenVals_ChannelGains, MakePoint(2, chan+1), &gain);
			GetTableCellVal(HiddenPanel, HiddenVals_ChannelOffsets, MakePoint(2, chan+1), &offset);
			GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(2, chan+1), &realplot);
			GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(3, chan+1), &imagplot);
			GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(4, chan+1), &magplot);

			//Set up the panels
			SetCtrlVal(panel, Spectrum_PolySubtract, polyon);
			SetCtrlVal(panel, Spectrum_PolyFitOrder, polyorder);
			SetCtrlVal(panel, Spectrum_PhaseKnob, phase);
			SetCtrlVal(panel, Spectrum_Gain, gain);
			SetCtrlVal(panel, Spectrum_Offset, offset);
			SetCtrlVal(panel, Spectrum_ChanColor, color);
			
			SetCtrlAttribute(panel, Spectrum_PolySubtract, ATTR_DIMMED, 0);
			SetCtrlAttribute(panel, Spectrum_PolyFitOrder, ATTR_DIMMED, 0);
			SetCtrlAttribute(panel, Spectrum_PhaseKnob, ATTR_DIMMED, 0);
			SetCtrlAttribute(panel, Spectrum_Gain, ATTR_DIMMED, 0);
			SetCtrlAttribute(panel, Spectrum_Offset, ATTR_DIMMED, 0);
			SetCtrlAttribute(panel, Spectrum_ChanColor, ATTR_DIMMED, 0);   
			
			if(realplot >= 0)
				SetCtrlAttribute(panel, Spectrum_Channel, ATTR_DIMMED, 0);
				
			
			ReplaceListItem(panel, Spectrum_Channel, 0, NULL, realplot);
			ReplaceListItem(panel, Spectrum_Channel, 1, NULL, imagplot);
			ReplaceListItem(panel, Spectrum_Channel, 2, NULL, magplot);

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
			unsigned long int color;
			int i, nl, polyon, polyorder, chan;
			double gain, offset;
			
			GetNumListItems(panel, control, &nl);
			if(nl < 1)
			{
				SetCtrlVal(panel, FID_PolySubtract, 0);
				SetCtrlVal(panel, FID_PolyFitOrder, 3);
				SetCtrlVal(panel, FID_Gain, 1.0);
				SetCtrlVal(panel, FID_Offset, 0.0);
				
				SetCtrlAttribute(panel, FID_PolySubtract, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, FID_PolyFitOrder, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, FID_Gain, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, FID_Offset, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, FID_ChanColor, ATTR_DIMMED, 1);
				
				break;
			}
				
			//Which channel?
			GetCtrlVal(panel, control, &chan);

			//Get all the values from the super secret hidden panel
			GetValueFromIndex(HiddenPanel, HiddenVals_PolySubtractOnOffRing, chan, &polyon);
			GetValueFromIndex(HiddenPanel, HiddenVals_PolyOrderValues, chan, &polyorder);
			GetValueFromIndex(HiddenPanel, HiddenVals_FIDChanColor, chan, &color);
			GetTableCellVal(HiddenPanel, HiddenVals_ChannelGains, MakePoint(1, chan+1), &gain);
			GetTableCellVal(HiddenPanel, HiddenVals_ChannelOffsets, MakePoint(1, chan+1), &offset);
			
			//Set up the panels
			SetCtrlVal(panel, FID_PolySubtract, polyon);
			SetCtrlVal(panel, FID_PolyFitOrder, polyorder);
			SetCtrlVal(panel, FID_Gain, gain);
			SetCtrlVal(panel, FID_Offset, offset);
			SetCtrlVal(panel, FID_ChanColor, color);
			
			SetCtrlAttribute(panel, FID_PolySubtract, ATTR_DIMMED, 0);
			SetCtrlAttribute(panel, FID_PolyFitOrder, ATTR_DIMMED, 0);
			SetCtrlAttribute(panel, FID_Gain, ATTR_DIMMED, 0);
			SetCtrlAttribute(panel, FID_Offset, ATTR_DIMMED, 0);
			SetCtrlAttribute(panel, FID_ChanColor, ATTR_DIMMED, 0);   

			
			break;
	}
	return 0;
}

int change_gain(int panel, int control, int chan, int fid_or_fft)
{
	//FID = 0, FFT = 1
	int np, plot, i, j, err = 0;
	double gain, ngain, ogain, *data;
	
	GetCtrlVal(panel, control, &gain);
	
	GetTableCellVal(HiddenPanel, HiddenVals_ChannelGains, MakePoint(fid_or_fft+1, chan+1), &ogain);

	ngain = gain;
	gain /= ogain;
	
	SetTableCellVal(HiddenPanel, HiddenVals_ChannelGains, MakePoint(fid_or_fft+1, chan+1), ngain);
	
	
	if(!fid_or_fft)
	{
		GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(1, chan+1), &plot);
		if(plot < 0)
			return 0;
		GetPlotAttribute(panel, FID_Graph, plot, ATTR_NUM_POINTS, &np);
		
		if(np < 1)
			return -1;
		
		data = malloc(sizeof(double)*np);
		GetPlotAttribute(panel, FID_Graph, plot, ATTR_PLOT_YDATA, data);

		for(i = 0; i<np; i++)
			data[i] *= gain;
		
		SetPlotAttribute(panel, FID_Graph, plot, ATTR_PLOT_YDATA, data);
	} else {
		for(j = 2; j<5; j++)
		{
			GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(j, chan+1), &plot);
			
			if(plot < 0)
				continue;
			
			GetPlotAttribute(panel, Spectrum_Graph, plot, ATTR_NUM_POINTS, &np);
		
			if(np < 1)
			{
				err -= (int)(pow(2, (j-2)));
				continue;
			}

			data = malloc(sizeof(double)*np);
			GetPlotAttribute(panel, Spectrum_Graph, plot, ATTR_PLOT_YDATA, data);

			for(i = 0; i<np; i++)
				data[i] *= gain;
		
			SetPlotAttribute(panel, Spectrum_Graph, plot, ATTR_PLOT_YDATA, data);
			free(data);
		}
	}
	
	return err;
}

int change_offset(int panel, int control, int chan, int fid_or_fft)
{
	//FID = 0, FFT = 1
	int np, plot, i, j, err = 0;
	double offset, noffset, ooffset, *data;
	
	GetCtrlVal(panel, control, &offset);
	
	GetTableCellVal(HiddenPanel, HiddenVals_ChannelOffsets, MakePoint(fid_or_fft+1, chan+1), &ooffset);

	noffset = offset;
	offset -= ooffset;
	if(offset == 0)
		return 0;
	
	SetTableCellVal(HiddenPanel, HiddenVals_ChannelOffsets, MakePoint(fid_or_fft+1, chan+1), noffset);
	
	if(!fid_or_fft)
	{
		GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(1, chan+1), &plot);
		if(plot < 0)
			return 0;
		
		GetPlotAttribute(panel, FID_Graph, plot, ATTR_NUM_POINTS, &np);
		
		if(np < 1)
			return -1;
		
		data = malloc(sizeof(double)*np);
		GetPlotAttribute(panel, FID_Graph, plot, ATTR_PLOT_YDATA, data);

		for(i = 0; i<np; i++)
			data[i] += offset;
		
		SetPlotAttribute(panel, FID_Graph, plot, ATTR_PLOT_YDATA, data);
		RefreshGraph(panel, Spectrum_Graph);
	} else {
		for(j = 2; j<5; j++)
		{
			GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(j, chan+1), &plot);
			if(plot < 0)
				continue;
			GetPlotAttribute(panel, Spectrum_Graph, plot, ATTR_NUM_POINTS, &np);
		
			if(np < 1)
			{
				err -= (int)(pow(2, (j-2)));
				continue;
			}

			data = malloc(sizeof(double)*np);
			GetPlotAttribute(panel, Spectrum_Graph, plot, ATTR_PLOT_YDATA, data);

			for(i = 0; i<np; i++)
				data[i] += offset;
		
			SetPlotAttribute(panel, Spectrum_Graph, plot, ATTR_PLOT_YDATA, data);
			RefreshGraph(panel, Spectrum_Graph);
		}
	}
	
	return err;
}

int CVICALLBACK ChangeFIDGain (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int chan;
			double gain;
			GetCtrlVal(panel, FID_ChanPrefs, &chan);
			
			change_gain(panel, control, chan, 0);
			
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
			int chan;
			double offset;
			
			GetCtrlVal(panel, FID_ChanPrefs, &chan);
		
			change_offset(panel, control, chan, 0);
			
			break;
	}
	return 0;
}

int change_poly(int chan)
{
	int order, on, cd;
	int nl;
	
	GetValueFromIndex(HiddenPanel, HiddenVals_PolyOrderValues, chan, &order);
	GetValueFromIndex(HiddenPanel, HiddenVals_PolySubtractOnOffRing, chan, &on);
	
	GetNumListItems(FID, FID_ChanPrefs, &nl);
	if(nl > 0)
		GetCtrlVal(FID, FID_ChanPrefs, &cd);
	else
		cd = -1;

	if(cd == chan)
	{
		SetCtrlVal(FID, FID_PolyFitOrder, order);
		SetCtrlVal(FID, FID_PolySubtract, on);
	}
	
	GetNumListItems(FFTSpectrum, Spectrum_ChanPrefs, &nl);
	if(nl > 0)
		GetCtrlVal(FFTSpectrum, Spectrum_ChanPrefs, &cd);
	else
		cd = -1;	if(cd == chan)
	{
		SetCtrlVal(FFTSpectrum, Spectrum_PolyFitOrder, order);
		SetCtrlVal(FFTSpectrum, Spectrum_PolySubtract, on);
	}
	
	return 0;
}
	


int CVICALLBACK ChangePolyFitOrder (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			/*
			//Changes the order of the polynomial fit. FID and FFT are tied together.
			int nl, i, chan, t=-1;
			int order, current_chan;
			
			if(panel == FFTSpectrum)
				current_chan = Spectrum_ChanPrefs;
			else
				current_chan = FID_ChanPrefs;
			
			
			GetCtrlVal(panel, control, &order);
			GetCtrlVal(panel, current_chan, &chan);
			
			GetNumListItems(HiddenPanel, HiddenVals_PolyOrderValues, &nl);
			if(chan >= nl)
				return -3;
			
			ReplaceListItem(HiddenPanel, HiddenVals_PolyOrderValues, chan, NULL, order);
			
			change_poly(chan);
			
			if(panel == FFTSpectrum)
			{
				GetNumListItems(panel, Spectrum_TransientNum, &nl);
				if(nl > 0)
					GetCtrlVal(panel, Spectrum_TransientNum, &t);
			} else {
				GetNumListItems(panel, FID_TransientNum, &nl);
				if(nl > 0)
					GetCtrlVal(panel, FID_TransientNum, &t);
			}

			if(t >= 0)
				change_fid_and_fft(t);
			*/
			break;
	}
	return 0;
}

int CVICALLBACK ChangePolySubtract (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			//Changes whether or not the data plotted will be fit to a polynomial before plotting. FID and FFT are tied together.
			/*
			int nl, i, chan, t = -1;
			int on, current_chan;
			
			if(panel == FFTSpectrum)
				current_chan = Spectrum_ChanPrefs;
			else if(panel == FID)
				current_chan = FID_ChanPrefs;
			else
				return -1;
			
			GetCtrlVal(panel, control, &on);
			GetNumListItems(panel, current_chan, &nl);
			if(nl > 0)
				GetCtrlVal(panel, current_chan, &chan);
			
			GetNumListItems(HiddenPanel, HiddenVals_PolySubtractOnOffRing, &nl);
			if(chan >= nl)
				return -3;
			
			ReplaceListItem(HiddenPanel, HiddenVals_PolySubtractOnOffRing, chan, NULL, on);
			
			change_poly(chan);
			
			if(panel == FFTSpectrum)
			{
				GetNumListItems(panel, Spectrum_TransientNum, &nl);
				if(nl > 0)
					GetCtrlVal(panel, Spectrum_TransientNum, &t);
			} else {
				GetNumListItems(panel, FID_TransientNum, &nl);
				if(nl > 0)
					GetCtrlVal(panel, FID_TransientNum, &t);
			}

			if(t >= 0)
				change_fid_and_fft(t);
 			*/
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
			
			int chan;
			double gain;
			GetCtrlVal(panel, FID_ChanPrefs, &chan);
			
			change_gain(panel, control, chan, 1);
			
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
			int chan;
			double offset;
			
			GetCtrlVal(panel, Spectrum_ChanPrefs, &chan);

			change_offset(panel, control, chan, 1);
			
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
			
			change_fid_or_fft_chan(panel, control, 1);
			
			break;
	}
	return 0;
}

int change_fid_or_fft_chan (int panel, int control, int fid_or_fft)
{
	//Updates the chanprefs in response to changes in which channels are on.
	//FID = 0
	//FFT = 1
	/*
	int graph, chanprefs, transients; 
	
	if(fid_or_fft == 0)
	{
		graph = FID_Graph;
		chanprefs = FID_ChanPrefs;
		transients = FID_TransientNum;
	} else if (fid_or_fft == 1)
	{
		graph = Spectrum_Graph;
		chanprefs = Spectrum_ChanPrefs;
		transients = Spectrum_TransientNum;
	} else
		return -1;
	
	int i, on, nc, current_chan, cc, j;
	char *chanlabel = malloc(20);

	GetNumListItems(panel, chanprefs, &nc);
	
	GetCtrlVal(panel, control, &on);
	
	if(nc > 0)
	{
		GetCtrlVal(panel, chanprefs, &current_chan);
		DeleteListItem(panel, chanprefs, 0, -1);
	} else
		current_chan = 0;
	
	nc = 0;
	int k = 0;
	for(i = 0; i<8; i++)
	{
		if(channel_control(i, fid_or_fft, &j))
			nc++;

		if(j == control)
			cc = i;
	}

	if(nc <= 1)
		SetCtrlAttribute(panel, chanprefs, ATTR_VISIBLE, 0);
	else
		SetCtrlAttribute(panel, chanprefs, ATTR_VISIBLE, 1);

	j = 0;
	for(i = 0; i<nc; i++)
	{
		while(j < 8)
		{
			if(channel_control(j++, fid_or_fft, NULL))
				break;
		}
		sprintf(chanlabel, "Chan %d", j);
		InsertListItem(panel, chanprefs, -1, chanlabel, j-1);
	}

	if(current_chan == cc && nc >= 1)
		SetCtrlVal(panel, chanprefs, cc);
	
	if(!fid_or_fft)
		ChangeFIDChanPrefs(panel, FID_ChanPrefs, EVENT_COMMIT, NULL, NULL, NULL);
	else
		ChangeSpecChanPrefs(panel, Spectrum_ChanPrefs, EVENT_COMMIT, NULL, NULL, NULL); 	
	 

	int currentplot;
	if(!fid_or_fft)
		GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(1, cc+1), &currentplot);
	else
	{
		GetCtrlIndex(FFTSpectrum, Spectrum_Channel, &j);
		GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(j+2, cc+1), &currentplot);
	}

	if(currentplot < 0 && currentplot != -2)
		return 0;
	else if (currentplot == -2)
	{
		int t, nl;
		GetNumListItems(panel, transients, &nl);
		if(nl >= 1)
			GetCtrlIndex(panel, transients, &t);
		else
			return -1;
		
		return change_fid_and_fft(t);
	}
	
	if(fid_or_fft && !on) 
	{
		for(i = 2; i<=4; i++)
		{
			GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(i, cc+1), &currentplot);
			if(currentplot >=0)
				SetPlotAttribute(panel, graph, currentplot, ATTR_TRACE_VISIBLE, 0);
		}
	} else
		SetPlotAttribute(panel, graph, currentplot, ATTR_TRACE_VISIBLE, on);
	
	RefreshGraph(panel, graph);
	*/
	return 0;
}
	
		

int CVICALLBACK ChangeFIDChannel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			change_fid_or_fft_chan(panel, control, 0);
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
			int chan, order;
			double phase;
			
			GetCtrlVal(panel, control, &order);
			GetCtrlVal(panel, Spectrum_ChanPrefs, &chan);
			
			GetTableCellVal(HiddenPanel, HiddenVals_PhaseCorrectionValues, MakePoint(chan+1, order), &phase);

			SetCtrlVal(panel, Spectrum_PhaseKnob, phase);
			
			break;
	}
	return 0;
}



 
void CVICALLBACK SaveConfigToFile (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	char *filename = malloc(MAX_PATHNAME_LEN);

	int val = FileSelectPopup("", "*.bin", "*.bin", "Save Configuration", VAL_SAVE_BUTTON, 0, 0, 1, 1, filename);
	
	if(val != VAL_NO_FILE_SELECTED)
		save_configuration_to_file(filename);

}
	
	
void CVICALLBACK LoadConfigurationFromFile (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	char *filename = malloc(MAX_PATHNAME_LEN);

	int val = FileSelectPopup("", "*.bin", "*.bin", "Load Configuration", VAL_LOAD_BUTTON, 0, 0, 1, 1, filename);
	
	if(val != VAL_NO_FILE_SELECTED)
		load_configuration_from_file(filename);
}

int CVICALLBACK ShowHidden (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			DisplayPanel(HiddenPanel);
			break;

	}
	return 0;
}

int CVICALLBACK ControlHidden (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			int i;
			GetActiveTreeItem(panel, control, &i);
			
			printf("%d\n", i);
			
			
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
			unsigned long int color;
			int chan, nl;
			
			GetCtrlVal(panel, control, &color);
			
			GetNumListItems(panel, Spectrum_ChanPrefs, &nl);
			if(nl <= 0)
				break;
			
			GetCtrlIndex(panel, Spectrum_ChanPrefs, &chan);
			
			if(chan >= 8)
				break;
			
			ReplaceListItem(HiddenPanel, HiddenVals_SpectrumChanColor, chan, NULL, color);
			
			//See if there's something plotted, if so, change the color
			int plot;
			for(int i = 2; i<=4; i++)
			{
				GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(i, chan+1), &plot);
				if(plot >= 0)
					SetPlotAttribute(FFTSpectrum, Spectrum_Graph, plot, ATTR_TRACE_COLOR, color);
			}
			
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
			unsigned long int color;
			int chan, nl;
			
			//Get the color
			GetCtrlVal(panel, control, &color);
			
			GetNumListItems(panel, FID_ChanPrefs, &nl);
			if(nl <= 0)
				break;
			
			GetCtrlIndex(panel, FID_ChanPrefs, &chan);
			
			if(chan >= 8)
				break;
			
			//Update the hidden panel
			ReplaceListItem(HiddenPanel, HiddenVals_FIDChanColor, chan, NULL, color);
			
			//See if there's something plotted, if so, change the color
			int plot;
			GetTableCellVal(HiddenPanel, HiddenVals_PlotIDs, MakePoint(1, chan+1), &plot);
			if(plot >= 0)
				SetPlotAttribute(FID, FID_Graph, plot, ATTR_TRACE_COLOR, color);
			
			break;
	}
	return 0;
}

int CVICALLBACK QuitGraph (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			DiscardPanel(panel);
			
		break;
	}
	return 0;
}

void CVICALLBACK ViewProgramChart (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	//Shows the program as a chart.
	/*
	PPROGRAM *p = malloc(sizeof(PPROGRAM));
	p = get_current_program();
	
	int i, j, first = 0, nc = 0, ni = p->n_inst;
	int **flags = malloc(ni*sizeof(int *)), *con = malloc(sizeof(int)*24);
	for(i = 0; i<ni; i++)
		flags[i] = malloc(sizeof(int)*24);
	
	for(i = 0; i<24; i++)
		con[i] = 0;
	
	for(i = 0; i<ni; i++)
	{
		for(j = 0; j<24; j++)
		{
			if(p->flags[i] & (int)pow(2, j))
			{
				flags[i][j] = 1;
				if(!con[j])
				{
					con[j] = 1;
					nc++;
				}
			}
			else
				flags[i][j] = 0;
		}
	}
	
	//con is the list of channels on, flags[][] is a 2D array with the instructions in it in boolean form, nc is the number of channels on.
	//nflags will be an array that only contains the channels that are on.
	int **nflags = malloc(ni*sizeof(int *));
	int *chans = malloc(ni*sizeof(int));
	j = 0;
	for(i = 0; i<24; i++)
		if(con[i])
			chans[j++] = i;

	free(con);
	
	for(i = 0; i<ni; i++)
	{
		nflags[i] = malloc(sizeof(int)*nc);
		for(j = 0; j<nc; j++)
			nflags[i][j] = flags[i][chans[j]];
	}
	
	free(flags);
	
	//Now the display as a function of time.
	double ltime, time;
	int dec, gct = 1000, *decs = malloc(sizeof(int)*ni);
	for(i = 0; i<ni; i++)
	{
		time = p->instruction_time[i];
		ltime = log(time);
		dec = 0;
		
		while(((double)(int)(time/10)) == (time/10) && dec <= (int)ltime)
		{
			dec++;
			time /= 10;
		}
		
		if(gct > dec)
			gct = dec;
	}
	
	//Now gct is the smallest multiple of 10 common to all instructions. Now we need to get the number of steps in the program.
	dec = 0;
	for(i = 0; i<ni; i++)
	{
		time = p->instruction_time[i];
//		time /= gct*10;
//		decs[i] = (int)time;
		decs[i] = (int)log((double)time);
		dec += decs[i];
	}
	
	//Now an array of size nc x dec.
	int k, *out = malloc(sizeof(int)*dec*nc);
	gct = 0;
	
	for(i = 0; i<ni; i++)
	{
		for(j = 0; j<decs[i]; j++)
		{
			for(k = 0; k<nc; k++)
			{
				out[gct++] = nflags[i][k];
		                                                                                                       	}
		}
	}
	
	int pan = NewPanel(0, "Pulse Program", 250, 200, 500, 600);
	int ctr = NewCtrl(pan, CTRL_DIGITAL_GRAPH, "", 0, 0);
	SetCtrlAttribute(pan, ctr, ATTR_BORDER_VISIBLE, 0);
	SetCtrlAttribute(pan, ctr, ATTR_HEIGHT, 500);
	SetCtrlAttribute(pan, ctr, ATTR_WIDTH, 600);

	int quitbut = NewCtrl(pan, CTRL_SQUARE_COMMAND_BUTTON, "Quit", 400, 600);
	SetCtrlAttribute(pan, quitbut, ATTR_VISIBLE, 0);
	SetPanelAttribute(pan, ATTR_CLOSE_CTRL, quitbut);
	
	InstallCtrlCallback(pan, quitbut, QuitGraph, NULL);
	
	PlotDigitalLines(pan, ctr, out, dec, VAL_INTEGER, nc);
	
	DisplayPanel(pan);
	
	*/

}

int run_hotkey(int panel, int control, int eventData1, int eventData2)
{
	//Sets up a series of hotkeys for the graph controls.
	switch (eventData1)
	{
		case (VAL_MENUKEY_MODIFIER | VAL_UP_ARROW_VKEY):
		case (VAL_SHIFT_AND_MENUKEY | '+'):
			zoom_graph(panel, control, 1, MC_XYAXIS);
		break;
		
		case (VAL_MENUKEY_MODIFIER | VAL_DOWN_ARROW_VKEY):
		case (VAL_MENUKEY_MODIFIER | '='):
			zoom_graph(panel, control, 0, MC_XYAXIS);
			break;
		
		case (VAL_SHIFT_MODIFIER | VAL_RIGHT_ARROW_VKEY):
			pan_graph(panel, control, MC_RIGHT);
			break;
			
		case (VAL_SHIFT_MODIFIER | VAL_LEFT_ARROW_VKEY):
			pan_graph(panel, control, MC_LEFT);
			break;
		
		case (VAL_SHIFT_MODIFIER | VAL_UP_ARROW_VKEY):
			pan_graph(panel, control, MC_UP);
			break;
			
		case (VAL_SHIFT_MODIFIER | VAL_DOWN_ARROW_VKEY):
			pan_graph(panel, control, MC_DOWN);
			break;
		
		case (VAL_SHIFT_AND_MENUKEY | 'H'):
			fit_graph(panel, control, MC_XAXIS);
			break;
		
		case (VAL_SHIFT_AND_MENUKEY | 'V'):
			fit_graph(panel, control, MC_YAXIS);
			break;
		
		case (VAL_SHIFT_AND_MENUKEY | VAL_UP_ARROW_VKEY):
			zoom_graph(panel, control, 1, MC_YAXIS);
			break;
		
		case (VAL_SHIFT_AND_MENUKEY |  VAL_DOWN_ARROW_VKEY):
			zoom_graph(panel, control, 0, MC_YAXIS);
			break;
		
		case (VAL_SHIFT_AND_MENUKEY | VAL_RIGHT_ARROW_VKEY):
			zoom_graph(panel, control, 1, MC_XAXIS);
			break;
		
		case (VAL_SHIFT_AND_MENUKEY | VAL_LEFT_ARROW_VKEY):
			zoom_graph(panel, control, 0, MC_XAXIS);
			break;
	
	}
	return 0;	
}

int CVICALLBACK FIDGraphClick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		int RMSBorder;
		case EVENT_LEFT_CLICK:
			/*
			GetCtrlVal(panel, FID_RMSBorders, &RMSBorder);
			if(RMSBorder)
				RMS_click(panel, control, eventData2, eventData1);
			break;
		
		case EVENT_LEFT_CLICK_UP:
			GetCtrlVal(panel, FID_RMSBorders, &RMSBorder);
			if(RMSBorder)
				RMS_click_up(panel, control, eventData2, eventData1);
			break;
		case EVENT_MOUSE_MOVE:
				GetCtrlVal(panel, FID_RMSBorders, &RMSBorder);
				if(RMSBorder)
					RMS_mouse_move(panel, control, eventData2, eventData1);
				break;
		case EVENT_RIGHT_CLICK:
			RunPopupMenu(RC_menu, RCMenus_GraphMenu, panel, eventData1, eventData2, 0, 0, 0, 0);
			break;
		case EVENT_KEYPRESS:
			run_hotkey(panel, control, eventData1, eventData2);
			*/
			break;
			
			
	}
	return 0;
}

void CVICALLBACK AutoscalingOnOff (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int xy = -1, control = FID_Graph;
	if(panel == FFTSpectrum)
		control = Spectrum_Graph;
	
	if(menuItem == RCMenus_GraphMenu_AutoScaling)
		xy = MC_XYAXIS;
	else if(menuItem == RCMenus_GraphMenu_FitHorizontally)
		xy = MC_XAXIS;
	else if(menuItem == RCMenus_GraphMenu_FitGraphVertically)
		xy = MC_YAXIS;
	
	if(xy >= 0)
		fit_graph(panel, control, xy);
}

int zoom_graph (int panel, int control, int in, int xy)
{
	//Give this a panel and a control and tell it to zoom in or out (in = 0 -> zoom out, in = 1 -> zoom in)
	//xy = MC_XAXIS gives you zoom on the x, xy = MC_YAXIS gives you zoom on y only, xy = MC_XYAXIS gives zoom on both x and y.
	int xmode, ymode;
	double xmin, xmax, ymin, ymax, min, max;
	
	GetAxisScalingMode(panel, control, VAL_BOTTOM_XAXIS, &xmode, &xmin, &xmax);
	GetAxisScalingMode(panel, control, VAL_LEFT_YAXIS, &ymode, &ymin, &ymax);
	
	if(xy == MC_XAXIS || xy == MC_XYAXIS)
	{
		max = xmax - xmin;
		min = (xmax + xmin)/2;
		max += (0.1-0.2*in)*max;
		max /= 2;
		xmax = min+max;
		xmin = min-max;
	}
	
	if(xy == MC_YAXIS || xy == MC_XYAXIS)
	{
		max = ymax - ymin;
		min = (ymax + ymin)/2;
		max += (0.1-0.2*in)*max;
		max /= 2;
		ymax = min+max;
		ymin = min-max;
	}
	
	SetAxisScalingMode(panel, control, VAL_BOTTOM_XAXIS, VAL_MANUAL, xmin, xmax);
	SetAxisScalingMode(panel, control, VAL_LEFT_YAXIS, VAL_MANUAL, ymin, ymax);
	
	RefreshGraph(panel, control);

	return 0;
}

int pan_graph (int panel, int control, int dir)
{
	int xmode;
	double xmin, xmax, disp, diff, axis;
	
	if(dir == MC_LEFT || dir == MC_RIGHT)
		axis = VAL_BOTTOM_XAXIS;
	else if(dir == MC_UP || dir == MC_DOWN)
		axis = VAL_LEFT_YAXIS;
	else
		return -1;
	
	GetAxisScalingMode(panel, control, axis, &xmode, &xmin, &xmax);
	
	if(dir == MC_LEFT || dir == MC_DOWN)
		disp = -0.05;
	else
		disp = 0.05;

	diff = xmax - xmin;
	
	xmin += disp*diff;
	xmax += disp*diff;

	SetAxisScalingMode(panel, control, axis, VAL_MANUAL, xmin, xmax);
	
	RefreshGraph(panel, control);

	return 0;
}

int fit_graph (int panel, int control, int xy)
{
	int xmode, ymode, val;
	double xmin, xmax, ymin, ymax;
	
	GetAxisScalingMode(panel, control, VAL_BOTTOM_XAXIS, &xmode, NULL, NULL);
	GetAxisScalingMode(panel, control, VAL_LEFT_YAXIS, &ymode, NULL, NULL);

	if(xy == MC_XAXIS || xy == MC_XYAXIS)
		SetAxisScalingMode(panel, control, VAL_BOTTOM_XAXIS, VAL_AUTOSCALE, NULL, NULL);
	
	if(xy == MC_YAXIS || xy == MC_XYAXIS)
		SetAxisScalingMode(panel, control, VAL_LEFT_YAXIS, VAL_AUTOSCALE, NULL, NULL);
	
	RefreshGraph(panel, control);
	
	GetAxisScalingMode(panel, control, VAL_BOTTOM_XAXIS, &val, &xmin, &xmax);
	GetAxisScalingMode(panel, control, VAL_LEFT_YAXIS, &val, &ymin, &ymax);
	
	SetAxisScalingMode(panel, control, VAL_BOTTOM_XAXIS, xmode, xmin, xmax);
	SetAxisScalingMode(panel, control, VAL_LEFT_YAXIS, ymode, ymin, ymax);
	
	return 0;
}
	

void CVICALLBACK ZoomGraph (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int in = 1;
	if(menuItem == RCMenus_GraphMenu_ZoomGraphOut)
		in = 0;
	
	int control = FID_Graph;
	if(panel == FFTSpectrum)
		control = Spectrum_Graph;
	
	zoom_graph(panel, control, in, MC_XYAXIS);

}

void CVICALLBACK PanGraph (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	
	int dir;
	int control = FID_Graph;
	if(panel == FFTSpectrum)
		control = Spectrum_Graph;
	
	if(menuItem == RCMenus_GraphMenu_PanLeft)
		dir = MC_LEFT;
	else if(menuItem == RCMenus_GraphMenu_PanRight)
		dir = MC_RIGHT;
	else if(menuItem == RCMenus_GraphMenu_PanUp)
		dir = MC_UP;
	else if(menuItem == RCMenus_GraphMenu_PanDown)
		dir = MC_DOWN;
	else
		dir = -1;

	if (dir >= 0)
		pan_graph(panel, control, dir);
	
	
}

int CVICALLBACK ColorVal (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			unsigned long int color;
			int i;
			
			GetCtrlVal(panel, control, &color);
			GetCtrlIndex(panel, control, &i);
			
			
			printf("Channel %d: %ld\n", i, color);
			break;
	}
	return 0;
}


int ttls_in_use (PPROGRAM *p, int *ttls)
{
	/*
	//Give this an empty array of size 24 and a PPROGRAM and it will tell you if a given TTL is in use or not.
	if(p == NULL)
		return -1;
	
	int i, j;
	
	for(i = 0; i<24; i++)
	{
		ttls[i] = 0;
		for(j = 0; j<p->n_inst; j++)
		{
			if(p->flags[j] & (int)pow(2, i))
			{
				ttls[i] = 1;
				break;
			}
		}
	}
	*/
	return 0;
}
	

int setup_broken_ttls ()
{
	
	/*
	int i, j, fc, *bttls = malloc(sizeof(int)*24), *ttls = malloc(sizeof(int)*24);
	PPROGRAM *p = malloc(sizeof(PPROGRAM));
	char *message = malloc(200), *resp = malloc(3);
	p = get_current_program();
	ttls_in_use(p, ttls);
	
	for(i = 0; i<24; i++)
		GetValueFromIndex(HiddenPanel, HiddenVals_TTLBroken, i, &bttls[i]);
	
	for(i = 0; i<24; i++)
	{
		if(bttls[i] && (ttls[i] || i == ttl_trigger))
		{
			sprintf(message, "There is a conflict in TTL %d. If you would like to swap with another TTL line, enter the line (0-23). Enter -1 if you would like to keep it where it is.", i);
			PromptPopup("TTL Conflict", message, resp, 2);
			if(sscanf(resp, "%d", &fc) > 0 && fc >= 0 && fc <=23)
			{
				for(j = 0; j<p->n_inst; j++)
					swap_ttls(i, fc);
				
				j = ttls[i];
				ttls[i] = ttls[fc];
				ttls[fc] = j;
			}
			
		}
		
		for(j = 0; j<ninstructions; j++)
			SetCtrlAttribute(inst[j], Load_TTLs(i), ATTR_DIMMED, bttls[i]);
	}
				
	*/
	return 0;
}

int CVICALLBACK QuitBrokenTTL (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	DiscardPanel(panel);
	return 0;
}

int CVICALLBACK ToggleBrokenTTL (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int *ctrls = malloc(24*sizeof(int)), index;
			ctrls = callbackData;
			
			for(int i = 0; i<24; i++)
			{
				if(ctrls[i] == control)
				{
					index = i;
					break;
				}
			}
			
			int val;
			GetCtrlVal(panel, control, &val);
			ReplaceListItem(HiddenPanel, HiddenVals_TTLBroken, index, NULL, val);
			
			setup_broken_ttls();
			
			break;
	}
	return 0;
}

void CVICALLBACK BrokenTTLs (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int top, left, width, height;
	
	GetPanelAttribute(panelHandle, ATTR_TOP, &top);
	GetPanelAttribute(panelHandle, ATTR_LEFT, &left);
	GetPanelAttribute(panelHandle, ATTR_WIDTH, &width);
	GetPanelAttribute(panelHandle, ATTR_HEIGHT, &height);

	int l = (width-660)/2 + left, t = (height-50)/2 + top;
	
	int p = NewPanel(0, "Specify Broken TTLs", t, l, 50, 650), val;
	int *ctrls = malloc(sizeof(int)*24);
	
	for(int i = 0; i<24; i++)
	{
		ctrls[i] = NewCtrl(p, CTRL_ROUND_LED_LS, "", 10, 5+25*i+((int)((double)i/6))*10);
		SetCtrlAttribute(p, ctrls[i], ATTR_CTRL_MODE, VAL_HOT);
		
		InstallCtrlCallback(p, ctrls[i], ToggleBrokenTTL, ctrls);
		GetValueFromIndex(HiddenPanel, HiddenVals_TTLBroken, i, &val);
		SetCtrlVal(p, ctrls[i], val);
	}
	
	int qb = NewCtrl(p, CTRL_SQUARE_COMMAND_BUTTON, "Quit", 400, 600);
	SetCtrlAttribute(p, qb, ATTR_VISIBLE, 0);
	SetPanelAttribute(p, ATTR_CLOSE_CTRL, qb);
	
	InstallCtrlCallback(p, qb, QuitBrokenTTL, NULL);
	
	DisplayPanel(p);

}

int toggle_RMS_borders (int panel, int on)
{
	/*
	int ctrl = -1;
	int RMS1, RMS2, visible;
	if(panel == FFTSpectrum)
	{
		ctrl = Spectrum_Graph;
		RMS1 = Spectrum_RMS1;
		RMS2 = Spectrum_RMS2;
		visible = Spectrum_CursorOn;
	}
	else if(panel == FID)
	{
		ctrl = FID_Graph;
		RMS1 = FID_RMS1;
		RMS2 = FID_RMS2;
		visible = FID_CursorOn;
	}
	else
		return -1;
	
	if(on) {
		EnableExtendedMouseEvents(panel, ctrl, 0.01);
		SetCursorAttribute(panel, ctrl, 1, ATTR_CURSOR_ENABLED, 0);
		SetCursorAttribute(panel, ctrl, 2, ATTR_CURSOR_ENABLED, 0);
		
		SetCursorAttribute(panel, ctrl, 1, ATTR_CURSOR_POINT_STYLE, VAL_SMALL_CROSS);
		SetCursorAttribute(panel, ctrl, 2, ATTR_CURSOR_POINT_STYLE, VAL_SMALL_CROSS);
		
		SetCursorAttribute(panel, ctrl, 1, ATTR_CURSOR_MODE, VAL_FREE_FORM);
		SetCursorAttribute(panel, ctrl, 2, ATTR_CURSOR_MODE, VAL_FREE_FORM);
		
		SetCursorAttribute(panel, ctrl, 1, ATTR_CURSOR_COLOR, VAL_GREEN);
		SetCursorAttribute(panel, ctrl, 2, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);
		
		SetCtrlAttribute(panel, RMS1, ATTR_CTRL_MODE, VAL_HOT);
		SetCtrlAttribute(panel, RMS2, ATTR_CTRL_MODE, VAL_HOT);
		
	} else {
		DisableExtendedMouseEvents(panel, ctrl);
		
		SetCursorAttribute(panel, ctrl, 1, ATTR_CURSOR_ENABLED, 0);
		SetCursorAttribute(panel, ctrl, 2, ATTR_CURSOR_ENABLED, 0);
		
		SetCursorAttribute(panel, ctrl, 1, ATTR_CURSOR_POINT_STYLE, VAL_SMALL_CROSS);
		
		SetCursorAttribute(panel, ctrl, 1, ATTR_CURSOR_MODE, VAL_FREE_FORM);

		int vis;
		GetCtrlVal(panel, visible, &vis);
		
		if(vis)
			SetCursorAttribute(panel, ctrl, 1, ATTR_CURSOR_COLOR, VAL_WHITE);
		else
			SetCursorAttribute(panel, ctrl, 1, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);
		
		SetCursorAttribute(panel, ctrl, 2, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);
		
		SetCtrlAttribute(panel, RMS1, ATTR_CTRL_MODE, VAL_INDICATOR);
		SetCtrlAttribute(panel, RMS2, ATTR_CTRL_MODE, VAL_INDICATOR);
	}
	*/
	return 0;
}
		

int CVICALLBACK SetRMSBorders (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int ctrl = -1;
	if(panel == FFTSpectrum)
		ctrl = Spectrum_Graph;
	else if (panel == FID)
		ctrl = FID_Graph;
	else
		return -1;
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			int on;
			GetCtrlVal(panel, control, &on);
	
			if(on) {
				SetCtrlAttribute(panel, control, ATTR_LABEL_BOLD, 0);
			}
			
			toggle_RMS_borders(panel, !on);
			
			break;
		case EVENT_RIGHT_CLICK:
			int o;
			GetCtrlVal(panel, control, &o);
			if(o)
				SetCtrlAttribute(panel, control, ATTR_LABEL_BOLD, 0);
			else
				SetCtrlAttribute(panel, control, ATTR_LABEL_BOLD, 1);
			
			SetCtrlVal(panel, control, !o);
			toggle_RMS_borders(panel, !o);
			
			break;
	}
	return 0;
}

int getMousePositionOnGraph (int panel, int control, double *x, double *y)
{
	int top, left, high, wide;
	GetCtrlAttribute(panel, control, ATTR_PLOT_AREA_HEIGHT, &high);
	GetCtrlAttribute(panel, control, ATTR_PLOT_AREA_WIDTH, &wide);
	GetCtrlAttribute(panel, control, ATTR_PLOT_AREA_TOP, &top);
	GetCtrlAttribute(panel, control, ATTR_PLOT_AREA_LEFT, &left);
	double x1 = *x, y1 = *y;
	
	if(x1 < 0 || y1 < 0)
		return -1;
	
	if(y1 < top || y1 > top+high)
		return -1;
	
	if(x1 < left)
		x1 = left;
	else if(x1 > left+wide)
	{
		GetCtrlAttribute(panel, control, ATTR_WIDTH, &top);
		if(x1 > top)
			return -1;
		x1 = left+wide;
	}
	
	Point p = MakePoint(x1, y1);

	if(GetGraphCoordsFromPoint(panel, control, p, &x1, &y1) <= 0)
		return -1;
					
	double xgain, xoffset, ygain, yoffset;
	GetCtrlAttribute(panel, control, ATTR_XAXIS_GAIN, &xgain);
	GetCtrlAttribute(panel, control, ATTR_XAXIS_OFFSET, &xoffset);
	GetCtrlAttribute(panel, control, ATTR_YAXIS_GAIN, &ygain);
	GetCtrlAttribute(panel, control, ATTR_YAXIS_OFFSET, &yoffset);
	
	*x = (x1-xoffset)/xgain;
	*y = (y1-yoffset)/ygain;
	
	return 0;

}

int CVICALLBACK DisplayRMSBoundsCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int on;
			int graph = FID_Graph;
			
			GetCtrlVal(panel, control, &on);
			
			if(panel == FFTSpectrum)
				graph = Spectrum_Graph;
			else if (panel != FID) {
				SetCtrlVal(panel, control, 0);
				break;
			}
			
			if(on) {
				SetCursorAttribute(panel, graph, 3, ATTR_CURSOR_COLOR, RMS_BORDER_COLOR);
				SetCursorAttribute(panel, graph, 4, ATTR_CURSOR_COLOR, RMS_BORDER_COLOR);
			} else {
				SetCursorAttribute(panel, graph, 3, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);
				SetCursorAttribute(panel, graph, 4, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);
			}
			break;
	}
	return 0;
}

int CVICALLBACK CursorVisible (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int on;
			GetCtrlVal(panel, control, &on);
			int ctrl = FID_Graph;
			if(panel == FFTSpectrum)
				ctrl = Spectrum_Graph;
			else if(panel != FID)
				break;
			
			if(on) {
				SetCursorAttribute(panel, ctrl, 1, ATTR_CURSOR_COLOR, VAL_WHITE);
				SetCursorAttribute(panel, ctrl, 1, ATTR_CURSOR_ENABLED, 0);
			} else
				SetCursorAttribute(panel, ctrl, 1, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);

				
			break;
	}
	return 0;
}

int CVICALLBACK ChangeRMS (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		  /*
			int ctrl = FID_Graph, RMS1 = FID_RMS1, RMS2 = FID_RMS2;
			if(panel == FFTSpectrum) {
				ctrl = Spectrum_Graph;
				RMS1 = Spectrum_RMS1;
				RMS2 = Spectrum_RMS2;
			}
			
			double rms1, rms2, xoff, xgain;
			
			GetCtrlVal(panel, RMS1, &rms1);
			GetCtrlVal(panel, RMS2, &rms2);
			
			GetCtrlAttribute(panel, ctrl, ATTR_XAXIS_GAIN, &xgain);
			GetCtrlAttribute(panel, ctrl, ATTR_XAXIS_OFFSET, &xoff);
			
			rms1 = (rms1-xoff)/xgain;
			rms2 = (rms2-xoff)/xgain;
			
			setup_RMS(panel, rms1, rms2);
			*/
			break;
	}
	return 0;
}

int CVICALLBACK ChangeCursorMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
		case EVENT_RIGHT_CLICK:

			break;
	}
	return 0;
}

void CVICALLBACK SaveConfig (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	save_configuration_to_file(savedsessionloc);
	if(!file_exists(defaultsloc))
	{
		int val = ConfirmPopup("Default Configuration Missing", "The Default configuration file is missing. Would you like to use the current configuration as the default?");
		if(val)
			save_configuration_to_file(defaultsloc);
	}
}

int update_instr_data_nd(int panel, int change) {
	/* This takes care of the three-way cycle between initial, increment and final
	Feed it panel to know what instruction we're dealing with, then you want to know which box you want to change based on the other two values.
	
	change = 0 -> Use inc/fin to calculate init
	change = 1 -> Use int/fin to calculate inc
	change = 2 -> Use int/inc to calculate fin.
	
	Since these have specific restrictions, we'll enforce that inc is an integer, that int has a certain minimum, etc.*/
	
	
	int init, inc, final, numsteps;
	
	GetCtrlVal(panel, MDInstr_IntInstrData, &init);
	GetCtrlVal(panel, MDInstr_IncInstrData, &inc);
	GetCtrlVal(panel, MDInstr_FInstrData, &final);
	GetCtrlVal(panel, MDInstr_NumSteps, &numsteps);
	
	numsteps--;
	
	switch(change) {
		case 0:
			init = final-inc*numsteps;
			// If init is less than 0, set it to 0 and keep increment the same.
			if(init < 0) {
				init = 0;
				final = 0+inc*numsteps;
			}
			
		break;
		case 1:
			inc = (int)((final-init)/numsteps); // Rounds down.
			
			final = init+inc*numsteps; // If it's not evenly divisible, keep the initial value the same.
		break;
		case 2:
			final = init+inc*numsteps;
			
			// If final is less than 0, set it to 0 and keep the increment the same. This can only happen if inc < 0
			if(final < 0) {
				final = 0;
				init = 0-inc*numsteps;
			}
				
		break;
	}
	
	// Set the values now
	SetCtrlVal(panel, MDInstr_IntInstrData, init);
	SetCtrlVal(panel, MDInstr_IncInstrData, inc);
	SetCtrlVal(panel, MDInstr_FInstrData, final);
	
	int instr_num, instr_data;
	GetCtrlVal(panel, MDInstr_InstrNum, &instr_num);
	GetCtrlVal(inst[instr_num], PulseInstP_Instr_Data, &instr_data);
	
	// Get the time each increment step will take
	double instr_time = calculate_instr_length(instr_num);
	instr_time /= instr_data;
	
	
	// What is the time, in ns..
	double init_t = instr_time*init;
	double inc_t = instr_time*inc;
	double f_t = instr_time*final;
	
	// Calculate what the new units should be
	double init_units = calculate_units(init_t);
	double inc_units = calculate_units(inc_t);
	double final_units = calculate_units(f_t);
	
	// Change the times appropriately.
	init_t /= init_units;
	inc_t /= inc_units;
	f_t /= final_units;
	
	// Finally, let's just make it so that it always displays 6 numbers.
	int prec = get_precision(init_t, 6);
	SetCtrlAttribute(panel, MDInstr_InitTimeData, ATTR_PRECISION, prec);
	
	prec = get_precision(inc_t, 6);
	SetCtrlAttribute(panel, MDInstr_IncTimeData, ATTR_PRECISION, prec);
	
	prec = get_precision(f_t, 6);
	SetCtrlAttribute(panel, MDInstr_FTimeData, ATTR_PRECISION, prec);
	
	// Set the values appropriately
	SetCtrlVal(panel, MDInstr_InitTimeData, init_t);
	SetCtrlVal(panel, MDInstr_IncTimeData, inc_t);
	SetCtrlVal(panel, MDInstr_FTimeData, f_t);
	
	// Set the units correctly.
	int init_index, inc_index, f_index;
	GetIndexFromValue(panel, MDInstr_InitTimeDataUnits, &init_index, init_units);
	GetIndexFromValue(panel, MDInstr_InitTimeDataUnits, &inc_index, inc_units);
	GetIndexFromValue(panel, MDInstr_InitTimeDataUnits, &f_index, final_units);
	
	// Set the defaults so that we can change the units if need be.
	SetCtrlAttribute(panel, MDInstr_InitTimeDataUnits, ATTR_DFLT_INDEX, init_index);
	SetCtrlAttribute(panel, MDInstr_IncTimeDataUnits, ATTR_DFLT_INDEX, inc_index);
	SetCtrlAttribute(panel, MDInstr_FTimeDataUnits, ATTR_DFLT_INDEX, f_index);
	
	// Now actually set the indexes.
	SetCtrlIndex(panel, MDInstr_InitTimeDataUnits, init_index);
	SetCtrlIndex(panel, MDInstr_IncTimeDataUnits, inc_index);
	SetCtrlIndex(panel, MDInstr_FTimeDataUnits, f_index);
	return 0;
}

int CVICALLBACK ChangeIntInstr (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				update_instr_data_nd(panel, 2); // Update by preserving the increment value.
			break;
	}
	return 0;
}

int CVICALLBACK ChangeIncInstrData (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				update_instr_data_nd(panel, 2); // Update by moving final out more.
			break;
	}
	return 0;
}

int CVICALLBACK ChangeFInstrData (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				update_instr_data_nd(panel, 0); // Update by moving the first one back.
			break;
	}
	return 0;
}

double calculate_instr_length(int instr) {
	// Calculates the length of each instruction.
	// Loop lengths count the length of the entire loop.
	// I don't understand JSR/RTS instructions that well, so they are ignored.
	// Branch instructions have infinite time, so they are ignored as well.
	// I believe STOP instructions are not executed, so the time is actually 0.
	// WAIT instructions are problematic, but I'll treat them as CONTINUE.
	// END_LOOP is counted as part of LOOP.
	
	int instruction, instr_data;
	int pan = inst[instr];
	double instr_len, instr_units;
	
	// Get what we need.
	GetCtrlVal(pan, PulseInstP_Instructions, &instruction); 
	if(instruction == STOP || instruction == BRANCH || instruction == JSR || instruction == RTS || instruction == END_LOOP) {
		return 0.0;
	}
	
	GetCtrlVal(pan, PulseInstP_InstDelay, &instr_len);
	GetCtrlVal(pan, PulseInstP_TimeUnits, &instr_units);

	if (instruction == CONTINUE  || instruction == WAIT) {
		return instr_len*instr_units; // Return the value in ns.
	}
	
	GetCtrlVal(pan, PulseInstP_Instr_Data, &instr_data);
	if (instruction == LONG_DELAY ) {
		return instr_len*instr_units*instr_data; // Return the value in ns;
	}
	
	if (instruction == LOOP) {
		int search_instr, search_instr_data, end_instr = -1;
		
		double n_len, n_len_u, out = instr_len*instr_units; // The loop instruction itself takes some time.
		
		// Find the end of the loop.
		for(int i = instr+1; i <= ninstructions; i++) {
			GetCtrlVal(inst[i], PulseInstP_Instructions, &search_instr);

			if(search_instr != STOP && search_instr != BRANCH && search_instr != JSR && search_instr != RTS && search_instr != END_LOOP) {
				n_len = calculate_instr_length(i);
				
				if (n_len == -1.0) {
					break;
				} else {
					out += n_len; // Add the next thing if there's no error.
				}
			}
			
			if(search_instr == END_LOOP) {
				GetCtrlVal(inst[i], PulseInstP_Instr_Data, &search_instr_data);
				GetCtrlVal(inst[i], PulseInstP_InstDelay, &n_len);
				GetCtrlVal(inst[i], PulseInstP_TimeUnits, &n_len_u);
				
				out += n_len*n_len_u; // Here's where you count the END_LOOP bit.
				
				if(search_instr_data == instr) {
					end_instr = i;
					break;
				}
			} else if (search_instr == BRANCH || search_instr == STOP) {
				// If it's BRANCH or STOP, you'll never get to the end of the loop, so just break now.
				break;
			}
		}
		
		if(end_instr == -1)
			return -1.0; // Can't find the end of it, so it's no good.
		
		return out*instr_data; // The whole loop is iterated a bunch of times.

	}
	
	return -2.0; // If it's something else somehow, throw some weird error.
	
}



double get_and_set_old_units(int panel, int control) {
	// This is a horrible way to do this, but we'll store the previous units on the 2D blue unit controls
	// as their default indexes. This returns the old units, then sets the default index to the new units.
	
	int newindex, oldindex;
	double oldval;
	
	GetCtrlIndex(panel, control, &newindex);
	GetCtrlAttribute(panel, control, ATTR_DFLT_INDEX, &oldindex);
	GetValueFromIndex(panel, control, oldindex, &oldval);
	
	SetCtrlAttribute(panel, control, ATTR_DFLT_INDEX, newindex);
	
	return oldval;

}

int CVICALLBACK ChangeFTimeDataUnits (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				/*double time, units, old_units = get_and_set_old_units(panel, control);
				GetCtrlVal(panel, MDInstr_FTimeData, &time);
				GetCtrlVal(panel, control, &units);
				
				time *= old_units/units;
				SetCtrlVal(panel, MDInstr_FTimeData, time); // Sets the new units here.
				
				// Set the precision so it only ever displays 6 digits.
				int prec = get_precision(time, 6);
				SetCtrlAttribute(panel, MDInstr_FTimeData, ATTR_PRECISION, prec);
				*/
			break;
	}
	return 0;
}

int CVICALLBACK ChangeIncTimeDataUnits (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			/*  	
			double time, units, old_units = get_and_set_old_units(panel, control);
				GetCtrlVal(panel, MDInstr_IncTimeData, &time);
				GetCtrlVal(panel, control, &units);
				
				time *= old_units/units;
				SetCtrlVal(panel, MDInstr_IncTimeData, time); // Sets the new units here.
				
				// Set the precision so it only ever displays 6 digits.
				int prec = get_precision(time, 6);
				SetCtrlAttribute(panel, MDInstr_IncTimeData, ATTR_PRECISION, prec);
			*/
			break;
	}
	return 0;
}

int CVICALLBACK ChangeInitTimeDataUnits (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			/*  	
			double time, units, old_units = get_and_set_old_units(panel, control);
			GetCtrlVal(panel, MDInstr_InitTimeData, &time);
			GetCtrlVal(panel, control, &units);
			
			time *= old_units/units;
			SetCtrlVal(panel, MDInstr_InitTimeData, time); // Sets the new units here.
			
			// Set the precision so it only ever displays 6 digits.
			int prec = get_precision(time, 6);
			SetCtrlAttribute(panel, MDInstr_InitTimeData, ATTR_PRECISION, prec);
			*/
			break;
	}
	return 0;
}

int CVICALLBACK MoveInstr (int panel, int control, int event,
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
			GetCtrlVal(panel, PulseInstP_InstNum, &num);
			
			// How far apart are the instructions - they're evenly spaced, so pick the first two.
			if (ninstructions > 1) {
				int top1, top2, diff;
				
				GetPanelAttribute(inst[0], ATTR_TOP, &top1);	
				GetPanelAttribute(inst[1], ATTR_TOP, &top2);
				
				diff = top1-top2; // This is a positive number, so if we're moving up, move the cursor by this amount, otherwise move it by the negative of this amount.
				
				// Now we just decide if it was called by the up button or the down button.
				switch (control) {
					case PulseInstP_UpButton:
						//move_instruction(num-1, num);
						SetCursorPos(pos.x, pos.y+diff);
					break;
					case PulseInstP_DownButton:
						//move_instruction(num+1, num);
						SetCursorPos(pos.x, pos.y-diff);
					break;
				}
			}
			break;
	}
	return 0;
}

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

int CVICALLBACK ChangePhaseCycleLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

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

			break;
	}
	return 0;
}

int CVICALLBACK InstrChangeCycle (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

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
