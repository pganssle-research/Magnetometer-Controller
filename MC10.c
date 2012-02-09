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
#include <NIDAQmx.h>

#include <Magnetometer Controller.h>	// Then the libraries specific to
#include <PulseProgramTypes.h>			// this software.
#include <cvitdms.h>
#include <cviddc.h>
#include <UIControls.h>
#include <MathParserLib.h>
#include <MCUserDefinedFunctions.h>
#include <MC10.h> 
#include <DataLib.h>
#include <PulseProgramLib.h>
#include <SaveSessionLib.h>
#include <General.h>

static TaskHandle acquireSignal, counterTask;    
extern PVOID RtlSecureZeroMemory(PVOID ptr, SIZE_T cnt);

// Multithreading variable declarations
DefineThreadSafeScalarVar(int, QuitUpdateStatus, 0); 
DefineThreadSafeScalarVar(int, QuitIdle, 0);
DefineThreadSafeScalarVar(int, DoubleQuitIdle, 0);
DefineThreadSafeScalarVar(int, Status, PB_STOPPED);
DefineThreadSafeScalarVar(int, Running, 0);
DefineThreadSafeScalarVar(int, Initialized, 0);

int lock_pb, lock_DAQ, lock_tdm; 		// Thread locks
int lock_uidc, lock_uipc, lock_ce, lock_af; 

//////////////////////////////////////////////////////
//                                                  //
//              Main Program Functions              //
//                                                  //
//////////////////////////////////////////////////////

int main (int argc, char *argv[])
{
	//Generate locks, thread safe variables
	CmtNewLock(NULL, OPT_TL_PROCESS_EVENTS_WHILE_WAITING, &lock_pb);
	CmtNewLock(NULL, OPT_TL_PROCESS_EVENTS_WHILE_WAITING, &lock_DAQ);
	CmtNewLock(NULL, OPT_TL_PROCESS_EVENTS_WHILE_WAITING, &lock_tdm);
	CmtNewLock(NULL, OPT_TL_PROCESS_EVENTS_WHILE_WAITING, &lock_uidc);
	CmtNewLock(NULL, OPT_TL_PROCESS_EVENTS_WHILE_WAITING, &lock_uipc);
	CmtNewLock(NULL, OPT_TL_PROCESS_EVENTS_WHILE_WAITING, &lock_ce);
	CmtNewLock(NULL, OPT_TL_PROCESS_EVENTS_WHILE_WAITING, &lock_af);
	
	
	InitializeQuitUpdateStatus();
	InitializeQuitIdle();
	InitializeDoubleQuitIdle();
	InitializeStatus();
	InitializeInitialized();
	InitializeRunning();
	
	SetQuitUpdateStatus(0);
	SetQuitIdle(0);
	SetDoubleQuitIdle(0);
	SetStatus(PB_STOPPED);
	SetRunning(0);
	SetInitialized(0);
	
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	
	//Main panel
	char *uifname = "Magnetometer Controller.uir"; // So it's easy to change this if we need to.
	
	if(load_ui(uifname)) // This function loads the UI and creates the structs.
		return -1;
 	RunUserInterface ();
	DiscardPanel (mc.mp);
	
	//Discard Locks
	CmtDiscardLock(lock_pb);
	CmtDiscardLock(lock_DAQ);
	CmtDiscardLock(lock_tdm);
	CmtDiscardLock(lock_uidc);
	CmtDiscardLock(lock_uipc);
	CmtDiscardLock(lock_ce);
	CmtDiscardLock(lock_af);
	UninitializeQuitIdle();
	UninitializeDoubleQuitIdle();
	UninitializeStatus();
	UninitializeQuitUpdateStatus();
	UninitializeInitialized();
	UninitializeRunning();

	return 0;
}


//////////////////////////////////////////////////////
//                                      	        //
//            	Main Panel Callbacks	 	  		//
//                                                  //
//////////////////////////////////////////////////////

int CVICALLBACK StartProgram (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// This is the button which starts the main program, or if the pulseblaster
			// is waiting for a trigger, it sends a software trigger.
			
			// Send a software trigger if it's waiting for one.
			int status = update_status(0);
			if(status & PB_WAITING) {
				pb_start_safe(1);
				break;
			}
			
			// Don't do anything if we're running.
			if(status & PB_RUNNING || GetRunning())
				break;
			
			// Get the filename and description.
			int len, len2;
			char *path = NULL, *fname = NULL, *desc = NULL;
			
			// Pathname first
			GetCtrlValStringLength(mc.path[1], mc.path[0], &len);
			GetCtrlValStringLength(mc.basefname[1], mc.basefname[0], &len2);
			path = malloc(len+len2+11);
			GetCtrlVal(mc.path[1], mc.path[0], path);
			
			if(!FileExists(path, NULL)) {
				MessagePopup("Directory Not Found", "The data storage directory was not found. Please create it and get back to me.");
				goto error;
			}
		
			// Now the current filename.
			fname = malloc(len2+5);
			GetCtrlVal(mc.basefname[1], mc.basefname[0], fname);
			if(get_current_fname(path, fname, 1) != 1)
				goto error;
			
			SetCtrlVal(mc.cdfname[1], mc.cdfname[0], fname);
			if(path[strlen(path)-1] != '\\') {
				sprintf(path, "%s\\", path);
			}
			
			sprintf(path, "%s%s.tdm", path, fname);	// Update the path with the full filename
			
			// Copy these into the current experiment structure
			CmtGetLock(lock_ce);
			if(ce.path != NULL)
				free(ce.path);
			
			if(ce.fname != NULL)
				free(ce.fname);
			
			ce.path = malloc(strlen(path)+1);
			strcpy(ce.path, path);
			
			ce.fname = malloc(strlen(fname)+1);
			strcpy(ce.fname, fname);
			
			SetCtrlVal(mc.cdfname[1], mc.cdfname[0], ce.fname);  // Update the UI appropriately
			
			// Then description
			GetCtrlValStringLength(mc.datadesc[1], mc.datadesc[0], &len);
			desc = malloc(len+1);
			GetCtrlVal(mc.datadesc[1], mc.datadesc[0], desc);
	
			if(ce.desc != NULL)
				free(ce.desc);
			
			ce.desc = malloc(strlen(desc)+1);
			strcpy(ce.desc, desc);
			CmtReleaseLock(lock_ce);
			
			// Start an asynchronous thread to run the experiment.
			CmtScheduleThreadPoolFunctionAdv(DEFAULT_THREAD_POOL_HANDLE, IdleAndGetData, NULL, 0, discardIdleAndGetData, EVENT_TP_THREAD_FUNCTION_END, NULL, RUN_IN_SCHEDULED_THREAD, NULL); 
			
			error:
			
			if(path != NULL)
				free(path);
			
			if(fname != NULL)
				free(fname);
			
			if(desc != NULL)
				free(desc);
			
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
			// This is for shutting down the execution of the program. It does this in a 
			// thread-safe manner by changing the thread-safe variables QuitIdle, 
			// DoubleQuitIdle and QuitUpdateStatus. QuitIdle triggers at the end of the
			// current acquisition. DoubleQuitIdle triggers during acquisition.
			
			if(GetQuitIdle())
				SetDoubleQuitIdle(1);
			else
				SetQuitIdle(1);
			
			SetQuitUpdateStatus(1);
			
			if(!GetInitialized())
				pb_init_safe(1);
			
			pb_stop_safe(1);
		
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
			int ldm;
			GetCtrlVal(mc.ldatamode[1], mc.ldatamode[0], &ldm);
			
			// If someone edited the description, update it before you exit.
			if(ldm) {
				int len;
				GetCtrlValStringLength(mc.datafbox[1], mc.datafbox[0], &len);
				char *path = malloc(len+1);
				GetCtrlVal(mc.datafbox[1], mc.datafbox[0], path);
				
				update_file_info(path);
				
				free(path);
			}
			
			save_session(NULL);	// Save the session before we leave.
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
	int ev = save_session(NULL);
	
	if(ev)
		display_tdms_error(ev);
	
}

void CVICALLBACK SaveConfigToFile (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	char *filename = malloc(MAX_PATHNAME_LEN+MAX_FILENAME_LEN+1);
	int rv = FileSelectPopup("", ".nc", ".nc", "Save Configuration To File", VAL_SAVE_BUTTON, 0, 0, 1, 1, filename);
	if(rv == 0)
		return;
	
	int ev = save_session(filename);
	free(filename);

	if(ev)
		display_tdms_error(ev);
}

void CVICALLBACK LoadConfigurationFromFile (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	char *filename = malloc(MAX_PATHNAME_LEN+MAX_FILENAME_LEN+1);
	int rv = FileSelectPopup("", ".xml", ".xml", "Load Configuration From File", VAL_LOAD_BUTTON, 0, 1, 1, 1, filename);
	if(rv == 0)
		return;
	
	int ev = load_session(filename);
	free(filename);

	if(ev)
		display_ddc_error(ev);
	
}

int CVICALLBACK DirectorySelect (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			char *path = malloc(MAX_PATHNAME_LEN+1); 
			int rv = DirSelectPopup(uidc.dlpath, "Select new save dir", 1, 1, path);
			if(rv != VAL_NO_DIRECTORY_SELECTED)
				select_directory(path);
			
			free(path);
			
			break;
	}
	return 0;
}

int CVICALLBACK ChangeDataBox (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			select_data_item();
			break;
		case EVENT_VAL_CHANGED:
			int len, ldm;
			GetCtrlVal(mc.ldatamode[1], mc.ldatamode[0], &ldm);
			
			GetCtrlValStringLength(panel, control, &len);
			char *path = malloc(len+1);
			GetCtrlVal(panel, control, path);
			
			// Grab the old path.
			GetCtrlAttribute(panel, control, ATTR_DFLT_VALUE_LENGTH, &len);
			char *old_path = (len >= 1)?malloc(len+1):NULL;
			if(old_path != NULL)
				GetCtrlAttribute(panel, control, ATTR_DFLT_VALUE, old_path);
			
			if(ldm) 
				load_file_info(path, old_path);
		 
			// Save the new position if it's a file.
			int ind, icon;
			GetCtrlIndex(panel, control, &ind);
			GetListItemImage(panel, control, ind, &icon);
			if(icon != VAL_FOLDER) 
				SetCtrlAttribute(mc.datafbox[1], mc.datafbox[0], ATTR_DFLT_VALUE, path);
			else
				SetCtrlAttribute(mc.datafbox[1], mc.datafbox[1], ATTR_DFLT_VALUE, "");
			
			if(path != NULL)
				free(path);
			
			if(old_path != NULL)
				free(old_path);
		
			break;
	}
	return 0;
}

int CVICALLBACK ChangeLoadInfoMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// Turning this on loads the data info that is currently selected.
			// Turning this off moves us to the "next experiment" mode.
			int mode;
			GetCtrlVal(panel, control, &mode);
			
			if(mode) {
				int len;
				char *name = NULL;
				
				GetCtrlValStringLength(mc.datafbox[1], mc.datafbox[0], &len);
				name = malloc(len+1);
				GetCtrlVal(mc.datafbox[1], mc.datafbox[0], name);
				
				load_file_info(name, NULL); // Nowhere to save the old description.
				
				SetActiveCtrl(mc.datafbox[1], mc.datafbox[0]);
				
				free(name);
			} else {
				int len;
				char *name, *path;
				
				GetCtrlValStringLength(mc.basefname[1], mc.basefname[0], &len);
				name = malloc(len+5);
				GetCtrlVal(mc.basefname[1], mc.basefname[0], name);
				
				GetCtrlValStringLength(mc.path[1], mc.path[0], &len);
				path = malloc(len+strlen(name)+10);
				GetCtrlVal(mc.path[1], mc.path[0], path); 
				
				get_current_fname(path, name, 1);
				
				SetCtrlVal(mc.cdfname[1], mc.cdfname[0], name);
				
				free(name);
				free(path);
			}
				
				
				
			break;
	}
	return 0;
}


int CVICALLBACK PopoutTab (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch(event) {
		case EVENT_RIGHT_CLICK:
			/*int top, left;
			int old_pan = pc.PPConfigPan;
			pc.PPConfigPan = DuplicatePanel(0, pc.PPConfigPan, "Pulse Program Config", 250, 150);
			
			GetPanelAttribute(pc.PPConfigCPan, ATTR_TOP, &top);
			GetPanelAttribute(pc.PPConfigCPan, ATTR_LEFT, &left);
			
			pc.PPConfigCPan = DuplicatePanel(pc.PPConfigPan, pc.PPConfigCPan, "", top, left);
			
			for(int i = 0; i < uipc.max_ni; i++) {
				GetPanelAttribute(pc.cinst[i], ATTR_TOP, &top);
				GetPanelAttribute(pc.cinst[i], ATTR_LEFT, &left);
			
				pc.cinst[i] = DuplicatePanel(pc.PPConfigCPan, pc.cinst[i], "", top, left);
				if(i < uipc.ni)
					DisplayPanel(pc.cinst[i]);
			}
			
			DeleteTabPage(mc.mtabs[1], mc.mtabs[0], 3, 1);

			int quit = NewCtrl(pc.PPConfigPan, CTRL_SQUARE_PUSH_BUTTON, "Quit", 0, 0);
			SetCtrlAttribute(pc.PPConfigPan, quit, ATTR_VISIBLE, 0);
			InstallCtrlCallback(pc.PPConfigPan, quit, ppconfig_popin, NULL);
			
			SetPanelAttribute(pc.PPConfigPan, ATTR_TITLEBAR_VISIBLE, 1);
			SetPanelAttribute(pc.PPConfigPan, ATTR_CLOSE_CTRL, quit);
			
			DisplayPanel(pc.PPConfigCPan);
			DisplayPanel(pc.PPConfigPan);
			*/	
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
			for(i=0; i<uipc.ni; i++) {
				if(pc.inst[i] == panel) {
					from = i;
					break;
				}
			}
			
			if(i == uipc.ni)
				break;	// Error.
			
			GetCtrlVal(panel, pc.ins_num, &to);
			
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
			GetCtrlVal(panel, pc.ins_num, &num);
			
			// How far apart are the instructions - they're evenly spaced, so pick the first two.
			if (uipc.ni > 1) {
				int top1, top2;	 // We need to know how much we moved to know how much to move the cursor
				GetCtrlAttribute(panel, control, ATTR_TOP, &top1);
				
				// Now we just decide if it was called by the up button or the down button and move the instruction.
				if(control == pc.uparrow && num >= 1)
					move_instruction(num-1, num);
				else if(control == pc.downarrow && num < (uipc.ni-1)) 
					move_instruction(num+1, num);
				else
					break;
				
				// Finally we find out how much it moved and move the cursor.
				GetCtrlAttribute(panel, control, ATTR_TOP, &top2);
				SetCursorPos(pos.x, pos.y+(top2-top1));
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
				GetCtrlVal(panel, pc.ins_num, &num);
				
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
			change_instr_delay(panel);
			
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
			GetCtrlVal(panel, pc.ins_num, &num);
			
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
			GetCtrlVal(panel, pc.ins_num, &num);
			
			change_instr_units(panel);
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
			change_instr_data(panel);
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
			int val;
			GetCtrlVal(panel, control, &val);
			
			set_scan_panel(panel, val);
			break;
	}
	return 0;
} 

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
			GetCtrlVal(panel, pc.cins_num, &num);
			
			int state = get_nd_state(num);
			
			if(up)
				state++;
			else
				state--;
			
			state = (state+3)%3; 	// In C, mod is signed, so we need to add 4 first here
			
			int val;
			GetCtrlAttribute(pc.inst[num], pc.ins_num, ATTR_FRAME_COLOR, &val);
			
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
			
			GetCtrlVal(panel, pc.dim, &dim);
			GetCtrlVal(panel, pc.nsteps, &steps);
			
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
			GetCtrlVal(panel, pc.cins_num, &num);
			
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
			GetCtrlVal(panel, pc.cins_num, &num);
			
			if(control == pc.del_fin || control == pc.dat_fin)
				update_nd_increment(num, MC_INC);
			else
				update_nd_increment(num, MC_FINAL);
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
			GetCtrlVal(panel, pc.cins_num, &num);
			
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
			GetCtrlVal(panel, pc.ins_num, &num);
			
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
			GetCtrlVal(panel, pc.pcsteps, &steps);
			GetCtrlVal(panel, pc.pclevel, &cyc);
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
			GetCtrlVal(panel, pc.ins_num, &num);
			
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
			GetCtrlVal(panel, pc.ins_num, &num);
			change_cycle_step(num);
			break;
	}
	return 0;
}


/************ Expanded Instruction Callbacks ************/
int CVICALLBACK MoveInstButtonExpanded (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// Where the cursor is now
			POINT pos;
			GetCursorPos(&pos);
			
			int cData = (int)callbackData;
			int up = get_bits(cData, 0, 0);
			int step = get_bits(cData, 1, 11); 
			int num = (cData-(up+step))/2048;
			
			int cyc, steps;
			GetCtrlIndex(pc.inst[num], pc.pclevel, &cyc);
			steps = uipc.cyc_steps[cyc];
			
			// Determine if we should move it, and get the info we need to do so
			int nstep;
			if(up && step >= 1)
				nstep = step-1;
			else if (!up && step < steps-1)
				nstep = step+1;
			else
				break;
			
			move_expanded_instruction(num, nstep, step);	// Move the instruction
			
			// Move the cursor if you need to.
			if(nstep > 0) {
				int top1, top2;
				GetPanelAttribute(panel, ATTR_TOP, &top1);
				GetPanelAttribute(uipc.cyc_pans[num][nstep], ATTR_TOP, &top2);
				
				SetCursorPos(pos.x, pos.y+(top2-top1));
			} else {
				int top1, top2, left1, left2;
				int ptop1, ptop2;
				GetPanelAttribute(panel, ATTR_TOP, &ptop1);
				GetCtrlAttribute(panel, control, ATTR_TOP, &top1);
				GetCtrlAttribute(panel, control, ATTR_LEFT, &left1);
				
				GetCtrlAttribute(pc.inst[num], pc.pcstep, ATTR_TOP, &top2);
				GetCtrlAttribute(pc.inst[num], pc.pcstep, ATTR_LEFT, &left2);
				
				SetCursorPos(pos.x+(left2-left1), pos.y+(top2-(ptop1+top1)));
			}
		
			break;
	}
	return 0;
}

int CVICALLBACK ChangePhaseCycleStepExpanded (int panel, int control, int event,
	void *callbackData, int eventData1, int eventData2)
{
	switch(event)
	{
		case EVENT_COMMIT:
			// This is the "move instruction to any spot" function.
			
			// Get the event data
			int cData = (int)callbackData;
			int step = get_bits(cData, 0, 9);
			int num = (cData-step)/1024;
			
			int nstep;
			GetCtrlIndex(panel, control, &nstep);
			if(nstep == step)
				break;
			
			move_expanded_instruction(num, nstep, step);
			SetCtrlIndex(panel, control, step);
			break;
	}
	return 0;
}

int CVICALLBACK DeleteInstructionCallbackExpanded (int panel, int control, int event, void *callbackData, int eventData1, int eventData2) {
	switch(event) {
		case EVENT_COMMIT:
			// This is the "delete a step" function
			
			// Get the event data
			int cData = (int)callbackData;
			int step = get_bits(cData, 0, 9);
			int num = (cData-step)/1024;

			int cyc;
			GetCtrlIndex(pc.inst[num], pc.pclevel, &cyc);
			if(uipc.cyc_steps[cyc] < 3)
				break;
			
			delete_expanded_instruction(num, step);
			break;
	}
	return 0;
}

int CVICALLBACK ExpandPhaseCycle (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
		case EVENT_LEFT_DOUBLE_CLICK:
			int num;
			GetCtrlVal(panel, pc.ins_num, &num);
			
			set_phase_cycle_expanded(num, 1);
			break;
	}
	return 0;
}

int CVICALLBACK CollapsePhaseCycle (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
		case EVENT_LEFT_DOUBLE_CLICK:
			int num;
			GetCtrlVal(panel, pc.ins_num, &num);
			
			set_phase_cycle_expanded(num, 0);
			break;
	}
	return 0;
}

int CVICALLBACK InstrCallbackExpanded (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				change_instruction_panel(panel);
			break;
	}
	return 0;
}

int CVICALLBACK ChangeInstDelayExpanded(int panel, int control, int event, void *callbackData, int eventData1, int eventData2) {
	switch(event) {
		case EVENT_COMMIT:
			break;
	}
	return 0;
}

int CVICALLBACK ChangeTUnitsExpanded (int panel, int control, int event, void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
			break;
	}
	return 0;
}

int CVICALLBACK InstrDataCallbackExpanded(int panel, int control, int event, void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
			break;
	}
	return 0;
}

int CVICALLBACK Change_ScanExpanded (int panel, int control, int event, void *callbackData, int eventData1, int eventData2) {
	switch(event) {
		case EVENT_COMMIT:
			break;
	}
	return 0;
}

//////////////////////////////////////////////////////
//                                      	        //
//            Pulse Prog Tabs Callbacks	 	  		//
//                                                  //
//////////////////////////////////////////////////////

/***************** Basic Prog Setup Callbacks ******************/

int CVICALLBACK Change_Trigger (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			change_trigger_ttl();
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

/**************** Load/Save/New Prog Callbacks *****************/
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
			load_prog_popup();
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
			save_prog_popup();
			break;
	}
	return 0;
}

/************* Sampling Space Position Callbacks ***************/

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


/******************* Basic Config Callbacks ********************/ 
int CVICALLBACK ChangeNP_AT_SR (int panel, int control, int event, void *callbackData, int eventData1, int eventData2) {
	switch(event) {
		case EVENT_COMMIT:
			// Called by number of poitns, acquisition time and sampling rate.
			// callbackData should be 0, 1 or 2, and is passed to change_np_or_sr(int np_sr_at);
			
			change_np_or_sr((int)callbackData);
			break;
		case EVENT_RIGHT_CLICK:
			// Allow the user to select what happens when they change this particular element
			int submenu, *choice = malloc(sizeof(int)*2), *choiceval = malloc(sizeof(int)*2);
			
			// Figure out which situation we're in and set the popup correctly.
			if(control == pc.sr[0]) {
				submenu = RCMenus_SampleRate;
				choice[0] = RCMenus_SampleRate_NumPoints;
				choiceval[0] = 2;
				
				choice[1] = RCMenus_SampleRate_AcquisitionTime;
				choiceval[1] = 0;
			} else if (control == pc.at[0]) {
				submenu = RCMenus_AcquisitionTime;
				choice[0] = RCMenus_AcquisitionTime_SampleRate;
				choiceval[0] = 1;
				
				choice[1] = RCMenus_AcquisitionTime_NumPoints;
				choiceval[1] = 2;
			} else if (control == pc.np[0]) {
				submenu = RCMenus_NumPoints;
				choice[0] = RCMenus_NumPoints_AcquisitionTime;
				choiceval[0] = 0;
				
				choice[1] = RCMenus_NumPoints_SampleRate;
				choiceval[1] = 1;
			} else {
				free(choice);
				free(choiceval);
			}
			
			int rv = RunPopupMenu(mc.rcmenu, submenu, panel, eventData1, eventData2, 0, 0, 0, 0);
			
			InstallCtrlCallback(panel, control, ChangeNP_AT_SR, (void*)(choiceval[((rv == choice[0])?0:1)]));
			
			SetMenuBarAttribute(mc.rcmenu, rv, ATTR_CHECKED, 1);
			SetMenuBarAttribute(mc.rcmenu, (rv == choice[0])?choice[1]:choice[0], ATTR_CHECKED, 0);
			
			free(choice);
			free(choiceval);
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
			change_nt();
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
			int ndon;
			GetCtrlVal(panel, control, &ndon);
			set_ndon(!ndon);
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
			int time_ctrl;
			if(control == pc.delu_init)
				time_ctrl = pc.del_init;
			else if(control == pc.delu_fin)
				time_ctrl = pc.del_fin;
			else if(control == pc.delu_inc)
				time_ctrl = pc.del_inc;
			else
				break;
			
			int old_units, new_units;
			GetCtrlIndex(panel, control, &new_units);
			GetCtrlAttribute(panel, control, ATTR_DFLT_INDEX, &old_units);
			
			double val;
			GetCtrlVal(panel, time_ctrl, &val);
			val *= pow(1000, old_units-new_units);
			
			SetCtrlVal(panel, time_ctrl, val);
			SetCtrlAttribute(panel, time_ctrl, ATTR_PRECISION, get_precision(val, 5));
			SetCtrlAttribute(panel, control, ATTR_DFLT_INDEX, new_units);
			SetCtrlAttribute(panel, time_ctrl, ATTR_PRECISION, get_precision(val, MCUI_DEL_PREC));

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
			GetCtrlVal(panel, pc.cins_num, &num);
				
			update_nd_from_exprs(num);
			break;
			
		case EVENT_RIGHT_CLICK:
			// Right clicking on either control gives a popup with the error, if there was an error.
			int dat = 0, del = 0;
			if(control == pc.cexpr_data)
				dat = 1;
			else if(control == pc.cexpr_delay)
				del = 1;
			
			if((del && uipc.err_del) || (dat && uipc.err_dat)) {
				char *err_message;
				int size, i, *pos, err;
				
				// Copy the specific control parameters into generic parameters.
				if(dat) {
					err = uipc.err_dat;
					size = uipc.err_dat_size;
					pos = malloc(sizeof(int)*size);
					
					for(i=0; i<size; i++)
						pos[i] = uipc.err_dat_pos[i];
				} else {
					err = uipc.err_del;
					size = uipc.err_del_size;
					pos = malloc(sizeof(int)*size);
					
					for(i=0; i<size; i++)
						pos[i] = uipc.err_del_pos[i];
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
			if(uipc.skip_err) {
				int len = get_parse_error(uipc.skip_err, NULL);	// Length of the message
				char *err_message = malloc(len+1);					// Allocate space for the message
				
				get_parse_error(uipc.skip_err, err_message); 		// Length of the message
				
				char *output = generate_expression_error_message(err_message, uipc.skip_err_pos, uipc.skip_err_size);
				
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
			toggle_ic();
			break;
	}
	return 0;
}

int CVICALLBACK ChangeDevice (int panel, int control, int event, void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
				load_DAQ_info();
			break;
	}
	return 0;
}

int CVICALLBACK ChangePBDevice (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

// Channel range setup
int CVICALLBACK ChangeCurrentChan (int panel, int control, int event, void *callbackData, int eventData1, int eventData2) {
	switch (event)
	{
		case EVENT_COMMIT:
				change_chan();
			break;
	}
	return 0;
}

int CVICALLBACK ChangeChannelRange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				change_range();
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
	load_prog_popup();
}

void CVICALLBACK NewProgramMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	clear_program();
}

void CVICALLBACK SaveProgramMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	save_prog_popup();
}

void CVICALLBACK BrokenTTLsMenu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	// Loads the Broken TTL panel
	int bt = LoadPanel(0, pc.uifname, BrokenTTLs);
	DisplayPanel(bt);
	
	// Set up the TTLs.
	int *TTLs = get_broken_ttl_ctrls();
	
	for(int i = 0; i < 24; i++) {
		InstallCtrlCallback(bt, TTLs[i], ToggleBrokenTTL, (void *)i);
		SetCtrlVal(bt, TTLs[i], (uipc.broken_ttls & (1<<i)?1:0));
	}
	
	free(TTLs);
	
	// Dim the menu bar until we destroy the panel later.
	SetMenuBarAttribute(mc.mainmenu, mc.sbttls, ATTR_DIMMED, 1);
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
	load_DAQ_info();
}

//////////////////////////////////////////////////////
//                                      	        //
//            Broken TTL Panel Callbacks	   		//
//                                                  //
//////////////////////////////////////////////////////
int CVICALLBACK BrokenTTLsExit (int panel, int control, int event, void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
			// Throws away the panel, since we're done.
			DiscardPanel(panel);
			
			// Un-dim the menu bar item
			SetMenuBarAttribute(mc.mainmenu, mc.sbttls, ATTR_DIMMED, 0);
			break;
	}
	return 0;
}

int CVICALLBACK ToggleBrokenTTL (int panel, int control, int event, void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
			// Check if the TTL in question is in use and offer to swap if necessary.
			
			int val, loc = (int)(callbackData), flag = 1<<loc;
			GetCtrlVal(panel, control, &val);
			
			if(val) {
				int reserved = ttls_in_use();		// Get the ttls currently in use.
				
				// If it's in use, offer to swap with whatever you want. Currently only one
				// level of swapping. On cancel, no swap takes place.
				if(reserved & flag) {
					char *resp = malloc(3);
					char *message = malloc(200);
					sprintf(message, "There is a conflict in TTL %d. If you would like to swap with another TTL line, enter the line (0-23). Enter -1 or cancel if you would like to keep it where it is.", loc);

					if(PromptPopup("TTL Conflict", message, resp, 2) != VAL_USER_CANCEL) {
						int fc;
						if(sscanf(resp, "%d", &fc) > 0 && fc >= 0 && fc < 24)
							swap_ttl(loc, fc);
					}
					free(resp);
				}
				
				CmtGetLock(lock_uipc);
				uipc.broken_ttls = uipc.broken_ttls|flag;	// Add the flag
				CmtReleaseLock(lock_uipc);
			} else {
				CmtGetLock(lock_uipc);
				uipc.broken_ttls = uipc.broken_ttls-(uipc.broken_ttls&flag);	// Remove the flag
				CmtReleaseLock(lock_uipc);
			}
				
			setup_broken_ttls(); // Update the controls.
			break;
	}
	return 0;
}

int CVICALLBACK BrokenTTLsClearAll (int panel, int control, int event, void *callbackData, int eventData1, int eventData2) {
	switch (event){
		case EVENT_COMMIT:
			// Resets all broken TTLs.
			int *TTLs = get_broken_ttl_ctrls();
			
			for(int i = 0; i < 24; i++)
				SetCtrlVal(panel, TTLs[i], 0);
			
			free(TTLs);
			
			CmtGetLock(lock_uipc);
			uipc.broken_ttls = 0;
			CmtReleaseLock(lock_uipc);
			setup_broken_ttls();
			break;
	}
	return 0;
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
	load_data_popup();
}

/********* Acquisition Space Navigation ***********/
int CVICALLBACK DatChangeIDPos (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				set_data_from_nav(panel);
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
			
			set_data_from_nav(panel);
			
			break;
	}
	return 0;
}

void CVICALLBACK ChangeTransientView (int menuBar, int menuItem, void *callbackData,
		int panel)
{   
	// This is the callback for when you switch between the various transient views.
	// The mechanism is that when the callbacks are initialized, the callbackData for
	// each one is installed to indicate what uidc.disp_update should be. Casting
	// callbackData to an int returns this value.
	// 0 = Show average
	// 1 = Latest transient
	// 2 = No change
	
	int new, old = mc.vtviewopts[uidc.disp_update];
	
	CmtGetLock(lock_uidc);
	uidc.disp_update = (int)callbackData;	// Update the uidc variable.
	CmtReleaseLock(lock_uidc);
	
	new = mc.vtviewopts[uidc.disp_update];
	
	// Update which one's checked.
	if(new != old) {
		SetMenuBarAttribute(mc.mainmenu, new, ATTR_CHECKED, 1);
		SetMenuBarAttribute(mc.mainmenu, old, ATTR_CHECKED, 0);
	}
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

int CVICALLBACK ToggleFIDChan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int num = -1, i;
			for(i = 0; i < 8; i++) {
				if(control == dc.fchans[i]) {
					num = i;
					break;
				}
			}
			
			toggle_fid_chan(num);
			
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
			update_fid_chan_box();
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
			int num;
			GetCtrlVal(dc.fid, dc.fcring, &num);
			
			change_fid_offset(num);
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
			int num;
			GetCtrlVal(dc.fid, dc.fcring, &num);
			
			change_fid_gain(num);
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
			int num;
			GetCtrlVal(dc.fid, dc.fcring, &num);
			if(num < 0 || num >= 8)
				break;
			
			CmtGetLock(lock_uidc);
			GetCtrlVal(dc.fid, dc.fccol, &uidc.fcol[num]);
			CmtReleaseLock(lock_uidc);
			change_fid_chan_col(num);
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
			CmtGetLock(lock_uidc);
			GetCtrlIndex(panel, control, &uidc.schan); // Update this value
			CmtReleaseLock(lock_uidc);
			
			update_spec_fft_chan();
			break;
	}
	return 0;
}

int CVICALLBACK ToggleSpecChan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			int num = -1, i;
			for(i = 0; i < 8; i++) {
				if(control == dc.schans[i]) {
					num = i;
					break;
				}
			}
			
			toggle_spec_chan(num);
			
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
			update_spec_chan_box();
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
			// Update the UI file from the uidc values.
			int chan, order;
			GetCtrlVal(dc.spec, dc.scring, &chan);
			GetCtrlVal(dc.sphorder[1], dc.sphorder[0], &order);
			
			// If either of these is invalid, skip this bit.
			if(chan >= 0 && chan < 8 && order >= 0 && order < 3) {
				SetCtrlVal(dc.sphase[1], dc.sphase[0], uidc.sphase[chan][order]);	
			}
			
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
			int chan, order;
			
			GetCtrlVal(panel, control, &phase);
			GetCtrlVal(dc.sphorder[1], dc.sphorder[0], &order);
			GetCtrlVal(panel, dc.scring, &chan);
			
			change_phase(chan, phase, order);
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
			int num;
			GetCtrlVal(dc.spec, dc.scring, &num);
			
			change_spec_offset(num);
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
			int num;
			GetCtrlVal(dc.spec, dc.scring, &num);
			
			change_spec_gain(num);
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
			int num;
			GetCtrlVal(dc.spec, dc.scring, &num);
			if(num < 0 || num >= 8)
				break;
			
			CmtGetLock(lock_uidc);
			GetCtrlVal(dc.spec, dc.sccol, &uidc.scol[num]);
			CmtReleaseLock(lock_uidc);
			change_spec_chan_col(num);
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


//////////////////////////////////////////////////////
//                                      	        //
//            Analog Outputs Channels	 	  		//
//                                                  //
//////////////////////////////////////////////////////


int CVICALLBACK ChangeNumAOuts (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				change_num_aouts();
			break;
	}
	return 0;
}

int CVICALLBACK ModifyAOChanInstr (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeAODev (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				int num = int_in_array(pc.ainst, panel, uipc.max_anum);
				if(num >= 0)
					change_ao_device(num);
			break;
	}
	return 0;
}

int CVICALLBACK ChangeAOInstrChan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeAOTrigChan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeChanNumSteps (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeAOChanDim (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK ChangeAOutChan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				int spot = int_in_array(pc.ainst, panel, uipc.max_anum);
				
				if(spot >= 0)
					change_ao_chan(spot);
			break;
	}
	return 0;
}

int CVICALLBACK DeleteAOInstr (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				int i;
				for(i = 0; i < uipc.max_anum; i++) {
					if(panel == pc.ainst[i])
						break;
				}
				
				if(i < uipc.max_anum)
					delete_aout(i);
			break;
	}
	return 0;
}
