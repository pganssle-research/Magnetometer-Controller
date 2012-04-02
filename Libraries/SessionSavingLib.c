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
#include <SaveSessionLib.h>
#include <SessionSavingLibPrivate.h>

#include <UIControls.h>
#include <Magnetometer Controller.h>
#include <PulseProgramLib.h>
#include <DataLib.h>
#include <MCUserDefinedFunctions.h>

#include <General.h>
#include <ErrorDefs.h>
#include <ErrorLib.h>

#include <spinapi.h>

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
	
	if(rv) {  
		if(is_mc_error(rv)) {
			display_error(rv);
		} else {
			display_xml_error(rv); 
		}
	}
	
	// Now load what UI stuff that needs to be loaded
	setup_broken_ttls_safe();
	
	CmtGetLock(lock_uidc);
	for(i = 0; i < 8; i++) {
		change_fid_chan_col(i);
		change_spec_chan_col(i);
	}
	CmtReleaseLock(lock_uidc);
	
	// Initialize the data navigation box
	int len, len1, len2;
	GetCtrlValStringLength(mc.datafbox[1], mc.datafbox[0], &len1);
	GetCtrlValStringLength(mc.path[1], mc.path[0], &len2);
	
	char *path = malloc(((len1 > len2)?len1:len2)+1);
	GetCtrlVal(mc.path[1], mc.path[0], path);
	select_directory(path);
	
	// Initialize the experiment choice
	GetCtrlValStringLength(mc.basefname[1], mc.basefname[0], &len);
	char *fname = malloc(len+5);	// Need to add 0000.tdm to this eventually.
	GetCtrlVal(mc.basefname[1], mc.basefname[0], fname);
	
	int ldm;
	GetCtrlVal(mc.ldatamode[1], mc.ldatamode[0], &ldm);
	int fns = get_current_fname(NULL, fname, !ldm, NULL);
	
	if(fns > 0) {
		fname = realloc(fname, fns);
	}
	
	if(get_current_fname(path, fname, !ldm, NULL) == 0) {
		int rv = 0;
		char *npath = get_full_path(path, fname, NULL, &rv);
		if(npath != NULL) {
			free(path);
			path = npath;
		}
	}
	
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
	uipc.pfpath = NULL;
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
	
	uidc.polyon = 0;
	uidc.polyord = 0;
	
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
	pc.uses_pb[0] = PulseProg_UsePulseBlaster;
	pc.numcycles[0] = PulseProg_PhaseCycles;
	pc.trans[0] = PulseProg_TransientNum;

	// And the panel bits
	pc.trig_ttl[1] = pc.PProgPan;
	pc.ninst[1] = pc.PProgPan;
	pc.rc[1] = pc.PProgPan;
	pc.uses_pb[1] = pc.PProgPan;
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
	ce.bfname = NULL;
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
	char *fname = NULL;

	// XML Variables
	CVIXMLDocument xml_doc = -1;
	
	CVIXMLElement r_e = 0;			// Root
	CVIXMLElement chan_e  = 0;		// Reused
	
	// General
	CVIXMLElement gen_e = 0, df_e = 0, pf_e = 0;
	
	// Preferences
	CVIXMLElement pref_e = 0, npsrat_e = 0;
	
	// Pulse Program Configuration
	CVIXMLElement ppc_e, pbdev_e, dev_e, tc_e, cc_e;
	ppc_e = pbdev_e = dev_e = tc_e = cc_e = 0;
	
	// Data Display
	CVIXMLElement dd_e, fid_e, spec_e, phase_e;
	dd_e = fid_e = spec_e = phase_e = 0;
	
	// Buffers
	char *buff = NULL;
	int blen = 0, len;
	int ibuff;
	
	char **chan_names = NULL;
	int *on_indices = NULL;
	
	char *trues = "1", *falses = "0";
	
	int i, j, rv = 0;
	
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

	// Create a 200-char buffer to play with
	buff = malloc(200);
	blen = 200;

	// Create the document and root element
	if(rv = CVIXMLNewDocument(MCXML_ROOT, &xml_doc)) { goto error; } 

	//////////////////////////////////////////////////
	//												//
	//					 Root						//
	//												//
	//////////////////////////////////////////////////
	// Get the root element
	if(rv = CVIXMLGetRootElement(xml_doc, &r_e)) { goto error; }
	
	// Save version info as attribute
	CVIXMLAddAttribute(r_e, MCXML_VER, MC_VERSION_STRING);
	CVIXMLAddAttribute(r_e, MCXML_VNUM, MC_VNUMSTR);

	// Now create the highest-level child elements.
	if(rv = CVIXMLNewElement(r_e, -1, MCXML_GEN, &gen_e)) { goto error; }		// General controls
	if(rv = CVIXMLNewElement(r_e, -1, MCXML_PREFS, &pref_e)) { goto error; }	// User preferences
	if(rv = CVIXMLNewElement(r_e, -1, MCXML_PPC, &ppc_e)) { goto error; }		// Pulse program configuration
	if(rv = CVIXMLNewElement(r_e, -1, MCXML_DDISP, &dd_e)) { goto error; }		// Data display info
	
	CVIXMLDiscardElement(r_e);
	r_e = 0;
	
	//////////////////////////////////////////////////
	//												//
	//					 General					//
	//												//
	//////////////////////////////////////////////////

	////// Base Attributes:
	//Active Tab 
	int tab;
	GetActiveTabPage(mc.mtabs[1], mc.mtabs[0], &tab);
	
	sprintf(buff, "%d", tab);
	if(rv = CVIXMLAddAttribute(gen_e, MCXML_ACTIVETAB, buff)) { goto error; }
	
	////// Elements
	if(rv = CVIXMLNewElement(gen_e, -1, MCXML_DATAFILE, &df_e)) { goto error; }
	if(rv = CVIXMLNewElement(gen_e, -1, MCXML_PROGFILE, &pf_e)) { goto error; }
	
	///////////////////////////////
	//	 		Data File 	 	 //
	//////////////////////////////
	// Base file name
	GetCtrlValStringLength(mc.basefname[1], mc.basefname[0], &len);
	buff = realloc_if_needed(buff, &blen, ++len, 1);
	
	GetCtrlVal(mc.basefname[1], mc.basefname[0], buff);
	if(rv = CVIXMLAddAttribute(df_e, MCXML_BFNAME, buff)) { goto error; }
	
	// File path
	GetCtrlValStringLength(mc.path[1], mc.path[0], &len);   
	buff = realloc_if_needed(buff, &blen, ++len, 1);
	
	GetCtrlVal(mc.path[1], mc.path[0], buff);
	if(rv = CVIXMLAddAttribute(df_e, MCXML_FPATH, buff)) { goto error; }
	
	// Load path
	if(rv = CVIXMLAddAttribute(df_e, MCXML_LPATH, (uidc.dlpath != NULL)?uidc.dlpath:"")) { goto error; }
	
	// Load File Info
	GetCtrlVal(mc.ldatamode[1], mc.ldatamode[0], &ibuff);
	if(rv = CVIXMLAddAttribute(df_e, MCXML_LFINFO, (ibuff)?trues:falses)) { goto error; }
	
	// Description is the value of the element.
	GetCtrlValStringLength(mc.datadesc[1], mc.datadesc[0], &len);
	if(len < 1) {
		strcpy(buff, "");		
	} else {
		buff = realloc_if_needed(buff, &blen, ++len, 1);
		GetCtrlVal(mc.datadesc[1], mc.datadesc[0], buff);
	}
	
	if(rv = CVIXMLSetElementValue(df_e, buff)) { goto error; }
	
	///////////////////////////////
	//	 	  Program File 	 	 //
	///////////////////////////////
	// This element has no value, just attributes
	
	// Program Path
	if(rv = CVIXMLAddAttribute(pf_e, MCXML_LPATH, (uipc.ppath != NULL)?uipc.ppath:"")) { goto error; }
	
	// Program File Path
	if(rv = CVIXMLAddAttribute(pf_e, MCXML_FPATH, (uipc.pfpath != NULL)?uipc.pfpath:"")) { goto error; }
	
	
	// Now discard the elements, we're done.
	CVIXMLDiscardElement(pf_e);
	CVIXMLDiscardElement(df_e);
	CVIXMLDiscardElement(gen_e);
	gen_e = df_e = pf_e = 0;
	
	//////////////////////////////////////////////////
	//												//
	//				   Preferences					//
	//												//
	//////////////////////////////////////////////////
	
	// Create elements
	if(rv = CVIXMLNewElement(pref_e, -1, MCXML_NPSRAT, &npsrat_e)) { goto error; }
	
	////// Base Attributes
	// Transient View Preference
	sprintf(buff, "%d", uidc.disp_update);
	if(rv = CVIXMLAddAttribute(pref_e, MCXML_TVIEW, buff)) { goto error; }
	
	////// Elements
	// NP, SR and AT prefs.
	void *data;
	int nppref, srpref, atpref;
	
	GetCtrlAttribute(pc.np[1], pc.np[0], ATTR_CALLBACK_DATA, &data);
	nppref = (int)data;
	
	GetCtrlAttribute(pc.sr[1], pc.sr[0], ATTR_CALLBACK_DATA, &data);
	srpref = (int)data;
	
	GetCtrlAttribute(pc.at[1], pc.at[0], ATTR_CALLBACK_DATA, &data);
	atpref = (int)data;
	
	// NP First
	sprintf(buff, "%d", nppref);
	if(rv = CVIXMLAddAttribute(npsrat_e, MCXML_NPPREF, buff)) { goto error; }
	
	// SR Next
	sprintf(buff, "%d", srpref);
	if(rv = CVIXMLAddAttribute(npsrat_e, MCXML_SRPREF, buff)) { goto error; }
	
	// AT Last
	sprintf(buff, "%d", atpref);
	if(rv = CVIXMLAddAttribute(npsrat_e, MCXML_ATPREF, buff)) { goto error; }
	
	CVIXMLDiscardElement(npsrat_e);
	CVIXMLDiscardElement(pref_e);
	npsrat_e = pref_e =  0;

	//////////////////////////////////////////////////
	//												//
	//			   Pulse Program Config				//
	//												//
	//////////////////////////////////////////////////
	
	////// Elements
	if(rv = CVIXMLNewElement(ppc_e, -1, MCXML_PBDEV, &pbdev_e)) { goto error; }
	if(rv = CVIXMLNewElement(ppc_e, -1, MCXML_DEV, &dev_e)) { goto error; }
	
	///////////////////////////////
	//	 	PulseBlaster 	 	 //
	///////////////////////////////
	// Has no value or elements - only attributes
	
	// Index
	int nl, devind = -1;
	GetNumListItems(pc.pbdev[1], pc.pbdev[0], &nl);
	
	if(nl > 0) {
		GetCtrlIndex(pc.pbdev[1], pc.pbdev[0], &devind);		
	}
	
	sprintf(buff, "%d", devind);
	if(rv = CVIXMLAddAttribute(pbdev_e, MCXML_INDEX, buff)) { goto error; }
	
	// Trigger TTL
	sprintf(buff, "%d", uipc.trigger_ttl);
	if(rv = CVIXMLAddAttribute(pbdev_e, MCXML_TRIGTTL, buff)) { goto error; }
	
	// Broken TTLS
	sprintf(buff, "%d", uipc.broken_ttls);
	if(rv = CVIXMLAddAttribute(pbdev_e, MCXML_BROKENTTLS, buff)) { goto error; }

	///////////////////////////////
	//	 		 DAQ 		 	 //
	///////////////////////////////
	
	//// Attributes
	
	// Name and Index
	devind = -1;
	GetNumListItems(pc.dev[1], pc.dev[0], &nl);

	if(nl > 0) {
		GetCtrlIndex(pc.dev[1], pc.dev[0], &devind);
		GetLabelLengthFromIndex(pc.dev[1], pc.dev[0], devind, &len);
		
		buff = realloc_if_needed(buff, &blen, ++len, 1);
		
		GetLabelFromIndex(pc.dev[1], pc.dev[0], devind, buff);
	} else {
		sprintf(buff, "");
	}
	
	if(rv = CVIXMLAddAttribute(dev_e, MCXML_NAME, buff)) { goto error; }
	
	sprintf(buff, "%d", devind);
	if(rv = CVIXMLAddAttribute(dev_e, MCXML_INDEX, buff)) { goto error; }
	
	// Number of chans on
	sprintf(buff, "%d", uidc.onchans);
	if(rv = CVIXMLAddAttribute(dev_e, MCXML_NCHANSON, buff)) { goto error; }
	
	// Current channel index
	int cind;
	if(uidc.onchans < 1) {
		cind = -1;	
	} else {
		// We'll need this later
		chan_names = calloc(uidc.onchans, sizeof(char *));
		on_indices = malloc(uidc.onchans * sizeof(int));

		j = 0;

		for(i = 0; i < uidc.nchans; i++) {
			if(uidc.chans[i]) {
				GetLabelLengthFromIndex(pc.ic[1], pc.ic[0], i, &len);
				
				on_indices[j] = i;
				chan_names[j] = malloc(len+1);
		
				GetLabelFromIndex(pc.ic[1], pc.ic[0], i, chan_names[j]);
				
				strcpy(chan_names[j], chan_names[j]+1); // Truncate trailing character.
				
				if(++j >= uidc.onchans) { break; }  
			}
		}
		
		for(j; j < uidc.onchans; j++) {
			chan_names[j] = malloc(2);
			strcpy(chan_names[j], "");
			
			on_indices[j] = -1;
		}
		
		GetCtrlIndex(pc.ic[1], pc.ic[0], &cind);
	}
	
	sprintf(buff, "%d", cind);
	if(rv = CVIXMLAddAttribute(dev_e, MCXML_CURCHAN, buff)) { goto error; }
	
	// Sub-Elements
	if(rv = CVIXMLNewElement(dev_e, -1, MCXML_TRIGCHAN, &tc_e)) { goto error; }
	if(rv = CVIXMLNewElement(dev_e, -1, MCXML_COUNTCHAN, &cc_e)) { goto error; }
	
	//// Trigger Channel
	// Attributes
	
	// Index
	int tind = -1;
	strcpy(buff, "");
	GetNumListItems(pc.trigc[1], pc.trige[0], &nl);
	
	if(nl > 0) {
		GetCtrlIndex(pc.trigc[1], pc.trigc[0], &tind);
		
		GetLabelLengthFromIndex(pc.trigc[1], pc.trigc[0], tind, &len);
		
		if(len > 0) {
			buff = realloc_if_needed(buff, &blen, ++len, 1);
			GetLabelFromIndex(pc.trigc[1], pc.trigc[0], tind, buff);
		} 
	}
	
	if(rv = CVIXMLSetElementValue(tc_e, buff)) { goto error; }	// Name of the channel
		
	sprintf(buff, "%d", tind);
	if(rv = CVIXMLAddAttribute(tc_e, MCXML_INDEX, buff)) { goto error; }
	
	// Edge
	int edge;
	GetCtrlVal(pc.trige[1], pc.trige[0], &edge);
	sprintf(buff, "%d", edge);
	
	if(rv = CVIXMLAddAttribute(tc_e, MCXML_TRIGEDGE, buff)) { goto error; }
	
	// Counter Channel
	cind = -1;
	strcpy(buff, "");
	
	GetNumListItems(pc.cc[1], pc.cc[0], &nl);
	if(nl > 0) {
		GetCtrlIndex(pc.cc[1], pc.cc[0], &cind);
		
		GetLabelLengthFromIndex(pc.cc[1], pc.cc[0], cind, &len);
		
		if(len > 0) {
			buff = realloc_if_needed(buff, &blen, ++len, 1);
			GetLabelFromIndex(pc.cc[1], pc.cc[0], cind, buff);
		}
	}
	
	if(rv = CVIXMLSetElementValue(cc_e, buff)) { goto error; } // Name
	
	sprintf(buff, "%d", cind);
	if(rv = CVIXMLAddAttribute(cc_e, MCXML_INDEX, buff)) { goto error; }
	
	// Now go through and create elements for each channel that's on.
	for(i = 0; i < uidc.onchans; i++) {
		if(rv = CVIXMLNewElement(dev_e, -1, MCXML_CHANNEL, &chan_e)) { goto error; }
		
		// Index
		sprintf(buff, "%d", on_indices[i]);
		if(rv = CVIXMLAddAttribute(chan_e, MCXML_INDEX, buff)) { goto error; }
		
		// Range
		sprintf(buff, "%.1f", uidc.range[i]);
		if(rv = CVIXMLAddAttribute(chan_e, MCXML_RANGE, buff)) { goto error; }
		
		// Name (value)
		if(rv = CVIXMLSetElementValue(chan_e, chan_names[i])) { goto error; }
		
		CVIXMLDiscardElement(chan_e);
		chan_e = 0;
	}
	
	CVIXMLDiscardElement(tc_e);
	CVIXMLDiscardElement(cc_e);
	CVIXMLDiscardElement(dev_e);
	CVIXMLDiscardElement(pbdev_e);
	CVIXMLDiscardElement(ppc_e);
	ppc_e = pbdev_e = dev_e = cc_e = tc_e = 0;
	
	//////////////////////////////////////////////////
	//												//
	//			      Data Display					//
	//												//
	//////////////////////////////////////////////////
	/////// Base Attributes
	// Polynomial fitting on
	if(rv = CVIXMLAddAttribute(dd_e, MCXML_POLYON, uidc.polyon?trues:falses)) { goto error; }
	
	// Polynomial fit order
	sprintf(buff, "%d", uidc.polyord);
	if(rv = CVIXMLAddAttribute(dd_e, MCXML_POLYORD, buff)) { goto error; }
	
	////// Elements
	int as;
	if(rv = CVIXMLNewElement(dd_e, -1, MCXML_FID, &fid_e)) { goto error; }
	if(rv = CVIXMLNewElement(dd_e, -1, MCXML_SPEC, &spec_e)) { goto error; }
	
	//// Add the FID attributes
	// Number of Channels On
	sprintf(buff, "%d", uidc.fnc);
	if(rv = CVIXMLAddAttribute(fid_e, MCXML_NCHANSON, buff)) { goto error; }
	
	// Autoscale
	GetCtrlVal(dc.fid, dc.fauto, &as);
	if(rv = CVIXMLAddAttribute(fid_e, MCXML_AUTOSCALE, as?trues:falses)) {	goto error; }	// Autoscale on/off
	
	// Current channel
	int curchan = -1;
	if(uidc.fnc == 1) {
		for(i = 0; i < 8; i++) {
			if(uidc.fchans[i]) { 
				curchan = i;
				break; 
			}	
		}
	} else if (uidc.fnc > 1) {
		GetCtrlVal(dc.fid, dc.fcring, &curchan);	
	}
	
	sprintf(buff, "%d", curchan);
	if(rv = CVIXMLAddAttribute(fid_e, MCXML_CURCHAN, buff)) { goto error; }
	
	//// Add the Spectrum attributes
	// Number of Channels On
	sprintf(buff, "%d", uidc.snc);
	if(rv = CVIXMLAddAttribute(spec_e, MCXML_NUM, buff)) { goto error; }
	
	//Autoscale
	GetCtrlVal(dc.spec, dc.sauto, &as);
	if(rv = CVIXMLAddAttribute(spec_e, MCXML_AUTOSCALE, as?trues:falses)) { goto error; }
	
	// Number of phase orders
	sprintf(buff, "%d", MCD_NPHASEORDERS);
	if(rv = CVIXMLAddAttribute(spec_e, MCXML_NPHASEORDERS, buff)) { goto error; }
	
	// Current channel
	curchan = -1;
	if(uidc.snc == 1) { 
		for(i = 0; i < 8; i++) {
			if(uidc.schans[i]) { 
				curchan = i;
				break; 
			}	
		}
	} else {
		GetCtrlVal(dc.spec, dc.scring, &curchan);	
	}
	
	sprintf(buff, "%d", curchan);
	if(rv = CVIXMLAddAttribute(spec_e, MCXML_CURCHAN, buff)) { goto error; }
	
	// Now create each of the channel elements
	for(i = 0; i < 8; i++) {
		//// FID
		if(rv = CVIXMLNewElement(fid_e, -1, MCXML_CHANNEL, &chan_e)) { goto error; }
		
		// Number
		sprintf(buff, "%d", i);
		if(rv = CVIXMLAddAttribute(chan_e, MCXML_INDEX, buff)) { goto error; }
		
		// On
		if(rv = CVIXMLAddAttribute(chan_e, MCXML_ON, uidc.fchans[i]?trues:falses)) { goto error; }
		
		// Color
		sprintf(buff, "%06x", uidc.fcol[i]);
		if(rv = CVIXMLAddAttribute(chan_e, MCXML_COLOR, buff)) { goto error; }
		
		// Gain
		sprintf(buff, "%f", uidc.fgain[i]);
		if(rv = CVIXMLAddAttribute(chan_e, MCXML_GAIN, buff)) { goto error; }
		
		// Offset
		sprintf(buff, "%f", uidc.foff[i]);
		if(rv = CVIXMLAddAttribute(chan_e, MCXML_OFFSET, buff)) { goto error; }
		
		if(i < uidc.onchans && chan_names[i] != NULL) {
			// Put the name as the value.
			if(rv = CVIXMLSetElementValue(chan_e, chan_names[i])) { goto error; }
		}

		CVIXMLDiscardElement(chan_e);
		chan_e = 0;
		
		//// Spectrum
		if(rv = CVIXMLNewElement(spec_e, -1, MCXML_CHANNEL, &chan_e)) { goto error; }
		if(rv = CVIXMLAddAttribute(chan_e, MCXML_ON, uidc.fchans[i]?trues:falses)) { goto error; }
		
		// Number
		sprintf(buff, "%d", i);
		if(rv = CVIXMLAddAttribute(chan_e, MCXML_INDEX, buff)) { goto error; }
		
		// On
		if(rv = CVIXMLAddAttribute(chan_e, MCXML_ON, uidc.schans[i]?trues:falses)) { goto error; }
		
		// Color
		sprintf(buff, "%06x", uidc.scol[i]);
		if(rv = CVIXMLAddAttribute(chan_e, MCXML_COLOR, buff)) { goto error; }
		
		// Gain
		sprintf(buff, "%f", uidc.sgain[i]);
		if(rv = CVIXMLAddAttribute(chan_e, MCXML_GAIN, buff)) { goto error; }
		
		// Offset
		sprintf(buff, "%f", uidc.soff[i]);
		if(rv = CVIXMLAddAttribute(chan_e, MCXML_OFFSET, buff)) { goto error; }
		
		// Phase correction
		for(j = 0; j < MCD_NPHASEORDERS; j++) {
			if(rv = CVIXMLNewElement(chan_e, -1, MCXML_PHASE, &phase_e)) { goto error; }
			
			// Order
			sprintf(buff, "%d", j);
			if(rv = CVIXMLAddAttribute(phase_e, MCXML_PHASEORDER, buff)) { goto error; }
			
			// Value
			sprintf(buff, "%f", uidc.sphase[i][j]);
			if(rv = CVIXMLSetElementValue(phase_e, buff)) { goto error; }
			
			
			CVIXMLDiscardElement(phase_e);
			phase_e = 0;
		}
	}
	
	CVIXMLDiscardElement(fid_e);
	CVIXMLDiscardElement(spec_e);
	CVIXMLDiscardElement(dd_e);
	fid_e = spec_e = dd_e = 0;
	
	// Save the document			   
	rv = CVIXMLSaveDocument(xml_doc, 1, fname);
	
	error:
	if(xml_doc != -1)
		CVIXMLDiscardDocument(xml_doc);
	
	// Free all the memory that was allocated and not freed.
	if(fname != NULL) { free(fname); }
	if(buff != NULL) { free(buff); }
	
	if(chan_names != NULL) { free_string_array(chan_names, uidc.onchans); }
	if(on_indices != NULL) { free(on_indices); }
	
	// Now discarding the elements.
	if(r_e != 0) { CVIXMLDiscardElement(r_e); }
	
	if(gen_e != 0) { CVIXMLDiscardElement(gen_e); }
	if(pref_e != 0) { CVIXMLDiscardElement(pref_e); }
	if(ppc_e != 0) { CVIXMLDiscardElement(ppc_e); }
	if(dd_e != 0) { CVIXMLDiscardElement(dd_e); }
	
	if(chan_e != 0) { CVIXMLDiscardElement(chan_e); }
	if(df_e != 0) { CVIXMLDiscardElement(df_e); }
	if(pf_e != 0) { CVIXMLDiscardElement(pf_e); }
	if(npsrat_e != 0) { CVIXMLDiscardElement(npsrat_e); }
	if(pbdev_e != 0) { CVIXMLDiscardElement(pbdev_e); }
	if(dev_e != 0) { CVIXMLDiscardElement(dev_e); }
	if(tc_e != 0) { CVIXMLDiscardElement(tc_e); }
	if(cc_e != 0) { CVIXMLDiscardElement(cc_e); }
	if(fid_e != 0) { CVIXMLDiscardElement(fid_e); }
	if(spec_e != 0) { CVIXMLDiscardElement(spec_e); }
	if(phase_e != 0) { CVIXMLDiscardElement(phase_e); }
	
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

	int i, j, rv = 0;
	char *fbuff = (filename == NULL)?session_fname:filename;
	char *fname = NULL, *dname = NULL, *buff2 = NULL;
	
	// XML Variables
	CVIXMLDocument xml_doc = -1;
	
	CVIXMLElement r_e = 0;			// Root
	CVIXMLElement chan_e  = 0;		// Reused
	ListType chan_list = 0, phase_list = 0;
	
	// General
	CVIXMLElement gen_e = 0, df_e = 0, pf_e = 0;
	
	// Preferences
	CVIXMLElement pref_e = 0, npsrat_e = 0;
	
	// Pulse Program Configuration
	CVIXMLElement ppc_e, pbdev_e, dev_e, tc_e, cc_e;
	ppc_e = pbdev_e = dev_e = tc_e = cc_e = 0;
	
	// Data Display
	CVIXMLElement dd_e, fid_e, spec_e, phase_e;
	dd_e = fid_e = spec_e = phase_e = 0;
	
	// Attributes
	CVIXMLAttribute buff_a = 0, name_a = 0, ind_a = 0;
	
	// Buffers
	char *buff = NULL;
	int blen = 0, len, nl, ind;
	int ibuff;
	
	// Get the file
	int flen = strlen(fbuff);
	if(strlen("xml") > strlen(PPROG_EXTENSION)) {
		flen += strlen("xml")+2;
	} else {
		flen += strlen(PPROG_EXTENSION) + 2;
	}
	
	fname = malloc(flen);
	
	blen = 200;
	buff = malloc(200);
	
	if(safe) { 
		CmtGetLock(lock_uidc);
		CmtGetLock(lock_uipc);
	}
	
	// Unlike with the save_session function, we'll end with the pulse program, since we need
	// to make sure broken ttls and trigger ttls and such are set up.
	sprintf(fname, "%s.xml", fbuff);
	if(rv = CVIXMLLoadDocument(fname, &xml_doc)) { goto error; }
	
	// Get the root element and the main elements.
	if(rv = CVIXMLGetRootElement(xml_doc, &r_e) < 0) { goto error; }
	if(rv = CVIXMLGetChildElementByTag(r_e, MCXML_GEN, &gen_e) < 0) { goto error; }
	if(rv = CVIXMLGetChildElementByTag(r_e, MCXML_PREFS, &pref_e) < 0) { goto error; }  
	if(rv = CVIXMLGetChildElementByTag(r_e, MCXML_PPC, &ppc_e) < 0) { goto error; }
	if(rv = CVIXMLGetChildElementByTag(r_e, MCXML_DDISP, &dd_e) < 0) { goto error; }

	
	//////////////////////////////////////////////////
	//												//
	//					 General					//
	//												//
	//////////////////////////////////////////////////
	
	if(gen_e != 0) {
		////// Attributes
		if(rv = CVIXMLGetAttributeByName(gen_e, MCXML_ACTIVETAB, &buff_a) < 0) { goto error; }
	
		if(buff_a != 0) {
			// Active Tab
			if(rv = CVIXMLGetAttributeValue(buff_a, buff)) { goto error; }
			if(sscanf(buff, "%d", &ibuff) == 1 && ibuff != -1) {
				SetActiveTabPage(mc.mtabs[1], mc.mtabs[0], ibuff);	
			}
	
			CVIXMLDiscardAttribute(buff_a);
			buff_a = 0;
		}

		////// Child elements
		if(rv = CVIXMLGetChildElementByTag(gen_e, MCXML_DATAFILE, &df_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(gen_e, MCXML_PROGFILE, &pf_e)) { goto error; }
		
		CVIXMLDiscardElement(gen_e);
		gen_e = 0;
	}
	
	
	//// Data File
	if(df_e != 0) {
		//// Attributes
		// Base filename
		if(rv = CVIXMLGetAttributeByName(df_e, MCXML_BFNAME, &buff_a)) { goto error; }
		if(buff_a != 0) {
			if(rv = CVIXMLGetAttributeValueLength(buff_a, &len)) { goto error; }
			
			if(len > 0) {
				buff = realloc_if_needed(buff, &blen, ++len, 1);
				
				if(rv = CVIXMLGetAttributeValue(buff_a, buff)) { goto error; }
				
				SetCtrlVal(mc.basefname[1], mc.basefname[0], buff);
			}
			CVIXMLDiscardAttribute(buff_a);
			buff_a = 0;
		}
		
		// File path
		buff = get_attribute_val(df_e, MCXML_FPATH, buff, &blen, &rv);
		if(rv < 0) { goto error; }
		
		if(strlen(buff) > 0) {
			SetCtrlVal(mc.path[1], mc.path[0], buff);
		}
		
		// Load path
		buff = get_attribute_val(df_e, MCXML_LPATH, buff, &blen, &rv);
		if(rv < 0) { goto error; }
		
		if(strlen(buff) > 0) { 
			if(uidc.dlpath != NULL) { free(uidc.dlpath); }
			uidc.dlpath = malloc(strlen(buff)+1);
			
			strcpy(uidc.dlpath, buff);
		}
		
		// Load Data Info Bool
		int ldm;
		if(rv = get_attribute_int_val(df_e, MCXML_LFINFO, &ldm, 0)) { goto error; }
		
		SetCtrlVal(mc.ldatamode[1], mc.ldatamode[0], ldm);
		
		// Now the description - which is the value.
		buff = get_element_val(df_e, buff, &blen, &rv);
		if(rv < 0) { goto error; }
		
		SetCtrlVal(mc.datadesc[1], mc.datadesc[0], buff);
		
		CVIXMLDiscardElement(df_e);
		df_e = 0;
	}
	
	///// Program File
	if(pf_e != 0) { 
		// Load path
		if(rv = CVIXMLGetAttributeByName(df_e, MCXML_LPATH, &buff_a) < 0) { goto error; }
		if(buff_a != 0) {
			if(rv = CVIXMLGetAttributeValueLength(buff_a, &len)) { goto error; }
			
			if(len > 0) {
				if(uipc.ppath != NULL) { free(uipc.ppath); }
				uipc.ppath = malloc(++len);
				
				if(rv = CVIXMLGetAttributeValue(buff_a, uipc.ppath)) { goto error; }
			}
			
			CVIXMLDiscardAttribute(buff_a);
			buff_a = 0;
		}
		
		
		// Program File Path
		if(rv = CVIXMLGetAttributeByName(df_e, MCXML_FPATH, &buff_a) < 0) { goto error; }
		if(buff_a != 0) {
			if(rv = CVIXMLGetAttributeValueLength(buff_a, &len)) { goto error; }
			
			if(len > 0) {
				if(uipc.pfpath != NULL) { free(uipc.pfpath); }
				uipc.pfpath = malloc(++len);
				
				if(rv = CVIXMLGetAttributeValue(buff_a, uipc.pfpath)) { goto error; }
			}
			
			CVIXMLDiscardAttribute(buff_a);
			buff_a = 0;
		}
	}
	
	//////////////////////////////////////////////////
	//												//
	//				   Preferences					//
	//												//
	//////////////////////////////////////////////////

	if(pref_e != 0) {
		////// Attributes
		if(rv = CVIXMLGetAttributeByName(pref_e, MCXML_TVIEW, &buff_a)) { goto error; }
		if(buff_a != 0) {
			if(rv = CVIXMLGetAttributeValue(buff_a, buff)) { goto error; }
			
			sscanf(buff, "%d", &uidc.disp_update);
			
			CVIXMLDiscardAttribute(buff_a);
			buff_a = 0;
		}
		
		if(rv = CVIXMLGetChildElementByTag(pref_e, MCXML_NPSRAT, &npsrat_e)) { goto error; } 	// NP, SR, AT Prefs

		CVIXMLDiscardElement(pref_e);
		pref_e = 0;
	}
	
	if(npsrat_e != 0) {	// Only do this stuff if we found the element.
		int np = -1, sr = -1, at = -1;
		
		// NP
		if(rv = CVIXMLGetAttributeByName(npsrat_e, MCXML_NPPREF, &buff_a)) { goto error; }
		if(buff_a != 0) {
			if(rv = CVIXMLGetAttributeValue(buff_a, buff)) { goto error; }
			
			sscanf(buff, "%d", &np);
			
			CVIXMLDiscardAttribute(buff_a);
			buff_a = 0;	
		}
		
		// SR
		if(rv = CVIXMLGetAttributeByName(npsrat_e, MCXML_SRPREF, &buff_a)) { goto error; }
		if(buff_a != 0) {
			if(rv = CVIXMLGetAttributeValue(buff_a, buff)) { goto error; }
			
			sscanf(buff, "%d", &sr);
			
			CVIXMLDiscardAttribute(buff_a);
			buff_a = 0;	
		}
		
		// AT
		if(rv = CVIXMLGetAttributeByName(npsrat_e, MCXML_ATPREF, &buff_a)) { goto error; }
		if(buff_a != 0) {
			if(rv = CVIXMLGetAttributeValue(buff_a, buff)) { goto error; }
			
			sscanf(buff, "%d", &at);
			
			CVIXMLDiscardAttribute(buff_a);
			buff_a = 0;	
		}
	  	
		CtrlCallbackPtr cb;		// Need to get a pointer to the callback function
		GetCtrlAttribute(pc.np[1], pc.np[0], ATTR_CALLBACK_FUNCTION_POINTER, &cb);
		
		if(np != -1) { InstallCtrlCallback(pc.np[1], pc.np[0], cb, (void *)np); }
		if(sr != -1) { InstallCtrlCallback(pc.sr[1], pc.sr[0], cb, (void *)sr); }
		if(at != -1) { InstallCtrlCallback(pc.at[1], pc.at[0], cb, (void *)at); }
		
		CVIXMLDiscardElement(npsrat_e);
		npsrat_e = 0;
	}
	
	//////////////////////////////////////////////////
	//												//
	//			   Pulse Program Config				//
	//												//
	//////////////////////////////////////////////////
	
	if(ppc_e != 0) {
		// Child Elements
		if(rv = CVIXMLGetChildElementByTag(ppc_e, MCXML_PBDEV, &pbdev_e)) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(ppc_e, MCXML_DEV, &dev_e)) { goto error; }
		
		CVIXMLDiscardElement(ppc_e);
		ppc_e = 0;
	}
	
	
	// PulseBlaster
	if(pbdev_e != 0) {
		// Device index
		if(rv = CVIXMLGetAttributeByName(pbdev_e, MCXML_INDEX, &buff_a)) { goto error; }
		if(buff_a != 0) {
			if(rv = CVIXMLGetAttributeValue(buff_a, buff)) { goto error; }
			
			ind = -1;
			sscanf(buff, "%d", &ind);
			
			if(ind >= 0) {
				GetNumListItems(pc.pbdev[1], pc.pbdev[0], &nl);
				
				if(nl > ind) {
					SetCtrlIndex(pc.pbdev[1], pc.pbdev[0], ind);	
				}
			}
			
			CVIXMLDiscardAttribute(buff_a);
			buff_a = 0;	
		}
		
		// Trigger TTL
		if(rv = CVIXMLGetAttributeByName(pbdev_e, MCXML_TRIGTTL, &buff_a)) { goto error; }
		if(buff_a != 0) {
			if(rv = CVIXMLGetAttributeValue(buff_a, buff)) { goto error; }
			
			int trig_ttl = -1;
			sscanf(buff, "%d", &trig_ttl);
			
			if(trig_ttl > 0 && trig_ttl < 24) {
				uipc.trigger_ttl = trig_ttl;	
				SetCtrlVal(pc.trig_ttl[1], pc.trig_ttl[0], trig_ttl);
			}
			
			
			CVIXMLDiscardAttribute(buff_a);
			buff_a = 0;	
		}
		
		// Broken TTLS
		if(rv = CVIXMLGetAttributeByName(pbdev_e, MCXML_BROKENTTLS, &buff_a)) { goto error; }
		if(buff_a != 0) {
			if(rv = CVIXMLGetAttributeValue(buff_a, buff)) { goto error; }
		
			sscanf(buff, "%d", &uipc.broken_ttls);
		
			CVIXMLDiscardAttribute(buff_a);
			buff_a = 0;	
		}
	
		CVIXMLDiscardElement(pbdev_e);
		pbdev_e = 0;
	}
	
	// DAQ
	if(dev_e != 0) {
		// Device name
		ind = -1;
		strcpy(buff, "");
		
		// Device name
		if(rv = CVIXMLGetAttributeByName(dev_e, MCXML_NAME, &buff_a)) { goto error; }
		if(buff_a != 0) { 
			if(rv = CVIXMLGetAttributeValueLength(buff_a, &len)) { goto error; }
			
			if(len > 0) {
				buff = realloc_if_needed(buff, &blen, ++len, 1);
				
				if(rv = CVIXMLGetAttributeValue(buff_a, buff)) { goto error; }
			}
			
			CVIXMLDiscardAttribute(buff_a);
			buff_a = 0;
		}
		
		// Device Index
		if(rv = CVIXMLGetAttributeByName(dev_e, MCXML_INDEX, &buff_a)) { goto error; }
		if(buff_a != 0) { 
			if(rv = CVIXMLGetAttributeValue(buff_a, buff)) { goto error; }
			
			sscanf(buff, "%d", &ind);
			
			CVIXMLDiscardAttribute(buff_a);
			buff_a = 0;
		}
		
		// Try and figure out what device we want from the information available.
		GetNumListItems(pc.dev[1], pc.dev[0], &nl);
		if(nl > 0) {
			int old_ind;
			GetCtrlIndex(pc.dev[1], pc.dev[0], &old_ind);
			
			if(strlen(buff) > 0) {
				int nind;
				GetIndexFromValue(pc.dev[1], pc.dev[0], &nind, buff);
				
				if(nind >= 0) { 
					ind = nind;
				}
			}
			
			// If we found something, reload DAQ info.
			if(ind >= 0 && ind < nl) {
				SetCtrlIndex(pc.dev[1], pc.dev[0], ind);
			
				if(ind != old_ind) {
					load_DAQ_info_safe(0, 0, 1);	
				}
			}
		}
		
		// Get the channels on.
		int chanson;
		if(rv = get_attribute_int_val(dev_e, MCXML_NCHANSON, &chanson, -1)) { goto error; }
		if(chanson >= 1) {
			// Find the first channel.
			if(rv = CVIXMLFindElements(dev_e, MCXML_CHANNEL, &chan_list) < 0) { goto error; }
			nl = ListNumItems(chan_list);
			float range;
			
			GetCtrlValStringLength(pc.dev[1], pc.dev[0], &len);
			if(len > 0) {
				dname = malloc(len+2);
			
				GetCtrlVal(pc.dev[1], pc.dev[0], dname);
				
				if(dname[strlen(dname)] != '/') {
					strcpy(dname+strlen(dname), "/");	
				}
			} else {
				dname = malloc(1);
				strcpy(dname, "");
			}
			
			int dl = strlen(dname);
			int blen2 = dl;
			int oind = -1, nind = -1;
			int nl2;
			int success = 0;
			
			GetNumListItems(pc.ic[1], pc.ic[0], &nl2);
			
			for(i = 0; i < nl; i++) {  // One-based index, apparently
				ListRemoveItem(chan_list, &chan_e, FRONT_OF_LIST);
				if(chan_e != 0) {
					// Index
					if(rv = get_attribute_int_val(chan_e, MCXML_INDEX, &ind, -1)) { goto error; }
					
					// Name
					buff = get_element_val(chan_e, buff, &blen, &rv);
					if(rv < 0) { goto error; }
					
					// Find the relevant channel based on this information
					buff2 = realloc_if_needed(buff2, &blen2, strlen(buff)+dl+1, 1);
					
					sprintf(buff2, "%s%s", dname, buff);
					
					GetIndexFromValue(pc.ic[1], pc.ic[0], &nind, buff);
					
					success = 1;
					if(nind >= 0 && !uidc.chans[ind]) {
						toggle_ic_ind(nind);
					} else if(ind >= 0 && ind < nl2 && !uidc.chans[ind]) {
						toggle_ic_ind(ind);	
					} else {
						nl--;
						i--;
						success = 0;
					}
					
					
					if(success) {
						// Range
						if(rv = get_attribute_float_val(chan_e, MCXML_RANGE, &range, uidc.range[i])) { goto error; }
					
						if(range > 0) {
							uidc.range[i] = range;
						}
					}
					
					CVIXMLDiscardElement(chan_e);
					chan_e = 0;
				}
			}
			
			free(buff2);
			buff2 = NULL;
			
			free(dname);
			dname = NULL;
			
			ListDispose(chan_list);
			chan_list = 0;
		}
		
		// Current channel.
		if(rv = get_attribute_int_val(dev_e, MCXML_CURCHAN, &ind, -1)) { goto error; }
		
		if(ind >= 0 && ind < uidc.onchans) {
			SetCtrlVal(pc.curchan[1], pc.curchan[0], ind);
		}
		
		// Trigger channel.
		if(rv = CVIXMLGetChildElementByTag(dev_e, MCXML_TRIGCHAN, &tc_e) < 0) { goto error; }
		if(tc_e != 0) { 
			// Get the edge first.
			int edge;
			if(rv = get_attribute_int_val(tc_e, MCXML_TRIGEDGE, &edge, -1)) { goto error; }
			
			if(edge != -1) { 
				SetCtrlVal(pc.trige[1], pc.trige[0], edge);
			}
			
			// Now try and find the trigger.
			buff = get_element_val(tc_e, buff, &blen, &rv);
			if(rv != 0) { goto error; }
			
			if(rv = get_attribute_int_val(tc_e, MCXML_INDEX, &ind, -1)) { goto error; }
			
			int nind;
			GetIndexFromValue(pc.trigc[1], pc.trigc[0], &nind, buff);
			GetNumListItems(pc.trigc[1], pc.trigc[0], &nl);
			
			if(nind >= 0) {
				SetCtrlIndex(pc.trigc[1], pc.trigc[0], nind);	
			} else if(ind >= 0 && ind < nl) {
				SetCtrlIndex(pc.trigc[1], pc.trigc[0], ind);	
			}
			
			
			CVIXMLDiscardElement(tc_e);
			tc_e = 0;
		}
		
		// Counter channel
		if(rv = CVIXMLGetChildElementByTag(dev_e, MCXML_COUNTCHAN, &cc_e) < 0) { goto error; }
		if(cc_e != 0) {
			buff = get_element_val(cc_e, buff, &blen, &rv);
			if(rv < 0) { goto error; }
			
			if(rv = get_attribute_int_val(cc_e, MCXML_INDEX, &ind, -1)) { goto error; }
			
			CVIXMLDiscardElement(cc_e);
			cc_e = 0;
		}
		
		CVIXMLDiscardElement(dev_e);
		dev_e = 0;
	}
	
	
	//////////////////////////////////////////////////
	//												//
	//			       Data Display					//
	//												//
	//////////////////////////////////////////////////

	if(dd_e != 0) {
		// Attributes
		int pord, pon;
		
		if(rv = get_attribute_int_val(dd_e, MCXML_POLYON, &pon, 0)) { goto error; }
		uidc.polyon = pon?1:0;
		SetCtrlVal(dc.fid, dc.fpolysub, pon);
		SetCtrlVal(dc.spec, dc.spolysub, pon);
		
		if(rv = get_attribute_int_val(dd_e, MCXML_POLYORD, &pord, -1)) { goto error; }
		
		if(pord > -1) {
			uidc.polyord = pord;
			
			SetCtrlVal(dc.fid, dc.fpsorder, pord);
			SetCtrlVal(dc.spec, dc.spsorder, pord);	
		}
		
		// Child elements
		if(rv = CVIXMLGetChildElementByTag(dd_e, MCXML_FID, &fid_e) < 0) { goto error; }
		if(rv = CVIXMLGetChildElementByTag(dd_e, MCXML_SPEC, &spec_e) < 0) { goto error; }
		
		CVIXMLDiscardElement(dd_e);
		dd_e = 0;
	}
	
	// FID
	if(fid_e != 0) { 
		int as;
		if(rv = get_attribute_int_val(fid_e, MCXML_AUTOSCALE, &as, -1)) { goto error; }
		
		if(as != -1) {
			SetCtrlVal(dc.fid, dc.fauto, as);
		}
		
		// Get the channel information.
		if((rv = CVIXMLFindElements(fid_e, MCXML_CHANNEL, &chan_list)) < 0) { goto error; }
	
		if(chan_list != 0) {
			nl = ListNumItems(chan_list);

			for(i = 0; i < nl; i++) {
				ListRemoveItem(chan_list, &chan_e, FRONT_OF_LIST);
				if(chan_e != 0) { 
					// Index
					if(rv = get_attribute_int_val(chan_e, MCXML_INDEX, &ind, -1)) { goto error; }
					if(ind >= 0 && ind < 8) {
						int color, on;
						// On
						if(rv = get_attribute_int_val(chan_e, MCXML_ON, &on, 0)) { goto error; }
						set_fid_chan(ind, on);
				
						// Color
						buff = get_attribute_val(chan_e, MCXML_COLOR, buff, &blen, &rv);
						if(rv < 0) { goto error; }
						
						if(sscanf(buff, "%x", &color) == 1 && color >= 0) {
							uidc.fcol[ind] = color;
						}
						
						// Gain and offset
						if(rv = get_attribute_float_val(chan_e, MCXML_GAIN, &uidc.fgain[ind], 1.0)) { goto error; }
						if(rv = get_attribute_float_val(chan_e, MCXML_OFFSET, &uidc.foff[ind], 0.0)) { goto error; }
					}
			
					// Not doing anything with the channel name at the moment.
			
					CVIXMLDiscardElement(chan_e);
					chan_e = 0;
				}
			}
	
			ListDispose(chan_list);
			chan_list = 0;
		}
		
		if(uidc.fnc > 1) {
			if(rv = get_attribute_int_val(fid_e, MCXML_CURCHAN, &ind, -1)) { goto error; }
			
			if(ind >= 0 && ind < 8) {
				SetCtrlVal(dc.fid, dc.fcring, ind);
				update_fid_chan_box();
			}
		}
		
		CVIXMLDiscardElement(fid_e);
		fid_e = 0;
	}
	
	// FFT display
	if(spec_e != 0) {
		int as;
		if(rv = get_attribute_int_val(spec_e, MCXML_AUTOSCALE, &as, 0)) { goto error; }
		SetCtrlVal(dc.spec, dc.sauto, as);
		
		// At the moment we don't really need the number of phase orders or the number of chans, they can be inferred.
		if((rv = CVIXMLFindElements(spec_e, MCXML_CHANNEL, &chan_list)) < 0) { goto error; }
		if(chan_list != 0) {
			nl = ListNumItems(chan_list);
			
			for(i = 0; i < nl; i++) {
				ListRemoveItem(chan_list, &chan_e, FRONT_OF_LIST);
				if(chan_e != 0) { 
					// Index
					if(rv = get_attribute_int_val(chan_e, MCXML_INDEX, &ind, -1)) { goto error; }

					if(ind >= 0 && ind < 8) {
						int color, on;
						// On
						if(rv = get_attribute_int_val(chan_e, MCXML_ON, &on, 0)) { goto error; }
						set_spec_chan(ind, on);
	
						// Color
						buff = get_attribute_val(chan_e, MCXML_COLOR, buff, &blen, &rv);
						if(rv < 0) { goto error; }
						
						if(sscanf(buff, "%x", &color) == 1 && color >= 0) {
							uidc.scol[ind] = color;	
						}

						// Gain and offset
						if(rv = get_attribute_float_val(chan_e, MCXML_GAIN, &uidc.sgain[ind], 1.0)) { goto error; }
						if(rv = get_attribute_float_val(chan_e, MCXML_OFFSET, &uidc.soff[ind], 0.0)) { goto error; }
						
						
						// Phase information
						if((rv = CVIXMLFindElements(chan_e, MCXML_PHASE, &phase_list)) < 0) { goto error; }
						if(phase_list != 0) {
							int nlp = ListNumItems(phase_list);
							
							for(j = 0; j < nlp; j++) {
								ListRemoveItem(phase_list, &phase_e, FRONT_OF_LIST);
								
								if(phase_e != 0) { 																	    
									int ord;
									
									if(rv = get_attribute_int_val(phase_e, MCXML_PHASEORDER, &ord, -1)) { goto error; }
									
									if(ord >= 0 && ord < MCD_NPHASEORDERS) {
										float corr = 0.0;
										
										buff = get_element_val(phase_e, buff, &blen, &rv);
										if(rv < 0) { goto error; }
										
										sscanf(buff, "%f", &corr);
										
										uidc.sphase[ind][ord] = corr;
									}   
									
									CVIXMLDiscardElement(phase_e);
									phase_e = 0;
								}
							}
						}
						
						ListDispose(phase_list);
						phase_list = 0;
					}
				
					CVIXMLDiscardElement(chan_e);
					chan_e = 0;
				}
			}
			
			ListDispose(chan_list);
			chan_list = 0;
		}
		
		// Get the current channel in the box.
		if(uidc.snc > 1) {
			if(rv = get_attribute_int_val(spec_e, MCXML_CURCHAN, &ind, -1)) { goto error; }
			
			if(ind >= 0 && ind < 8) {
				SetCtrlVal(dc.spec, dc.scring, ind);
				update_spec_chan_box();
			}
		}
		
		CVIXMLDiscardElement(spec_e);
		spec_e = 0;
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
	if(dname != NULL) { free(dname); }
	if(buff2 != NULL) { free(buff2); }
	if(buff != NULL) { free(buff); }
	
	if(chan_e != 0) { CVIXMLDiscardElement(chan_e); }
	if(phase_e != 0) { CVIXMLDiscardElement(phase_e); }
	
	chan_e = 0;
	phase_e = 0;
	
	// Must dispose of lists.
	if(chan_list != 0) {
		nl = ListNumItems(chan_list);
		for(i = 0; i < nl; i++) {
			ListRemoveItem(chan_list, &chan_e, FRONT_OF_LIST);
			
			CVIXMLDiscardElement(chan_e);
			chan_e = 0;
		}
		
		ListDispose(chan_list);
	}
	
	if(phase_list != 0) {
		nl = ListNumItems(phase_list);
		for(i = 0; i < nl; i++) {
			ListRemoveItem(phase_list, &phase_e, FRONT_OF_LIST);
			
			CVIXMLDiscardElement(phase_e);
			phase_e = 0;
		}
	}
	
	// Now the rest of the elements
	if(r_e != 0) { CVIXMLDiscardElement(r_e); }
	if(gen_e != 0) { CVIXMLDiscardElement(gen_e); }
	if(df_e != 0) { CVIXMLDiscardElement(df_e); }
	if(pf_e != 0) { CVIXMLDiscardElement(pf_e); }
	if(pref_e != 0) { CVIXMLDiscardElement(npsrat_e); }
	if(ppc_e != 0) { CVIXMLDiscardElement(ppc_e); }
	if(pbdev_e != 0) { CVIXMLDiscardElement(pbdev_e); }
	if(dev_e != 0) { CVIXMLDiscardElement(dev_e); }
	if(tc_e != 0) { CVIXMLDiscardElement(tc_e); }
	if(cc_e != 0) { CVIXMLDiscardElement(cc_e); }
	if(dd_e != 0) { CVIXMLDiscardElement(dd_e); }
	if(fid_e != 0) { CVIXMLDiscardElement(fid_e); }
	if(spec_e != 0) { CVIXMLDiscardElement(spec_e); }
	
	// And the attributes
	if(buff_a != 0) { CVIXMLDiscardAttribute(buff_a); }
	if(name_a != 0) { CVIXMLDiscardAttribute(name_a); }
	if(ind_a != 0) { CVIXMLDiscardAttribute(ind_a); }
	
	// Discard the document
	if(xml_doc != -1) { CVIXMLDiscardDocument(xml_doc); }
	
	if(safe) { 
		CmtReleaseLock(lock_uidc);
		CmtReleaseLock(lock_uipc);
	}
	return rv;
}

char *get_element_val(CVIXMLElement elem, char *buff, int *blen, int *ev) {
	int rv = 0;
	int failed = 1;
	int bl = 0, len;
	
	if(blen != NULL) { bl = *blen; }
	
	
	if(elem != 0) {
		if(rv = CVIXMLGetElementValueLength(elem, &len) && len != 0) { goto error; }
		
		if(len > 0) {
			buff = realloc_if_needed(buff, &bl, ++len, 1);
			
			if(rv = CVIXMLGetElementValue(elem, buff)) { goto error; }
			
			failed = 0;
		} else {
			rv = 0;	
		}
	}
	
	error:
	
	if(failed) {
		buff = realloc_if_needed(buff, &bl, 2, 1);
		strcpy(buff, "");
	}
	
	if(blen != NULL) { *blen = bl; }
	if(ev != NULL) { *ev = rv; }
	
	return buff;
}

char *get_attribute_val(CVIXMLElement elem, char *att_name, char *buff, int *blen, int *ev) {
	// Get the value from an attribute. If buff == NULL, this will allocate a new
	// array. Otherwise it will reallocate buff if necessary, buff is returned.
	// On failure, buff is set to "". Only pass dynamically allocated memory to this.
	// Result must be freed.
	
	CVIXMLAttribute att = 0;
	int bl = 0, failed = 1, rv = 0, len;
	
	if(att_name == NULL) { rv = MCSS_ERR_NOATTNAME; goto error; }
	
	if(blen != NULL) {
		bl = *blen;
	}

	if(elem != 0) {
		if((rv = CVIXMLGetAttributeByName(elem, att_name, &att)) < 0) { goto error; }
		
		if(att != 0) { 
			if((rv = CVIXMLGetAttributeValueLength(att, &len) < 0) && len != 0) { goto error; }
			if(len > 0) {
				buff = realloc_if_needed(buff, &bl, ++len, 1);
				
				if((rv = CVIXMLGetAttributeValue(att, buff)) < 0) { goto error; }
				
				failed = 0;
			} else {
				rv = 0;	
			}
		}
	}
	
	error:
	if(att != 0) { CVIXMLDiscardAttribute(att); }
	
	// Not what you're used to, I imagine
	if(failed) {
		buff = realloc_if_needed(buff, &bl, 2, 1);
		strcpy(buff, "");
	}
	
	if(blen != NULL) { *blen = bl; }
	if(ev != NULL) { *ev = rv; }
	
	return buff;
}

int get_attribute_float_val(CVIXMLElement elem, char *att_name, float *val, float dflt_val) {
	int rv = 0;
	CVIXMLAttribute att = 0;
	char buff[32] ="";		// We'll assume that no floats are represented with more than 31 digits.
	float out = dflt_val;

	if(val == NULL) { rv = MCSS_ERR_NOOUT; goto error; }
	if(att_name == NULL) { rv = MCSS_ERR_NOATTNAME; goto error; }

	if(elem != 0) {
		if(rv = CVIXMLGetAttributeByName(elem, att_name, &att)) { goto error; }
	
		if(att != 0) {
			if(rv = CVIXMLGetAttributeValue(att, buff)) { goto error; }
			float fbuff = 0;
			if(sscanf(buff, "%f", &fbuff) == 1) {
				out = fbuff;	
			}
		}
	}

	error:
	if(att != 0) { CVIXMLDiscardAttribute(att); }

	*val = out;
	return rv;	
}

int get_attribute_int_val(CVIXMLElement elem, char *att_name, int *val, int dflt_val) {
	int rv = 0;
	CVIXMLAttribute att = 0;
	char buff[32] ="";		// We'll assume that no ints are represented with more than 31 digits.
	int out = dflt_val;
	
	if(val == NULL) { rv = MCSS_ERR_NOOUT; goto error; }
	if(att_name == NULL) { rv = MCSS_ERR_NOATTNAME; goto error; }
	
	if(elem != 0) {
		if(rv = CVIXMLGetAttributeByName(elem, att_name, &att)) { goto error; }
		
		if(att != 0) {
			if(rv = CVIXMLGetAttributeValue(att, buff)) { goto error; }
			int ibuff = 0;
			if(sscanf(buff, "%d", &ibuff) == 1) {
				out = ibuff;	
			}
		}
	}
	
	error:
	if(att != 0) { CVIXMLDiscardAttribute(att); }
	
	*val = out;
	return rv;
}

void display_xml_error(int err) {
	char *message = malloc(500);
	CVIXMLGetErrorString(err, message, 500);
	
	MessagePopup("XML Error", message);
	free(message);
}
