//////////////////////////////////////////////////////////////////////
// 																	//
//			  Session/Preference Saving Library v1.0				//
//					Paul Ganssle, 07/15/2011						//
//																	//
//	This is a library intended for containing all the functions		//
// 	related to saving the contents of the windows between sessions	//
// 	as well as the user's preferences, like the user-defined 		//
// 	functions.														//
// 																	//
//////////////////////////////////////////////////////////////////////

/***************************  Version History  ******************************

1.0 	This is the first version of this as a library separate from the 
		main source code. It makes use of all the structs as defined in 
		UIControls.h, as well as the netcdf format.
		
*****************************************************************************/

// Includes
#include "FileBrowser.h"
#include <cvixml.h>
#include "toolbox.h"
#include <userint.h>
#include <ansi_c.h>
#include <spinapi.h>					// SpinCore functions
#include <NIDAQmx.h>

#include <PulseProgramTypes.h>
#include <FileSave.h>
#include <cvitdms.h>
#include <cviddc.h>
#include <UIControls.h>					// For manipulating the UI controls
#include <MathParserLib.h>
#include <MCUserDefinedFunctions.h>
#include <PulseProgramLib.h>			
#include <SaveSessionLib.h>
#include <DataLib.h>
#include <Magnetometer Controller.h>
#include <General.h>
#include <PPConversion.h>
#include <Version.h>

// Globals
char *session_fname = "SavedSession";

/****************************** UI Manipulation ******************************/
int load_ui(char *uifname) { // Function for creating the ppcontrols structure
	// This loads the user interface and populates the structs that contain pointers that can be
	// passed selectively to library functions. These structs are globals, so they don't need to
	// be passed as arguments here.
	
	// The uifile will probably be needed later when we make more instructions, so it goes in pc
	pc.uifname = malloc(strlen(uifname)+1);
	strcpy(pc.uifname, uifname);
	
	pc.pulse_inst = PulseInstP;
	pc.md_inst = MDInstr;
	pc.a_inst = AOInstPan;
	
	// Start building the tabs now.
	if ((mc.mp = LoadPanel (0, uifname, MainPanel)) < 0)
		return 1;
	
	// First get the tab panels.
	GetPanelHandleFromTabPage(mc.mp, MainPanel_MainTabs, 0, &dc.fid);
	GetPanelHandleFromTabPage(mc.mp, MainPanel_MainTabs, 1, &dc.spec);
	GetPanelHandleFromTabPage(mc.mp, MainPanel_MainTabs, 2, &pc.PProgPan);
	GetPanelHandleFromTabPage(mc.mp, MainPanel_MainTabs, 3, &pc.PPConfigPan);
	GetPanelHandleFromTabPage(mc.mp, MainPanel_MainTabs, 4, &pc.AOutPan);

	// Then the menu bars
	mc.mainmenu = GetPanelMenuBar(mc.mp); 	// Main menu
	mc.rcmenu = LoadMenuBar(0, uifname, RCMenus);	// Right click menu

	// Then the container panels								 
	pc.PProgCPan = LoadPanel(pc.PProgPan, uifname, PPPanel); // Pulse program instr container
	pc.PPConfigCPan = LoadPanel(pc.PPConfigPan, uifname, PPConfigP); // ND instr container
	pc.AOutCPan = LoadPanel(pc.AOutPan, uifname, AOConP);
	
	// Create two panels for containing the current location in acquisition space.
	dc.fcloc = dc.cloc[0] = LoadPanel(dc.fid, uifname, CurrentLoc);
	dc.scloc = dc.cloc[1] = LoadPanel(dc.spec, uifname, CurrentLoc);
	
	// Move everything to where it belongs
	SetPanelPos(pc.PProgCPan, 5, 2); 	// Move the instruction container where it should go
	DisplayPanel(pc.PProgCPan); 		// Display the instruction container
	
	SetPanelPos(pc.PPConfigCPan, 32, 8); 	// Move MD instruction container to where it belongs
	DisplayPanel(pc.PPConfigCPan);		// Display the MD instruction container
	SetPanelAttribute(pc.PPConfigCPan, ATTR_DIMMED, 1);
		
	int i;
	for(i = 0; i<2; i++) { // Place the current location panels and display them
		SetPanelPos(dc.cloc[i], 250, 1200);
		DisplayPanel(dc.cloc[i]);
	}
	
	// Display analog output container panel.
	SetPanelPos(pc.AOutCPan, 10, 10);
	SetPanelAttribute(pc.AOutCPan, ATTR_TITLEBAR_VISIBLE, 0);
	DisplayPanel(pc.AOutCPan);
	
	// Now all the constant stuff
	initialize_uicontrols();
	initialize_data();
	initialize_program();
	initialize_ce();
	
	// Load the DAQ and PB values.
	load_DAQ_info_safe(1, 1, 1);
	load_pb_info(0);

	// Now load the previous session
	int rv = load_session(NULL, 1);
	
	if(rv) {  display_xml_error(rv); }
	
	// Now load what UI stuff that needs to be loaded
	setup_broken_ttls_safe();
	
	CmtGetLock(lock_uidc);
	for(i = 0; i < 8; i++) {
		change_fid_chan_col(i);
		change_spec_chan_col(i);
	}
	CmtReleaseLock(lock_uidc);
	
	// Initialize the data navigation box
	int len;
	GetCtrlValStringLength(mc.datafbox[1], mc.datafbox[0], &len);
	char *path = malloc(len+1);
	GetCtrlVal(mc.path[1], mc.path[0], path);
	select_directory(path);
	
	// Initialize the experiment choice
	GetCtrlValStringLength(mc.basefname[1], mc.basefname[0], &len);
	char *fname = malloc(len+5);	// Need to add 0000.tdm to this eventually.
	GetCtrlVal(mc.basefname[1], mc.basefname[0], fname);
	
	int ldm;
	GetCtrlVal(mc.ldatamode[1], mc.ldatamode[0], &ldm);
	
	get_current_fname(path, fname, !ldm);

	if(path[strlen(path)-1] == '\\') { path[strlen(path)-1] = '\0'; }
	
	path = realloc(path, strlen(path)+strlen(fname)+6);
	
	strcat(path, "\\");
	strcat(path, fname);
	strcat(path, ".tdm");
	
	SetCtrlVal(mc.datafbox[1], mc.datafbox[0], path);
	
	if(ldm) {
		load_file_info_safe(path, NULL);
	} else { 
		SetCtrlVal(mc.cdfname[1], mc.cdfname[0], fname);
	}
	
	free(fname);
	free(path);
	
	SetCtrlVal(pc.trig_ttl[1], pc.trig_ttl[0], uipc.trigger_ttl);
	set_ttl_trigger(pc.inst[0], -1, 1);

	// Do this for whatever reason.
	update_experiment_nav_safe();
	
	DisplayPanel (mc.mp);
	
	// If necessary, select the pulseblaster.
	int nl;
	GetNumListItems(pc.pbdev[1], pc.pbdev[0], &nl);
	
	if(nl) {
		int pb;
		GetCtrlVal(pc.pbdev[1], pc.pbdev[0], &pb);
		pb_select_board(pb);
	}
	
	// Add the tooltip to the skip condition
	SetCtrlToolTipAttribute(pc.skiptxt[1], pc.skiptxt[0], CTRL_TOOLTIP_ATTR_TEXT, get_tooltip(1));
	SetCtrlToolTipAttribute(pc.skiptxt[1], pc.skiptxt[0], CTRL_TOOLTIP_ATTR_ENABLE, 1);
	
	return 0;
}

void setup_broken_ttls() {
	// Set all the TTLS that are broken to dimmed, everything else un-dimmed
	int i, j, dimmed;
	for(i = 0; i < 24; i++) {
		dimmed = ((1<<i) & uipc.broken_ttls)?1:0;
		for(j = 0; j < uipc.max_ni; j++) 
			SetCtrlAttribute(pc.inst[j], pc.TTLs[i], ATTR_DIMMED, dimmed);
	}
}

void setup_broken_ttls_safe() {
	CmtGetLock(lock_uipc);
	setup_broken_ttls();
	CmtReleaseLock(lock_uipc);
}

/*************************** Panel Releases ***************************/

void ppconfig_popout() {
	// Releases the ppconfig panel.	
	
}

int CVICALLBACK ppconfig_popin(int panel, int control, int event, void *callbackData, int eventData1, int eventData2) {
	// Confines the ppconfig window.
	
	DiscardPanel(panel);
	
	return 0;
}


/*************************** Struct Initialization ***************************/

void initialize_program() {
	// Gets the initial values for uipc. For now it just initializes to
	// a blank program, but eventually this will load a program from file

	CmtGetLock(lock_uipc);
	
	uipc.ni = 1;
	uipc.max_ni = 1;
	uipc.broken_ttls = 0;
	uipc.total_time = 0.0;
	uipc.trigger_ttl = 0;
	
	uipc.nd = 0;
	uipc.max_nd = 0;
	uipc.ndins = 0;
	uipc.dim_steps = NULL;
	uipc.dim_ins = NULL;
	uipc.ins_dims = NULL;
	uipc.ins_state = NULL;
	
	uipc.nd_delays = NULL;
	uipc.nd_data = NULL;
	uipc.err_dat = 0;
	uipc.err_dat_size = 0;
	uipc.err_del = 0;
	uipc.err_dat_size = 0;
	uipc.err_dat_pos = NULL;
	uipc.err_del_pos = NULL;
	
	uipc.max_n_steps = 0;
	uipc.skip_err = 0;
	uipc.skip_err_size = 0;
	uipc.skip_err_pos = NULL;
	uipc.skip_locs = NULL;
	
	uipc.nc = 0;
	uipc.max_nc = 0;
	uipc.ncins = 0;
	uipc.n_cyc_pans = 0;
	uipc.cyc_steps = NULL;
	uipc.cyc_ins = NULL;
	uipc.ins_cycs = NULL;
	uipc.cyc_pans = NULL;
	uipc.c_instrs = NULL;
	uipc.max_cinstrs = NULL;
	
	uipc.anum = 0;
	uipc.max_anum=0;
	uipc.anum_devs = 0;
	uipc.anum_all_chans = 0;
	uipc.anum_avail_chans = 0;
	
	uipc.adev_display = NULL;
	uipc.adev_true = NULL;
	uipc.ao_avail_chans = NULL;
	uipc.ao_all_chans = NULL;
	
	uipc.ac_varied = NULL;
	uipc.ac_dim = NULL;
	uipc.ao_devs = NULL;
	uipc.ao_chans = NULL;
	uipc.ao_vals = NULL;
	uipc.ao_exprs = NULL;
	
	uipc.ppath = NULL;
	CmtReleaseLock(lock_uipc);

	
	//	Now allocate memory for the instruction arrays, then create one of each
	pc.inst = malloc(sizeof(int));
	pc.cinst = malloc(sizeof(int));
	pc.ainst = malloc(sizeof(int));
	
	pc.inst[0] = LoadPanel(pc.PProgCPan, pc.uifname, PulseInstP);
	pc.cinst[0] = LoadPanel(pc.PPConfigCPan, pc.uifname, MDInstr);
	pc.ainst[0] = LoadPanel(pc.AOutCPan, pc.uifname, AOInstPan);

	SetPanelPos(pc.inst[0], 25, 7);	// Move the first instruction to where it belongs
	DisplayPanel(pc.inst[0]);			// Display the first instruction
	SetCtrlAttribute(pc.inst[0], pc.xbutton, ATTR_DISABLE_PANEL_THEME, 1);
	
	SetPanelPos(pc.cinst[0], 25, 7); 	// Move the first MD instruction to where it belongs
	DisplayPanel(pc.cinst[0]);			// Display the first MD instruction
	SetPanelAttribute(pc.cinst[0], ATTR_DIMMED, 1);
	
	SetPanelPos(pc.ainst[0], 60, 5);
	DisplayPanel(pc.ainst[0]);
	set_aout_dimmed_safe(0, 1, 0);
	
	// Set up the initial callback data for np, sr and at callbacks.
	InstallCtrlCallback(pc.sr[1], pc.sr[0], ChangeNP_AT_SR, (void *)0);
	InstallCtrlCallback(pc.at[1], pc.at[0], ChangeNP_AT_SR, (void *)1);
	InstallCtrlCallback(pc.np[1], pc.np[0], ChangeNP_AT_SR, (void *)0);
	
	// Set up the callback data for the transient view selection menu
	for(int i = 0; i < 3; i++)
		InstallMenuCallback(mc.mainmenu, mc.vtviewopts[i], ChangeTransientView, (void *)i);
}

void initialize_data() {
	// Set up the default values in the uidc variable.
	CmtGetLock(lock_uidc);
	uidc.nt = 1;
	uidc.nd = 0;
	
	// By default, the 1st channel is on
	uidc.fnc = 1;
	uidc.snc = 1;
	
	uidc.schan = 0; // By default, we look at the real channel.
	uidc.sr = 0.0;
	
	// Default colors
	uidc.fcol[0] = uidc.scol[0] = VAL_WHITE;
	uidc.fcol[1] = uidc.scol[1] = VAL_GREEN;
	uidc.fcol[2] = uidc.scol[2] = VAL_RED;
	uidc.fcol[3] = uidc.scol[3] = VAL_CYAN;
	uidc.fcol[4] = uidc.scol[4] = VAL_YELLOW;
	uidc.fcol[5] = uidc.scol[5] = VAL_GRAY;
	uidc.fcol[6] = uidc.scol[6] = VAL_MAGENTA;
	uidc.fcol[7] = uidc.scol[7] = 0xFF6103;	// Cadmium Orange

	// Default gains are 1, offsets are 0, channels are off (except the first one)
	// Plot ids are -1 (no plot)
	for(int i = 0; i < 8; i++) {
		uidc.fgain[i] = uidc.sgain[i] = 1.0;
		uidc.foff[i] = uidc.soff[i] = 0.0;
		uidc.fplotids[i] = -1;
		
		for(int j = 0; j < 3; j++) {
			uidc.sphase[i][j] = 0.0;	// Phases default to 0 correction.
			uidc.splotids[i][j] = -1;	// By coincidence there are 3 plots for the spectrum
		}
		
		uidc.range[i] = 10.0;
		uidc.fchans[i] = uidc.schans[i] = 0;
	}	
	
	uidc.fchans[0] = uidc.schans[0] = 1;
	
	uidc.devindex = 0;
	uidc.nchans = 0;					// There are no channels before we get them
	uidc.chans = NULL;
	
	uidc.disp_update = 0;
	
	uidc.dlpath = NULL;
	CmtReleaseLock(lock_uidc);
}

void initialize_uicontrols() {
	// Function that populates the static portions of mc, pc and dc;  
	int i;
	// Populate the main panel controls
	mc.mtabs[0] = MainPanel_MainTabs;
	mc.pathb[0] = MainPanel_DirectorySelect;
	mc.path[0] = MainPanel_Path;
	mc.cdfname[0] = MainPanel_CurrentFile;
	mc.datadesc[0] = MainPanel_DataDescription;
	mc.datafbox[0] = MainPanel_DataDirectory;
	mc.ldatamode[0] = MainPanel_LoadInfoMode;
	mc.basefname[0] = MainPanel_Filename;
	mc.pbrun[0] = MainPanel_Running;
	mc.pbwait[0] = MainPanel_Waiting;
	mc.pbstop[0] = MainPanel_Stopped;
	mc.mainstatus[0] = MainPanel_IsRunning;
	mc.startbut[0] = MainPanel_Start;
	mc.stopbut[0] = MainPanel_Stop;
	
	// Then populate the main panel panels
	mc.mtabs[1] = mc.mp;
	mc.pathb[1] = mc.mp;
	mc.path[1] = mc.mp;
	mc.cdfname[1] = mc.mp;
	mc.datadesc[1] = mc.mp;
	mc.datafbox[1] = mc.mp;
	mc.ldatamode[1] = mc.mp;
	mc.basefname[1] = mc.mp;
	mc.pbrun[1] = mc.mp;
	mc.pbwait[1] = mc.mp;
	mc.pbstop[1] = mc.mp;
	mc.mainstatus[1] = mc.mp;
	mc.startbut[1] = mc.mp;
	mc.stopbut[1] = mc.mp;
	
	// Populate the menu bars

	// Parent menus:
	mc.file = MainMenu_File;	
	mc.view = MainMenu_View;	
	mc.setup = MainMenu_SetupMenu;
	
	// File controls
	mc.fnew = MainMenu_File_New;	
	mc.fnewacq = MainMenu_File_New_NewAcquisition;
	mc.fnewprog = MainMenu_File_New_NewProgram;		
	mc.fsave = MainMenu_File_Save;				
	mc.fsavedata = MainMenu_File_Save_SaveData;		
	mc.fsaveprog = MainMenu_File_Save_SaveProgram;			
	mc.fload = MainMenu_File_Load;				
	mc.floaddata = MainMenu_File_Load_LoadData;			
	mc.floadrecdat = MainMenu_File_Load_LoadRecentData;
	mc.floadprog = MainMenu_File_Load_LoadProgram;
	mc.floadrecprog	= MainMenu_File_Load_LoadRecentProgram;	
	mc.fquit = MainMenu_File_Quit;				
	
	// View controls
	mc.vpchart = MainMenu_View_ProgramChart;			
	mc.vtview = MainMenu_View_TransView;		
	mc.vtviewopts[0] = MainMenu_View_TransView_ViewAverage;	
	mc.vtviewopts[1] = MainMenu_View_TransView_ViewLatestTrans;	
	mc.vtviewopts[2] = MainMenu_View_TransView_ViewNoUpdate;	
	mc.vcstep = MainMenu_View_ChangeDimension;			
	
	// Setup controls
	mc.supdaq = MainMenu_SetupMenu_UpdateDAQ;
	mc.ssaveconfig = MainMenu_SetupMenu_SaveCurrentConfig;
	mc.ssaveconfig_file = MainMenu_SetupMenu_SaveConfig;
	mc.sloadconfig = MainMenu_SetupMenu_LoadConfigFromFile;
	mc.sbttls = MainMenu_SetupMenu_BrokenTTLsMenu;
	
	// Populate the data tab controls
	dc.fgraph = FID_Graph;		//FID controls first
	dc.fauto = FID_Autoscale;
	dc.fxpos = FID_CursorX;
	dc.fypos = FID_CursorY;
	dc.fcring = FID_ChanPrefs;
	dc.fccol = FID_ChanColor;
	dc.fcgain = FID_Gain;
	dc.fcoffset = FID_Offset;
	dc.fpolysub = FID_PolySubtract;
	dc.fpsorder = FID_PolyFitOrder;
	
	dc.fchans[0] = FID_Chan1;	// FID channel controls
	dc.fchans[1] = FID_Chan2;
	dc.fchans[2] = FID_Chan3;
	dc.fchans[3] = FID_Chan4;
	dc.fchans[4] = FID_Chan5;
	dc.fchans[5] = FID_Chan6;
	dc.fchans[6] = FID_Chan7;
	dc.fchans[7] = FID_Chan8;

	dc.sgraph = Spectrum_Graph;	//Next Spectrum controls
	dc.sauto = Spectrum_Autoscale;
	dc.sxpos = Spectrum_CursorX;
	dc.sypos = Spectrum_CursorY;
	dc.scring = Spectrum_ChanPrefs;
	dc.sccol = Spectrum_ChanColor;
	dc.scgain = Spectrum_Gain;
	dc.scoffset = Spectrum_Offset;
	dc.spolysub = Spectrum_PolySubtract;
	dc.spsorder = Spectrum_PolyFitOrder;
	
	dc.schans[0] = Spectrum_Chan1;	// Spectrum channel controls
	dc.schans[1] = Spectrum_Chan2;
	dc.schans[2] = Spectrum_Chan3;
	dc.schans[3] = Spectrum_Chan4;
	dc.schans[4] = Spectrum_Chan5;
	dc.schans[5] = Spectrum_Chan6;
	dc.schans[6] = Spectrum_Chan7;
	dc.schans[7] = Spectrum_Chan8;
	
	dc.sphase[0] = Spectrum_PhaseKnob;
	dc.sphorder[0] = Spectrum_PhaseCorrectionOrder;
	dc.sfftring[0] = Spectrum_Channel;

	dc.sphase[1] = dc.spec;	// Initialize the panels for the dc
	dc.sphorder[1] = dc.spec;
	dc.sfftring[1] = dc.spec;
	
	dc.ctrans = CurrentLoc_TransientNum; 	// Now the current location controls
	dc.idrings[0] = CurrentLoc_IDVal1;		// Ring control
	dc.idlabs[0] = CurrentLoc_ID1;			// Label
	dc.idrings[1] = CurrentLoc_IDVal2;		
	dc.idlabs[1] = CurrentLoc_ID2;	
	dc.idrings[2] = CurrentLoc_IDVal3;		
	dc.idlabs[2] = CurrentLoc_ID3;	
	dc.idrings[3] = CurrentLoc_IDVal4;		
	dc.idlabs[3] = CurrentLoc_ID4;	
	dc.idrings[4] = CurrentLoc_IDVal5;		
	dc.idlabs[4] = CurrentLoc_ID5;	
	dc.idrings[5] = CurrentLoc_IDVal6;		
	dc.idlabs[5] = CurrentLoc_ID6;	
	dc.idrings[6] = CurrentLoc_IDVal7;		
	dc.idlabs[6] = CurrentLoc_ID7;	
	dc.idrings[7] = CurrentLoc_IDVal8;		
	dc.idlabs[7] = CurrentLoc_ID8;

	
	// Finally populate the pulse program controls
	pc.trig_ttl[0] = PulseProg_Trigger_TTL;
	pc.ninst[0] = PulseProg_NumInst;
	pc.rc[0] = PulseProg_ContinuousRun;
	pc.numcycles[0] = PulseProg_PhaseCycles;
	pc.trans[0] = PulseProg_TransientNum;

	// And the panel bits
	pc.trig_ttl[1] = pc.PProgPan;
	pc.ninst[1] = pc.PProgPan;
	pc.rc[1] = pc.PProgPan;
	pc.numcycles[1] = pc.PProgPan;
	pc.trans[1] = pc.PProgPan;

	pc.idrings[0][0] = PulseProg_IDVal1;		// Ring control
	pc.idlabs[0][0] = PulseProg_ID1;			// Label
	pc.idrings[1][0] = PulseProg_IDVal2;		
	pc.idlabs[1][0] = PulseProg_ID2;	
	pc.idrings[2][0] = PulseProg_IDVal3;		
	pc.idlabs[2][0] = PulseProg_ID3;	
	pc.idrings[3][0] = PulseProg_IDVal4;		
	pc.idlabs[3][0] = PulseProg_ID4;	
	pc.idrings[4][0] = PulseProg_IDVal5;		
	pc.idlabs[4][0] = PulseProg_ID5;	
	pc.idrings[5][0] = PulseProg_IDVal6;		
	pc.idlabs[5][0] = PulseProg_ID6;	
	pc.idrings[6][0] = PulseProg_IDVal7;		
	pc.idlabs[6][0] = PulseProg_ID7;	
	pc.idrings[7][0] = PulseProg_IDVal8;		
	pc.idlabs[7][0] = PulseProg_ID8;
	
	for(i = 0; i<8; i++) {
		pc.idrings[i][1] = pc.PProgPan;
		pc.idlabs[i][1] = pc.PProgPan;
	}
	
	pc.ndon[0] = PPConfig_NDimensionalOn;		// Now the Program Config Tab
	pc.ndims[0] = PPConfig_NumDimensions;
	pc.tfirst[0] = PPConfig_TransAcqMode;
	pc.sr[0] = PPConfig_SampleRate;
	pc.nt[0] = PPConfig_NTransients;
	pc.np[0] = PPConfig_NPoints;
	pc.at[0] = PPConfig_AcquisitionTime;
	pc.dev[0] = PPConfig_Device;
	pc.pbdev[0] = PPConfig_PBDeviceSelect;
	pc.nc[0] = PPConfig_NumChans;
	pc.ic[0] = PPConfig_AcquisitionChannel;
	pc.cc[0] = PPConfig_CounterChan;
	pc.curchan[0] = PPConfig_ChannelGain;
	pc.range[0] = PPConfig_ChannelRange;
	pc.trigc[0] = PPConfig_Trigger_Channel;
	pc.trige[0] = PPConfig_TriggerEdge;
	pc.skip[0] = PPConfig_SkipCondition;
	pc.skiptxt[0] = PPConfig_SkipConditionExpr;
	pc.timeest[0] = PPConfig_EstimatedTime;
	
	pc.ndon[1] = pc.PPConfigPan;		// And the panel bits
	pc.ndims[1] = pc.PPConfigPan;
	pc.tfirst[1] = pc.PPConfigPan;
	pc.sr[1] = pc.PPConfigPan;
	pc.nt[1] = pc.PPConfigPan;
	pc.np[1] = pc.PPConfigPan;
	pc.at[1] = pc.PPConfigPan;
	pc.dev[1] = pc.PPConfigPan;
	pc.pbdev[1] = pc.PPConfigPan;
	pc.nc[1] = pc.PPConfigPan;
	pc.ic[1] = pc.PPConfigPan;
	pc.cc[1] = pc.PPConfigPan;
	pc.curchan[1] = pc.PPConfigPan;
	pc.range[1] = pc.PPConfigPan;
	pc.trigc[1] = pc.PPConfigPan;
	pc.trige[1] = pc.PPConfigPan;
	pc.skip[1] = pc.PPConfigPan;;
	pc.skiptxt[1] = pc.PPConfigPan;
	pc.timeest[1] = pc.PPConfigPan;
	
	pc.ins_num = PulseInstP_InstNum;	// The inst panel controls
	pc.instr = PulseInstP_Instructions;
	pc.instr_d = PulseInstP_Instr_Data;
	pc.delay = PulseInstP_InstDelay;
	pc.delayu = PulseInstP_TimeUnits;
	pc.pcon = PulseInstP_PhaseCyclingOn;
	pc.pcstep = PulseInstP_PhaseCycleStep;
	pc.pclevel = PulseInstP_PhaseCycleLevel;
	pc.pcsteps = PulseInstP_NumCycles;
	pc.scan = PulseInstP_Scan;
	pc.uparrow = PulseInstP_UpButton;
	pc.downarrow = PulseInstP_DownButton;
	pc.xbutton = PulseInstP_xButton;
	pc.expandpc = PulseInstP_ExpandButton;
	pc.collapsepc = PulseInstP_CollapseButton;
	
	pc.TTLs[0] = PulseInstP_TTL0;	 	// Each TTL is its own control
	pc.TTLs[1] = PulseInstP_TTL1;	 	
	pc.TTLs[2] = PulseInstP_TTL2;	 	
	pc.TTLs[3] = PulseInstP_TTL3; 		
	pc.TTLs[4] = PulseInstP_TTL4; 		
	pc.TTLs[5] = PulseInstP_TTL5;	 	
	pc.TTLs[6] = PulseInstP_TTL6; 		
	pc.TTLs[7] = PulseInstP_TTL7; 		
	pc.TTLs[8] = PulseInstP_TTL8;	 	
	pc.TTLs[9] = PulseInstP_TTL9; 		
	pc.TTLs[10] = PulseInstP_TTL10;	
	pc.TTLs[11] = PulseInstP_TTL11;	
	pc.TTLs[12] = PulseInstP_TTL12;	
	pc.TTLs[13] = PulseInstP_TTL13;	
	pc.TTLs[14] = PulseInstP_TTL14;	
	pc.TTLs[15] = PulseInstP_TTL15;	
	pc.TTLs[16] = PulseInstP_TTL16;	
	pc.TTLs[17] = PulseInstP_TTL17;	
	pc.TTLs[18] = PulseInstP_TTL18;	
	pc.TTLs[19] = PulseInstP_TTL19;	
	pc.TTLs[20] = PulseInstP_TTL20; 	
	pc.TTLs[21] = PulseInstP_TTL21; 	
	pc.TTLs[22] = PulseInstP_TTL22; 	
	pc.TTLs[23] = PulseInstP_TTL23;
	
	pc.cins_num = MDInstr_InstrNum;	// The cinst panel controls now
	pc.cinstr = MDInstr_Instructions;
	
	pc.disp_init = MDInstr_InitDisplay;
	pc.del_init = MDInstr_InitTime;
	pc.delu_init = MDInstr_InitTimeUnits;
	pc.dat_init = MDInstr_InitInstrData;
	pc.disp_inc = MDInstr_IncDisplay;
	pc.del_inc = MDInstr_IncTime;
	pc.delu_inc = MDInstr_IncTimeUnits;
	pc.dat_inc = MDInstr_IncInstrData;
	pc.disp_fin = MDInstr_FDisplay;
	pc.del_fin = MDInstr_FTime;
	pc.delu_fin = MDInstr_FTimeUnits;
	pc.dat_fin = MDInstr_FInstrData;
	
	pc.cexpr_delay = MDInstr_IncDelayExpression;
	pc.cexpr_data  = MDInstr_IncDataExpression;
	pc.nsteps = MDInstr_NumSteps;
	pc.dim = MDInstr_Dimension;
	pc.vary = MDInstr_VaryInstr;

	// Now the analog output controls.
	pc.anum[1] = pc.AOutCPan;
	pc.andon[1] = pc.AOutPan;
	pc.andims[1] = pc.AOutPan;
	
	pc.anum[0] = AOConP_NumAOuts;
	pc.andon[0] = AOutTab_NDimensionalOn;
	pc.andims[0] = AOutTab_NumDimensions;
	
	pc.ainitval = AOInstPan_InitChanVal;
	pc.aincval = AOInstPan_ChanIncVal;
	pc.aincexpr = AOInstPan_ExpressionCtrl;
	pc.afinval = AOInstPan_ChanValFin;
	pc.asteps = AOInstPan_ChanNumSteps;
	pc.adim = AOInstPan_DimRing;
	pc.aindon = AOInstPan_NDToggle;
	pc.aodev = AOInstPan_ChanDev;
	pc.aochan = AOInstPan_AOutChan;
	pc.axbutton = AOInstPan_xButton;
}

void initialize_ce() {
	// Initializes ce. This will cause memory leaks if ce has already been set
	// and the memory has not been freed. This mainly sets things to be NULL and such.
	CmtGetLock(lock_ce);
	
	ce.nchan = -1;		// Allows for a test condition to see if this is set.
	ce.ninst = -1;
	ce.t_first = 1;	
	
	// Null for a bunch of stuff
	ce.fname = NULL;
		
	ce.aTask = ce.cTask = ce.oTask = NULL;
	ce.ctname = ce.atname = ce.otname = NULL;
	ce.update_thread = -1;
	ce.atset = 0;
	ce.ctset = 0;

	ce.ccname = NULL;
	ce.icnames = ce.ocnames = NULL;
	
	ce.ochanson = NULL;
	ce.ao_vals = NULL;
	
	ce.cind = 0;
	ce.steps_size = 0;
	ce.cstep = NULL;
	ce.steps = NULL;
	
	ce.ilist = NULL;
	
	ce.fname = NULL;
	ce.path = NULL;
	ce.desc = NULL;
	
	ce.cind = -1;
	
	CmtReleaseLock(lock_ce);
}

int *get_broken_ttl_ctrls() {
	// Convenience function, gets the TTL controls on the Broken TTLs panel
	// The output is malloced, so make sure to free it when you are done.
	int *TTLs = malloc(sizeof(int)*24);
	
	TTLs[0] = BrokenTTLs_TTL0;
	TTLs[1] = BrokenTTLs_TTL1;
	TTLs[2] = BrokenTTLs_TTL2;
	TTLs[3] = BrokenTTLs_TTL3;
	TTLs[4] = BrokenTTLs_TTL4;
	TTLs[5] = BrokenTTLs_TTL5;
	TTLs[6] = BrokenTTLs_TTL6;
	TTLs[7] = BrokenTTLs_TTL7;
	TTLs[8] = BrokenTTLs_TTL8;
	TTLs[9] = BrokenTTLs_TTL9;
	TTLs[10] = BrokenTTLs_TTL10;
	TTLs[11] = BrokenTTLs_TTL11;
	TTLs[12] = BrokenTTLs_TTL12;
	TTLs[13] = BrokenTTLs_TTL13;
	TTLs[14] = BrokenTTLs_TTL14;
	TTLs[15] = BrokenTTLs_TTL15;
	TTLs[16] = BrokenTTLs_TTL16;
	TTLs[17] = BrokenTTLs_TTL17;
	TTLs[18] = BrokenTTLs_TTL18;
	TTLs[19] = BrokenTTLs_TTL19;
	TTLs[20] = BrokenTTLs_TTL20;
	TTLs[21] = BrokenTTLs_TTL21;
	TTLs[22] = BrokenTTLs_TTL22;
	TTLs[23] = BrokenTTLs_TTL23;
	
	return TTLs;
}

/********************************  File I/O  *********************************/
int save_session(char *filename, int safe) { // Primary session saving function
	// Generates a pair of files, an xml file named filename.xml and a program named
	// filename.tdms. If you pass NULL, session_fname is used.
	//
	// If safe evaluates true, this will be executed in a thread-safe manner.
	
	
	//////////////////////////////////////////////////////////////////////////////////
	//																			  	//
	// 			Instructions for saving more things (if it's not clear):			//
	//																				//
	// General note on this: Any time you call a CVIXML function, it should be of 	// 
	// the form : if(rv = CVIXMLFunction(args)) { goto error;}.  					//
	// 																				//
	// It could cause memory leak issues if errors aren't handled this way. The only//
	// exceptions to this is CVIXMLDiscardElement() or CVIXMLDiscardAttribute() 	//
	// which don't need error handling.												//
	//																				//
	// The instructions are the same for adding elements and attributes, but the    //
	// functions are slightly different (CVIXMLSetAttributeValue(attribute, value), //
	// for example.																	//
	//																				//
	// 1. In SessionSavingLib.h, #define a macro for generating the name of the    	//
	//    thing. These should all have the prefix MCXML_.							//
	//																				//
	// 2. Declare all variables at the beginning, and initialize them to 0 or NULL 	//
	//    (0 for all elements and attributes, NULL for strings/pointers) I've been 	// 
	//	  using the convention of name_e for elements and name_a for attributes.  	//
	//																				//
	// 3. Set the value using CVIXMLSetElementValue(element, value) - all elements 	//
	//    must be in the form of strings. There are a number of allocated buffer    //
	//	  vars for the express purpose of "casting" from other data types to string.//
	//	  Figure out the length that you are going to be using, and to be safe use  //
	//    the function g = realloc_if_needed(buff, g, max_len, s). s is the 		//
	//    increment value for the buffer memory, g is the current allocation size,  //
	//    max_len is the length that you might need.								//
	//																				//
	// 4. As soon as you're done with the element, use CVIXMLDiscardElement(element)//
	//    on the variable, then assign element = 0; This is just how I'm handling 	//
	//    the memory management. I'm not sure how much it really takes up, but it's //
	//	  not bad to do here. It can cause serious problems if you don't set the   	//
	//    element back to 0 after calling CVIXMLDiscardElement(element).			//
	//																				//
	// 5. Finally, go to "error" and find where everything's being freed up and add //
	//	  the line: if(element != 0) { CVIXMLDiscardElement(element); }. 			//
	//    This is necessary for memory management under error conditions.			//
	//																				//
	//////////////////////////////////////////////////////////////////////////////////
	
	// Initialize arrays that we'll need to dynamically allocate here as NULL, so
	// that if there's an error, you can tell if they need to be freed.
	char *fbuff = (filename == NULL)?session_fname:filename;
	char *fname = NULL, *buff = NULL, *buff2 = NULL;

	// General stuff
	char *bfname = NULL, *fpath = NULL, *dlpath = NULL, *ppath = NULL, *ddesc = NULL;
	
	// PulseProgConfig stuff
	char *devname, *cnames, *trige, *trigc, *countc, *curcc; 
	devname = cnames = trige = trigc = NULL;
	
	// DataDisp stuff
	char *gains, *colors, *offsets, *f_str;
	gains = colors = offsets = f_str = NULL;
	
	int i, ind, rv = 0;
	
	int flen = strlen(fbuff);
	if(strlen("xml") > strlen(PPROG_EXTENSION)) {
		flen += strlen("xml")+2;	
	} else {
		flen += strlen(PPROG_EXTENSION)+2;	
	}
	
	fname = malloc(flen);
	
	// We're going to start by saving the program, because that has a ui_cleanup call in it
	sprintf(fname, "%s.%s", fbuff, PPROG_EXTENSION);
	SavePulseProgram(NULL, fname, safe);	// This is a thread-safe function - call as appropriate.
	
	sprintf(fname, "%s.xml", fbuff);
	
	// Now create an XML file
	if(safe) { 
		CmtGetLock(lock_uipc);
		CmtGetLock(lock_uidc);
	}
	
	CVIXMLDocument xml_doc = -1;
	CVIXMLElement r_e = 0, pref_e = 0, gen_e = 0, ppc_e = 0, dd_e = 0;
	CVIXMLElement npsrat_e = 0, tview_e = 0;					 
	CVIXMLElement bf_e, fpath_e, at_e, dlpath_e, ppath_e, lfinfo_e, ddesc_e;
	CVIXMLElement dev_e, pbdev_e, chans_e, chann_e, chanr_e, trig_e, bttls_e, curcc_e, countc_e;
   	CVIXMLElement fid_e, spec_e, col_e, gain_e, off_e, con_e;
	bf_e = fpath_e = at_e = dlpath_e = ppath_e = lfinfo_e = ddesc_e = 0;
	dev_e = pbdev_e = chans_e = chann_e = chanr_e = trig_e = bttls_e = curcc_e = countc_e = 0;
	fid_e = spec_e = col_e = gain_e = off_e = con_e = 0;
	
	int g = 200, s = 200, len, g2 = 200;
	buff = malloc(g);
	
	// Create the document
	if(rv = CVIXMLNewDocument(MCXML_RETAG, &xml_doc)) { goto error; } 
	
	// Get the root element
	if(rv = CVIXMLGetRootElement(xml_doc, &r_e)) { goto error; }
	
	// Save version info as attribute
	CVIXMLAddAttribute(r_e, MCXML_VER, MC_VERSION_STRING); 

	// Now create the highest-level child elements.
	if(rv = CVIXMLNewElement(r_e, -1, MCXML_PREFS, &pref_e)) { goto error; }	// User preferences
	if(rv = CVIXMLNewElement(r_e, -1, MCXML_GEN, &gen_e)) { goto error; }		// General controls
	if(rv = CVIXMLNewElement(r_e, -1, MCXML_PPC, &ppc_e)) { goto error; }		// Pulse program configuration
	if(rv = CVIXMLNewElement(r_e, -1, MCXML_DDISP, &dd_e)) { goto error; }		// Data display info
	
	CVIXMLDiscardElement(r_e);
	r_e = 0;
	
	// Now we'll go through and save each piece of information
	//////////////////////////////////////////////////
	//												//
	//				   Preferences					//
	//												//
	//////////////////////////////////////////////////
	// Get the NP, SR and AT prefs.
	void *data;
	int nppref, srpref, atpref;
	
	GetCtrlAttribute(pc.np[1], pc.np[0], ATTR_CALLBACK_DATA, &data);
	nppref = (int)data;
	
	GetCtrlAttribute(pc.sr[1], pc.sr[0], ATTR_CALLBACK_DATA, &data);
	srpref = (int)data;
	
	GetCtrlAttribute(pc.at[1], pc.at[0], ATTR_CALLBACK_DATA, &data);
	atpref = (int)data;
	
	sprintf(buff, "%d;%d;%d", nppref, srpref, atpref);	// Make it a string
	
	if(rv = CVIXMLNewElement(pref_e, -1, MCXML_NPSRAT, &npsrat_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(npsrat_e, buff)) { goto error; }
	
	
	// Get the transient view preferences.
	sprintf(buff, "%d", uidc.disp_update);
	
	if(rv = CVIXMLNewElement(pref_e, -1, MCXML_TVIEW, &tview_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(tview_e, buff)) { goto error; }
	
	CVIXMLDiscardElement(tview_e);
	CVIXMLDiscardElement(npsrat_e);
	CVIXMLDiscardElement(pref_e);
	tview_e = npsrat_e = pref_e =  0;
	
	//////////////////////////////////////////////////
	//												//
	//					 General					//
	//												//
	//////////////////////////////////////////////////
	// Figure out the active tab page and save it.
	int tab;
	GetActiveTabPage(mc.mtabs[1], mc.mtabs[0], &tab);
	sprintf(buff, "%d", tab);
	
	if(rv = CVIXMLNewElement(gen_e, -1, MCXML_ACTIVETAB, &at_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(at_e, buff)) { goto error; }

	CVIXMLDiscardElement(at_e);
	at_e = 0;
	
	// Get the filenames and such that we need to save.
	GetCtrlValStringLength(mc.basefname[1], mc.basefname[0], &len);
	bfname = malloc(len+1);
	
	GetCtrlValStringLength(mc.path[1], mc.path[0], &len);
	
	fpath = malloc(len+1);
	
	GetCtrlValStringLength(mc.datadesc[1], mc.datadesc[0], &len);
	if(len > 0) { 
		ddesc = malloc(len+1);
		GetCtrlVal(mc.datadesc[1], mc.datadesc[0], ddesc);
	}
	
	GetCtrlVal(mc.basefname[1], mc.basefname[0], bfname);
	GetCtrlVal(mc.path[1], mc.path[0], fpath);

	if(uidc.dlpath == NULL)  { uidc.dlpath = fpath; }
	if(uipc.ppath == NULL) { uipc.ppath = "Programs"; }
	
	// Create the elements
	// Base data file name
	if(rv = CVIXMLNewElement(gen_e, -1, MCXML_DFBNAME, &bf_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(bf_e, bfname)) { goto error; }

	// Data path (as displayed)
	if(rv = CVIXMLNewElement(gen_e, -1, MCXML_DFPATH, &fpath_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(fpath_e, fpath)) { goto error; }
	
	// Data load path
	if(rv = CVIXMLNewElement(gen_e, -1, MCXML_DLPATH, &dlpath_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(dlpath_e, uidc.dlpath)) { goto error; }
	
	// Pulse program load path
	if(rv = CVIXMLNewElement(gen_e, -1, MCXML_PPATH, &ppath_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(ppath_e, uipc.ppath)) { goto error; }
	
	// Data description
	if(rv = CVIXMLNewElement(gen_e, -1, MCXML_DDESC, &ddesc_e)) { goto error; }
	if(ddesc != NULL){  
		if(rv = CVIXMLSetElementValue(ddesc_e, ddesc)) { goto error; }
	} else {
		if(rv = CVIXMLSetElementValue(ddesc_e, " ")) { goto error; }
	}
	
	// Load data preference
	int lfinfo;
	GetCtrlVal(mc.ldatamode[1], mc.ldatamode[0], &lfinfo);
	sprintf(buff, "%d", lfinfo);
	
	if(rv = CVIXMLNewElement(gen_e, -1, MCXML_LFINFO, &lfinfo_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(lfinfo_e, buff)) { goto error; }
	
	free(ddesc);
	free(bfname);
	free(fpath);
	ddesc = bfname = fpath = NULL;
	
	CVIXMLDiscardElement(bf_e);
	CVIXMLDiscardElement(fpath_e);
	CVIXMLDiscardElement(ppath_e);
	CVIXMLDiscardElement(dlpath_e);
	CVIXMLDiscardElement(lfinfo_e);
	CVIXMLDiscardElement(ddesc_e);
	CVIXMLDiscardElement(gen_e);
	gen_e = bf_e = fpath_e = dlpath_e = ppath_e = lfinfo_e = ddesc_e = 0;

	//////////////////////////////////////////////////
	//												//
	//			   Pulse Program Config				//
	//												//
	//////////////////////////////////////////////////
	// Start by putting the trigger TTL as an attribute on the element
	int trig_ttl;
	GetCtrlVal(pc.trig_ttl[1], pc.trig_ttl[0], &trig_ttl);
	sprintf(buff, "%d", trig_ttl);
	
	if(rv = CVIXMLAddAttribute(ppc_e, MCXML_TRIGTTL, buff)) { goto error; }
	
	// Get the device name and index and add them to the xml file.
	int nl, cind;
	GetNumListItems(pc.dev[1], pc.dev[0], &nl);
	
	if(nl > 0) {		// This will throw errors if no devices are present
		int devind;
		GetCtrlIndex(pc.dev[1], pc.dev[0], &devind);
		GetLabelLengthFromIndex(pc.dev[1], pc.dev[0], devind, &len);
		devname = malloc(len+1);
		
		GetLabelFromIndex(pc.dev[1], pc.dev[0], devind, devname);
		sprintf(buff, "%d", devind);		// Device index needs to be a string
	} else {
		devname = malloc(3);
		strcpy(devname, "");
		strcpy(buff, "-1");
	}
	
	if(rv = CVIXMLNewElement(ppc_e, -1, MCXML_DEV, &dev_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(dev_e, devname)) { goto error; }
	if(rv = CVIXMLAddAttribute(dev_e, MCXML_INDEX, buff)) { goto error; }
	
	free(devname);
	devname = NULL;
	
	// Now save the Pulseblaster device.
	int pb_dev = -1;
	GetNumListItems(pc.pbdev[1], pc.pbdev[0], &nl);
	
	if(nl > 1) { GetCtrlVal(pc.pbdev[1], pc.pbdev[0], &pb_dev); }
	
	if(pb_dev >= 0) {
		sprintf(buff, "%d", pb_dev);
		
		if(rv = CVIXMLNewElement(ppc_e, -1, MCXML_PBDEV, &pbdev_e)) { goto error; }
		if(rv = CVIXMLSetElementValue(pbdev_e, buff)) { goto error; }
		
		CVIXMLDiscardElement(pbdev_e);
		pbdev_e = 0;
	}
	
	// Now the channels that are on and various information about them.
	int nchans, conlen = 0, cnamelen = 0;
	GetNumListItems(pc.curchan[1], pc.curchan[0], &nchans);
	
	if(nchans) {
		// g is at least 200, and the max number of channels is 8, so even
		// assuming each channel is of size maxint, we would only need 88 bytes
		// to store all of them, so we'll just use buff to hold the indices. 
		if(nchans > 8) { nchans = 8;}
	
		int cinlen = 1, cind, g3 = 200;	// For the channel names.
		cnames = malloc(g3);
		buff2 = malloc(g2);

		strcpy(buff, "");
		strcpy(cnames, "");

		for(i = 0; i < nchans; i++) {
			GetValueFromIndex(pc.curchan[1], pc.curchan[0], i, &cind);
			
			GetLabelLengthFromIndex(pc.ic[1], pc.ic[0], cind, &len);
			cinlen += ++len; // Len needs a null termination, cinlen needs a semicolon
			
			buff2 = realloc_if_needed(buff2, &g2, len, s);
			cnames = realloc_if_needed(cnames, &g3, cinlen, s);
			
			GetLabelFromIndex(pc.ic[1], pc.ic[0], cind, buff2);
			
			sprintf(buff, "%s%d;", buff, cind);
			sprintf(cnames, "%s%s;", cnames, &buff2[1]);
		}
		
		// Now just turn that trailing semicolon into a null.
		buff[strlen(buff)-1] = '\0';
		cnames[strlen(cnames)-1] = '\0';
		free(buff2);
		buff2 = NULL;
	} else {
		cnames = malloc(3);
		strcpy(cnames, "");
		strcpy(buff, "");
	}
	
	// Now write these to their respective elements
	if(rv = CVIXMLNewElement(ppc_e, -1, MCXML_CHANSON, &chans_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(chans_e, buff)) { goto error; }
	
	if(rv = CVIXMLNewElement(ppc_e, -1, MCXML_CHANNAMES, &chann_e))  { goto error; }
	if(rv = CVIXMLSetElementValue(chann_e, cnames)) { goto error; }
	
	// Add the number of channels attribute.
	sprintf(buff, "%d", nchans);
	if(rv = CVIXMLAddAttribute(chans_e, MCXML_NUM, buff)) { goto error; }
	
	// Next we'll do the channel ranges.
	strcpy(buff, "");
	for(i = 0; i < nchans; i++)
		sprintf(buff, "%s%.1f;", buff, uidc.range[i]);		
	
	buff[(strlen(buff)>0)?strlen(buff)-1:0] = '\0';
	
	if(rv = CVIXMLNewElement(ppc_e, -1, MCXML_CHANRANGE, &chanr_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(chanr_e, buff)) { goto error; }
	
	free(cnames);
	cnames = NULL;
	
	// Save the current channel and its index.
	if(nchans) {
		GetCtrlIndex(pc.curchan[1], pc.curchan[0], &ind);
		GetLabelLengthFromIndex(pc.curchan[1], pc.curchan[0], ind, &len);
		curcc = malloc(len+1);
		
		GetLabelFromIndex(pc.curchan[1], pc.curchan[0], ind, curcc);
		sprintf(buff, "%d", ind);
	} else {
		curcc = malloc(3);
		strcpy(curcc, "");
		strcpy(buff, "-1");
	}
	
	if(rv = CVIXMLNewElement(ppc_e, -1, MCXML_CURCHAN, &curcc_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(curcc_e, curcc)) { goto error; } 
	if(rv = CVIXMLAddAttribute(curcc_e, MCXML_INDEX, buff))  { goto error; }
	
	free(curcc);
	curcc = NULL;
	
	// Save the counter channel and its index.
	GetNumListItems(pc.cc[1], pc.cc[0], &nl);
	if(nl > 0) {
		GetCtrlIndex(pc.cc[1], pc.cc[0], &ind);
		GetLabelLengthFromIndex(pc.cc[1], pc.cc[0], ind, &len);
		countc = malloc(len+1);
		
		GetLabelFromIndex(pc.cc[1], pc.cc[0], ind, countc);
		sprintf(buff, "%d", ind);
	} else {
		countc = malloc(3);
		
		strcpy(countc, "");
		strcpy(buff, "-1");
	}
	
	if(rv = CVIXMLNewElement(ppc_e, -1, MCXML_COUNTCHAN, &countc_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(countc_e, countc)) { goto error; }
	
	if(rv = CVIXMLAddAttribute(countc_e, MCXML_INDEX, buff)) { goto error; }
	
	free(countc);
	countc = NULL;
	
	// Now the trigger channel.
	GetNumListItems(pc.trigc[1], pc.trigc[0], &nl);
	if(nl > 0) {
		GetCtrlIndex(pc.trigc[1], pc.trigc[0], &ind);
		GetLabelLengthFromIndex(pc.trigc[1], pc.trigc[0], ind, &len);
		trigc = malloc(len+1);
		
		GetLabelFromIndex(pc.trigc[1], pc.trigc[0], ind, trigc);
	} else {
		trigc = malloc(3);
		strcpy(trigc, "");
	}
	
	int edge;
	GetCtrlVal(pc.trige[1], pc.trige[0], &edge);
	sprintf(buff, "%d", edge);
	
	if(rv = CVIXMLNewElement(ppc_e, -1, MCXML_TRIGCHAN, &trig_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(trig_e, trigc)) { goto error; }
	if(rv = CVIXMLAddAttribute(trig_e, MCXML_TRIGEDGE, buff)) { goto error; }
	free(trigc);
	trigc = NULL;
	
	// Now save the broken ttls.
	sprintf(buff, "%d", uipc.broken_ttls);
	if(rv = CVIXMLNewElement(ppc_e, -1, MCXML_BROKETTLS, &bttls_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(bttls_e, buff)) { goto error; }
	
	// Discard the element
	CVIXMLDiscardElement(ppc_e);
	CVIXMLDiscardElement(dev_e);
	CVIXMLDiscardElement(chans_e);
	CVIXMLDiscardElement(chann_e);
	CVIXMLDiscardElement(chanr_e);
	CVIXMLDiscardElement(trig_e);
	CVIXMLDiscardElement(bttls_e);
	CVIXMLDiscardElement(curcc_e);
	CVIXMLDiscardElement(countc_e);
	ppc_e = dev_e = chans_e = chann_e = chanr_e = trig_e = bttls_e = curcc_e = countc_e = 0;
	
	//////////////////////////////////////////////////
	//												//
	//			      Data Display					//
	//												//
	//////////////////////////////////////////////////
	//// Make the FID and Spectrum elements
	int as;
	if(rv = CVIXMLNewElement(dd_e, -1, MCXML_FID, &fid_e)) { goto error; }
	
	// Add the autoscale attribute to  FID element.
	GetCtrlVal(dc.fid, dc.fauto, &as);
	sprintf(buff, "%d", as);
	if(rv = CVIXMLAddAttribute(fid_e, MCXML_AUTOSCALE, buff)) {	goto error; }	// Autoscale on/off
	
	// Colors will be ######, so that's 6 chars per, plus 7 for seps and a 1 for a newline
	// Precision on gains and offsets will be 4. We'll start with the assumption that most
	// values will be less than 1000, so we'll allocate 10 chars per. If the number is higher,
	// we'll just allocate more space.
	
	int digs, g3 = 89, g4 = 89, s2 = 20;
	colors = malloc(57);
	gains =  malloc(g3);
	offsets = malloc(g4);
	
	strcpy(buff, "");	// For which channels are on
	strcpy(colors, "");
	strcpy(gains, "");
	strcpy(offsets, "");
	f_str = malloc(strlen("%s%.8lf;")+1);
	int gainl = 0, offl = 0;
	
	for(i = 0; i < 8; i++) {
		if(uidc.fchans[i]) { sprintf(buff, "%s%d;", buff, i); }
		
		// Colors
		sprintf(colors, "%s%06x;", colors, uidc.fcol[i]);
		
		// The gains
		digs = (int)log10(uidc.fgain[i])+1;
		sprintf(f_str, "%%s%%.%dlf;", (digs>=8)?1:8-digs);
		digs = (digs>=8)?digs+3:11;
		gainl+= digs;
		
		gains = realloc_if_needed(gains, &g3, gainl, s2);
		
		sprintf(gains, f_str, gains, uidc.fgain[i]);
		
		// The offsets
		digs = (uidc.foff[i] > 0)?((int)log10(uidc.foff[i])+1):1;
		sprintf(f_str, "%%s%%.%dlf;", (digs>=8)?1:8-digs);
		digs = (digs>=8)?digs+3:11;
		offl+= digs;
		
		offsets = realloc_if_needed(offsets, &g4, offl, s2);
		
		sprintf(offsets, f_str, offsets, uidc.foff[i]);
	}
	
	// Now we need to get rid of the trailing semicolon
	len = strlen(buff);
	
	buff[(len > 0)?len-1:0] = '\0';
	colors[strlen(colors)-1] = '\0';
	gains[strlen(gains)-1] = '\0';
	offsets[strlen(offsets)-1] = '\0';
	
	// Finally write these things to elements.
	if(rv = CVIXMLNewElement(fid_e, -1, MCXML_CHANSON, &con_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(con_e, buff)) { goto error; }
	sprintf(buff, "%d", uidc.fnc);
	if(rv = CVIXMLAddAttribute(con_e, MCXML_NUM, buff)) { goto error; } 	// Num channels on

	
	if(rv = CVIXMLNewElement(fid_e, -1, MCXML_COLORS, &col_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(col_e, colors)) { goto error; }
	
	if(rv = CVIXMLNewElement(fid_e, -1, MCXML_GAINS, &gain_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(gain_e, gains)) { goto error; }
	
	if(rv = CVIXMLNewElement(fid_e, -1, MCXML_OFF, &off_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(off_e, offsets)) { goto error; }
	
	// Discard the elements
	CVIXMLDiscardElement(con_e);
	CVIXMLDiscardElement(col_e);
	CVIXMLDiscardElement(gain_e);
	CVIXMLDiscardElement(off_e);
	CVIXMLDiscardElement(fid_e);
	fid_e = con_e = col_e = gain_e = off_e = 0;

	//// Make all the elements and attributes on the Spec element
	if(rv = CVIXMLNewElement(dd_e, -1, MCXML_SPEC, &spec_e)) { goto error; }
	
	GetCtrlVal(dc.spec, dc.sauto, &as);
	sprintf(buff, "%d", as);
	if(rv = CVIXMLAddAttribute(spec_e, MCXML_AUTOSCALE, buff)) { goto error; }
	
	strcpy(buff, "");	// For which channels are on
	strcpy(colors, "");
	strcpy(gains, "");
	strcpy(offsets, "");
	gainl = offl = 0;
	
	for(i = 0; i < 8; i++) {
		if(uidc.schans[i]) { sprintf(buff, "%s%d;", buff, i); }
		
		// Colors
		sprintf(colors, "%s%06x;", colors, uidc.scol[i]);
		
		// The gains
		digs = (int)log10(uidc.sgain[i])+1;
		sprintf(f_str, "%%s%%.%dlf;", (digs>=8)?1:8-digs);
		digs = (digs>=8)?digs+3:11;
		gainl+= digs;
		
		gains = realloc_if_needed(gains, &g3, gainl, s2);
		
		sprintf(gains, f_str, gains, uidc.sgain[i]);
		
		// The offsets
		digs = (uidc.soff[i] > 0)?((int)log10(uidc.soff[i])+1):0;
		sprintf(f_str, "%%s%%.%dlf;", (digs>=8)?1:8-digs);
		digs = (digs>=8)?digs+3:11;
		offl+= digs;
		
		offsets = realloc_if_needed(offsets, &g4, offl, s2);
		
		sprintf(offsets, f_str, offsets, uidc.soff[i]);
	}
	
	// Again, we need to get rid of the trailing semicolon
	len = strlen(buff);
	
	buff[(len > 0)?len-1:0] = '\0';
	colors[strlen(colors)-1] = '\0';
	gains[strlen(gains)-1] = '\0';
	offsets[strlen(offsets)-1] = '\0';
	
	// Finally write these things to elements.
	if(rv = CVIXMLNewElement(spec_e, -1, MCXML_CHANSON, &con_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(con_e, buff)) { goto error; }
	
	sprintf(buff, "%d", uidc.snc);
	if(rv = CVIXMLAddAttribute(con_e, MCXML_NUM, buff)) { goto error; }
	
	if(rv = CVIXMLNewElement(spec_e, -1, MCXML_COLORS, &col_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(col_e, colors)) { goto error; }
	
	if(rv = CVIXMLNewElement(spec_e, -1, MCXML_GAINS, &gain_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(gain_e, gains)) { goto error; }
	
	if(rv = CVIXMLNewElement(spec_e, -1, MCXML_OFF, &off_e)) { goto error; }
	if(rv = CVIXMLSetElementValue(off_e, offsets)) { goto error; }
	
	free(colors);
	free(gains);
	free(offsets);
	free(f_str);
	colors = gains = offsets = f_str = NULL;
	
	CVIXMLDiscardElement(con_e);
	CVIXMLDiscardElement(col_e);
	CVIXMLDiscardElement(gain_e);
	CVIXMLDiscardElement(off_e);
	CVIXMLDiscardElement(spec_e);
	spec_e = con_e = col_e = gain_e = off_e = 0;
	
	// Save the document
	rv = CVIXMLSaveDocument(xml_doc, 1, fname);
	
	error:
	if(xml_doc != -1)
		CVIXMLDiscardDocument(xml_doc);
	
	// Free all the memory that was allocated and not freed.
	if(fname != NULL) { free(fname); }
	if(bfname != NULL) { free(bfname); }
	if(fpath != NULL) { free(fpath); }
	if(dlpath != NULL) { free(dlpath); }
	if(ppath != NULL) { free(ppath); }
	if(ddesc != NULL) { free(ddesc); }
	if(devname != NULL) { free(buff); }
	if(cnames != NULL) { free(cnames); }
	if(trige != NULL) { free(trige); }
	if(trigc != NULL) { free(trigc); }
	if(gains != NULL) { free(gains); }
	if(colors != NULL) { free(colors); }
	if(offsets != NULL) { free(offsets); }
	if(f_str != NULL) { free(f_str); }
	if(buff != NULL) { free(buff); }
	if(buff2 != NULL) { free(buff2); }
	
	// Now discarding the elements.
	if(r_e != 0) { CVIXMLDiscardElement(r_e); }
	if(pref_e != 0) { CVIXMLDiscardElement(pref_e); }
	if(gen_e != 0) { CVIXMLDiscardElement(gen_e); }
	if(ppc_e != 0) { CVIXMLDiscardElement(ppc_e); }
	if(dd_e != 0) { CVIXMLDiscardElement(dd_e); }
	if(npsrat_e != 0) { CVIXMLDiscardElement(npsrat_e); }
	if(tview_e != 0) { CVIXMLDiscardElement(tview_e); }
	if(bf_e != 0) { CVIXMLDiscardElement(bf_e); }
	if(fpath_e != 0) { CVIXMLDiscardElement(fpath_e); }
	if(dlpath_e != 0) { CVIXMLDiscardElement(dlpath_e); }
	if(ddesc_e != 0) { CVIXMLDiscardElement(ddesc_e); }
	if(lfinfo_e != 0) { CVIXMLDiscardElement(lfinfo_e); }
	if(ppath_e != 0) { CVIXMLDiscardElement(ppath_e); }
	if(at_e != 0) { CVIXMLDiscardElement(at_e); }
	if(dev_e != 0) { CVIXMLDiscardElement(dev_e); }
	if(pbdev_e != 0) { CVIXMLDiscardElement(pbdev_e); }
	if(chans_e != 0) { CVIXMLDiscardElement(chans_e); }
	if(chann_e != 0) { CVIXMLDiscardElement(chann_e); }
	if(chanr_e != 0) { CVIXMLDiscardElement(chanr_e); }
	if(trig_e != 0) { CVIXMLDiscardElement(trig_e); }
	if(bttls_e != 0) { CVIXMLDiscardElement(bttls_e); }
	if(curcc_e != 0) { CVIXMLDiscardElement(curcc_e); }
  	if(countc_e != 0) { CVIXMLDiscardElement(countc_e); }
	if(fid_e != 0) { CVIXMLDiscardElement(fid_e); }
	if(spec_e != 0) { CVIXMLDiscardElement(spec_e); }
	if(col_e != 0) { CVIXMLDiscardElement(col_e); }
	if(gain_e != 0) { CVIXMLDiscardElement(gain_e); }
	if(off_e != 0) { CVIXMLDiscardElement(off_e); }
	if(con_e != 0) { CVIXMLDiscardElement(con_e); }

	if(safe) {
		CmtReleaseLock(lock_uipc);
		CmtReleaseLock(lock_uidc);
	}
	
	return rv;
}

int load_session(char *filename, int safe) { // Primary session loading function
 	// Generates a pair of files, an xml file named filename.xml and a program named
	// filename.tdms. If you pass NULL, session_fname is used.
	//
	// If safe returns true, this will get the thread locks lock_uidc,
	// lock_uipc and lock_tdm, lock_DAQ.
	
	//////////////////////////////////////////////////////////////////////////////////
	//																			  	//
	// 			Instructions for loading more things (if it's not clear):			//
	//																				//
	// General note on this: Any time you call a CVIXML function, it should be of 	// 
	// the form : if(rv = CVIXMLFunction(args)) { goto error;}.  					//
	// 																				//
	// It could cause memory leak issues if errors aren't handled this way. The only//
	// exceptions to this is CVIXMLDiscardElement() or CVIXMLDiscardAttribute() 	//
	// which don't need error handling.												//
	//																				//
	// The instructions are the same for adding elements and attributes, but the    //
	// functions are slightly different (CVIXMLSetAttributeValue(attribute, value), //
	// for example.																	//
	//																				//
	// 1. In SessionSavingLib.h, #define a macro for generating the name of the    	//
	//    thing. These should all have the prefix MCXML_.							//
	//																				//
	// 2. Declare all variables at the beginning, and initialize them to 0 or NULL 	//
	//    (0 for all elements and attributes, NULL for strings/pointers) I've been 	// 
	//	  using the convention of name_e for elements and name_a for attributes.  	//
	//																				//
	// 3. Before the parent element is discarded, get the element tag using 		//
	//    CVIXMLGetChildElementByTag(parent, MCXML_Tagname, &name_e)				//
	//																				//
	// 4. When you are ready to get the value from the XML file, encapsulate the 	//
	//    relevant section under the heading if(name_e != 0). In the if statement,  //
	//    Start with CVIXMLGetElementValueLength(name_e, &len) to get the length of //
	//    the value you're retrieving. Then call g = realloc_if_needed(...) to make //
	//    sure there's enough room in the buffer. Get the value using the function	//
	//    CVIXMLGetElementValue(name_e, buff). You can then use sscanf or whatever  //
	//    to get it in a more friendly format if needed.							//
	//																				//
	// 5. When you have gotten the value and used it however you want, still within //
	//    the if(name_e != 0) statement, discard the element/attr and set the var 	//
	//    back to 0.		//														//
	//																				//
	// 6. Finally, go to "error" and find where everything's being freed up and add //
	//	  the line: if(element != 0) { CVIXMLDiscardElement(element); }. 			//
	//    This is necessary for memory management under error conditions.			//
	//																				//
	//////////////////////////////////////////////////////////////////////////////////

	// Initialize arrays that we'll need to dynamically allocate here as NULL, so
	// that if there's an error, you can tell if they need to be freed.
	int i, nl, ind, num, len, rv = 0;
	int *inds = NULL;
	float *ranges = NULL;
	char *fbuff = (filename == NULL)?session_fname:filename;
	char *fname = NULL, *buff = NULL, *buff2 = NULL, *p = NULL;

	// PulseProgConfig and also ddesc
	char *devname, *cnames, *trige, *trigc, *countc, *curcc, *ddesc;
	devname = cnames = trige = trigc = countc = curcc = ddesc = NULL;
	
	// Our first memory allocation stuff.
	int g = 200, s = 200, g2 = 200;
	buff = malloc(g);
	buff2 = malloc(g2);

	int flen = strlen(fbuff);
	if(strlen("xml") > strlen(PPROG_EXTENSION)) {
		flen += strlen("xml")+2;
	} else {
		flen += strlen(PPROG_EXTENSION) + 2;
	}
	
	fname = malloc(flen);
	
	if(safe) { 
		CmtGetLock(lock_uidc);
		CmtGetLock(lock_uipc);
	}
	
	CVIXMLDocument xml_doc = -1;
	
	// Elements
	CVIXMLElement r_e = 0, pref_e = 0, gen_e = 0, ppc_e = 0, dd_e = 0;
	CVIXMLElement npsrat_e = 0, tview_e = 0;
	CVIXMLElement at_e,  bf_e, fpath_e, ppath_e, dlpath_e, lfinfo_e, ddesc_e;
	CVIXMLElement dev_e, pbdev_e, chans_e, chann_e, chanr_e, trig_e, bttls_e, curcc_e, countc_e;
	CVIXMLElement fid_e, spec_e, con_e, col_e, gain_e, off_e;
	at_e = bf_e = fpath_e = ppath_e = dlpath_e = lfinfo_e = ddesc_e = 0;
	dev_e = pbdev_e = chans_e = chann_e = chanr_e = trig_e = bttls_e = curcc_e = countc_e = 0;
	fid_e = spec_e = con_e = col_e = gain_e = off_e = 0;
	
	// Attributes
	CVIXMLAttribute ind_a = 0, num_a = 0;
	CVIXMLAttribute trttl_a = 0, edge_a = 0;
	CVIXMLAttribute as_a = 0;
	
	// Unlike with the save_session function, we'll end with the pulse program, since we need
	// to make sure broken ttls and trigger ttls and such are set up.
	sprintf(fname, "%s.xml", fbuff);
	if(rv = CVIXMLLoadDocument(fname, &xml_doc)) { goto error; }
	
	// Get the root element and the main elements.
	if(rv = CVIXMLGetRootElement(xml_doc, &r_e)) { goto error; }
	if(rv = CVIXMLGetChildElementByTag(r_e, MCXML_PREFS, &pref_e)) { goto error; }
	if(rv = CVIXMLGetChildElementByTag(r_e, MCXML_GEN, &gen_e)) { goto error; }
	if(rv = CVIXMLGetChildElementByTag(r_e, MCXML_PPC, &ppc_e)) { goto error; }
	if(rv = CVIXMLGetChildElementByTag(r_e, MCXML_DDISP, &dd_e)) { goto error; }
	
	//////////////////////////////////////////////////
	//												//
	//				   Preferences					//
	//												//
	//////////////////////////////////////////////////
	
	// Get the child elements
	if(pref_e != 0) {
		if(rv = CVIXMLGetChildElementByTag(pref_e, MCXML_NPSRAT, &npsrat_e)) { goto error; } 	// NP, SR, AT Prefs
		if(rv = CVIXMLGetChildElementByTag(pref_e, MCXML_TVIEW, &tview_e)) { goto error; }		// Transient view pref
		
		CVIXMLDiscardElement(pref_e);
		pref_e = 0;
	}
	
	if(npsrat_e != 0) {	// Only do this stuff if we found the element.			
		if(rv = CVIXMLGetElementValue(npsrat_e, buff))	// This can't overrun the buffer
			goto error;
	
		int np = -1, sr = -1, at = -1;
		sscanf(buff, "%d;%d;%d", &np, &sr, &at);
		
		CtrlCallbackPtr cb;		// Need to get a pointer to the callback function
		GetCtrlAttribute(pc.np[1], pc.np[0], ATTR_CALLBACK_FUNCTION_POINTER, &cb);
		
		if(np != -1) { InstallCtrlCallback(pc.np[1], pc.np[0], cb, (void *)np); }
		if(sr != -1) { InstallCtrlCallback(pc.sr[1], pc.sr[0], cb, (void *)sr); }
		if(at != -1) { InstallCtrlCallback(pc.at[1], pc.at[0], cb, (void *)at); }
		
		CVIXMLDiscardElement(npsrat_e);
		npsrat_e = 0;
	}
	
	
	if(tview_e != 0) {
		if(rv = CVIXMLGetElementValue(tview_e, buff)) { goto error; }
		
		int tview = -1;
		sscanf(buff, "%d", &tview);
		
		if(tview < 0 || tview > 2) { tview = 0; }
		
		uidc.disp_update = tview; // Update the uidc var.
		
		// Need to make sure that the menu bar checking reflects the uidc var. 

		for(i = 0; i < 3; i++) {
			SetMenuBarAttribute(mc.mainmenu, mc.vtviewopts[i], ATTR_CHECKED, (i == tview)?1:0);
		}
		
		// Free memory
		CVIXMLDiscardElement(tview_e);
		tview_e = 0;
	}
	
	//////////////////////////////////////////////////
	//												//
	//					 General					//
	//												//
	//////////////////////////////////////////////////
	
	// First we get all the elements.
	if(gen_e != 0) {
		if(rv = CVIXMLGetChildElementByTag(gen_e, MCXML_ACTIVETAB, &at_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(gen_e, MCXML_DFBNAME, &bf_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(gen_e, MCXML_DFPATH, &fpath_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(gen_e, MCXML_DLPATH, &dlpath_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(gen_e, MCXML_DDESC, &ddesc_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(gen_e, MCXML_LFINFO, &lfinfo_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(gen_e, MCXML_PPATH, &ppath_e)) { goto error; }
		
		CVIXMLDiscardElement(gen_e);
		
		gen_e = 0;
	}
	
	// Then one by one we try and get their values.
	// Active tab
	if(at_e != 0) {
		if(rv = CVIXMLGetElementValue(at_e, buff)) { goto error; }
		
		int tab = -1;
		sscanf(buff, "%d", &tab);
		if(tab != -1)
			SetActiveTabPage(mc.mtabs[1], mc.mtabs[0], tab);
		
		CVIXMLDiscardElement(at_e);
		at_e = 0;
	}
	
	// Base filename
	if(bf_e != 0) {
		if(rv = CVIXMLGetElementValueLength(bf_e, &len)) { goto error; }
		
		buff = realloc_if_needed(buff, &g, len, s);
		
		if(rv = CVIXMLGetElementValue(bf_e, buff)) { goto error; }
		
		if(len > 0) 
		 SetCtrlVal(mc.basefname[1], mc.basefname[0], buff); 
		
		CVIXMLDiscardElement(bf_e);
		bf_e = 0;
	}
	
	// Data path
	if(fpath_e != 0) {
		if(rv = CVIXMLGetElementValueLength(fpath_e, &len)) { goto error; }
		
		buff = realloc_if_needed(buff, &g, len, s);
		
		if(rv = CVIXMLGetElementValue(fpath_e, buff)) { goto error; }
		
		if(len > 0)
			SetCtrlVal(mc.path[1], mc.path[0], buff);
		
		CVIXMLDiscardElement(fpath_e);
		fpath_e = 0;
	}
	
	// Data load path
	if(dlpath_e != 0) {
		if(rv = CVIXMLGetElementValueLength(dlpath_e, &len)) { goto error; }
		
		buff = realloc_if_needed(buff, &g, len, s);
		
		if(rv = CVIXMLGetElementValue(dlpath_e, buff)) { goto error; }
		
		// We don't want to set dlpath to anything but NULL unless we actually have something to put there.
		if(len > 0) {
			uidc.dlpath = malloc(len+1);
			strcpy(uidc.dlpath, buff);
		}

		CVIXMLDiscardElement(dlpath_e);
		dlpath_e = 0;
	}
	
	// Data description
	if(ddesc_e != 0) {
		if((rv = CVIXMLGetElementValueLength(ddesc_e, &len)) && rv != 1) { goto error; }
		ddesc = malloc(len+1);
		
		if(len > 0) {
			if(rv = CVIXMLGetElementValue(ddesc_e, ddesc)) { goto error; }
		
			SetCtrlVal(mc.datadesc[1], mc.datadesc[0], ddesc);
		}
		
		free(ddesc);
		ddesc = NULL;

		CVIXMLDiscardElement(ddesc_e);
		ddesc_e = 0;
	}
	
	// Program load path
	if(ppath_e != 0) {
		if(rv = CVIXMLGetElementValueLength(ppath_e, &len)) { goto error; }
		
		buff = realloc_if_needed(buff, &g, len, s);
		
		// As above, load into a buffer first, in case there's an error.
		if(rv = CVIXMLGetElementValue(ppath_e, buff)) { goto error; }
		
		if(len > 0) {
			uipc.ppath = malloc(len+1);
			strcpy(uipc.ppath, buff);;
		}
		
		CVIXMLDiscardElement(ppath_e);
		ppath_e = 0;
	}
	
	// Load file info preference
	if(lfinfo_e != 0) {								// Boolean, can't overrun buffer.
		if(rv = CVIXMLGetElementValue(lfinfo_e, buff)) { goto error; }
		
		int lfinfo = 0;			// Default to no
		sscanf(buff, "%d", &lfinfo);
		
		SetCtrlVal(mc.ldatamode[1], mc.ldatamode[0], lfinfo);
		
		CVIXMLDiscardElement(lfinfo_e);
		lfinfo_e = 0;
	}
	
	//////////////////////////////////////////////////
	//												//
	//			   Pulse Program Config				//
	//												//
	//////////////////////////////////////////////////
	
	// Get the elements and attributes, if possible.
	if(ppc_e != 0) {
		if(rv = CVIXMLGetAttributeByName(ppc_e, MCXML_TRIGTTL, &trttl_a))  { goto error; }
		if(rv = CVIXMLGetChildElementByTag(ppc_e, MCXML_DEV, &dev_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(ppc_e, MCXML_PBDEV, &pbdev_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(ppc_e, MCXML_CHANSON, &chans_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(ppc_e, MCXML_CHANNAMES, &chann_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(ppc_e, MCXML_CHANRANGE, &chanr_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(ppc_e, MCXML_CURCHAN, &curcc_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(ppc_e, MCXML_COUNTCHAN, &countc_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(ppc_e, MCXML_TRIGCHAN, &trig_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(ppc_e, MCXML_BROKETTLS, &bttls_e)) { goto error; }
		
		CVIXMLDiscardElement(ppc_e);
		ppc_e = 0;
	}
	
	// Now we go through and get all the elements and attributes we found.
	// Trigger TTL
	if(trttl_a != 0) {
		if(rv = CVIXMLGetAttributeValue(trttl_a, buff)) { goto error; }
		
		int trig_ttl = -1;
		sscanf(buff, "%d", &trig_ttl);
		
		if(trig_ttl >= 0 && trig_ttl < 24) {
			uipc.trigger_ttl = trig_ttl;
			SetCtrlVal(pc.trig_ttl[1], pc.trig_ttl[0], trig_ttl);
		}
		
		CVIXMLDiscardAttribute(trttl_a);
		trttl_a = 0;
	}
	
	// Device
	if(dev_e != 0) {
		int o_ind = -2;
		
		// Get the index attribute first, I guess.
		if(rv = CVIXMLGetAttributeByName(dev_e, MCXML_INDEX, &ind_a)) { goto error; }
		
		if(ind_a != 0) {
			if(rv = CVIXMLGetAttributeValue(ind_a, buff)) { goto error; }
			
			sscanf(buff, "%d", &o_ind);
			CVIXMLDiscardAttribute(ind_a);
			ind_a = 0;
		}
		
		if(o_ind != -1) {
			// Then the value
			if(rv = CVIXMLGetElementValueLength(dev_e, &len)) { goto error; }
		
			buff = realloc_if_needed(buff, &g, len, s);
		
			if(rv = CVIXMLGetElementValue(dev_e, buff)) { goto error; }
		
			// Now get the value as close as possible.
			GetNumListItems(pc.dev[1], pc.dev[0], &nl);
			if(nl > 0 && len > 0) {
				int old_ind;
				GetCtrlIndex(pc.dev[1], pc.dev[0], &old_ind);
				GetIndexFromValue(pc.dev[1], pc.dev[0], &ind, buff);
				if(ind >= 0)
					SetCtrlIndex(pc.dev[1], pc.dev[0], ind);
				else if (o_ind >= 0 && o_ind < nl)
					SetCtrlIndex(pc.dev[1], pc.dev[0], o_ind);
				else if (o_ind >= nl)
					SetCtrlIndex(pc.dev[1], pc.dev[0], nl-1);
				
				// We need to reload the DAQ info for the given device if it's been changed.
				GetCtrlIndex(pc.dev[1], pc.dev[0], &ind);
				if(ind != old_ind) {
					load_DAQ_info_safe(0, 0, 1);
				}
			}
		}
		
		CVIXMLDiscardElement(dev_e);
		dev_e = 0;
	}
	
	// PulseBlaster Device
	if(pbdev_e != 0) { 
		if(rv = CVIXMLGetElementValue(pbdev_e, buff)) { goto error; } 	// Can't cause buffer overruns
		
		int pbdev = 0, nl;
		sscanf(buff, "%d", &pbdev);
		
		GetNumListItems(pc.pbdev[1], pc.pbdev[0], &nl);
		
		if(pbdev < nl)
			SetCtrlVal(pc.pbdev[1], pc.pbdev[0], pbdev);
		
		CVIXMLDiscardElement(pbdev_e);
		pbdev_e = 0;
	}
	
	// Channels
	if(chans_e != 0) {
		// First get the number of channels attribute.
		if(rv = CVIXMLGetAttributeByName(chans_e, MCXML_NUM, &num_a)) { goto error; }
		
		if(num_a != 0) {
			num = -2;
			if(rv = CVIXMLGetAttributeValue(num_a, buff)) { goto error; }
			
			sscanf(buff, "%d", &num);
			CVIXMLDiscardAttribute(num_a);
			num_a = 0;
		}
		
		// Get which indexes they are.
		if(num > 0) {
			inds =  malloc(sizeof(int)*num);
			CmtGetLock(lock_uidc);
			uidc.onchans = num;
			
			if(rv = CVIXMLGetElementValue(chans_e, buff)) { goto error; }
			p = strtok(buff, ";");
			i = 0;
			// This should read out each one.
			while(p != NULL && i < num) {
				if(sscanf(p, "%d", &ind) && ind < uidc.nchans && ind >= 0) {
					inds[i++] = ind;
				} else {
					inds[i++] = -1;
					uidc.onchans--;
				}
				p = strtok(NULL, ";");
			}
			
			CmtReleaseLock(lock_uidc);
		}
		CVIXMLDiscardElement(chans_e);
		chans_e = 0;
		
		// Now we'll actually try and set them to the original channels.
		if(chann_e != 0 && inds != NULL) {
			if(rv = CVIXMLGetElementValueLength(chann_e, &len)) { goto error; }
			
			// The values are of the form Device/channel
			int dlen;
			GetCtrlValStringLength(pc.dev[1], pc.dev[0], &dlen);
			devname = malloc(++dlen);
			GetCtrlVal(pc.dev[1], pc.dev[0], devname);
			
			buff = realloc_if_needed(buff, &g, len+1, s);
			
			// Read the value into the buffer.
			if(len > 0 &&  (rv = CVIXMLGetElementValue(chann_e, buff))) { goto error; }
			
			i = -1;
			p = strtok(buff, ";");
			while(p != NULL && i < num-1) {
				if(inds[++i] >= 0) {
					sprintf(buff2, "%s/%s", devname, p);	// Make the name
					GetIndexFromValue(pc.ic[1], pc.ic[0], &ind, buff2);
					if(ind >= 0 && ind < nl)
						inds[i] = ind;
				}
				
				p = strtok(NULL, ";");
			}
		}
		
		if(chann_e != 0) { CVIXMLDiscardElement(chann_e); }
		chann_e = 0;
		
		// Now we get the ranges.
		if(chanr_e != 0 && inds != NULL) {
			if(rv = CVIXMLGetElementValue(chanr_e, buff)) { goto error; }
			
			i = -1;
			p = strtok(buff, ";");
			ranges = calloc(sizeof(float)*num, sizeof(float));
			float range;
			while(p != NULL && i < num-1) {
				if(inds[++i] >= 0 && sscanf(p, "%f", &range) && range > 0.0)
					ranges[i] = range;
				
				p = strtok(NULL, ";"); 
			}
		}
		
		if(chanr_e != 0) { CVIXMLDiscardElement(chanr_e); }
		chanr_e = 0;
		
		if(inds != NULL) { DeleteListItem(pc.curchan[1], pc.curchan[0], 0, -1); }
		
		// Now we should have a list of ranges and indices, and we can set up
		// the proper user interface.
		int j = 0;
		int vlen;
		
		uidc.onchans = 0;
		for(i = 0; i < num; i++) {
			if(inds[i] < 0)
				continue;
			
			GetLabelLengthFromIndex(pc.ic[1], pc.ic[0], inds[i], &len);
			GetValueLengthFromIndex(pc.ic[1], pc.ic[0], inds[i], &vlen);
			
			buff = realloc_if_needed(buff, &g, len, s);
			buff2 = realloc_if_needed(buff2, &g2, len, s);
			
			GetLabelFromIndex(pc.ic[1], pc.ic[0], inds[i], buff);
			GetValueFromIndex(pc.ic[1], pc.ic[0], inds[i], buff2);
			
			add_chan(&buff[1], inds[i]);
			buff[0] = 149;
			ReplaceListItem(pc.ic[1], pc.ic[0], inds[i], buff, buff2);
			
			if(ranges[i] > 0)
				uidc.range[j] = ranges[i];
			
			j++;	// Counter of how many things we've added.
		}

		SetCtrlVal(pc.nc[1], pc.nc[0], uidc.onchans);
		
		if(ranges != NULL) { free(ranges); }
		if(inds != NULL) { free(inds); }
		if(devname != NULL) { free(devname); }
		
		ranges = NULL;
		inds = NULL;
		devname = NULL;
	}

	// Current channel
	if(curcc_e != 0) {
		if(rv = CVIXMLGetAttributeByName(curcc_e, MCXML_INDEX, &ind_a)) { goto error; }
		
		int o_ind = -1;

		if((rv = CVIXMLGetElementValueLength(curcc_e, &len)) && rv != 1) { goto error; } 
		GetNumListItems(pc.curchan[1], pc.curchan[0], &nl);

		if(ind_a != 0) {
			if(rv = CVIXMLGetAttributeValue(ind_a, buff)) { goto error; }
			
			sscanf(buff, "%d", &o_ind);
			
			CVIXMLDiscardAttribute(ind_a);
			ind_a = 0;
		}
		
		if(nl > 0 && len > 0) {
			buff = realloc_if_needed(buff, &g, len+1, s);
			if(rv = CVIXMLGetElementValue(curcc_e, buff)) { goto error; }
			
			// Find the index that this points to.
			ind = -1;
			for(i = 0; i < nl; i++) {
				GetLabelLengthFromIndex(pc.curchan[1], pc.curchan[0], i, &len);
				buff2 = realloc_if_needed(buff2, &g2, len+1, s);
				
				GetLabelFromIndex(pc.curchan[1], pc.curchan[0], i, buff2);
				
				if(strcmp(buff2, buff) == 0) {
					ind = i;
					break;
				}
			}
			
			if(ind >= 0)
				SetCtrlIndex(pc.curchan[1], pc.curchan[0], ind);
			else if (o_ind >= 0 && o_ind < nl)
				SetCtrlIndex(pc.curchan[1], pc.curchan[0], o_ind);
			else if (o_ind >= nl)
				SetCtrlIndex(pc.curchan[1], pc.curchan[0], nl-1);
		}
		
		CVIXMLDiscardElement(curcc_e);
		curcc_e = 0;
	}
	
	// Counter Channel
	if(countc_e != 0) {
		int o_ind = -2;
		if(rv = CVIXMLGetAttributeByName(countc_e, MCXML_INDEX, &ind_a)) { goto error;}
		
		if(ind_a != 0) {
			if(rv = CVIXMLGetAttributeValue(ind_a, buff)) { goto error; }
			
			sscanf(buff, "%d", &o_ind);
			
			CVIXMLDiscardAttribute(ind_a);
			ind_a = 0;
		}
		
		if((rv = CVIXMLGetElementValueLength(countc_e, &len)) && rv != 1) { goto error; }
		
		GetNumListItems(pc.cc[1], pc.cc[0], &nl);
		if(nl > 0 && len > 0) {
			buff = realloc_if_needed(buff, &g, len+1, s);
			if(rv = CVIXMLGetElementValue(countc_e, buff)) { goto error; }
			
			// Find the index that this points to.
			ind = -1;
			for(i = 0; i < nl; i++) {
				GetLabelLengthFromIndex(pc.cc[1], pc.cc[0], i, &len);
				buff2 = realloc_if_needed(buff2, &g2, len+1, s);
				
				GetLabelFromIndex(pc.cc[1], pc.cc[0], i, buff2);
				
				if(strcmp(buff2, buff) == 0) {
					ind = i;
					break;
				}
			}
			
			if(ind >= 0)
				SetCtrlIndex(pc.cc[1], pc.cc[0], ind);
			else if (o_ind >= 0 && o_ind < nl)
				SetCtrlIndex(pc.cc[1], pc.cc[0], o_ind);
			else if (o_ind >= nl)
				SetCtrlIndex(pc.cc[1], pc.cc[0], nl-1);
		}
		
		CVIXMLDiscardElement(countc_e);
		countc_e = 0;
	}

	// Trigger channel
	if(trig_e) {
		if((rv = CVIXMLGetElementValueLength(trig_e, &len)) && rv != 1) { goto error; }
		
		GetNumListItems(pc.trigc[1], pc.trigc[0], &nl);
		if(nl > 0 && len > 0) {
			buff = realloc_if_needed(buff, &g, len+1, s);
			if(rv = CVIXMLGetElementValue(trig_e, buff)) { goto error; }
			
			// Find the index that this points to, if it's there.
			ind = -1;
			for(i = 0; i < nl; i ++) {
				GetLabelLengthFromIndex(pc.trigc[1], pc.cc[0], i, &len);
				buff2 = realloc_if_needed(buff2, &g2, len+1, s);
				
				GetLabelFromIndex(pc.trigc[1], pc.trigc[0], i, buff2);
				
				if(strcmp(buff2, buff) == 0) {
					ind = i;
					break;
				}
			}
			
			if(ind >= 0)
				SetCtrlIndex(pc.trigc[1], pc.trigc[0], ind);
		}
		
		if(rv = CVIXMLGetAttributeByName(trig_e, MCXML_TRIGEDGE, &edge_a)) { goto error; }
		
		if(edge_a != 0) {
			if(rv = CVIXMLGetAttributeValue(edge_a, buff)) { goto error; } 
			
			int edge = -1;
			sscanf(buff, "%d", &edge);
			
			if(edge >= 0) 
				SetCtrlVal(pc.trige[1], pc.trige[0], edge);	
			
			CVIXMLDiscardAttribute(edge_a);
			edge_a = 0;
		}
		
		CVIXMLDiscardElement(trig_e);
		trig_e = 0;
	}
	
	// Broken TTLS
	if(bttls_e != 0) {
		if(rv = CVIXMLGetElementValue(bttls_e, buff)) { goto error; }
		
		sscanf(buff, "%d", &uipc.broken_ttls);
		
		CVIXMLDiscardElement(bttls_e);
		bttls_e = 0;
	}
	
	//////////////////////////////////////////////////
	//												//
	//			      Data Display					//
	//												//
	//////////////////////////////////////////////////
	// Get the main elements
	if(dd_e != 0) {
		if(rv = CVIXMLGetChildElementByTag(dd_e, MCXML_FID, &fid_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(dd_e, MCXML_SPEC, &spec_e)) { goto error; }
		
		CVIXMLDiscardElement(dd_e);
		dd_e = 0;
	}
	
	// Most of this is exactly the same thing, so we'll just loop through.
	int pan[2] = {dc.fid, dc.spec};
	int as_c[2] = {dc.fauto, dc.sauto};
	int ch_c[2][8];
	for(i = 0; i < 8; i++) {
		ch_c[0][i] = dc.fchans[i];
		ch_c[1][i] = dc.schans[i];
	}

	CVIXMLElement *element, e;
	
	for(int j = 0; j < 2; j++) {
		element = j?&spec_e:&fid_e;
		e = *element;
		
		if(e != 0) {
			// The easy one first - autoscale.
			if(rv = CVIXMLGetAttributeByName(e, MCXML_AUTOSCALE, &as_a)) { goto error; }
		
			if(as_a != 0) {
				if(rv = CVIXMLGetAttributeValue(as_a, buff)) { goto error; }
			
				int as = -1;
				sscanf(buff, "%d", &as);
			
				if(as != -1) { SetCtrlVal(pan[j], as_c[j], as); }
			
				CVIXMLDiscardAttribute(as_a);
				as_a = 0;
			}
			
			// Now we go through and do channels, colors, gains and offsets.
			// Channels
			if(rv = CVIXMLGetChildElementByTag(e, MCXML_CHANSON, &con_e)) { goto error; }
			if(con_e != 0) {
				// Get the number attribute first.
				if(rv = CVIXMLGetAttributeByName(con_e, MCXML_NUM, &num_a)) { goto error; }
				
				num = -1;
				if(num_a != 0) {
					if(rv = CVIXMLGetAttributeValue(num_a, buff)) { goto error; }
					
					sscanf(buff, "%d", &num);
					CVIXMLDiscardAttribute(num_a);
					num_a = 0;
				}
				
				if(rv = CVIXMLGetElementValueLength(con_e, &len) < 0) { goto error; }
				
				if(rv == 0 && len > 0) {
					buff = realloc_if_needed(buff, &g, len+1, s);
					
					if(rv = CVIXMLGetElementValue(con_e, buff)) { goto error; }
					
					int chan, on;
					i = 0;
					p = strtok(buff, ";");
					
					while(p != NULL && i < 8) {
						chan = -1;
						sscanf(p, "%d", &chan);
						
						if(chan >= 0 && chan < 8) {
							GetCtrlVal(pan[j], ch_c[j][chan], &on);
							if(!on) {
								SetCtrlVal(pan[j], ch_c[j][chan], 1);
								j?toggle_spec_chan(chan):toggle_fid_chan(chan);
							}
						}
						
						i++;
						p = strtok(NULL, ";");
					}
				} else if (num == 0) {
					int val;
					for(i = 0; i < 8; i++) {
						GetCtrlVal(pan[j], ch_c[j][i], &val);
						if(val) {
							SetCtrlVal(pan[j], ch_c[j][i], 0);
							j?toggle_spec_chan(i):toggle_fid_chan(i);
						}
					}
				}
				
				CVIXMLDiscardElement(con_e);
				con_e = 0;
			}
			
			// Colors
			if(rv = CVIXMLGetChildElementByTag(e, MCXML_COLORS, &col_e)) { goto error; }
			if(col_e != 0) { 
				if(rv = CVIXMLGetElementValueLength(col_e, &len)) { goto error; }
				
				if(len > 0) {
					buff = realloc_if_needed(buff, &g, len+1, s);
					
					if(rv = CVIXMLGetElementValue(col_e, buff)) { goto error; }
					
					i = 0;
					p = strtok(buff, ";");
					while(p != NULL && i < 8) {
						sscanf(p, "%x", j?&uidc.scol[i++]:&uidc.fcol[i++]);
						p = strtok(NULL, ";");
					}
				}
				
				CVIXMLDiscardElement(col_e);
				col_e = 0;
			}
			
			// Gains
			if(rv = CVIXMLGetChildElementByTag(e, MCXML_GAINS, &gain_e)) { goto error; }
			if(gain_e != 0) {
				if(rv = CVIXMLGetElementValueLength(gain_e, &len)) { goto error; }
				
				if(len > 0) {
					buff = realloc_if_needed(buff, &g, len+1, s);
					
					if(rv = CVIXMLGetElementValue(gain_e, buff)) { goto error; }
					
					i = 0; 
					p = strtok(buff, ";");
					while(p != NULL && i < 8) {
						sscanf(p, "%f", j?&uidc.sgain[i++]:&uidc.fgain[i++]);
						p = strtok(NULL, ";");
					}
				}
				
				CVIXMLDiscardElement(gain_e);
				gain_e = 0;
			}
			
			// Offsets
			if(rv = CVIXMLGetChildElementByTag(e, MCXML_OFF, &off_e)) { goto error; }
			if(off_e != 0) {
				if(rv = CVIXMLGetElementValueLength(off_e, &len)) { goto error; }
				
				if(len > 0) { 
					buff = realloc_if_needed(buff, &g, len+1, s);
				
					if(rv = CVIXMLGetElementValue(off_e, buff)) { goto error; }
				
					i = 0;
					p = strtok(buff, ";");
					while(p != NULL && i < 8) {
						sscanf(p, "%f", j?&uidc.soff[i++]:&uidc.foff[i++]);
						p = strtok(NULL, ";");
					}
				}
				
				CVIXMLDiscardElement(off_e);
				off_e = 0;
			}   
			
		}
		
		CVIXMLDiscardElement(e);
		*element = 0;
	}
	
	
	// Finally, we load the program
	sprintf(fname, "%s.%s", fbuff, PPROG_EXTENSION);
	if(FileExists(fname, NULL)) {
		PPROGRAM *p = LoadPulseProgram(fname, safe, &rv);
		
		if(rv) { goto error; }
		
		if(p != NULL) {
			set_current_program(p);
			free_pprog(p);
		}
	}
	
	
	error:
	// Now free memory that's been allocated and not freed
	if(fname != NULL) { free(fname); }
	if(buff != NULL) { free(buff); }
	if(buff2 != NULL) { free(buff2); }
	if(devname != NULL) { free(devname); }
	if(cnames != NULL) { free(cnames); }
	if(trige != NULL) { free(trige); }
	if(trigc != NULL) { free(trigc); }
	if(countc != NULL) { free(countc); }
	if(curcc != NULL) { free(curcc); }
	if(inds != NULL) { free(inds); }
	if(ranges != NULL) { free(ranges); }
	if(ddesc != NULL) { free(ddesc); }
	
	// And discard the elements
	if(dev_e != 0) { CVIXMLDiscardElement(dev_e); }
	if(chans_e != 0) { CVIXMLDiscardElement(chans_e); }
	if(pbdev_e != 0) { CVIXMLDiscardElement(pbdev_e); }
	if(chann_e != 0) { CVIXMLDiscardElement(chann_e); }
	if(chanr_e != 0) { CVIXMLDiscardElement(chanr_e); }
	if(trig_e != 0) { CVIXMLDiscardElement(trig_e); }
	if(bttls_e != 0) { CVIXMLDiscardElement(bttls_e); }
	if(curcc_e != 0) { CVIXMLDiscardElement(curcc_e); }
	if(countc_e != 0) { CVIXMLDiscardElement(countc_e); }
	if(r_e != 0) { CVIXMLDiscardElement(r_e); }
	if(pref_e != 0) { CVIXMLDiscardElement(pref_e); }
	if(gen_e != 0) { CVIXMLDiscardElement(gen_e); }
	if(ppc_e != 0) { CVIXMLDiscardElement(ppc_e); }
	if(dd_e != 0) { CVIXMLDiscardElement(dd_e); }
	if(npsrat_e != 0) { CVIXMLDiscardElement(npsrat_e); }
	if(tview_e != 0) { CVIXMLDiscardElement(tview_e); }
	if(at_e != 0) { CVIXMLDiscardElement(at_e); }
	if(bf_e != 0) { CVIXMLDiscardElement(bf_e); }
	if(fpath_e != 0) { CVIXMLDiscardElement(fpath_e); }
	if(dlpath_e != 0) { CVIXMLDiscardElement(dlpath_e); }
	if(ppath_e != 0) { CVIXMLDiscardElement(ppath_e); }
	if(ddesc_e != 0) { CVIXMLDiscardElement(ddesc_e); }
	if(lfinfo_e != 0) { CVIXMLDiscardElement(lfinfo_e); }
	if(fid_e != 0) { CVIXMLDiscardElement(fid_e); }
	if(spec_e != 0) { CVIXMLDiscardElement(spec_e); }
	if(col_e != 0) { CVIXMLDiscardElement(col_e); }
	if(gain_e != 0) { CVIXMLDiscardElement(gain_e); }
	if(off_e != 0) { CVIXMLDiscardElement(off_e); }
	if(con_e != 0) { CVIXMLDiscardElement(con_e); }

	
	// And the attributes
	if(ind_a != 0) { CVIXMLDiscardAttribute(ind_a); }
	if(num_a != 0) { CVIXMLDiscardAttribute(num_a); }
	if(trttl_a != 0) { CVIXMLDiscardAttribute(trttl_a); }
	if(edge_a != 0) { CVIXMLDiscardAttribute(edge_a); }
	if(as_a != 0) { CVIXMLDiscardAttribute(as_a); }
	
	// Discard the document
	if(xml_doc != -1) { CVIXMLDiscardDocument(xml_doc); }
	
	if(safe) { 
		CmtReleaseLock(lock_uidc);
		CmtReleaseLock(lock_uipc);
	}
	return rv;
}

void display_xml_error(int err) {
	char *message = malloc(500);
	CVIXMLGetErrorString(err, message, 500);
	
	MessagePopup("XML Error", message);
	free(message);
}
