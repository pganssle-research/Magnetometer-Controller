/********************** Legacy Functions for v2.0 **************************/

//////////////////////////////////////////////////////
//                                                  //
//              Main Program Functions              //
//                                                  //
//////////////////////////////////////////////////////
int main (int argc, char *argv[]) // The original main() function
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
	
	//Panels in the tabs
	GetPanelHandleFromTabPage (panelHandle, MainPanel_MainTabs, 0, &FID);
	GetPanelHandleFromTabPage (panelHandle, MainPanel_MainTabs, 1, &FFTSpectrum);
	GetPanelHandleFromTabPage (panelHandle, MainPanel_MainTabs, 2, &Pulse_Prog_Tab);
	GetPanelHandleFromTabPage (panelHandle, MainPanel_MainTabs, 3, &Pulse_Prog_Config);
	
	//Menu bars
	panelHandleMenu = GetPanelMenuBar(panelHandle);
	RC_menu = LoadMenuBar(0, "Magnetometer Controller.uir", RCMenus);

	//Child Panels in Pulse Program Tab
	Pulse_Prog_Panel = LoadPanel (Pulse_Prog_Tab, "Magnetometer Controller.uir", PPPanel);
	PPConfigSubPanel = LoadPanel(Pulse_Prog_Config, "Magnetometer Controller.uir", PPConfigP);
	HiddenPanel = LoadPanel(0, "Magnetometer Controller.uir", HiddenVals);
	inst[0] = LoadPanel (Pulse_Prog_Panel, "Magnetometer Controller.uir", PulseInstP);
	cinst[0] = LoadPanel (PPConfigSubPanel, "Magnetometer Controller.uir", MDInstr);

	SetPanelPos (Pulse_Prog_Panel, 5, 2);
	SetPanelPos (inst[0], 40, 7);

	SetPanelPos (PPConfigSubPanel, 32, 8);
	SetPanelPos (cinst[0], 28, 7);

	DisplayPanel(Pulse_Prog_Panel);
	DisplayPanel(inst[0]);
	
	SetPanelAttribute(PPConfigSubPanel, ATTR_DIMMED, 1);
	SetPanelAttribute(cinst[0], ATTR_DIMMED, 1);    
	DisplayPanel(PPConfigSubPanel);
	DisplayPanel(cinst[0]);

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
