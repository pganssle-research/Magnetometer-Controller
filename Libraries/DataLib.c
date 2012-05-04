//////////////////////////////////////////////////////////////////////
// 																	//
//			  	 Data Saving and Display Library					//
//					Paul Ganssle, 08/01/2011						//
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

#include <analysis.h>
#include <userint.h>
#include "toolbox.h"
#include <ansi_c.h>
#include <utility.h>

#include <Version.h> 
#include <General.h>
#include <MCUserDefinedFunctions.h>

#include <UIControls.h>
#include <DataLib.h>
#include <DataLibPriv.h>
#include <PulseProgramLib.h>
#include <FileSave.h>
#include <PPConversion.h> // For now.

#include <ErrorDefs.h>
#include <ErrorLib.h>

//////////////////////////////////////////////////////////////
// 															//
//				Running Experiment Functions				//
// 															//
//////////////////////////////////////////////////////////////
int CVICALLBACK IdleAndGetData (void *functionData) {
	// Main function running in an asynchronous thread for data acquisition, etc.
	
	// Start by getting the current program.
	PPROGRAM *p = get_current_program_safe();
	
	// Run the experiment
	int rv = run_experiment(p);
	
	CmtExitThreadPoolThread(rv);	
	
	return 0;
}

void CVICALLBACK discardIdleAndGetData (int poolHandle, int functionID, unsigned int event, int value, void *callbackData)
{
	// Clear out the data stuff.
	CmtGetLock(lock_ce);
	CmtGetLock(lock_DAQ);
	
	clear_DAQ_task();
	
	if(ce.ao_vals != NULL) {
		free(ce.ao_vals);
		ce.ao_vals = NULL;
	}
	
	if(ce.ochanson != NULL) {
		free(ce.ochanson);
		ce.ochanson = NULL;
	}
	
	ce.ocnames = free_string_array(ce.ocnames, ce.nochans);
	ce.nochans = 0;
	
	// Clear out some current experiment stuff.
	if(ce.ilist != NULL) {
		free(ce.ilist);
		ce.ilist = NULL;
	}
	
	CmtReleaseLock(lock_DAQ);
	CmtReleaseLock(lock_ce);
	
	if(GetInitialized())  { pb_close_safe(0); }
	
	SetMenuBarAttribute(mc.mainmenu, mc.fload, ATTR_DIMMED, 0); // People can load data again.

}

int CVICALLBACK UpdateStatus (void *functiondata)
{
	// Checks the pb status 20 times a second and updates
	// the controls accordingly. Can't find a way to have any
	// kind of pushed notification for that.
	int rv;
	while(!GetQuitUpdateStatus())
	{
		rv = update_status_safe(1);
		if(rv < 0 || rv > 32 || rv & PB_STOPPED) 
			break;

		Delay(0.05);
	}
	
	CmtGetLock(lock_ce);
	ce.update_thread = -1;
	CmtReleaseLock(lock_ce);
	
	CmtExitThreadPoolThread(0);
	
	return 0;
}


int run_experiment(PPROGRAM *p) {
	// The main thread for running an experiment. This should be called from the main 
	// run-program type thread (IdleAndGetData).
	
	int scan = p->scan;
	int ev = 0, done = 0;
	int ce_locked = 0, daq_locked = 0;
	int ct = 0;
	int cont_mode = 1;		// If cont_mode is on, it is checked in the first iteration of the loop anyway.
							// This will be more relevant when cont_run in ND is implemented
	
	double *data = NULL, *avg_data = NULL;
	
	SetQuitIdle(0);
	SetDoubleQuitIdle(0);
	SetQuitUpdateStatus(0);
	
	// Update some controls for the running experiment.
	SetMenuBarAttribute(mc.mainmenu, mc.fload, ATTR_DIMMED, 1); // Can't have people loading new data now.
	
	SetCtrlVal(mc.mainstatus[1], mc.mainstatus[0], 1);	// Experiment is running!
	SetCtrlAttribute(mc.mainstatus[1], mc.mainstatus[0], ATTR_LABEL_TEXT, "Running");
	SetRunning(1);
	
	// Initialize ce
	CmtGetLock(lock_ce);
	ce_locked = 1;
	ce.cind = 0;
	ce.p = p;
	
/*	if(ce.cstep != NULL) {			// In case this has been allocated already.
		free(ce.cstep);
		ce.cstep = NULL;
	}  */
	
	if(p->varied) {
//		ce.cstep = malloc(sizeof(int)*(p->nCycles+p->nDims));
//		get_cstep(0, ce.cstep, p->maxsteps, p->nCycles+p->nDims); 
	}
	
	if(scan) {
		if(setup_DAQ_task_safe(1, 1, 0) != 0) {
			ev = 1;
			goto error;
		}
		
		// Initialize the TDM file
		//if(ev = initialize_tdm_safe(0, 1)) { goto error; }
		
		update_experiment_nav();
	}
	
	if(p->nAout) {
		if(setup_DAQ_aouts_safe(1, 1, 0) != 0) {
			ev = 2;
			goto error;
		}
	}
	
	int aouts = ce.nochans?1:0;
	ce.cind = -1; // Initialize to -1 so that prepare_next_step will work.
	
	CmtReleaseLock(lock_ce);
	ce_locked = 0;
	
	// Make sure the pulseblaster is initiated.
	if(!GetInitialized() && p->use_pb) {
		ev = pb_init_safe(1);
		if(ev < 0) { goto error; }
	}

	// For the moment, continuous acquisition in ND is not implemented
	// In the future, it will just repeat whatever experiment you tell it to
	// indefinitely.
	GetCtrlVal(pc.rc[1], pc.rc[0], &cont_mode);
	if(cont_mode && p->varied) {  SetCtrlVal(pc.rc[1], pc.rc[0], 0); }
	
	// Main loop to keep track of where we're at in the experiment, etc.
	while(!GetQuitIdle() && ct <= p->nt) {
		if(cont_mode) { GetCtrlVal(pc.rc[1], pc.rc[0], &cont_mode); }
			
		if(done = prepare_next_step_safe(p) != 0)
		{
			/* if(cont_mode && done == 1) {
				CmtGetLock(lock_ce);		// No possibility of break/goto, don't need to set ce_locked
				ct--;
				p->nt++;
				done = prepare_next_step(p);
				CmtReleaseLock(lock_ce);
			}  */ // Fix this a bit later
			
			if(done < 0) { ev = done; }
		}
		
		CmtGetLock(lock_ce);
		ce_locked = 1;
		
		ct = get_ct(&ce);
		if(ct < 0) { ev = ct; goto error; }
		
		CmtReleaseLock(lock_ce);
		ce_locked = 0;
		
	
		if(!done) {
			// This is the part where data acquisition takes place.
			
			if(scan ||  aouts) {
				// Get data from device
				CmtGetLock(lock_DAQ);
				daq_locked = 1;
				
				if(scan) {
					if(ev = DAQmxStartTask(ce.cTask)) { goto error; }			// Tasks should be started BEFORE the program is run.
					if(ev = DAQmxStartTask(ce.aTask)) { goto error; }
				}
				
				if(aouts) {
					// Since this task is just a change in the DC output levels,
					// we can currently just start it, update the values, then stop
					// it. When triggering and function outputs are added, we'll have 
					// to move the DAQmxStopTask call to later.
					
					if(ev = DAQmxStartTask(ce.oTask)) { goto error; }
					
					if(ev = update_DAQ_aouts_safe(!daq_locked, !ce_locked)) { goto error; }
					
					if(ev = DAQmxStopTask(ce.oTask)) { goto error; }
				}
				
				CmtReleaseLock(lock_DAQ);
				daq_locked = 0;
			}
			//  Program and start the pulseblaster.
			if(p->use_pb) {
				if(program_pulses_safe(ce.ilist, ce.ninst, 1) < 0) { goto error; }
				if(pb_start_safe(1) < 0) { goto error; }
			}
			
			// Check the status once now.
			update_status_safe(0);
			
			// Start checking for updates in another thread.
			if(p->use_pb) {
				CmtGetLock(lock_ce);
				ce_locked = 1;
				if(ce.update_thread == -1)
					CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE, UpdateStatus, NULL, &ce.update_thread);
				else {
					int threadstat;
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE, ce.update_thread, ATTR_TP_FUNCTION_EXECUTION_STATUS, &threadstat);
					if(threadstat > 1) 
						CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE, ce.update_thread, 0);
				}
				CmtReleaseLock(lock_ce);
				ce_locked = 0;
			
				// Chill out and wait for the next step.
				while( !GetQuitIdle() && !(GetStatus()&PB_STOPPED)) {	// Wait for things to finish.
					Delay(0.01);
				}
			}
			
			if(scan) {
				data = get_data(ce.aTask, ce.p->np, ce.nchan, ce.p->nt, ce.p->sr, &ev, 1);
			
				CmtGetLock(lock_DAQ);
				daq_locked = 1;
			
				// Stop the tasks qhen you are done with them. They can be started again
				// later. Clear them when you are really done.
				
				if(ev = DAQmxStopTask(ce.cTask)) { if(ev != 200010) { goto error; } }
				if(ev = DAQmxStopTask(ce.aTask)) { if(ev != 200010) { goto error; } }
				
				CmtReleaseLock(lock_DAQ);
				daq_locked = 0;
				
				if(ev)  { goto error; }
			
				// Update the navigation controls.
				if(ce.p->nDims) { 
					update_experiment_nav(); 
				} else {	
					update_transients(); 
				}

				// Only request the average data if you need it.
				if(avg_data != NULL) {
					free(avg_data);
					avg_data = NULL;
				}
				
				if(ce.p->nt > 1) {
					if(ev = update_avg_data_mcd(MCD_AVGCACHE_FNAME, &avg_data, p, data, ce.cind, ce.nchan, ce.hash)) { goto error; }
				}
				
				if(ce.cind == 0) {
					if(ev = initialize_mcd_safe(ce.path, ce.bfname, ce.num, p, ce.nchan, ce.tstart, ce.hash)) { goto error; }	
				}
				
				if(ev = save_data_mcd(ce.path, p, data, ce.cind, ce.nchan, ce.tdone)) { goto error; }
				
				CmtGetLock(lock_uidc);
				
				if(uidc.disp_update < 2) {	// We're plotting stuff for 0 (Avg) or 1 (Latest)
					CmtGetLock(lock_ce);
					if(uidc.disp_update && ct > 1) { 
						plot_data(avg_data, p->np, p->sr, ce.nchan);
					} else {
						plot_data(data, p->np, p->sr, ce.nchan);
					}
					
					// Update the nav controls now.
					for(int i = 0; i < 2; i++) {
						set_nav_from_lind(ce.cind, dc.cloc[i], !uidc.disp_update);
					}
					
					CmtReleaseLock(lock_ce);
				}
				
				CmtReleaseLock(lock_uidc);
			
				// Free memory
				if(avg_data != NULL) {
					free(avg_data);
					avg_data = NULL;
				}
			
				free(data);
				data = NULL;
			}
		}
	
	}
	
	error:
	
	if(ce_locked) {
		CmtReleaseLock(lock_ce);	
	}
	
	SetCtrlVal(mc.mainstatus[1], mc.mainstatus[0], 0);	// Experiment is done!
	SetCtrlAttribute(mc.mainstatus[1], mc.mainstatus[0], ATTR_LABEL_TEXT, "Stopped");
	SetRunning(0);
	
	if(data != NULL)  { free(data); }
	if(avg_data != NULL) { free(avg_data); }
	
	if(ev)  {
		if(is_mc_error(ev)) {  
			display_error(ev); 
		} else {
			uInt32 es_size = DAQmxGetExtendedErrorInfo(NULL, 0);
			
			char *es = malloc((es_size > 0)?(es_size+1):100);
			
			if(es_size > 0) {
				DAQmxGetExtendedErrorInfo(es, es_size);
			} else {
				sprintf(es, "Unknown DAQmx Error, code was %d", ev);
			}
			
			 MessagePopup("DAQmx Error", es);
			free(es);
		}
	}
	
	if(daq_locked) {
		CmtReleaseLock(lock_DAQ);
	}

	return ev;
}


double *get_data(TaskHandle aTask, int np, int nc, int nt, double sr, int *error, int safe) {
	// This is the main function dealing with getting data from the DAQ.
	// ce must be initialized properly and the DAQ task needs to have been started.
	//
	// Errors:
	// 1: Invalid inputs (transients or number of points)
	// 2: Data could not be read
	// 3: Data missing after read
	// 4: Acquisition interrupted by quit command.
	
	
	int32 nsread;
	int j, err = 0;
	float64 *out = NULL;
	char *errmess = NULL;
	
	// These are malformed program conditions
	if(np < 1) {
		err = 1;
		goto error;
	}
	
	// First we need to wait for the DAQ to finish doing its thing. This could take a while, so the function
	// will just loop around waiting for this to happen. That's one big reason why this whole thing should
	// be in a single idle-and-get-data thread. Eventually, we'll calculate the expected amount of time this
	// should take and add a few seconds to it, so that this doesn't time out before it should.
	unsigned int bc;
	int i = 0, locked = 0;
	double time = Timer();
	errmess = malloc(400);
	
	while(!GetDoubleQuitIdle()) {
		if(safe) { 
			CmtGetLock(lock_DAQ); 
			locked = 1;
		}
		
		if(err = DAQmxGetReadAttribute(aTask, DAQmx_Read_AvailSampPerChan, &bc)) { goto error; }

		if(bc >= np) 	// It's ready to be read out
			break;
		
		if(safe) { 	// Don't release it until we've read them! 
			CmtReleaseLock(lock_DAQ); 
			locked = 0;
		}  
		
		// Timeout condition -> May want to add a setting for this
		/*
		if(i == 20) {
			sprintf(errmess, "The readout task is about to time out. It has been occuring for %lf seconds and has read %d samples. Allow it to wait another 2 seconds?", Timer() - time, bc);
			j = ConfirmPopup("Timeout Warning", errmess);
			
			if(j)
				i -= 40;
			else {
				err = 2;
				goto error;
			}
		}
		
		i++;
		*/
		
		Delay(0.1);
	}
	
	if(GetDoubleQuitIdle()) {
		err = 4;
		goto error;	
	}
	
	free(errmess);
	errmess = NULL;
	
	// Now we know it's ready to go, so we should read out the values.
	out = malloc(sizeof(float64)*np*nc);
	if(err = DAQmxReadAnalogF64(aTask, -1, 0.0, DAQmx_Val_GroupByChannel, out, np*nc, &nsread, NULL)) { goto error; }
	
	if(nsread != np) { 
		err = 3;
		goto error;
	}
	
	error:
	
	if(safe && locked) { CmtReleaseLock(lock_DAQ); }
	
	if(errmess != NULL) { free(errmess); }
	
	if(err && out != NULL) {
		free(out);
		out = NULL;
	}
		
	*error = err;
	return (double*)out;
}

int prepare_next_step (PPROGRAM *p) {
	// Updates the ce variable for the next step based on the loaded PPROGRAM
	// Returns 0 if everything's good, returns 1 if we've finished the experiment
	// Returns a negative number on error.
	// ct/nt counters are 1-based indices.

	// It's all the same if you aren't using varied instructions
	ce.cind++;
	int ct = get_ct(&ce);
	
	if(!p->varied) {
		if(ct < p->nt) {	// Transients are internally on a zero-based index.
			if(p->use_pb) {
				if(ce.ilist == NULL) { 
					ce.ilist = generate_instructions(p, ce.cind, &ce.ninst);
				}
			}
			return 0;
		} else {
			return 1;	
		}
	}
	
	// Free the old ilist
	if(ce.ilist != NULL) {
		free(ce.ilist);
		ce.ilist = NULL;
	}
	
	int end = get_cstep(ce.cind, ce.cstep, ce.steps, ce.steps_size);
	
	if(end != 1) {
		return 1;	
	} else if(p->skip) {
		if(p->skip_locs[ce.cind]) {
			return prepare_next_step(p);
		}	
	}
	
	if(p->use_pb) {
		ce.ilist = generate_instructions(p, ce.cind, &ce.ninst); // Generate the next instructions list.
	
		if(ce.ilist == NULL) { return -2; }
	}
	
	return 0;
}

int setup_cexp(CEXP *cexp) {
	// Initializes the CEXP for proper linear indexing
	// If filename is not null, this also sets up the hash.
	if(cexp == NULL) { return MCEX_ERR_NOCEXP; }
	if(cexp->p == NULL) { return MCEX_ERR_NOPROG; }

	int rv = 0; 
	PPROGRAM *p = cexp->p;
	CEXP c = *cexp;
	
	c.cind = 0;
	
	if(c.cstep != NULL) {
		free(c.cstep);
		c.cstep = NULL;
	}
	
	if(c.steps != NULL) {
		free(c.steps);
		c.steps = NULL;
	}
	
	c.steps_size = 0;
	if(p->varied) { 
		c.cstep = calloc(p->steps_size, sizeof(unsigned int));
	}
	
	add_hash_to_cexp(cexp);
	
	*cexp = c;
	return rv;
}

int add_hash_to_cexp(CEXP *cexp) {
	if(cexp->fname != NULL) {
		cexp->hash = basic_string_hash(cexp->fname);	
	} else {
		return MCEX_ERR_NOFNAME;	
	}
	
	return 0;	
}

int get_ct(CEXP *cexp) {
	CEXP c = *cexp;
	if(!c.p->varied) {
		return c.cind;
	} else if (c.cstep == NULL || c.steps_size == 0) {
		return MCEX_ERR_NOSTEPS;
	}
	
	int ct = 0;
	
	unsigned int tmode, npc, nd;
	tmode = c.p->tmode;
	npc = c.p->nCycles;
	nd = c.p->nDims;
	
	if(tmode == MC_TMODE_ID || (tmode == MC_TMODE_PC && npc == 0)) {
		ct = c.cstep[c.steps_size-1];
	} else if (tmode == MC_TMODE_TF) {
		ct = c.cstep[0];	
	} else if (tmode == MC_TMODE_PC) {
		ct = get_lindex(&(c.cstep[nd]), &(c.steps[nd]), npc+1);
	} else {
		return MCEX_ERR_INVALIDTMODE;
	}
	
	return ct;
}

int prepare_next_step_safe(PPROGRAM *p) {
	CmtGetLock(lock_ce);
	int rv = prepare_next_step(p);
	CmtReleaseLock(lock_ce);
	
	return rv;
}

//////////////////////////////////////////////////////////////
// 															//
//				File Read/Write Operations					//
// 															//
//////////////////////////////////////////////////////////////
/********************* Data Operations ******************/

int initialize_tdm() {
	// This is the first thing you should call if you are trying to save data to a TDM
	// file. It will put all the metadata and set everything up, returning what you need.
	// This assumes that ce is properly initialized.
	//
	// Returns negative on error:
	// -1: Filename or program is null.
	// -2: No active channels.
	// All other values are from DDC files.
	
	PPROGRAM *p = ce.p;
	char *filename = ce.path;
	
	if(filename == NULL || p == NULL) { return -1; }
	
	if(ce.nchan < 1) { return -2; }

	int i, len, np, nd, ncyc, varied, lindex;
	int rv = 0;
	char *name = NULL, *desc = NULL, *vname = NULL;
	char *csstr = NULL;
	
	varied = p->varied;
	nd = p->nDims;
	ncyc = p->nCycles;
	np = p->np;
	
	DDCFileHandle data_file = NULL;
	DDCChannelGroupHandle mcg = NULL, acg = NULL;
	DDCChannelHandle *chans = NULL, *achans = NULL;

	// This is the case where we are first creating the file - these are stored in the ce.
	name = malloc(strlen(ce.fname)+1);
	desc = malloc(strlen(ce.desc)+1);
	
	strcpy(name, ce.fname);
	name[strlen(name)-4] = '\0';
	strcpy(desc, ce.desc);
	
	// The version name.
	vname = MC_VERSION_STRING;
	
	if(rv = DDC_CreateFile(filename, "TDM", name, desc, name, vname, &data_file)) { goto error; }
	if(rv = DDC_AddChannelGroup(data_file, MCTD_MAINDATA, "Main Data Group", &mcg)) { goto error; }
	if(p->nt > 1) {
		if(rv = DDC_AddChannelGroup(data_file, MCTD_AVGDATA, "", &acg)) { goto error; }
	}
	
	free(desc);
	free(name);
	vname = desc = name = NULL;
	
	// Current time
	ce.tstart = ce.tdone = time(NULL);

	// Now since it's the first time, we need to set up the attributes, metadata and program.
	if(rv = DDC_CreateChannelGroupProperty(mcg, MCTD_NP, DDC_Int32, np)) { goto error; } 
	if(rv = DDC_CreateChannelGroupProperty(mcg, MCTD_SR, DDC_Double, p->sr)) { goto error; }
	if(rv = DDC_CreateChannelGroupProperty(mcg, MCTD_NT, DDC_Int32, p->nt)) { goto error; }
	if(rv = DDC_CreateChannelGroupProperty(mcg, MCTD_TIMESTART, DDC_Timestamp, ce.tstart)) { goto error; }
	if(rv = DDC_CreateChannelGroupProperty(mcg, MCTD_NDIM, DDC_UInt8, nd+1)) { goto error; }
	if(rv = DDC_CreateChannelGroupProperty(mcg, MCTD_NCYCS, DDC_UInt8, ncyc)) { goto error; }
	if(rv = DDC_CreateChannelGroupProperty(mcg, MCTD_NCHANS, DDC_UInt8, ce.nchan)) { goto error; }
	if(rv = DDC_CreateChannelGroupProperty(mcg, MCTD_PTMODE, DDC_Int32, p->tmode)) { goto error; }
	if(rv = DDC_CreateChannelGroupProperty(mcg, MCTD_CSTEP, DDC_Int32, -1)) { goto error; }
	if(rv = DDC_CreateChannelGroupProperty(mcg, MCTD_CSTEPSTR, DDC_String, "")) { goto error; }
	if(rv = DDC_CreateChannelGroupProperty(mcg, MCTD_TIMEDONE, DDC_Timestamp, ce.tdone)) { goto error; }
	
	// Now save the program.
	DDCChannelGroupHandle pcg = NULL;
	DDC_AddChannelGroup(data_file, MCTD_PGNAME, "", &pcg);
	
	save_programDDC(pcg, p);

	// Create the data channels
	chans = malloc(sizeof(DDCChannelHandle)*ce.nchan);
	achans = malloc(sizeof(DDCChannelHandle)*ce.nchan);
	for(i = 0; i < ce.nchan; i++) {
		if(rv = DDC_AddChannel(mcg, DDC_Double, ce.icnames[i], "", "V", &chans[i])) { goto error; }
		if(p->nt > 1) {
			if(rv = DDC_AddChannel(acg, DDC_Double, ce.icnames[i], "", "V", &achans[i])) { goto error; }
		}
	}
	
	// Save the file
	DDC_SaveFile(data_file);
	
	error:

	if(data_file != NULL) { DDC_CloseFile(data_file); }
	
	if(name != NULL) { free(name); }
	if(desc != NULL) { free(desc); }
	if(chans != NULL) { free(chans); }
	if(achans != NULL) { free(achans); }
	
	return rv;
}

int initialize_tdm_safe(int CE_lock, int TDM_lock) {
	if(CE_lock) { CmtGetLock(lock_ce); }
	if(TDM_lock) { CmtGetLock(lock_tdm); }
	
	int rv = initialize_tdm();
	
	if(TDM_lock) { CmtReleaseLock(lock_tdm); }
	if(CE_lock) { CmtReleaseLock(lock_ce); }
	
	return rv;
}

char *make_cstep_str(PPROGRAM *p, int cind, int avg, int *ev) {
	// Create a cstep string for indexing mcd files. 
	// If avg evaluates true, transients will be collapsed.
	// Output must be freed.
	
	int i, rv = 0;
	int *steps = NULL;
	char *out = NULL;
	char *title_format = NULL;
	char *buff = NULL;
	
	
	if(p == NULL) { rv = MCD_ERR_NOPROG; goto error; }
	if(cind < 0) { rv = MCD_ERR_BADCIND; goto error; }
	
	// Create the data array.
	int max = 0;
	for(i = 0; i < p->steps_size; i++) {
		if(p->steps[i] > max) {
			max = p->steps[i];
		}
	}
	
	int ndigits = (max > 0)?(((int)log10(max))+1):1;	// Number of digits per step
	
	int nsteps = 1;
	
	if(avg) {
		if(!p->nDims) {
			steps = malloc(sizeof(int)*1);
			steps[0] = 0;
			ndigits = 1;
		} else {
			nsteps = p->nDims;
			steps = malloc(p->nDims*sizeof(unsigned int));
			get_dim_step(p, cind, steps);
		}
	} else {
		nsteps = p->steps_size;
		steps = malloc(sizeof(unsigned int) * p->steps_size);
		memcpy(steps, p->steps, p->steps_size * sizeof(unsigned int));
	}
	
	out = malloc(nsteps*(ndigits+1)+2);
	
	title_format = malloc(6);
	buff = malloc(ndigits+2);
	
	sprintf(title_format, "%%0%dd,", ndigits);
	strcpy(out, "[");
	
	for(i = 1; i < nsteps; i++) {		   
		sprintf(buff, title_format, steps[i]);
		strcat(out, buff);
	}
	out[strlen(out)-1] = ']';

	error:
	if(rv != 0 && out != NULL) { 
		free(out);
		out = NULL;
	}
	
	if(buff != NULL) { free(buff); }
	if(title_format != NULL) { free(title_format); }
	if(steps != NULL) { free(steps); }
	
	*ev = rv;
	return out;
}

int initialize_mcd_safe(char *fname, char *basename, unsigned int num, PPROGRAM *p, unsigned int nc, time_t start, int64 hash) {
	CmtGetLock(lock_uidc);
	int rv = initialize_mcd(fname, basename, num, p, nc, start, hash);
	CmtReleaseLock(lock_uidc);
	
	return rv;
}

int initialize_mcd(char *fname, char *basename, unsigned int num, PPROGRAM *p, unsigned int nc, time_t tstart, int64 hash) {
	// Creates the proper headers for the data file.
	if(fname == NULL) { return MCD_ERR_NOFILENAME; }
	if(p == NULL) { return MCD_ERR_NOPROG; }
	
	int i, rv = 0;
	FILE *f = NULL;
	char *ext = NULL, *fbuff = NULL, *ds = NULL, *buff = NULL;
	fsave *fs = NULL, header = null_fs();
	float *phases = NULL;
	unsigned char *fchanson = NULL, *schanson = NULL;
	
	int fs_size = 0;

	// Enforce valid extension
	ext = get_extension(fname);
	if(ext != NULL || strcmp(ext, MCD_EXTENSION) != 0) {
		int elen = (ext != NULL)?strlen(ext):0;
		elen = strlen(fname)-elen;
		
		fbuff = calloc(elen+strlen(MCD_EXTENSION)+2, 1);
		
		strncpy(fbuff, fname, elen);
		strcat(fbuff, ".");
		strcat(fbuff, MCD_EXTENSION);
	} else {
		fbuff = malloc(strlen(fname)+1);
		strcpy(fbuff, fname);
	}
	
	fname = fbuff;		// C is pass-by-value, so this is just a pointer
	
	// Create the file - this WILL overwrite if it exists.
	f = fopen(fname, "wb+");
	if(f == NULL) { rv = MCD_ERR_NOFILE; goto error; }
	
	//////////////////////////
	//						//
	//		Data Header     //
	//						//
	//////////////////////////
	fs_size = MCD_DATANUM;
	fs = calloc(MCD_DATANUM, sizeof(fsave));
	int cf = 0;
	
	// File name
	fs[cf] = make_fs(MCD_FILENAME);
	if(rv = put_fs(&(fs[cf]), fname, FS_CHAR, strlen(fname)+1)) { goto error; }
	
	// Experiment name
	fs[++cf] = make_fs(MCD_EXPNAME);
	if(rv = put_fs(&(fs[cf]), basename, FS_CHAR, strlen(basename)+1)) { goto error; }
	
	// Experiment number
	fs[++cf] = make_fs(MCD_EXPNUM);
	if(rv = put_fs(&(fs[cf]), &num, FS_UINT, 1)) { goto error; }
	
	// Hash
	fs[++cf] = make_fs(MCD_HASH);
	if(rv = put_fs(&(fs[cf]), &ce.hash, FS_UINT64, 1)) { goto error; }
	
	// Number of channels
	fs[++cf] = make_fs(MCD_NCHANS);
	if(rv = put_fs(&(fs[cf]), &nc, FS_UINT, 1)) { goto error; }
	
	// Date stamp
	time_t ts;			// Get the time first
	time(&ts);
	
	struct tm *tptr;	// Get local time
	tptr = localtime(&ts);
	
	ds = malloc(MCD_DATESTAMP_MAXSIZE+1);	// Get the string
	strftime(ds, MCD_DATESTAMP_MAXSIZE+1, MCD_DATE_FORMAT, tptr);
	
	fs[++cf] = make_fs(MCD_DATESTAMP);
	if(rv = put_fs(&(fs[cf]), ds, FS_CHAR, strlen(ds)+1)) { goto error; }
	
	free(ds);
	ds = NULL;
	
	// Timestamps - both the same for now
	ds = calloc(MCD_TIMESTAMP_MAXSIZE+1, 1);
	strftime(ds, MCD_TIMESTAMP_MAXSIZE+1, MCD_TIME_FORMAT, tptr);
	
	fs[++cf] = make_fs(MCD_TIMESTART);	// Time started
	if(rv = put_fs(&(fs[cf]), ds, FS_CHAR, MCD_TIMESTAMP_MAXSIZE+1)) { goto error; }
	
	fs[++cf] = make_fs(MCD_TIMEDONE);	// Time completed
	if(rv = put_fs(&(fs[cf]), ds, FS_CHAR, MCD_TIMESTAMP_MAXSIZE+1)) { goto error; }
	
	int epoch = (int)ts;
	fs[++cf] = make_fs(MCD_TSTART);
	if(rv = put_fs(&(fs[cf]), &epoch, FS_UINT, 1)) { goto error; }
	
	fs[++cf] = make_fs(MCD_TDONE);
	if(rv = put_fs(&(fs[cf]), &epoch, FS_UINT, 1)) { goto error; }
	
	// Current step
	unsigned int cind = 0;
	fs[++cf] = make_fs(MCD_CIND);
	if(rv = put_fs(&(fs[cf]), &cind, FS_UINT, 1)) { goto error; }
	
	// Max steps
	fs[++cf] = make_fs(MCD_MAXSTEPS);
	int one = 1;
	if(p->nDims + p->nCycles > 1) {
		if(rv = put_fs(&(fs[cf]), p->maxsteps, FS_INT, p->nDims + p->nCycles)) { goto error; }
	} else {
		if(rv = put_fs(&(fs[cf]), &one, FS_INT, 1)) { goto error; }	
	}
	
	// Generate the container
	header = make_fs(MCD_DATAHEADER);
	if(rv = put_fs_container(&header, fs, cf)) { goto error; }
	
	// Print the container.
	if(rv = fwrite_fs(f, &header)) { goto error; }
	
	fs = free_fsave_array(fs, cf+1);
	header = free_fsave(&header);

	//////////////////////////////
	//							//
	//		Display Header     	//
	//							//
	//////////////////////////////
	fs = calloc(MCD_DISPNUM, sizeof(fsave));
	fs_size = MCD_DISPNUM;
	cf = 0;
	
	// Polynomial fit on/off - boolean UCHAR
	fs[cf] = make_fs(MCD_POLYFITON);
	if(rv = put_fs(&(fs[cf]), &(uidc.polyon), FS_UCHAR, 1)) { goto error; }
	
	// Polynomial fit order - UINT.
	fs[++cf] = make_fs(MCD_POLYFITORDER);
	if(rv = put_fs(&(fs[cf]), &(uidc.polyord), FS_UINT, 1)) { goto error; }
	
	// FFT Phases - Linear indexed array of the 3 phase components.
	unsigned int n_phase = 3*nc;
	phases = calloc(n_phase, sizeof(float)) ;
	for(int j = 0; j < nc; j++) {
		for(i = 0; i < 3; i++) {
			phases[3*j + i] = uidc.sphase[j][i];	
		}
	}
	
	fs[++cf] = make_fs(MCD_PHASE);
	if(rv = put_fs(&(fs[cf]), phases, FS_FLOAT, n_phase)) { goto error; }

	// Currently displayed FFT channel.
	fs[++cf] = make_fs(MCD_FFTCHAN);
	if(rv = put_fs(&(fs[cf]), &uidc.schan, FS_INT, 1)) { goto error; }

	// Gains and offsets - FLOAT x nc
	// Spectrum gains
	fs[++cf] = make_fs(MCD_SGAINS);
	if(rv = put_fs(&(fs[cf]), uidc.sgain, FS_FLOAT, nc)) { goto error; }
	
	// Spectrum offsets
	fs[++cf] = make_fs(MCD_SOFFS);
	if(rv = put_fs(&(fs[cf]), uidc.soff, FS_FLOAT, nc)) { goto error; }
	
	// FID Gains
	fs[++cf] = make_fs(MCD_FGAINS);
	if(rv = put_fs(&(fs[cf]), uidc.fgain, FS_FLOAT, nc)) { goto error; }
	
	// FID Offsets
	fs[++cf] = make_fs(MCD_FOFFS);
	if(rv = put_fs(&(fs[cf]), uidc.foff, FS_FLOAT, nc)) { goto error; }
	
	// Channels on
	fchanson = calloc(nc, sizeof(unsigned char));
	schanson = calloc(nc, sizeof(unsigned char));
	
	for(i = 0; i < nc; i++) {
		if(uidc.fchans[i]) {
			fchanson[i] = 1;
		}
		
		if(uidc.schans[i]) {
			schanson[i] = 1;	
		}
	}
	
	// Spectrum Channels on
	fs[++cf] = make_fs(MCD_SCHANSON);
	if(rv = put_fs(&(fs[cf]), schanson, FS_UCHAR, nc)) { goto error; }
	
	// FID Channels on
	fs[++cf] = make_fs(MCD_FCHANSON);
	if(rv = put_fs(&(fs[cf]), fchanson, FS_UCHAR, nc)) { goto error; }
	
	
	// Put it all in the container, then write the container to file.
	header = make_fs(MCD_DISPHEADER);
	if(rv = put_fs_container(&header, fs, cf)) { goto error; }
	
	if(rv = fwrite_fs(f, &header)) { goto error; }
	
	header = free_fsave(&header);
	fs = free_fsave_array(fs, fs_size);
	fs_size = 0;
	
	
	//////////////////////////////
	//							//
	//			Program     	//
	//							//
	//////////////////////////////
	
	rv = save_pprogram(p, f);
	if(rv != 0) { goto error; }		// That was anti-climactic.
	
	error:
	if(ext != NULL) { free(ext); }
	if(buff != NULL) { free(buff); }
	if(fbuff != NULL) { free(fbuff); }
	if(ds != NULL) { free(ds); }
	
	if(phases != NULL) { free(phases); }
	if(fchanson != NULL) { free(fchanson); }
	if(schanson != NULL) { free(schanson); }
	
	free_fsave(&header);
	if(fs != NULL) { free_fsave_array(fs, fs_size); }
	
	if(f != NULL) { fclose(f); }
	
	return rv;
}

int update_avg_data_mcd(char *fname, double **avg_data, PPROGRAM *p, double *data, int cind, int nc, int64 hash) {
	// This is basically for cacheing average data in a local file.
	// First call will create a file (overwrites when cind == 0)
	// Subsequent calls will update it with the new data.
	
	int i, rv = 0;

	if(fname == NULL) { return MCD_ERR_NOFILENAME; }
	if(data == NULL) { return MCD_ERR_NODATA; }
	if(p == NULL) { return MCD_ERR_NOPROG; }
	
	FILE *f = NULL;
	char *step_title = NULL, *buff = NULL;
	fsave ag = null_fs(), dg = null_fs(), chash = null_fs();
	
	if(!file_exists(fname) && cind > 0) {
		return MCD_ERR_NOFILE;
	}
	
	if(cind == 0) {
		f = fopen(fname, "wb+");
		
		// Write the hash to make sure it's the same thing.
		chash = make_fs(MCD_HASH);
		if(rv = put_fs(&chash, &hash, FS_INT, 1)) { goto error; }
		if(rv = fwrite_fs(f, &chash)) { goto error; }
	} else {
		f = fopen(fname, "rb+");
		
		// This file must open with the hash.
		long hoff = find_fsave_in_file(f, MCD_HASH, MCF_EOF);
		
		fseek(f, hoff, SEEK_SET);
		
		chash = read_fsave_from_file(f, &rv);
		if(rv != 0) { goto error; }
		
		if(*(chash.val.i) != hash) {
			rv = MCD_ERR_NOAVGDATA;		// Not the right file!
			goto error;
		}
	}
	
	if(f == NULL) { rv = MCD_ERR_NOFILE; goto error; }
	
	if(p->varied) {
		step_title = make_cstep_str(p, cind, 1, &rv);
	} else {
		step_title = malloc(strlen("[0]")+1);
		strcpy(step_title, "[0]");
	}
		
	if(rv != 0) { goto error; }
	
	int stlen;
	if(step_title != NULL) {
		stlen = strlen(step_title);	
	} else {
		rv = MCD_ERR_NOAVGDATA;
		goto error;
	}
	
	unsigned int data_size = p->np * nc;
	int ct = get_transient(p, cind);
	if(ct < 0) { rv = ct; goto error; }
	
	if(cind == 0) {
		// Create the AVGDATA group
		dg = make_fs(step_title);
		if(rv = put_fs(&dg, data, FS_DOUBLE, data_size)) { goto error; }
		
		ag = make_fs(MCD_AVGDATA);
		if(rv = put_fs_container(&ag, &dg, 1)) { goto error; }
		
		if(rv = fwrite_fs(f, &ag)) { goto error; }
	} else {
		// In this case we have to find the AVGDATA group
		// and either append the data or replace it.
		rewind(f);
		long ag_pos = find_fsave_in_file(f, MCD_AVGDATA, MCF_EOF);
		if(ag_pos < 0) {
			rv = ag_pos;
			goto error;
		}
		
		// This will set the file to be pointing to the contents.
		fseek(f, ag_pos, SEEK_SET);
		if(rv = get_fs_header_from_file(f, &ag)) { goto error; }
		
		
		// See if the relevant group already exists.
		long ad_pos = find_fsave_in_file(f, step_title, ag.size);
		if(ad_pos == MCF_ERR_FSAVE_NOT_FOUND) {
			// This means we need to insert the group at the end.
			dg = make_fs(step_title);
			if(rv = put_fs(&dg, data, FS_DOUBLE, data_size));
			
			// Add this to MCD_AVGDATA
			fseek(f, ag_pos, SEEK_SET);
			if(rv = fadd_fs_to_container(f, &dg)) { goto error; }
		} else if(ad_pos < 0) {
			rv = ad_pos;
			goto error;
		} else {
			fseek(f, ad_pos, SEEK_SET);
			
			dg = read_fsave_from_file(f, &rv);
			if(rv != 0) { goto error; }
			
			unsigned int data_size = dg.size/sizeof(double);
			
			// Perform the averaging now.
			for(i = 0; i < data_size; i++) {
				dg.val.d[i] = (dg.val.d[i]*ct + data[i])/(ct+1);
			}
			
			if(avg_data != NULL && data_size > 0) {
				*avg_data = malloc(data_size*sizeof(double));
				memcpy(*avg_data, dg.val.d, data_size*sizeof(double));
			}
			
			// Overwrite the data.
			unsigned int off = get_fs_header_size_from_ns(dg.ns);
			
			fseek(f, ad_pos, SEEK_SET);
			if(rv = overwrite_fsave_in_file(f, &dg)) { goto error; }
		}
	}
	
	error:
	
	if(f != NULL) { fclose(f); }
	if(step_title != NULL) { free(step_title); }
	if(buff != NULL) { free(buff); }
	
	free_fsave(&ag);
	free_fsave(&dg);
	free_fsave(&chash);
	
	return rv;
}


int save_data_mcd(char *fname, PPROGRAM *p, double *data, int cind, int nc, time_t done) {
	// Save data to existing file, with the header already written
	// 
	// New specification:
	// Files will contain ONLY parameters and main data files, no averages.
	// The headers are written ahead of tiem and save_data_mcd basically appends data to the
	// existing group, in a linear-indexed fashion. Based on the nature of the serialization
	// format, even for very long files it should be fairly quick to navigate between points.
	//
	// Since average data will be updated frequently, we'll generate "current buffer" files 
	// which contain the running average for the currently loaded file.
	//
	// The reasoning behind this is that if we have one file, our only options will be to 
	// pre-allocate all arrays or to risk frequently changing the size of TWO arrays. Since 
	// there's no good way to "insert" data into a file without simply buffering the remainder
	// of the file and writing it afterwards, this seems excessive.
	
	if(fname == NULL) { return MCD_ERR_NOFILENAME; }
	if(data == NULL) { return MCD_ERR_NODATA; }
	if(p == NULL) { return MCD_ERR_NOPROG; }
	
	if(!file_exists(fname)) {
		return MCD_ERR_NOFILE;	
	}
	
	int i, rv = 0;
	
	FILE *f = NULL;
	
	unsigned int data_size = p->np * nc;
	fsave mg = null_fs(), dg = null_fs(), *rgs = NULL;
	char *step_title = NULL;
	char *buff = NULL;
	char *ds = NULL;
	
	int num_rgs = 3;
	
	f = fopen(fname, "rb+");	// If it doesn't exist, there's a problem.
	if(f == NULL) { rv = MCD_ERR_NOFILE; goto error; }
	
	// Make the label for this particular step
	if(p->varied) {
		step_title = make_cstep_str(p, cind, 0, &rv);
		if(rv != 0) { goto error; }
	} else {
		step_title = malloc(strlen("[00000000]")+1);
		sprintf(step_title, "[%d]", cind);
	}
	
	// Generate the label fsave
	dg = make_fs(step_title);
	if(rv = put_fs(&dg, data, FS_DOUBLE, data_size)) { goto error; }
	
	if(cind == 0) {
		// If it's the first time through, create the main data group.
		mg = make_fs(MCD_MAINDATA);
		if(rv = put_fs_container(&mg, &dg, 1)) { goto error; }
		
		fseek(f, 0, SEEK_END);
		if(rv = fwrite_fs(f, &mg)) { goto error; }
	} else {
		// Otherwise we should find the main data group and append our data
		long mgp = find_fsave_in_file(f, MCD_MAINDATA, MCF_EOF);
		if(mgp < 0) { rv = mgp; goto error; }
		
		fseek(f, mgp, SEEK_SET);
		if(rv = fadd_fs_to_container(f, &dg)) { goto error; }
	}
	
	// Update the data header
	rgs = calloc(num_rgs, sizeof(fsave));
	int cf = 0;
	
	struct tm *tptr;
	tptr = localtime(&done);
	
	ds = calloc(MCD_TIMESTAMP_MAXSIZE+1, 1);
	strftime(ds, MCD_TIMESTAMP_MAXSIZE, MCD_TIME_FORMAT, tptr);
	
	rgs[cf] = make_fs(MCD_TIMEDONE);
	if(rv = put_fs(&(rgs[cf]), ds, FS_CHAR, MCD_TIMESTAMP_MAXSIZE+1)) { goto error; }
	
	int epoch = (int)done;
	rgs[++cf] = make_fs(MCD_TDONE);
	if(rv = put_fs(&(rgs[cf]), &epoch, FS_UINT, 1)) { goto error; }
	
	rgs[++cf] = make_fs(MCD_CIND);
	if(rv = put_fs(&(rgs[cf]), &cind, FS_UINT, 1)) { goto error; }
	
	rewind(f);
	
	if(rv = replace_fsaves(f, MCD_DATAHEADER, rgs, num_rgs, NULL)) { goto error; }
	
	error:
	if(f != NULL) { fclose(f); }
	
	if(step_title != NULL) { free(step_title); }
	if(buff != NULL) { free(buff); }
	if(ds != NULL) { free(ds); }
	
	free_fsave_array(rgs, num_rgs);
	free_fsave(&mg);
	free_fsave(&dg);
	
	return rv;
}

int save_data(double *data, double **avg) {
	return 0;	
}

 
int save_data_ddc(double *data, double **avg) {
	// In order for this to work, the ce variable needs to be set up appropriately. This function is
	// used to write data to an existing and open file. If for whatever reason 
	
	// Data is stored in one variable per channel. I believe it's indexed, so it should be just as fast to
	// to store it in one big variable as to store it as a bunch of small variables. We can run some tests
	// on that later if we want to see.
	
	PPROGRAM *p = ce.p;
	int rv = 0;
	
	if(p == NULL) { return -1; }
	
	int i, np = p->np, nd = p->nDims;
	char *csstr = NULL;
	double *avg_data = NULL;
	DDCFileHandle file = NULL, file2 = NULL;
	DDCChannelGroupHandle *cgs = NULL;
	DDCChannelHandle *achans = NULL, *mchans = NULL;
	DDCChannelGroupHandle mcg, acg;

	char *names[2] = {MCTD_MAINDATA, MCTD_AVGDATA};

	// Open the file.
	if(rv = DDC_OpenFileEx(ce.path, "TDM", 0, &file)) { goto error; }
//	if(rv = DDC_OpenFileEx(ce.path, "TDM", 0, &file2)) { goto error; }
	
	
	// Get the channel groups -> Allocate space for two, but only look for 1 if nt == 1
	cgs = malloc(sizeof(DDCChannelGroupHandle)*2);
	cgs[0] = NULL;
	cgs[1] = NULL;
	
	if(rv = get_ddc_channel_groups(file, names, (p->nt > 1)?2:1, cgs)) { goto error; }
	
	mcg = cgs[0];
	acg = cgs[1];
		
	// Save the current step as a linear index and as a string (for readability)
	if(rv = DDC_SetChannelGroupProperty(mcg, MCTD_CSTEP, ce.cind)) { goto error; }
	
	csstr = malloc(12*(p->nDims+2)+3); // Signed maxint is 10 digits, plus - and delimiters.
	int ct = get_ct(&ce);
	sprintf(csstr, "[%d", ct-1);
	for(i = 0; i < p->nDims; i++) {
		sprintf(csstr, "%s; %d", csstr, ce.cstep[i+p->nCycles]);	
	}
	
	sprintf(csstr, "%s]", csstr);
	
	if(rv = DDC_SetChannelGroupProperty(mcg, MCTD_CSTEPSTR, csstr)) { goto error; }
	free(csstr);
	csstr = NULL;
	
	// Save the time completed
	ce.tdone = time(NULL);
	if(rv = DDC_SetChannelGroupProperty(mcg, MCTD_TIMEDONE, ce.tdone)) { goto error; }
	
	// Grab the channels.
	mchans = malloc(sizeof(DDCChannelHandle)*ce.nchan);
	if(rv = DDC_GetChannels(mcg, mchans, ce.nchan)) { goto error; }
	
	// Save the data
	for(i = 0; i < ce.nchan; i++) {
		if(rv = DDC_AppendDataValues(mchans[i], &data[i*np], np)) { goto error; }
	}
	
	// Save the average data if needed.
	if(p->nt > 1) {
		// Grab the average channels.
		achans = malloc(sizeof(DDCChannelHandle)*ce.nchan);
		if(rv = DDC_GetChannels(acg, achans, ce.nchan)) { goto error; }

		ct = get_ct(&ce);
		if(ct < 0) { rv = ct; goto error; }
		
		if(ct == 1) {	// First run.
			for(i = 0; i < ce.nchan; i++) {
				if(rv = DDC_AppendDataValues(achans[i], &data[i*np], np)) { goto error;}
			}
		} else {
			// First get the old data
			int dli = (int)((ce.cind-1)/p->nt); // ct is a 1-based index.
			avg_data = load_data_tdm(dli, file, acg, achans, ce.p, ce.nchan, &rv);
			
			if(rv != 0) { goto error; }
			
			// Now update the average.
			for(i = 0; i < (p->np*ce.nchan); i++) {
				avg_data[i] = ((avg_data[i]*(ct-1))+data[i])/ct;
			}
			
			// And write it to file again.
			for(i = 0; i < ce.nchan; i++) {
				if(rv = DDC_ReplaceDataValues(achans[i], dli*np, &avg_data[i*np], np)) { goto error; }
			}
		}
	}
	
	// Save the data
	if(rv = DDC_SaveFile(file)) { goto error; };
	
	error:
	if(csstr != NULL) { free(csstr); }
	
	if(avg != NULL) {
		*avg = avg_data;
	} else if (avg_data != NULL) {
		free(avg_data);
	}
	
	if(file != NULL) { DDC_CloseFile(file); }

	return rv;
}

int save_data_safe(double *data, double **avg) {
	CmtGetLock(lock_tdm);
	CmtGetLock(lock_ce);
	
	int rv = save_data(data, avg);
	
	CmtReleaseLock(lock_ce);
	CmtReleaseLock(lock_tdm);
	
	return rv;
	
}

int load_experiment(char *filename, int prog) {
	// Loads the experiment from file into UIDC, data into the cache, etc.
	// 
	// Inputs:
	// filename: 	The path to the experiment file. Must end in .mcd.
	// prog:		Boolean - whether to load the program with it.
	//
	// Output:
	// Returns 0 on success, negative on failure.
	
	if(filename == NULL) { return MCD_ERR_NOFILENAME; }
	
	int rv = 0, i, cind = 0;
	int *glocs = NULL;
	dheader dh = null_dh();
	char *ext = NULL, **names = NULL;
	FILE *f = NULL;
	PPROGRAM *p;
	dstor data = null_ds();
	
	fsave dg = null_fs(), dig = null_fs();
	fsave *dihs = NULL;
	
	int dhs_size = 0, dihs_size = 0;
	
	ext = get_extension(filename);
	if(ext == NULL || strcmp(MCD_EXTENSION, ext+1) != 0) {
		
		rv = MCD_ERR_INVALID_EXTENSION;
		goto error;
	}
	
	if((f = fopen(filename, "rb")) == NULL) {
		rv = MCD_ERR_NOFILE;
		goto error;
	}
	
	// Start by reading out the data header.
	dh = load_dataheader_file(f, &rv);
	if(rv < 0) { goto error; }

	// Copy the stuff into the current experiment header.
	int len;
	if(dh.filename != NULL) {
		if(ce.fname != NULL) { free(ce.fname); ce.fname = NULL; }
		len = strlen(dh.filename)+1;
		
		if(len > 0) {
			ce.fname = malloc(len);
			strcpy(ce.fname, dh.filename);
		}
	}
	
	if(dh.expname != NULL) {
		if(ce.bfname != NULL) { free(ce.bfname); ce.bfname = NULL; }
		len = strlen(dh.expname)+1;
		
		if(len > 0) {
			ce.bfname = malloc(len);
			strcpy(ce.bfname, dh.expname);
		}
	}
	
	ce.num = dh.num;
	ce.hash = dh.hash;
	ce.nchan = dh.nchans;
	ce.cind = dh.cind;
	ce.tstart = dh.tstarted;
	ce.tdone = dh.tdone;
	
	// Need to load the pulse program next.
	p = load_pprogram(f, &rv); 
	if(rv < 0) { goto error; }
	
	if(ce.p != NULL) { free_pprog(ce.p); }
	ce.p = p;
	
	// Now maxsteps.
	if(dh.maxsteps != NULL) {
		if(ce.steps != NULL) { free(ce.steps); ce.steps = NULL; }
		
		ce.steps_size = 0;
		if((rv = get_steps_array_size(p->tmode, p->nCycles, p->nDims)) < 0) { goto error; }
		ce.steps_size = rv;
		rv = 0;
		
		ce.steps = get_steps_array(p->tmode, dh.maxsteps, p->nCycles, p->nDims, p->nt);
	}
	
	// Get the display values now - if they aren't found that's fine.
	long loc = find_fsave_in_file(f, MCD_DISPHEADER, MCF_WRAP);
	if(loc < 0) { goto disp_err; }
	
	fseek(f, loc, SEEK_SET);
	if(get_fs_header_from_file(f, &dig)) { goto disp_err; }
	
	dihs = read_all_fsaves_from_file(f, &rv, &dihs_size, dig.size);
	if(rv < 0 || dihs_size == 0) { rv = 0; goto disp_err; }
	
	names = calloc(dihs_size, sizeof(char *));
	for(i = 0; i < dihs_size; i++) { 
		if(dihs[i].ns == 0 || dihs[i].name == NULL) { continue; }
		names[i] = malloc(dihs[i].ns);
		memcpy(names[i], dihs[i].name, dihs[i].ns);
	}
	
	char *dih_names[MCD_DISPNUM] = {MCD_POLYFITON, MCD_POLYFITORDER, MCD_PHASE, MCD_FFTCHAN,
							  MCD_SGAINS, MCD_SOFFS, MCD_FGAINS, MCD_FOFFS,
							  MCD_SCHANSON, MCD_FCHANSON};
	
	int phpos = 3;
	
	void *dih_vals[MCD_DISPNUM] = {&(uidc.polyon), &(uidc.polyord), NULL, 
								&(uidc.schan), uidc.sgain, uidc.soff, uidc.fgain, uidc.foff,
								uidc.schans, uidc.fchans};
	
	glocs = strings_in_array(names, dih_names, dihs_size, MCD_DISPNUM);
	if(glocs == NULL) { goto disp_err; }
	
	int *ib;
	float *fb;
	int j;
	fsave fsb;
	
	for(i = 0; i < MCD_DISPNUM; i++) {
		if(dih_vals[i] == NULL || glocs[i] < 0) { continue; }
		
		j = glocs[i];
		fsb = dihs[j];
		int s = fsb.size/get_fs_type_size(fsb.type);
		switch(fsb.type) {
			case FS_UCHAR:
				ib = (int *)dih_vals[j];
				for(int k = 0; k < s; k++) {
					ib[k] = (int)(fsb.val.uc[k]);
				}
				break;
			case FS_UINT:
				ib = (unsigned int *)dih_vals[j];
				for(int k = 0; k < s; k++) {
					ib[k] = fsb.val.ui[k];	
				}
				break;
			case FS_INT:
				ib = (int *)dih_vals[j];
				for(int k = 0; k < s; k++) {
					ib[k] = fsb.val.i[k];	
				}
				break;
			case FS_FLOAT:
				fb = (float *)dih_vals[j];
				for(int k = 0; k < s; k++) {
					fb[k] = fsb.val.f[k];	
				}
				break;
		}
	}
	
	// The phase is its own thing, linearly indexed.
	if(glocs[phpos] >= 0) {
		fsb = dihs[phpos];
		int nc = (fsb.size/get_fs_type_size(fsb.type))/3;
		if(nc > 8) { nc = 8; }
		
		for(j = 0; j < nc; j++) {
			for(i = 0; i < 3; i++) {
				uidc.sphase[j][i] = fsb.val.f[3*j+i];
			}
		}
	}
	
	// Update the boxes and rings and shit.
	update_chan_ring(dc.fid, dc.fcring, uidc.fchans);
	update_chan_ring(dc.spec, dc.scring, uidc.schans);
	
	disp_err:
	if(glocs != NULL) {
		free(glocs);
		glocs = NULL;
	}
	
	if(names != NULL) {
		free(names);
		names = NULL;
	}
	
	dihs = free_fsave_array(dihs, dihs_size);
	dihs_size = 0;
	
	if(feof(f)) { rewind(f); }

	// Get the data
	data = load_all_data_file(f, ce.p, &dh, &rv);
	if(rv < 0) { goto error; }
	
	// Load the UI portions of this
	plot_data(data.data[0][0], data.np, ce.p->sr, data.nc);
	
	
	if(prog) {
		set_current_program(ce.p);
	}	
	
	update_fid_chan_box();
	update_spec_chan_box();
	update_spec_fft_chan();
	update_experiment_nav();  
	
	error:
	if(f != NULL) { fclose(f); }
	
	if(ext != NULL) { free(ext); }
	
	free_fsave(&dg);
	free_fsave(&dig);
	free_fsave_array(dihs, dihs_size);
	
	if(rv < 0) {
		data = free_ds(&data);
		ce.adata = free_ds(&ce.adata);
	} else {
		ce.adata = get_avg_data(data);	
	}
	
	ce.data = data;
	
	return rv;
}

int load_experiment_safe(char *filename, int prog) {
	
	int rv = load_experiment(filename, prog);	

	return rv;
}

int load_experiment_tdm(char *filename) {
	// Loads the experiment into the uidc file and loads the first experiment to the controls.
	// Filename must end in .tdm
	// Pass NULL to p if you are not interested in the program.
	// Otherwise, use free_pprog() when you are done with p.
	//
	// Errors:
	// -248 => Filename not valid
	// -249 => Groups not found
	// -250 => lindex exceeds bounds
	// -251 => Number of channels mismatched
	
	// The sundry variables
	int ev = 0, i, cind = -1;
	double *data = NULL;
	PPROGRAM *p = NULL;
	DDCFileHandle file = NULL;
	DDCChannelGroupHandle mcg = NULL, pcg = NULL, acg = NULL, *groups = NULL;
	DDCChannelHandle *chs = NULL, *achans = NULL;
	int ng, g = 40, s = 40, len;
	char *name_buff = NULL;
	char **cnames = NULL;
	int nc = -1;
					   
	// Check that the filename is conforming.
	if(filename == NULL || strlen(filename) < 5 || sscanf(&filename[strlen(filename)-4], ".tdm") != 0 || !FileExists(filename, NULL))  {
		ev = -248;
		goto error;
	}
	
	// Open the file for reading
	if(ev = DDC_OpenFileEx(filename, "TDM", 1, &file)) { goto error; }
	
	// Get the main and program channel names first (don't know if we need the average yet.
	char *names[2] = {MCTD_MAINDATA, MCTD_PGNAME};
	groups = malloc(sizeof(DDCChannelGroupHandle)*2);
	
	if(ev = get_ddc_channel_groups(file, names, 2, groups)) { goto error; }
	
	memcpy(&mcg, &groups[0], sizeof(DDCChannelGroupHandle));
	memcpy(&pcg, &groups[1], sizeof(DDCChannelGroupHandle));

	free(groups);	// Don't need this anymore.
	groups = NULL;
	
	// Grab the program.
	int err_val;
	p = load_programDDC(pcg, &ev);
	if(ev || p == NULL) { goto error; }
	
    // Get the current step index.
	if(ev = DDC_GetChannelGroupProperty(mcg, MCTD_CSTEP, &cind, sizeof(int))) { goto error; }
	
	// Get the number of channels in two ways, and check that they are consistent.
	unsigned char c_buff;
	if(ev = DDC_GetChannelGroupProperty(mcg, MCTD_NCHANS, &c_buff, sizeof(unsigned char))) { goto error; }
	
	int nchs = (int)c_buff;
	
	if(ev = DDC_GetNumChannels(mcg, &nc)) { goto error; }
	if(nc != nchs) {
		ev = -251;
		goto error;
	}
	
	// Allocate the channel array - I'm assuming they are in order.
	chs = malloc(sizeof(DDCChannelHandle)*nc);
	
	if(p->nt > 1) {
		char *avg_name = MCTD_AVGDATA;
		if(ev = get_ddc_channel_groups(file, &avg_name, 1, &acg)) { goto error; } 
		
		achans = malloc(sizeof(DDCChannelHandle)*nc);
		if(ev = DDC_GetChannels(acg, achans, nc)) { goto error; }
	}
	
	if(ev = DDC_GetChannels(mcg, chs, nc)) { goto error; }
	
	// Now allocate the array of cnames, then grab them. Currently these are not used, but I could see them
	// being useful in the future and it's easy to implement.
	cnames = malloc(sizeof(double*)*nc);
	for(i = 0; i < nc; i++) {
		cnames[i] = NULL;
	}
	
	for(i = 0; i < nc; i++) {
		if(DDC_GetChannelStringPropertyLength(chs[i], DDC_CHANNEL_NAME, &len)) { goto error; }
		
		cnames[i] = malloc(++len);
		
		if(DDC_GetChannelProperty(chs[i], DDC_CHANNEL_NAME, cnames[i], len))  { goto error; }
	}
	
	// Finally get the data for the 0th index
	int aom = (p->nt > 1 && acg != NULL);
	data = load_data_tdm(0, file, aom?acg:mcg, aom?achans:chs, p, nc, &ev);

	if(ev != 0) { goto error; }
	
	// If we're here, then we've finished loading all the data without any errors. We can now safely clear the 
	// old stuff and load the new stuff (hopefully).
	ce.cind = cind;
	ce.nchan = nc; 
	
	if(ce.p != NULL) { free_pprog(ce.p); }
	ce.p = p;
	
	if(ce.path != NULL) { free(ce.path); }
	
	ce.path = malloc(strlen(filename)+1);
	strcpy(ce.path, filename);
	
	// Now we want to plot the data and set up the experiment navigation.
	plot_data(data, p->np, p->sr, ce.nchan);
	free(data);	
	data = NULL;
	
	update_experiment_nav();

	error:
	
	// Free a bunch of memory - some of these things shouldn't be freed if ev wasn't reset,
	// as the pointer has been assigned to a field of ce.
	if(ev != 0 && p != NULL) { free_pprog(p); } // Don't free if ce.p points here!
	if(chs != NULL) { free(chs); }
	if(achans != NULL) { free(achans); }
	if(groups != NULL) { free(groups); }
	if(name_buff != NULL) { free(name_buff); }
	if(data != NULL) { free(data); }

	free_string_array(cnames, nc); // Don't reuse nc or this will break.
	
	if(file != NULL) { DDC_CloseFile(file); }
	
	return ev;
}

int load_experiment_tdm_safe(char *filename) {
	CmtGetLock(lock_tdm);
	CmtGetLock(lock_ce);
	CmtGetLock(lock_uidc);
	
	int rv = load_experiment_tdm(filename);
	
	CmtReleaseLock(lock_uidc);
	CmtReleaseLock(lock_ce);
	CmtReleaseLock(lock_tdm);
	
	return rv;
}

double *load_data_fname(char *filename, int lindex, PPROGRAM *p, int *ev) {
	// Loads MCD data from a not-opened file.
	// 
	// Inputs:
	// filename:	Path to the file you want to open
	// lindex:		Linear index to the data you want to retrieve.
	//				Pass -1 to get the average data
	//				Pass -2 to get an ND array of all data.
	//
	// Outputs:
	// p:			Pulse program used in generating the file.
	// ev: 			Error value. Returns 0 on success, negative number on failure.
	//
	// Returns:
	// Returns a dynamically allocated double array, which needs to be freed.
	
	double *data = NULL;
	int rv = 0;
	
	if(filename == NULL) { rv = MCD_ERR_NOFILENAME; goto error; }
	
	// Now load the relevant data.
	// Start by finding the data group.
	// TODO: Allow loading all data into the data cache.
	FILE *f = fopen(filename, "rb");
	if(f == NULL) { rv = MCD_ERR_NOFILE; goto error; }
	
	data = load_data_file(f, lindex, p, ev);
	
	
	error:
	fclose(f);
	
	*ev = rv;
	return data;
}
		 
dstor load_all_data_file(FILE *f, PPROGRAM *p, dheader *dhead, int *ev) {
	// Loads all data from MCD. The file will be rewound when passed to this function,
	// but not after. To retrieve the program, pass a dynamically allocated PPROGRAM
	// with p->valid set to 0. Passing NULL to p will retrieve the program from the
	// file, but it will not be passed to the calling function.
	//
	// The output is a 4D array:
	// data[nt][dims][nc][points]
	
	fsave dhg = null_fs(), dg = null_fs();
	fsave fsb = null_fs();
	fsave *all_data = NULL;
	dheader d = null_dh();
	int ind =-1, rv = 0, i, nc = 0;
	int ad_size = 0;
	
	dstor ds = null_ds(), ads = null_ds();
	int *cstep = NULL;  
	int *dim_step = NULL;
	
	if(f == NULL) { rv = MCD_ERR_NOFILE; goto error; }
	
	rewind(f);
	int null_p = (p == NULL);
	int null_d = (dhead == NULL);
	
	if(null_p) {
		p = load_pprogram(f, &rv);
		if(rv < 0) { goto error; }
	} else if(!p->valid) {
		PPROGRAM *pb = load_pprogram(f, &rv);
		if(rv < 0) { goto error; }
		memcpy(p, pb, sizeof(PPROGRAM));
		
		free(pb); // All the pointers are copied over, so this is OK.
	}
	
	if(!null_d && dhead->valid) {
		d = *dhead;	
	} else {
		d = load_dataheader_file(f, &rv);
		if(rv < 0) { goto error; }
	}

	// Need to get the current index.
	ind = d.valid?d.cind:-1;
	
	if(d.nchans < 1) { rv = MCD_ERR_NOCHANS; goto error; }
	
	nc = d.nchans;
	
	// Grab the data now - first we grab it, then we parse it.
	long loc = find_fsave_in_file(f, MCD_MAINDATA, MCF_WRAP);
	if(loc < 0) { rv = MCD_ERR_NODATA; goto error; }
	fseek(f, loc, SEEK_SET);    
		
	if(rv = get_fs_header_from_file(f, &dg)) { goto error; }

	
	all_data = read_all_fsaves_from_file(f, &rv, &ad_size, dg.size);
	if(rv < 0) { goto error; }
	
	if(ad_size < 1) { rv = MCD_ERR_NODATA; goto error; }
	
	if(ind < 0 || ind > ad_size) {
		ind = ad_size;	
	}
	
	int np = all_data[0].size/(nc*get_fs_type_size(all_data[0].type));
	if(np != p->np) {
		rv = MCD_ERR_MALFORMED_PROG;
		goto error;
	}
	
	// Generate the data storage structure metadata.
	ds.nt = p->nt;
	ds.maxds = 1;
	ds.nd = (p->nDims < 1)?1:p->nDims;
	ds.np = np;
	ds.nc = nc;
	
	if(p->nDims >= 0) {
		ds.dim_step = malloc(sizeof(int)*((p->nDims == 0)?1:p->nDims));
	} else {
		rv = MCD_ERR_MALFORMED_PROG;
		goto error;
	}
	
	for(i = 0; i < p->nDims; i++) {
		ds.dim_step[i] = p->maxsteps[i+p->nCycles];
		ds.maxds *= p->maxsteps[i+p->nCycles];
	}
	
	if(p->nDims < 1) {
		ds.dim_step[0] = 1;	
	}
	
	// Now we can allocate the data storage structure.
	if(rv = calloc_ds(&ds)) { goto error; }
	
	int t = 0, lindex = 0;
	dim_step = calloc(p->nDims, sizeof(int));
	
	for(i = 0; i <= ind; i++) {
		// Determine which one we're on.
		cstep = parse_cstep(all_data[i].name, &rv);
		if(rv < 0) { goto error; }
		
		if(p->nDims) {
			lindex = get_lindex(cstep, p->steps, p->steps_size);
			free(cstep);
			cstep = NULL;
		
			// Get the transient and dimensions-only linear index.
			t = get_transient(p, lindex);
		
			get_dim_step(p, lindex, dim_step);
			lindex = get_lindex(dim_step, p->maxsteps + p->nCycles, p->nDims);
		} else {
			t = cstep[0];
			lindex = 0;	
		}
		
		ds.data[t][lindex] = malloc(nc*np*sizeof(double));
		ds.data4[t][lindex] = calloc(nc, sizeof(double *));
		memcpy(ds.data[t][lindex], all_data[i].val.d, nc*np*sizeof(double));
		
		// Stored two ways for convenience.
		for(int j = 0; j < nc; j++) {
			ds.data4[t][lindex][j] = &(ds.data[t][lindex][j*np]);	
		}
	}
	
	error:
	if(cstep != NULL) { free(cstep); }
	if(dim_step != NULL) { free(dim_step); }
	
	free_fsave(&dhg);
	free_fsave(&dg);
	
	free_fsave_array(all_data, ad_size);
	
	if(!null_d) { *dhead = d; }
	
	ds.valid = 1;
	if(rv < 0) {
		ds = free_ds(&ds);
	}
	
	*ev = rv;
	
	return ds;
}

double *load_data_file(FILE *f, int lindex, PPROGRAM *p, int *ev) {
	// Loads MCD data from an opened file. The file will be rewound when passed to this
	// function, but not after.
	// 
	// Inputs:
	// f:			The file to get data from.
	// lindex:		Linear index to the data you want to retrieve.
	//
	// Outputs:
	// p:			Pulse program used in generating the file.
	// ev: 			Error value. Returns 0 on success, negative number on failure.
	//
	// Returns:
	// Returns a dynamically allocated double array, which needs to be freed.

	fsave dg = null_fs();
	fsave fdata = null_fs();
	
	double *data = NULL;
	int rv = 0;
	unsigned int fs_size = 0;
	int null_p = (p == NULL);
	
	if(f == NULL) { rv = MCD_ERR_NOFILE; goto error; }
	rewind(f);
	
	if(null_p) {
		p = load_pprogram(f, &rv);
		if(rv < 0) { goto error; }
	} else if(!p->valid) {
		PPROGRAM *pb = load_pprogram(f, &rv);
		if(rv < 0) { goto error; }
		memcpy(p, pb, sizeof(PPROGRAM));
		
		free(pb);
	}
	
	if(p == NULL) { rv = MCD_ERR_NOPROG; goto error; }
	
	long loc = find_fsave_in_file(f, MCD_MAINDATA, MCF_WRAP);
	if(loc < 0) { rv = MCD_ERR_NODATA; goto error; }
	
	fseek(f, loc, SEEK_SET);
	if(rv = get_fs_header_from_file(f, &dg)) { goto error; }
	
	// Convert the lindex as expected into the lindex as it's stored.
	// We're assuming that all navigation lindexes are calculated from
	// MC_TMODE_TF (transients first)
	if(lindex > 0) {
		lindex = convert_lindex(p, lindex, MC_TMODE_TF, p->tmode);
		if(lindex < 0) { 
			rv = lindex;
			goto error;
		}
	}
	
	if(lindex >= 0) {
		// Seek the lindex-th group in the file.
		loc = find_fsave_in_file(f, MCD_MAINDATA, dg.size);
		if(loc < 0) { 
			rv = loc;
			goto error; 
		}
	
		// Get the data
		fdata = read_fsave_from_file(f, &rv);
		if(rv < 0) { goto error; }

		if(fdata.val.d == NULL) { 
			rv = MCD_ERR_NODATA;
			goto error;
		} else {
			data = malloc(fdata.size);
			memcpy(data, fdata.val.d, fdata.size);
		}
	} else {
		rv = MCD_ERR_BADCIND;
		goto error;
	}
	
	error:
	free_fsave(&dg);
	free_fsave(&fdata);
	
	if(null_p && p != NULL) {
		free_pprog(p);
		p = NULL;
	}

	*ev = rv;
	return data;

}

double *load_data(char *filename, int lindex, PPROGRAM *p, int avg, int nch, int *rv) {
	// Loads data from a not-opened file. If avg evaluates as TRUE, it gets from the 
	// average channel. Otherwise it gets data from the main channel. Returns malloced
	// data of size p->np*nchans.
	
	int ev = 0;
	
	// Make sure it's a real file.
	if(filename == NULL || !FileExists(filename, NULL)) {
		*rv = -1;
		return NULL;
	}
	
	DDCFileHandle file = NULL;
	DDCChannelGroupHandle cg;
	DDCChannelHandle *chs = NULL;
	double *data = NULL;
	
	// Open the file for reading.
	if(ev = DDC_OpenFileEx(filename, "TDM", 1, &file)) { goto error; }
	
	
	// Get the channel group from the name.
	char *name = avg?MCTD_AVGDATA:MCTD_MAINDATA;
	if(ev = get_ddc_channel_groups(file, &name, 1, &cg)) { goto error; }
	
	int nc;
	if(ev = DDC_GetNumChannels(cg, &nc)) { goto error; }
	
	if(nc < nch) {		// Not enough channels present, that's a problem.
		ev = -2;
		goto error;
	}
	
	// Get the channels in that group.
	chs = malloc(sizeof(DDCChannelHandle)*nch); // Allocate the buffer
	if(ev = DDC_GetChannels(cg, chs, nch)) { goto error; }
	
	data = load_data_tdm(lindex, file, cg, chs, p, nch, rv);
		
	error:
	
	if(file != NULL) { DDC_CloseFile(file); }

	if(chs != NULL) { free(chs); }
	
	*rv = ev;
	return data;
}

double *load_data_safe(char *filename, int lindex, PPROGRAM *p, int avg, int nch, int *rv) {
	CmtGetLock(lock_tdm);
	double *data = load_data(filename, lindex, p, avg, nch, rv);
	CmtReleaseLock(lock_tdm);
	
	return data;
}

double *load_data_tdm(int lindex, DDCFileHandle file, DDCChannelGroupHandle mcg, DDCChannelHandle *chs, PPROGRAM *p, int nch, int *rv) {
	// This grabs data from an already open file and returns it as a malloced array of data. Make sure to free this when done with it.
	// This will return data of size np*nch. 
	
	double *data = NULL;
	int *maxsteps = NULL;
	int ev, i, np = p->np;
	
	// Figure out what cstep we want.
	maxsteps = malloc(sizeof(int)*(p->nDims+1)); // One for each indirect dimension, plus one for transients. Little-endian.
	
	// We also  need to prepare a variable containing the data from each channel each in a row, for the specified transient.
	data = malloc(sizeof(double)*p->np*nch);

	maxsteps[0] = p->nt;
	for(i = 1; i < p->nDims+1; i++) {
		maxsteps[i] = p->maxsteps[p->nCycles+i-1];
	}
	
	// I believe that the data we want is np points and is at position lindex*np, so let's get that.
	for(i = 0; i < nch; i++) {
		if(ev = DDC_GetDataValues(chs[i], lindex*np, np, &data[i*np]))
			goto error;
	}
	
	error:
	if(ev != 0 && data != NULL) {
		free(data);
		data = NULL;
	}				    
	
	if(maxsteps != NULL)
		free(maxsteps);
	
	*rv = ev;  // Pass on the return value
	
	return data;
} 

double *load_data_tdm_safe(int lindex, DDCFileHandle file, DDCChannelGroupHandle mcg, DDCChannelHandle *chs, PPROGRAM *p, int nch, int *rv) {
	CmtGetLock(lock_tdm);
	double *data = load_data_tdm(lindex, file, mcg, chs, p, nch, rv);
	CmtReleaseLock(lock_tdm);
	
	return data;
}
int get_ddc_channel_groups(DDCFileHandle file, char **names, int num, DDCChannelGroupHandle *cgs) {
	// Pass this a list of channel group names in the DDC file "file" and it returns an array of the handles.
	//
	// file = the file to get them from
	// names = an array of the names (size num)
	// num = the number of channel group handles to get
	// cgs = the array, malloced, of size sizeof(DDCChannelGroupHandle)*num.
	//
	// Errors:
	// -1: Invalid inputs
	// -2: Not enough channel groups found.
	// -3: All channel groups not found.
	
	if(num < 1 || file == NULL || names == NULL || cgs == NULL)
		return -1;
	
	int rv, i, j, g = 40, s = 40;
	char *buff = malloc(g);
	
	int ncg, len;
	DDCChannelGroupHandle *all_cgs = NULL;
	int *found = NULL;
	
	// Get all the channel groups so we can find the one we're looking for.
	if(rv = DDC_GetNumChannelGroups(file, &ncg)) { goto error; }
	if(ncg < num) {
		rv = -2;
		goto error;
	}
	
	all_cgs = malloc(sizeof(DDCChannelGroupHandle)*ncg);
	if(rv = DDC_GetChannelGroups(file, all_cgs, ncg)) { goto error; }
	
	// Go through each one and compare the names to find the one we're looking for
	found = calloc(sizeof(int)*num, sizeof(int));	// Starts out allocated with 0s.
	int nleft = num;
	for(i = 0; i < ncg; i++) {
		// Get the channel name from the list of all channels
		DDC_GetChannelGroupStringPropertyLength(all_cgs[i], DDC_CHANNELGROUP_NAME, &len); 
		buff = realloc_if_needed(buff, &g, ++len, s);	// Allocate space for the name if we need it.
		DDC_GetChannelGroupProperty(all_cgs[i], DDC_CHANNELGROUP_NAME, buff, len);
		
		for(j = 0; j < num; j++) {
			// Compare it to each of the channel names in the list
			if(!found[j]) {
				if(strcmp(names[j], buff) == 0) {
					found[j] = 1;
					nleft--;
					memcpy(&cgs[j], &all_cgs[i], sizeof(DDCChannelGroupHandle));	// Copy it over so we can free all_cgs
					break;
				}
			}
		}
		
		if(nleft == 0) { break; }
	}
	
	if(nleft)  { rv = -3; }
		
	error:
	
	if(buff != NULL) { free(buff); }
	if(all_cgs != NULL) { free(all_cgs); }
	if(found != NULL) { free(found); }
	
	return rv;
}

int get_ddc_channel_groups_safe(DDCFileHandle file, char **names, int num, DDCChannelGroupHandle *cgs) {
	CmtGetLock(lock_tdm);
	int rv = get_ddc_channel_groups(file, names, num, cgs);
	CmtReleaseLock(lock_tdm);
	
	return rv;
 }
 
 // Data Parsing Functions
dstor get_avg_data(dstor ds) {
	// Creates a new dstor containing the averaged data.
	dstor ads = null_ds();

	ads.nt = 1;
	ads.maxds = ds.maxds;
	ads.np = ds.np;
	ads.nc = ds.nc;
	
	if(ds.nd > 0) {
		ads.dim_step = malloc(ds.nd*sizeof(int));	
		
		memcpy(ads.dim_step, ds.dim_step, ds.nd*sizeof(int));
	}
	
	calloc_ds(&ads);
	
	int i, j, k, l;
	double w1 = 1.0, w2 = 1.0;
	int ps = ds.np*ds.nc;
	for(j = 0; j < ds.maxds; j++) {
		l = 0;
		for(i = 0; i < ds.nt; i++) {
			if(ds.data[i][j] != NULL) {
				if(ads.data[0][j] == NULL) { 
					ads.data[0][j] = calloc(ps, sizeof(double));
				}
				
				if(ads.data4[0][j] == NULL) {
					ads.data4[0][j] = calloc(ps, sizeof(double));	
				}
				
				l++;
				w1 = (l-1)/l;	// For a running average where we won't have to 
				w2 = 1/l;		// worry about overflows, hopefully.
				
				for(k = 0; k < ps; k++) {
					ads.data[0][j][k] = ads.data[0][j][k]*w1 + ds.data[i][j][k]*w2;	
				}
				
				for(k = 0; k < ds.nc; k++) {
					ads.data4[0][j][k] = &(ads.data[0][j][ds.np*k]);	
				}
			}
		}
	}
	
	ads.valid = 1;
	
	return ads;
}

int calloc_ds(dstor *ds) {
	int rv = 0;
	
	if(ds->nt < 1 || ds->maxds < 1) { return MCD_ERR_NODATA; }
	
	ds->data = calloc(ds->nt, sizeof(double **));
	ds->data4 = calloc(ds->nt, sizeof(double ***));
	
	for(int i = 0; i < ds->nt; i++) {
		ds->data[i] = calloc(ds->maxds, sizeof(double *));
		ds->data4[i] = calloc(ds->maxds, sizeof(double **));
		for(int j = 0; j < ds->maxds; j++) {
			ds->data4[j] = calloc(ds->nc, sizeof(double *));	
		}
	}
	
	return rv;
}

dstor free_ds(dstor *ds) {
	// Need to free dim_step, data and data4.
	// The [nc] level to data4 points to the data stored in data, and so it should
	// not be freed.
	if(ds->dim_step != NULL) {
		free(ds->dim_step);
		ds->dim_step = NULL;
	}
	
	if(ds->nt > 0 && ds->maxds > 0 && ds->nc > 0 &&
		(ds->data != NULL || ds->data4 != NULL)) {
		int i, j;
		for(i = 0; i < ds->nt; i++) {
			for(j = 0; j < ds->maxds; j++) {
				if(ds->data[i] != NULL && ds->data[i][j] != NULL) {
					free(ds->data[i][j]);
					ds->data[i][j] = NULL;
				}
				
				if(ds->data4[i] != NULL && ds->data4[i][j] != NULL) {
					free(ds->data4[i][j]);
					ds->data4[i][j] = NULL;
				}
			}
			
			if(ds->data[i] != NULL) { free(ds->data[i]); }
			if(ds->data4[i] != NULL) { free(ds->data4[i]); }
		}
		
		if(ds->data != NULL) { free(ds->data); ds->data = NULL; }
		if(ds->data4 != NULL) { free(ds->data4); ds->data4 = NULL; }
	}
	
	return null_ds();
}

dstor null_ds() {
	dstor ds = {.nt = -1,
				.maxds = -1,
				.np = -1,
				.nc = -1,
				.nd = -1,
				.dim_step = NULL,
				.data = NULL,
				.data4 = NULL,
				.valid = 0};
	
	return ds;
}

dheader load_dataheader_file(FILE *f, int *ev) {
	dheader d = null_dh();
	fsave fsb = null_fs();
	fsave dhg = null_fs();
	fsave *dhs = NULL;

	char **names = NULL;
	int i, rv = 0;
	int *glocs = NULL;
	unsigned int dhs_size = 0, nsize = 0;
	
	if(f == NULL) { rv = MCD_ERR_NOFILE; goto error; }
	rewind(f);
	
	// Find the header.
	long loc;
	if((loc = find_fsave_in_file(f, MCD_DATAHEADER, MCF_EOF)) < 0) { rv = loc; goto error; }
	fseek(f, loc, SEEK_SET);
	if(rv = get_fs_header_from_file(f, &dhg)) { goto error; }
	
	dhs = read_all_fsaves_from_file(f, &rv, &dhs_size, dhg.size);
	if(rv < 0) { goto error; }
	
	if(dhs_size == 0) {
		rv = MCD_ERR_BADHEADER;
		goto error;
	}
	
	// Read out all these FSAVES into the relevant variables.
	int tse = 0, tde = 0;
	int mspos = 11;
	char *dh_names[MCD_DATANUM] = {MCD_FILENAME, MCD_EXPNAME, MCD_EXPNUM, MCD_HASH,
								   MCD_NCHANS, MCD_DATESTAMP, MCD_TIMESTART, MCD_TIMEDONE,
								   MCD_TSTART, MCD_TDONE, MCD_CIND, MCD_MAXSTEPS};
	
	char **dh_string_vals[MCD_DATANUM] = {&(d.filename), &(d.expname), NULL, NULL, NULL, &(d.dstamp),
										  NULL, NULL, NULL, NULL, NULL, NULL};
	
	void *dh_vals[MCD_DATANUM] = {NULL, NULL, &(d.num), &(d.hash), &(d.nchans),
								   NULL, NULL, NULL, &tse, &tde,  &(d.cind), NULL};

	nsize = dhs_size;
	names = calloc(nsize, sizeof(char*));
	for(i = 0; i < nsize; i++) {
		if(dhs[i].ns == 0 || dhs[i].name == NULL) { continue; }
		names[i] = malloc(dhs[i].ns);
		memcpy(names[i], dhs[i].name, dhs[i].ns);
	}

	glocs = strings_in_array(names, dh_names, dhs_size, MCD_DATANUM);
	if(glocs == NULL) { rv = MCD_ERR_BADHEADER; goto error; }

	int j;
	char *sb;
	
	for(i = 0; i < MCD_DATANUM; i++) {
		if( glocs[i] < 0 || (dh_vals[i] == NULL && dh_string_vals[i] == NULL)) { continue; }
		j = glocs[i];
		
		fsb = dhs[j];
		
		switch(fsb.type) {
			case FS_CHAR:
				if(dh_string_vals[j] == NULL) { continue; }
				
				if(*dh_string_vals[j] != NULL) { 
					free(*dh_string_vals[j]);
					*dh_string_vals[j] = NULL;
				}
				
				sb = malloc(fsb.size);
				memcpy(sb, fsb.val.c, fsb.size);
				*dh_string_vals[j] = sb;
				break;
			case FS_INT:
				if(dh_vals[j] == NULL) { continue; }
				memcpy(dh_vals[j], fsb.val.i, fsb.size);
			case FS_UINT:
				if(dh_vals[j] == NULL) { continue; }
				
				*(unsigned int *)dh_vals[j] = *(fsb.val.ui);
				break;
			case FS_INT64:
				*(__int64 *)dh_vals[j] = *(fsb.val.ll);
				break;
			case FS_UINT64:
				*(unsigned __int64 *)dh_vals[j] = *(fsb.val.ull);
				break;
		}
	}
	
	if(glocs[mspos] >= 0) {
		fsb = dhs[glocs[mspos]];
		d.maxsteps = malloc(fsb.size);
		
		memcpy(d.maxsteps, fsb.val.i, fsb.size);
	}
	
	if(tse > 0) {
		d.tstarted = (time_t)tse;
	}
	
	if(tde > 0) {
		d.tdone = (time_t)tde;	
	}
	
	rewind(f);
	
	error:
	if(glocs != NULL) { free(glocs); }
	
	free_fsave_array(dhs, dhs_size);
	free_fsave(&dhg);
	
	free_string_array(names, nsize);
	
	d.valid = 1;
	
	if(rv < 0) { d = free_dh(&d); }
	*ev = rv;

	return d;
} 

dheader free_dh(dheader *d) {
	if(d != NULL) {
		if(d->filename != NULL) { 
			free(d->filename);
			d->filename = NULL;
		}
		
		if(d->expname != NULL) { 
			free(d->expname); 
			d->expname = NULL;
		}
		
		if(d->dstamp != NULL) {
			free(d->dstamp);
			d->dstamp = NULL;
		}
		
		if(d->maxsteps != NULL) {
			free(d->maxsteps);
			d->maxsteps = NULL;
		}
	}
	
	return null_dh();
}

dheader null_dh() {
	// Returns a null dataheader.
	
	dheader d = {.filename = NULL,
				 .expname = NULL,
			  	 .num = -1,
				 .hash = 0,
				 .dstamp = NULL,
				 .tstarted = NULL,
				 .tdone = NULL,
				 .nchans = 0,
				 .cind = 0,
				 .maxsteps = NULL,
				 .valid = 0};
 
	return d;
}
 
 
int *parse_cstep(char *step, int *ev) {
	// Parses titles of the form [0, 0, 0, 0]
	// If the result is not NULL, it is a dynamically allocated array which must be freed.
	int rv = 0;
	int *cstep = NULL;
	int strl = 0;

	if(step == NULL || (strl = strlen(step)) == 0) { rv = MCD_ERR_NOSTEPSTR; goto error; }
	
	// Figure out how many values there are first
	int i, s = 1;
	for(i = 0; i < strl; i++) {
		if(step[i] == ',') { 
			s++; 
		}
	}
	
	cstep = calloc(s, sizeof(unsigned int));
	char *m = step+1; // Skip the opening bracket.
	int n;
	for(i = 0; i < s; i++) {
		n = sscanf(m, "%d%*s", cstep+i);
		if(n != 1) { 
			rv = MCD_ERR_INVALIDSTR;
			goto error;
		}
		
		m = strchr(m, ',');
		if(m == NULL) { break; }
	}

	error:
	
	if(rv != 0) { 
		free(cstep);
		cstep = NULL;
	}
	
	*ev = rv;
	return cstep;
}

/******************** File Navigation *******************/
void select_data_item() {
	int index, type, len;
	char *path = NULL;
	
	GetCtrlIndex(mc.datafbox[1], mc.datafbox[0], &index);
	GetListItemImage(mc.datafbox[1], mc.datafbox[0], index, &type);
	
	GetCtrlValStringLength(mc.datafbox[1], mc.datafbox[0], &len);
	path = malloc(len+1);
	GetCtrlVal(mc.datafbox[1], mc.datafbox[0], path);
	
	if(type == VAL_FOLDER)
		select_directory(path);
	else if(type == VAL_NO_IMAGE)
		select_file(path);
	
	free(path);
}

void select_directory(char *path) {
	// Pass this a path, which is a folder. If it's not a folder, the folder that contains it will
	// be used. Path is set as the directory for load operations and the main UI objects associated
	// will be populated.
	
	if(!FileExists(path, NULL))
		return;				// Invalid path.
	
	SetCtrlVal(mc.path[1], mc.path[0], path);
	
	DeleteListItem(mc.datafbox[1], mc.datafbox[0], 0, -1);	// Clear the list.
	int ind = 0;											// Running tally of tine index
	
	char *spath = calloc(strlen(path)+strlen("*.mcd")+2, sizeof(char));

	int pchanged = 0;
	
	if(path[strlen(path)-1] == '\\') {						// Want to make sure we always know if there's
		path[strlen(path)-1] = '\0';						// a trailing backspace.
		pchanged = 1;	// Take note that we changed it -> this var is on loan.
	}
	
	// Add upwards navigation.
	char *c = strrchr(path, '\\');
	if(c != NULL) {
		strncpy(spath, path, c-path);
		if(FileExists(spath, NULL)) {
			InsertListItem(mc.datafbox[1], mc.datafbox[0], ind, "..", spath);
			SetListItemImage(mc.datafbox[1], mc.datafbox[0], ind++, VAL_FOLDER);
		}
	}
	
	sprintf(spath, "%s\\*", path);
	
	// Load the subfolders first
	char *filename = malloc(MAX_FILENAME_LEN+1);
	char *fullpath = malloc(MAX_PATHNAME_LEN+1);
	int rv = GetFirstFile(spath, 0, 0, 0, 0, 0, 1, filename);
	while(rv == 0) {	// Get the rest of them.
		sprintf(fullpath, "%s\\%s", path, filename);
		InsertListItem(mc.datafbox[1], mc.datafbox[0], ind, filename, fullpath);
		SetListItemImage(mc.datafbox[1], mc.datafbox[0], ind++, VAL_FOLDER);
		rv = GetNextFile(filename);
	}
	
	// Now load all the files.
	sprintf(spath, "%s\\%s", path, "*.mcd");
	
	rv = GetFirstFile(spath, 1, 0, 0, 0, 0, 1, filename);
	while(rv == 0) {	// Get the rest of them.
		sprintf(fullpath, "%s\\%s", path, filename);
		InsertListItem(mc.datafbox[1], mc.datafbox[0], ind, filename, fullpath);
		SetListItemImage(mc.datafbox[1], mc.datafbox[0], ind++, VAL_NO_IMAGE);
		rv = GetNextFile(filename);
	}
	
	if(pchanged)
		path[strlen(path)] = '\\';
	
	free(filename);
	free(spath);
	free(fullpath);
}

void select_file(char *path) {
	// Loads the file selected in the mc.datafbox list control. There is an "Are you sure?" message
	// before it actually loads. 
	
	char *fname = malloc(MAX_PATHNAME_LEN+1);
	char *msg = malloc(MAX_PATHNAME_LEN+1+strlen("Do you really want to load the file ?"));
	
	SplitPath(path, NULL, NULL, fname);
	sprintf(msg, "Do you really want to load the file %s?", fname);
	free(fname);
	
	int rv = ConfirmPopup("Really load data?", msg);
	
	free(msg);
	
	if(rv) {
		rv = load_experiment(path, 1);
		
		if(rv != 0)
			display_error(rv);
	}
}

void load_file_info(char *path, char *old_path) {
	// First we'll try to save the old description.
	if(old_path != NULL) {
		update_file_info(old_path);
	}
	
	if(!FileExists(path, NULL))	{ return; }

	int rv;
	DDCFileHandle file = NULL;
	int len;
	char *desc = NULL, *name = NULL;
	
	if(rv = DDC_OpenFileEx(path, "TDM", 1, &file)) { goto error; }
	
	// Get the base filename
	DDC_GetFileStringPropertyLength(file, DDC_FILE_NAME, &len);
	name = malloc(++len+10);
	DDC_GetFileProperty(file, DDC_FILE_NAME, name, len);

	// Set the base file name
	SetCtrlVal(mc.basefname[1], mc.basefname[0], name);

	// Calculate the next actual file name
	// First we need the base path.
	char *c = strrchr(path, '\\');
	if(c != NULL) {
		strncpy(name, c+1, strlen(c+1)-3);  // Copy filename without extension.
		name[strlen(c+1)-4] = '\0';		// Null terminate
		SetCtrlVal(mc.cdfname[1], mc.cdfname[0], name);
	}
	// Now get and set the description
	DDC_GetFileStringPropertyLength(file, DDC_FILE_DESCRIPTION, &len);
	desc = malloc(++len);
	
	DDC_GetFileProperty(file, DDC_FILE_DESCRIPTION, desc, len);
	
	ResetTextBox(mc.datadesc[1], mc.datadesc[0], desc);
	SetCtrlAttribute(mc.datadesc[1], mc.datadesc[0], ATTR_DFLT_VALUE, desc);
	
	error:
	if(file != NULL) { DDC_CloseFile(file); }
	if(name != NULL) { free(name); }
	if(desc != NULL) { free(desc); }
}

void load_file_info_safe(char *path, char *old_path) {
	CmtGetLock(lock_tdm);
	load_file_info(path, old_path);
	CmtReleaseLock(lock_tdm);
}

void update_file_info(char *path) {
	// Saves a new description for a TDM file.
	if(!FileExists(path, NULL)) { return; }
	
	int rv;
	DDCFileHandle file = NULL;
	int len;
	char *desc = NULL, *old_desc = NULL;
	
	// Get the description.
	GetCtrlValStringLength(mc.datadesc[1], mc.datadesc[0], &len);
	desc = (len >= 1)?malloc(len+1):NULL;
	if(desc != NULL) {
		GetCtrlVal(mc.datadesc[1], mc.datadesc[0], desc);
	}
	// Check if there's been a change. If not, don't bother.
	GetCtrlAttribute(mc.datadesc[1], mc.datadesc[0], ATTR_DFLT_VALUE_LENGTH, &len);
	old_desc = (len >= 1)?malloc(len+1):NULL;
	if(old_desc != NULL) {
		GetCtrlAttribute(mc.datadesc[1], mc.datadesc[0], ATTR_DFLT_VALUE, old_desc);
	}
	
	// If they are both NULL or they are the same, don't bother opening the file.
	if(old_desc == NULL && desc == NULL || 
		(old_desc != NULL && desc != NULL && strcmp(old_desc, desc) == 0)) { goto error; }
		
	if(rv = DDC_OpenFileEx(path, "TDM", 0, &file)) { goto error; }
	
	
	if(rv = DDC_SetFileProperty(file, DDC_FILE_DESCRIPTION, desc)) { goto error; }
	
	DDC_SaveFile(file);
	
	error:
	if(desc != NULL) { free(desc); }
	if(old_desc != NULL) { free(old_desc); }
	if(file != NULL) { DDC_CloseFile(file); }
}

void update_file_info_safe(char *path) {
	CmtGetLock(lock_tdm);
	update_file_info(path);
	CmtReleaseLock(lock_tdm);
}

//////////////////////////////////////////////////////////////
// 															//
//				UI Interaction Functions					//
// 															//
//////////////////////////////////////////////////////////////
void load_data_popup() {
	// The callback called from loading data.
	
	//CmtGetLock(lock_uidc); -> Placeholder, need to fix this eventually.
	char *path = malloc(MAX_FILENAME_LEN+MAX_PATHNAME_LEN);
	int rv = FileSelectPopup((uidc.dlpath != NULL)?uidc.dlpath:"", "*.tdm", ".tdm", "Load Experiment from File", VAL_LOAD_BUTTON, 0, 1, 1, 1, path);
	
	if(rv == VAL_NO_FILE_SELECTED) {
		free(path);
		return;
	} else {
		add_data_path_to_recent_safe(path);
	}
	
	int err_val = load_experiment_tdm_safe(path);
	
	free(path);
	
	if(err_val != 0)
		display_ddc_error(err_val);
	
}

void add_data_path_to_recent(char *opath) {
	// Adds  data file to the top of the most recent paths list.
	if(opath == NULL) {  return; }
	
	char *path = malloc(strlen(opath)+1);
	strcpy(path, opath);
	
	// TODO: Add to the recent data pulldown in the menu bar and the recent
	// program pulldown on the main page.

	
	// Make this directory the default next time we try and load something.
	char *c = strrchr(path, '\\'); // Finds last instance of the separator.
	c[1] = '\0';				   // Truncate after the last separator.

	if(c != NULL && FileExists(path, NULL)) {
		uidc.dlpath = malloc_or_realloc(uidc.dlpath, strlen(path)+1);
		strcpy(uidc.dlpath, path);
	}
	
	free(path);
}

void add_data_path_to_recent_safe(char *opath) {
	CmtGetLock(lock_uidc);
	add_data_path_to_recent(opath);
	CmtReleaseLock(lock_uidc);
}

/*********** Plotting Functions ************/
int plot_data(double *data, int np, double sr, int nc) {
	// Given data, this loads the data into the graphs.
	// np = Number of points
	// sr = sampling rate
	// nc = number of channels
	//
	// mode == 0 -> Plot FID only
	// mode == 1 -> Plot Spectrum only
	// mode == 2 -> Plot both.
	//
	// Data should be stored such that data[0->np-1] = channel 1, data[np+1->2*np] = channel 2, etc
	//
	// Everything is plotted on the first go-round, the channel viewing settings decide which plots are
	// visible. It's likely faster to just plot everything right away and switch the transparency settings.
	//
	// Errors:
	// 1: One or more of the inputs is invalid
	// 2: Memory error
	
	int ev = 0;
	
	// If the inputs are invalid, return 1.
	if(np <= 0 || sr <= 0.0 || nc <= 0 || data == NULL) { return 1; }
	
	clear_plots();	// Clear previous plot data.
	uidc.sr = sr; 	// Update the UIDC variable.

	// Turn on autoscale if checked
	int as;
	
	GetCtrlVal(dc.fid, dc.fauto, &as); // FID Autoscale
	if(as) {
		SetAxisScalingMode(dc.fid, dc.fgraph, VAL_BOTTOM_XAXIS, VAL_AUTOSCALE, 0, 0);
		SetAxisScalingMode(dc.fid, dc.fgraph, VAL_LEFT_YAXIS, VAL_AUTOSCALE, 0, 0);
	}
	
	GetCtrlVal(dc.spec, dc.sauto, &as); // Spectrum autoscale
	if(as) {
		SetAxisScalingMode(dc.spec, dc.sgraph, VAL_BOTTOM_XAXIS, VAL_AUTOSCALE, 0, 0);
		SetAxisScalingMode(dc.spec, dc.sgraph, VAL_LEFT_YAXIS, VAL_AUTOSCALE, 0, 0);
	}

	// We want to zero-pad the FFT out to a power of 2.
	int npfft = ceil(log10(np)/log10(2)); // ceil(log2(np))
	npfft = 1<<npfft; // Bitshifting 1 gives you pow(2, npfft);
	int i, j;
	
	double *curr_data = NULL, *fft_re = NULL, *fft_im = NULL, *fft_mag = NULL, *phase = NULL, *freq = NULL;
	NIComplexNumber *curr_fft = NULL;
	
	curr_data = malloc(sizeof(double)*np); // Buffer for each channel's data.
	fft_re = malloc(sizeof(double)*npfft/2); 	// Real channel
	fft_im = malloc(sizeof(double)*npfft/2); 	// Imaginary channel
	fft_mag = malloc(sizeof(double)*npfft/2); 	// 
	phase = malloc(sizeof(double)*npfft/2);	 	// Phase vector
	freq = malloc(sizeof(double)*npfft/2);   	// Frequency vectors
	curr_fft = malloc(sizeof(NIComplexNumber)*npfft); // Buffer for each channel's fft.
	
	// Check that the mallocs succeeded
	if(curr_data == NULL || curr_fft == NULL || fft_re == NULL || fft_im == NULL || fft_mag == NULL || phase == NULL || freq == NULL) {
		ev = 2;	
		goto error;
	 }
	// Generate the frequency vector for the phase evaluation
	for(i = 0; i < npfft/2; i++) {
		freq[i] = i*sr/(2*(npfft/2-1)); 
	}
	
	for(i = 0; i < nc; i++) {  // Iterate through the channels
		// Get the data for this channel
		// Pre-scale the data to have 0 change in the power spectrum - i.e. multiply by (2/np)
		for(j = 0; j < np; j++) {
			curr_data[j] = data[j+i*np]*2/np;
		}
		
		FFTEx(curr_data, np, npfft, NULL, FALSE, curr_fft);  // Do the fourier transform before any gain stuff
		
		if(uidc.fgain[i] != 1.0 || uidc.foff[i] != 0.0) {
			for(j = 0; j < np; j++)
				curr_data[j] = curr_data[j]*uidc.fgain[i] + uidc.foff[i];
		}
		
		uidc.fplotids[i] = PlotY(dc.fid, dc.fgraph, curr_data, np, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, uidc.fchans[i]?uidc.fcol[i]:VAL_TRANSPARENT);
		
		// Prepare the data.
		PolyEv1D(freq, npfft/2, (double *)uidc.sphase[i], 3, phase);
		for(j = 0; j < npfft/2; j++) {
			// Apply the initial phase correction
			fft_re[j] = cos(phase[j])*curr_fft[j].real - sin(phase[j])*curr_fft[j].imaginary;
			fft_im[j] = cos(phase[j])*curr_fft[j].imaginary + sin(phase[j])*curr_fft[j].real;
			fft_mag[j] = sqrt(pow(curr_fft[j].real, 2)+pow(curr_fft[j].imaginary, 2));	// Magnitude has no phase.
			
			// Then the gains and offset.
			fft_re[j] = fft_re[j]*uidc.sgain[i] + uidc.soff[i];
			fft_im[j] = fft_im[j]*uidc.sgain[i] + uidc.soff[i];
			fft_mag[j] = fft_mag[j]*uidc.sgain[i] + uidc.soff[i];
		}
		
		// Set the plot
		int col[3] = {0, 0, 0};
		if(uidc.schans[i] && uidc.schan<3 && uidc.schan >= 0) {
			col[uidc.schan] = 1;
		}
		
		uidc.splotids[i][0] = PlotY(dc.spec, dc.sgraph, fft_re, npfft/2, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, col[0]?uidc.scol[i]:VAL_TRANSPARENT);
		uidc.splotids[i][1] = PlotY(dc.spec, dc.sgraph, fft_im, npfft/2, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, col[1]?uidc.scol[i]:VAL_TRANSPARENT);
		uidc.splotids[i][2] = PlotY(dc.spec, dc.sgraph, fft_mag, npfft/2, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, col[2]?uidc.scol[i]:VAL_TRANSPARENT);
	}
	
	for(i = 0; i < 8; i++) {
		if(uidc.schans[i]) {
			SetCtrlAttribute(dc.sfftring[1], dc.sfftring[0], ATTR_DIMMED, 0);
			break;
		}
	}
	
	if(i >= 8)
		SetCtrlAttribute(dc.sfftring[1], dc.sfftring[0], ATTR_DIMMED, 1);
	
	// Set up the axis scale.
	SetCtrlAttribute(dc.fid, dc.fgraph, ATTR_XAXIS_GAIN, 1000/sr); 					// Value in ms. TODO:Make this configurable
	SetCtrlAttribute(dc.spec, dc.sgraph, ATTR_XAXIS_GAIN, sr/(2*((npfft/2)-1)));	// Frequency in Hz TODO: Make this configurable.	
	
	// Free our vectors
	error:
	if(curr_data != NULL) { free(curr_data); }
	if(curr_fft != NULL) { free(curr_fft); }
	if(fft_re != NULL) { free(fft_re); }
	if(fft_im != NULL) { free(fft_im); }
	if(fft_mag != NULL) { free(fft_mag); }
	if(phase != NULL) { free(phase); }
	if(freq != NULL) { free(freq); }
	
	return ev;
}

int plot_data_safe(double *data, int np, double sr, int nc) {
	CmtGetLock(lock_uidc);
	int rv = plot_data(data, np, sr, nc);
	CmtReleaseLock(lock_uidc);
	
	return rv;
}

int change_phase(int chan, double phase, int order) {
	// Call this when you change the phase correction of order "order" to "phase"
	// on channel "chan".
	//
	// Error -1 => Plots not found
	// Error -2 => Sampling rate not set
	// Error -3 => Invalid channel or phase order
	
	int c = chan; // Convenience
	
	// Check of the plots are actually there.
	if(uidc.splotids[c][0] < 0 || uidc.splotids[c][1] < 0)
		return -1;
	
	if(uidc.sr <= 0.0)
		return -2;
	
	if(order >= 3 || order < 0 || c >= 8 || c < 0)
		return -3;
	
	size_t data_size;
	GetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[c][0], ATTR_PLOT_YDATA_SIZE, &data_size);
	
	int np = data_size/sizeof(double);
	
	double *ph = malloc(data_size);
	double *rc = malloc(data_size);
	double *ic = malloc(data_size);
	double *orc = malloc(data_size);
	double *oic = malloc(data_size);
	
	// Generate the phase correction vector
	// TODO: Fix this so that the higher order corrections can be adjusted on a finer level.
	for(int i = 0; i < np; i++) { 
		ph[i] = (double)i*uidc.sr/(2*(double)np-1);	// This should be the frequency domain.
	}
	
	// The user can only change the phases one order at a time, so we're calculating the
	// offset of any given phase here. Since the first phase transform (up to uidc.sphase[c][order])
	// has already been applied, set the others to 0 and subtract the already applied phase from the
	// relevant order.
	double phases[3] = {0, 0, 0};
	phases[order] = phase-uidc.sphase[c][order];
	
	for(int i = 0; i < 3; i++)
		phases[i] /= 180/Pi();
	
	PolyEv1D(ph, np, phases, 3, ph); // In-place generation of the phase.
	
	// Get the data
	GetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[c][0], ATTR_PLOT_YDATA, orc);
	GetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[c][1], ATTR_PLOT_YDATA, oic);

	for(int i = 0; i < np; i++) {
		// Apply the initial phase correction
		rc[i] = cos(ph[i])*orc[i]-sin(ph[i])*oic[i];
		ic[i] = cos(ph[i])*oic[i]+sin(ph[i])*orc[i];
	}
	
	SetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[c][0], ATTR_PLOT_YDATA, rc);
	SetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[c][1], ATTR_PLOT_YDATA, ic);
	RefreshGraph(dc.spec, dc.sgraph);
	
	// Finally, free all the memory.
	free(ph);
	free(rc);
	free(ic);
	free(orc);
	free(oic);
	
	uidc.sphase[c][order] = phase;
	
	return 0;
}

int change_phase_safe(int chan, double phase, int order) {
	CmtGetLock(lock_uidc);
	int rv = change_phase(chan, phase, order);
	CmtReleaseLock(lock_uidc);
	
	return rv;
}


void update_experiment_nav() {
	// Call this to update things like the number of transients available, the number of
	// dimensions, etc. This should be called whenever you switch points in an indirect
	// dimension. This does not change the position you are in, it just updates the ring
	// controls and un-hides the relevant controls.
	
	int i, j;
	if(ce.p == NULL || ce.p->nt < 1){
		// If there is no program saved, no navigation is possible, dim/hide everything. 
		for(j = 0; j < 2; j++) {
			for(i = 0; i < 8; i++) {
				SetCtrlAttribute(dc.cloc[j], dc.idrings[i], ATTR_VISIBLE, 0);
				SetCtrlAttribute(dc.cloc[j], dc.idlabs[i], ATTR_VISIBLE, 0);
			}
			SetCtrlAttribute(dc.cloc[j], dc.ctrans, ATTR_DIMMED, 1);
			DeleteListItem(dc.cloc[j], dc.ctrans, 0, -1);
		}
		
		return;  // And done - that was easy.
	}
	
	PPROGRAM *p = ce.p; // Convenience
	
	int *steps = NULL;
	
	if(p->nDims) {   
		// Get an array from the linear index
		// The dimensionality of the cstep, which should be [nt, {dim1, ..., dimn}]
		// p->maxstep is stored as {pc1, ... pcn, dim1, ... , dimn}
		steps = malloc(sizeof(int)*(p->nDims));
		
		if(get_cstep((int)(ce.cind/(p->nt)), steps, &p->maxsteps[p->nCycles], p->nDims) != 1) { // On error, return
			free(steps);
			return;
		}
		
		for(i = 0; i < p->nDims; i++)
			steps[i]++;	// Convert to a 1-based index for this.
		
		char **c = NULL;
	
		// If it's multidimensional, go through and turn on each of the controls.
		for(j = 0; j < 2; j++) {
			// Update the multidimensional bit
			int nl, elems, step;
			for(i = 0; i < p->nDims; i++) { 
				SetCtrlAttribute(dc.cloc[j], dc.idrings[i], ATTR_VISIBLE, 1);
				SetCtrlAttribute(dc.cloc[j], dc.idlabs[i], ATTR_VISIBLE, 1);

				GetNumListItems(dc.cloc[j], dc.idrings[i], &nl);
				if(nl > 0)
					GetCtrlIndex(dc.cloc[j], dc.idrings[i], &step);
				else
					step = -1;
				
				// Update the number of steps in each dimension.
				if(nl > steps[i])
					DeleteListItem(dc.cloc[j], dc.idrings[i], steps[i], -1);	
				else if(nl < steps[i]) {
					c = generate_char_num_array(nl, steps[i], &elems);
					for(int k = 0; k < elems-1; k++) {
						InsertListItem(dc.cloc[j], dc.idrings[i], -1, c[k], k+nl); // Zero-based index in the vals.
						free(c[k]);
					}
					free(c);
					c = NULL;
				}
				
				GetNumListItems(dc.cloc[j], dc.idrings[i], &nl);
				if(step == -1)
					SetCtrlIndex(dc.cloc[j], dc.idrings[i], 0);
				else if(step < nl)
					SetCtrlIndex(dc.cloc[j], dc.idrings[i], step);
				else if(step >= nl)
					SetCtrlIndex(dc.cloc[j], dc.idrings[i], nl);
			}
		}
	}

	if(steps != NULL) { free(steps); }
	
	for(j = 0; j < 2; j++) {
		for(i = p->nDims; i < 8; i++) {
			SetCtrlAttribute(dc.cloc[j], dc.idrings[i], ATTR_VISIBLE, 0);
			SetCtrlAttribute(dc.cloc[j], dc.idlabs[i], ATTR_VISIBLE, 0);
		}
	}
	
	// Now update the transients
	update_transients();
}

void update_experiment_nav_safe() {
	CmtGetLock(lock_ce);
	update_experiment_nav();
	CmtReleaseLock(lock_ce);
}

void update_transients() {
	// Call this after you've changed the position in acquisition space, or in the event of a new
	// addition to the number of transients. ce.p must be set.
	
	if(ce.p == NULL) { return; }
	
	PPROGRAM *p = ce.p;
	
	// Define variables so we don't have to do it twice in-line.
	int nt, nl, ind, pan;
	char *tc = malloc(strlen("Transient ")+(int)ceil(log10(p->nt))+2); // Allocate maximum needed.

	// Whole thing needs to be done twice, once for fft and once for fid.
	for(int i = 0; i < 2; i++) {
		pan = dc.cloc[i];
		// How many transients to display
		if(p->nDims) {
			ind = get_selected_ind(pan, 0, NULL);
			nt = calculate_num_transients(ce.cind, ind, p->nt);
		} else {
			nt = ce.cind;
		}
		
		if(nt)
			SetCtrlAttribute(pan, dc.ctrans, ATTR_DIMMED, 0);
		else
			SetCtrlAttribute(pan, dc.ctrans, ATTR_DIMMED, 1);
		
		if(nt == 1) {
			// If it's one transient, delete all the transients and just replace them with one, "Transient 1"
			DeleteListItem(pan, dc.ctrans, 0, -1);
			InsertListItem(pan, dc.ctrans, 0, "Transient 1", 1);
		} else {
			// If it's more than one, do a check to see if the average one is in there.
			GetNumListItems(pan, dc.ctrans, &nl);
			if(nl == 1) {
				DeleteListItem(pan, dc.ctrans, 0, -1);
				nl = 0;
			} else if(nl > 1) {
				// Check to make sure that the "Average" entry is there.
				int len, buff;
				
				GetCtrlVal(pan, dc.ctrans, &buff);
				
				if(buff != 0) { 					// Average has value 0.
					DeleteListItem(pan, dc.ctrans, 0, -1); // Delete them all and start over.
					nl = 0;
				}
			}
			if(nl == 0) {
				InsertListItem(pan, dc.ctrans, 0, "Average", 0);
				nl++;
			}
			
			// Add any transients that need to be added.
			if(nl <= nt) {
				for(int j = nl; j <= nt; j++) {
					sprintf(tc, "Transient %d", j);					// Make the label.
					InsertListItem(pan, dc.ctrans, -1, tc, j);		// Insert item at the end
				}
			}
		}
	}
	free(tc);
}

void update_transients_safe() {
	CmtGetLock(lock_ce);
	update_transients();
	CmtReleaseLock(lock_ce);
}

void set_nav_from_lind(int lind, int panel, int avg) {
	// Feed this a lind vector and a parent panel (CurrentLoc-based) and it
	// sets the nav controls to what you want them to be. lind is a linear 
	// index in a space with size [nt {dim1, ..., dimn}]. ce.p must be set.
	// If you want to ignore the transient value and plot the average, set
	// avg = 1, otherwise 0 will give you the relevant transient.
	
	PPROGRAM *p = ce.p; 	// Convenience
	int i, t;
	
	// Break the lindex up into a vector we can iterate through 
	int *cstep = malloc(sizeof(int)*(p->nDims+1));
	int *steps = malloc(sizeof(int)*(p->nDims+1));
	for(i = 0; i < p->nDims; i++) {
		steps[i+1] = p->maxsteps[p->nCycles+i];
	}
	steps[0] = p->nt;
	
	if(get_cstep(lind, cstep, steps, p->nDims+1) != 1) { return; }	// Return on error
	
	// Set the values, then we're done.
	SetCtrlVal(panel, dc.ctrans, avg?0:cstep[0]+1);
	for(i = 0; i < p->nDims; i++) { SetCtrlVal(panel, dc.idrings[i], cstep[i+1]); }
	
	free(cstep);
	free(steps);
}

void set_nav_from_lind_safe(int lind, int panel, int avg) {
	CmtGetLock(lock_ce);
	set_nav_from_lind(lind, panel, avg);
	CmtReleaseLock(lock_ce);
}

void set_data_from_nav(int panel) {
	// Grabs the current index from the nav and plots the data from the file.
	// Panel should be a CurrentLoc panel
	int lind, t;
	
	// Gets the position from the nav.
	GetCtrlVal(panel, dc.ctrans, &t);
	
	if(ce.p->nDims) {
		lind = get_selected_ind(panel, (t != 0), NULL); // If t == 0, we're getting an average. 
		set_nav_from_lind(lind, dc.cloc[(panel == dc.cloc[0])?1:0], (t == 0));	// Setup the corresponding other tab.
	} else
		lind = (t == 0)?0:t-1;
	
	int rv = 0;
	
	double *data = load_data(ce.path, lind, ce.p, (t == 0), ce.nchan, &rv);
	
	if(rv != 0) {
		display_ddc_error(rv);
		goto error;
	} 
	
	plot_data(data, ce.p->np, ce.p->sr, ce.nchan);	// Plot the data.
	
	error:
	if(data != NULL) { free(data); }
}

void set_data_from_nav_safe(int panel) {
	CmtGetLock(lock_tdm);
	CmtGetLock(lock_ce);
	CmtGetLock(lock_uidc);
	
	set_data_from_nav(panel);
	
	CmtReleaseLock(lock_uidc);
	CmtReleaseLock(lock_ce);
	CmtReleaseLock(lock_tdm);
}

void clear_plots() {
	// Running this will clear all plots currently on the screen. Unlike in the previous version
	// this function will NOT clear the "location" bar. That is a separate function.
	
	// Clear all plots and annotations
	DeleteGraphPlot(dc.fid, dc.fgraph, -1, VAL_IMMEDIATE_DRAW);
	DeleteGraphPlot(dc.spec, dc.sgraph, -1, VAL_IMMEDIATE_DRAW);
	DeleteGraphAnnotation(dc.fid, dc.fgraph, -1);
	DeleteGraphAnnotation(dc.spec, dc.sgraph, -1);
	
	// Replace the plotids with -1 to indicate that they are no longer in use.
	for(int i = 0; i < 8; i++) {
		uidc.fplotids[i] = -1;
		for(int j = 0; j < 3; j++)
			uidc.splotids[i][j] = -1;
	}
}

void clear_plots_safe() {
	CmtGetLock(lock_uidc);
	clear_plots();
	CmtReleaseLock(lock_uidc);
}

int get_selected_ind(int panel, int t_inc, int *step) {
	// Gets the current index in the indirect dimensions. If t_inc is TRUE, the index is the linear
	// index for steps of the form [nt {dim1, ..., dimn}]. Otherwise, it's just the linear index for
	// [dim1, ..., dimn].
	//
	// Pass NULL to step if you don't want the vector index of the selected step, otherwise
	// step should be pre-allocated to be of size nDims for t_inc == FALSE or size nDims+1 for t_inc == TRUE
	//
	// Panel should be a CurrentLoc panel, otherwise this will be terrible.
	//
	// Returns index on success and negative number of failure.
	//
	// Error values:
	// -1: ce.p not set
	// -2: Number of transients is invalid
	// -3: Not multidimensional, didn't ask for transients
	// -4: Multidimensional, but no dimensions are available to be selected.
	// -5: Error in linear indexing
	
	PPROGRAM *p = ce.p;
	if(p == NULL) { return -1; }
	
	int t = 0, nl;
	
	if(t_inc) {
		GetNumListItems(panel, dc.ctrans, &nl);
		if(nl < 1 || p->nt < 1)  { return -2; }
		
		GetCtrlVal(panel, dc.ctrans, &t);
	}
	
	// The program is simple if it's not multi-dimensional.
	if(p->nDims < 1) {
		if(!t_inc)	
			return -3;   // Not multidimensional, didn't ask for transients.
		else
			return t-1;
	}
	
	// We'll start by generating the vector.
	int i, *cstep = malloc(sizeof(int)*p->nDims);
	cstep = memset(cstep, -1, sizeof(int));	// Preset these to -1.
	for(i = 0; i < p->nDims; i++) {
		GetNumListItems(panel, dc.idrings[i], &nl);
		if(nl < 1)
			break;
		
		GetCtrlVal(panel, dc.idrings[i], &cstep[i]);
	}
	
	if(nl < 1) {
		free(cstep);
		return -4;
	}
	
	int ind = get_lindex(cstep, &p->maxsteps[p->nCycles], p->nDims); // We only need the id lindex at the moment.
	
	if(ind < 0) { return -5; }
	
	// Add in transient if necessary.
	if(t_inc) {
		ind *= p->nt;
		ind += t-1; 
	}
	
	if(step != NULL) {
		memcpy(&step[t_inc?1:0], cstep, sizeof(int)*p->nDims); // Skip the first index if that one should have the transient in it.
		if(t_inc)
			step[0] = t-1;
	}
	
	free(cstep);
	return ind;
}

int get_selected_ind_safe(int panel, int t_inc, int *step) {
	CmtGetLock(lock_ce);
	int rv = get_selected_ind(panel, t_inc, step);
	CmtReleaseLock(lock_ce);
	
	return rv;
}

int calculate_num_transients(int cind, int ind, int nt) {
	// For a given index in acquisition space, we want to know how many transients we can display.
	// This returns that value. "ind" should be the position in indirect acquisition space, not 
	// including transients. cind should be [nt {dim1, ..., dimn}]
	
	if(nt > cind)
		return cind;
	
	int cindid = (int)(floor(cind/nt)); 	// This is the position in acquisition space we're in.
	
	if(ind > cindid)
		return -1; // Invalid position.
	else if(ind == cindid)
		return cind-(cindid*nt); 			// The number of transients completed in the most recent acquisition.
	else
		return nt;							// All transients should be available.
}

void change_fid_chan_col(int num) {
	// Given an already changed uidc var, we will update the relevant
	// controls for a given fid channel.
	
	if(num < 0 || num >= 8)
		return;
	int on_col = uidc.fcol[num];

	// Need to reduce all three colors separately
	int off_col = 0;
	int mask = 0xFF;
	off_col += (int)((mask&on_col)/4);
	
	mask = mask<<8;
	off_col +=(int)(((mask&on_col)>>8)/4)<<8;
	
	mask = mask << 8;
	off_col +=(int)(((mask&on_col)>>16)/4)<<16;
	
	// First set up the channel button control
	SetCtrlAttribute(dc.fid, dc.fchans[num], ATTR_ON_COLOR, on_col);
	SetCtrlAttribute(dc.fid, dc.fchans[num], ATTR_OFF_COLOR, off_col);
	
	
	// Update the plot color if necessary
	if(uidc.fchans[num] && uidc.fplotids[num] >= 0) {
		SetPlotAttribute(dc.fid, dc.fgraph, uidc.fplotids[num], ATTR_TRACE_COLOR, on_col);	
	}
}

void change_fid_chan_col_safe(int num) {
	CmtGetLock(lock_uidc);
	change_fid_chan_col(num);
	CmtReleaseLock(lock_uidc);
}

void change_spec_chan_col(int num) {
	// Given an already changed uidc var, we will update the relevant
	// controls for a given fid channel.
	
	if(num < 0 || num >= 8)
		return;
	
	int on_col = uidc.scol[num];
	
	// Need to reduce all three colors separately
	int off_col = 0;
	int mask = 0xFF;
	off_col += (int)((mask&on_col)/4);
	
	mask = mask<<8;
	off_col +=(int)(((mask&on_col)>>8)/4)<<8;
	
	mask = mask << 8;
	off_col +=(int)(((mask&on_col)>>16)/4)<<16;
	
	// First set up the channel button control
	SetCtrlAttribute(dc.spec, dc.schans[num], ATTR_ON_COLOR, on_col);
	SetCtrlAttribute(dc.spec, dc.schans[num], ATTR_OFF_COLOR, off_col);
	
	// Update the plot color if necessary

	if(uidc.schans[num] && uidc.splotids[num][uidc.schan] >= 0) {
		SetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[num][uidc.schan], ATTR_TRACE_COLOR, on_col);	
	}
}

void change_spec_chan_col_safe(int num) {
	CmtGetLock(lock_uidc);
	change_spec_chan_col(num);
	CmtReleaseLock(lock_uidc);
}

void change_fid_gain(int num) {
	// Updates the gain for the FID.
	
	if(num < 0 || num >= 8)
		return;
	
	int c = num;
	
	// Get the old gain
	float oldgain = uidc.fgain[c];
	GetCtrlVal(dc.fid, dc.fcgain, &uidc.fgain[c]);
	
	if(uidc.fplotids[c] < 0)  // If we don't need to update the plot, we're good
		return;
	
	// Get the plot data
	int np, i;
	GetPlotAttribute(dc.fid, dc.fgraph, uidc.fplotids[c], ATTR_NUM_POINTS, &np);
	
	double *data = malloc(sizeof(double)*np);
	double gc = uidc.fgain[c]/oldgain; // Gain change
	
	GetPlotAttribute(dc.fid, dc.fgraph, uidc.fplotids[c], ATTR_PLOT_YDATA, data);
	
	// Alter the gain.
	for(i = 0; i < np; i++) {
		data[i] *= gc;		
	}
	
	// Update the plot.
	SetPlotAttribute(dc.fid, dc.fgraph, uidc.fplotids[c], ATTR_PLOT_YDATA, data);
	
	free(data);
}

void change_fid_gain_safe(int num) {
	CmtGetLock(lock_uidc);
	change_fid_gain(num);
	CmtReleaseLock(lock_uidc);
}

void change_fid_offset(int num) {
	// Updates the gain for the FID.
	
	if(num < 0 || num >= 8)
		return;
	
	int c = num;
	
	// Get the old gain
	float oldoff = uidc.foff[c];
	GetCtrlVal(dc.fid, dc.fcoffset, &uidc.foff[c]);
	
	if(uidc.fplotids[c] < 0)  // If we don't need to update the plot, we're good
		return;
	
	// Get the plot data
	int np, i;
	GetPlotAttribute(dc.fid, dc.fgraph, uidc.fplotids[c], ATTR_NUM_POINTS, &np);
	
	double *data = malloc(sizeof(double)*np);
	double oc = uidc.foff[c] - oldoff; // Offset change
	
	GetPlotAttribute(dc.fid, dc.fgraph, uidc.fplotids[c], ATTR_PLOT_YDATA, data);
	
	// Alter the gain.
	for(i = 0; i < np; i++) {
		data[i] += oc;		
	}
	
	// Update the plot.
	SetPlotAttribute(dc.fid, dc.fgraph, uidc.fplotids[c], ATTR_PLOT_YDATA, data);
	
	free(data);
	
}

void change_fid_offset_safe(int num) {
	CmtGetLock(lock_uidc);
	change_fid_offset(num);
	CmtReleaseLock(lock_uidc);
}

void change_spec_gain(int num) {
	// Updates the gain for the Spectrum.
	if(num < 0 || num >= 8) { return; }
	
	int c = num;
	
	// Get the old gain
	float oldgain = uidc.sgain[c];
	GetCtrlVal(dc.spec, dc.scgain, &uidc.sgain[c]);
	
	if(uidc.splotids[c][0] < 0 || uidc.splotids[c][1] < 0 || uidc.splotids[c][2] < 0)  // If we don't need to update the plot, we're good
		return;
	
	// Get the plot data
	int np, i;
	GetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[c][0], ATTR_NUM_POINTS, &np);
	
	double **data = malloc(sizeof(double*)*3);
	double gc = uidc.sgain[c]/oldgain; // Gain change
	
	for(i = 0; i < 3; i++) {
		data[i] = malloc(sizeof(double)*np);
		GetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[c][i], ATTR_PLOT_YDATA, data[i]);
	}
	
	// Alter the gain.
	for(i = 0; i < np; i++) {
		data[0][i] *= gc;
		data[1][i] *= gc;
		data[2][i] *= gc;
	}
	
	// Update the plot
	for(i = 0; i < 3; i++) {
		SetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[c][i], ATTR_PLOT_YDATA, data[i]);
	 	free(data[i]);
	}
	free(data);
}

void change_spec_gain_safe(int num) {
	CmtGetLock(lock_uidc);
	change_spec_gain(num);
	CmtReleaseLock(lock_uidc);
}

void change_spec_offset(int num) {
	// Updates the gain for the Spectrum.
	if(num < 0 || num >= 8)
		return;
	
	int c = num;
	
	// Get the old gain
	float oldoff = uidc.soff[c];
	GetCtrlVal(dc.spec, dc.scoffset, &uidc.soff[c]);
	
	if(uidc.splotids[c][0] < 0 || uidc.splotids[c][1] < 0 || uidc.splotids[c][2] < 0)  // If we don't need to update the plot, we're good
		return;
	
	// Get the plot data
	int np, i;
	GetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[c][0], ATTR_NUM_POINTS, &np);
	
	if(np < 1)
		return;
	
	double **data = malloc(sizeof(double*)*3);
	double oc = uidc.soff[c]-oldoff; // Gain change
	for(i = 0; i < 3; i++) {
		data[i] = malloc(sizeof(double)*np);
		GetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[c][i], ATTR_PLOT_YDATA, data[i]);   
	}
	
	// Alter the gain.
	for(i = 0; i < np; i++) {
		data[0][i] += oc;
		data[1][i] += oc;
		data[2][i] += oc;
	}
	
	// Update the plot
	for(i = 0; i < 3; i++) {
		SetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[c][i], ATTR_PLOT_YDATA, data[i]);
	 	free(data[i]);
	}
	free(data);
	
}

void change_spec_offset_safe(int num) {
	CmtGetLock(lock_uidc);
	change_spec_offset(num);
	CmtReleaseLock(lock_uidc);
}

void toggle_fid_chan(int num) {
	if(num < 0 || num >= 8) {
		return;
	}
	
	int on;
	GetCtrlVal(dc.fid, dc.fchans[num], &on);
	
	set_fid_chan(num, !on);
}

void toggle_fid_chan_safe(int num) {
	CmtGetLock(lock_uidc);
	toggle_fid_chan(num);
	CmtReleaseLock(lock_uidc);
}

void set_fid_chan(int num, int on) {
	// Turns whether or not a given channel is on on or off and
	// updates all the relevant controls.

	if(num < 0 || num >= 8 || uidc.fchans[num] == on)
		return;
	
	// Update UIDC
	uidc.fchans[num] = on;
	
	if(on) {
		uidc.fnc++;			// One more channel is on	
	} else {
		uidc.fnc--;			// One fewer channel is on
	}

	// Turn the data on or off
	int i; 
	if(uidc.fplotids[num] >= 0) {
		SetPlotAttribute(dc.fid, dc.fgraph, uidc.fplotids[num], ATTR_TRACE_COLOR, on?uidc.fcol[num]:VAL_TRANSPARENT);
	}
	
	update_chan_ring(dc.fid, dc.fcring, uidc.fchans);
	
	// Set the box's contents to dimmed when no channels are on.
	// Reverse it if any channels are on.
	int c_on = (uidc.fnc < 1);
	
	SetCtrlVal(dc.fid, dc.fchans[num], on);
	
	SetCtrlAttribute(dc.fid, dc.fccol, ATTR_DIMMED, c_on);
	SetCtrlAttribute(dc.fid, dc.fcgain, ATTR_DIMMED, c_on);
	SetCtrlAttribute(dc.fid, dc.fcoffset, ATTR_DIMMED, c_on);
	
	if(!c_on) {
		update_fid_chan_box(); // Update the values of the chan box.
	}
}

void set_fid_chan_safe(int num, int on) {
	CmtGetLock(lock_uidc);
	set_fid_chan(num, on);
	CmtReleaseLock(lock_uidc);
}


void toggle_spec_chan(int num) {
	int on;
	GetCtrlVal(dc.spec, dc.schans[num], &on);
	
	set_spec_chan(num, !on);
}


void toggle_spec_chan_safe(int num) {
	CmtGetLock(lock_uidc);		  
	toggle_spec_chan(num);
	CmtReleaseLock(lock_uidc);
}

void set_spec_chan(int num, int on) {
	// Turns whether or not a given spectrum channel is on or off and updates the relevant controls
	
	if(num < 0 || num >= 8 || uidc.schans[num] == on) { return; }
	
	/****** TODO *******
	 Once the data loading and such is done, this also needs to hide or show
	 the appropriate data, update the channel preferences, etc.
	 *******************/
	
	// Update UIDC
	uidc.schans[num] = on;
	
	if(on) {
		uidc.snc++;
	} else {
		uidc.snc--;
	}

	// Turn the data on or off
	int i; 
	if(uidc.schan >= 0 && uidc.schan < 3 && uidc.splotids[num][uidc.schan] >= 0) {
		SetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[num][uidc.schan], ATTR_TRACE_COLOR, on?uidc.scol[num]:VAL_TRANSPARENT);
		for(i = 0; i < 8; i++) {
			if(uidc.schans[i] && uidc.splotids[i][0] >= 0) {
				SetCtrlAttribute(dc.sfftring[1], dc.sfftring[0], ATTR_DIMMED, 0);
				break;
			}
		}
		
		if(i >= 8)
			SetCtrlAttribute(dc.sfftring[1], dc.sfftring[0], ATTR_DIMMED, 1);
	}
	
	// Now we want to update the channel ring
	update_chan_ring(dc.spec, dc.scring, uidc.schans);
	
	// Set the box's contents to dimmed when no channels are on.
	// Reverse it if any channels are on.
	int c_on = (uidc.snc < 1);
	
	SetCtrlVal(dc.spec, dc.schans[num], on);
	
	SetCtrlAttribute(dc.spec, dc.sccol, ATTR_DIMMED, c_on);
	SetCtrlAttribute(dc.spec, dc.scgain, ATTR_DIMMED, c_on);
	SetCtrlAttribute(dc.spec, dc.scoffset, ATTR_DIMMED, c_on);
	SetCtrlAttribute(dc.sphorder[1], dc.sphorder[0], ATTR_DIMMED, c_on);
	SetCtrlAttribute(dc.sphase[1], dc.sphase[0], ATTR_DIMMED, c_on);
	
	// This one only gets turned on if there's something to phase.
	for(i = 0; i < 8; i++) {
		if(uidc.splotids[i][0] >= 0) {
			SetCtrlAttribute(dc.sfftring[1], dc.sfftring[0], ATTR_DIMMED, c_on);
			break;
		}
	}
	
	if(!c_on) {
		update_spec_chan_box(); // Update the values of the chan box.
	}
}

void set_spec_chan_safe(int num, int on) {
	CmtGetLock(lock_uidc);
	set_spec_chan(num, on);
	CmtReleaseLock(lock_uidc);
}

void update_chan_ring(int panel, int ctrl, int chans[]) {
	// Deletes and re-populates a given channel ring.
	
	int nl, ind, oldval = -1, oldind = -1, i;
	char name[8];
	GetNumListItems(panel, ctrl, &nl);
	
	// Clear the list if necessary
	if(nl > 0) {
		GetCtrlVal(panel, ctrl, &oldval);
		GetCtrlIndex(panel, ctrl, &oldind);
		
		DeleteListItem(panel, ctrl, 0, -1);
	}
	
	// Populate the list now
	for(i = 0; i < 8; i++) {
		if(chans[i]) {
			sprintf(name, "Chan %d", i+1);
			InsertListItem(panel, ctrl, -1, name, i);
		}
	}
	
	GetNumListItems(panel, ctrl, &nl);
	if(nl <= 1) {
		SetCtrlAttribute(panel, ctrl, ATTR_VISIBLE, 0);
	} else {
		SetCtrlAttribute(panel, ctrl, ATTR_VISIBLE, 1);
	
		// Check if the old index/value are still there, keep it the same
		// If not, go to the old index, or the the closest to the old index. 
		GetIndexFromValue(panel, ctrl, &ind, oldval);
		if(ind >= 0) {
			SetCtrlIndex(panel, ctrl, ind);
		} else if(oldind < nl) {
			SetCtrlIndex(panel, ctrl, oldind);	
		} else {
			SetCtrlIndex(panel, ctrl, nl-1);
		}
	}
	
}

void update_fid_chan_box() {
	// Called after a change in dc.fchanring to update the values in the box
	
	int c;
	GetCtrlVal(dc.fid, dc.fcring, &c);
	
	if(c > 8 || c < 0)
		return; 	// Something's fucked up - freak the shit out.
	
	SetCtrlVal(dc.fid, dc.fccol, uidc.fcol[c]);
	SetCtrlVal(dc.fid, dc.fcgain, uidc.fgain[c]);
	SetCtrlVal(dc.fid, dc.fcoffset, uidc.foff[c]);
}

void update_fid_chan_box_safe() {
	CmtGetLock(lock_uidc);
	update_fid_chan_box();
	CmtReleaseLock(lock_uidc);
}

void update_spec_chan_box() {
	// Called after a change in dc.schanring to update the values in the box (gain, offset, etc)
	int c;
	GetCtrlVal(dc.spec, dc.scring, &c);
	
	if(c > 8 || c < 0)
		return; 	// Something's fucked up - freak the shit out.
	
	SetCtrlVal(dc.spec, dc.sccol, uidc.scol[c]);
	SetCtrlVal(dc.spec, dc.scgain, uidc.sgain[c]);
	SetCtrlVal(dc.spec, dc.scoffset, uidc.soff[c]);
	
	int phord;
	GetCtrlVal(dc.sphorder[1], dc.sphorder[0], &phord);
	if(phord < 0 || phord >= 3)
		return; 	// Something's wrong.
	
	SetCtrlVal(dc.sphase[1], dc.sphase[0], uidc.sphase[c][phord]);
}

void update_spec_chan_box_safe() {
	CmtGetLock(lock_uidc);
	update_spec_chan_box();
	CmtReleaseLock(lock_uidc);
}

void update_spec_fft_chan() {
	// Run this whenever the uidc.schan value has changed.
	
	if(uidc.schan < 0 || uidc.schan >= 3)
		return;
	
	int i, col[3] = {0, 0, 0};
	
	// Go through and turn on or off the plots as appropriate.
	for(i = 0; i < 8; i++) {
		col[uidc.schan] = uidc.schans[i];
		
		if(uidc.splotids[i][0] >= 0) {
			SetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[i][0], ATTR_TRACE_COLOR, col[0]?uidc.scol[i]:VAL_TRANSPARENT);
		}
		
		if(uidc.splotids[i][1] >= 0) {
			SetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[i][1], ATTR_TRACE_COLOR, col[1]?uidc.scol[i]:VAL_TRANSPARENT);
		}
		
		if(uidc.splotids[i][2] >= 0) {
			SetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[i][2], ATTR_TRACE_COLOR, col[2]?uidc.scol[i]:VAL_TRANSPARENT);
		}
	}			   
}

void update_spec_fft_chan_safe() {
	CmtGetLock(lock_uidc);
	update_spec_fft_chan();
	CmtReleaseLock(lock_uidc);
}

/********* Device Interaction Settings ********/

int get_current_fname (char *path, char *fname, int next, time_t *cdate) {
	// Output is "fname", which must be pre-allocated.
	// Passing NULL to path returns the length of the new filename.
	// This will NOT check if the new filename will be valid, clearly
	// 
	// If "next" is TRUE, passes the next available one. Otherwise it
	// passes the most recent one.
	//
	// Errors are:
	// -1: Path or fname missing
	// -2: Malformed path or fname
	// -3: Maximum number of filenames used
	//
	// Success returns 0 or positive;
	
	if(fname == NULL) { return -1; }
	char *ext = MCD_EXTENSION, *str_format = NULL, *npath = NULL;
	char *dstr = NULL;
	time_t d = time(cdate);
	
	int i, rv = 0;
	
	// Allocate space for the full pathname.
	int flen = strlen(fname);
	if(flen < 1) { rv = -2; goto error; }
	
	// Make sure there are no instances of special characters in the filename
	// and terminate any trailing '\' characters.
	for(i = 0; i < flen; i++) {
		if(fname[i] != '\\') { break; }	
	}
	
	if(i > 0) {
		int ld = flen-i+1; // Also copy the null bit
		memcpy(fname, fname+i, ld);
		flen = strlen(fname);
		if(flen < 1) { rv = -2; goto error; }
	}
	
	flen += MCD_FNAME_FIXED_LEN + 1; // Not including extension, but including null.
	
	if(path == NULL) { 
		rv =  flen;
		goto error;
	}
	
	npath = malloc(strlen(path)+flen+strlen(ext)+2);  // +2 is for the dot and file separator
	strcpy(npath, path);
	
	// Make sure that there's no path separator at the end of path
	// or at the beginning or end of fname.
	int nlen = strlen(path);
	for(i = nlen-1; i >= 0; i--) {
		if(npath[i] != '\\') {
			break;
		}
	}
	npath[i+1] = '\0';
	
	if(strlen(npath) < 1 || !FileExists(npath, NULL)) { rv = -2; goto error; }

	strcpy(path, npath);
	
	// Create the mechanism for generating filenames.
	str_format = malloc(strlen("%s.%s")+strlen(MCD_FNAME_FORMAT)+2);
	sprintf(str_format, "%%s\\%s.%%s", MCD_FNAME_FORMAT);
	
	struct tm *tptr = localtime(&d);
	dstr = malloc(MCD_FNAME_DATE_LEN+1);
	strftime(dstr, MCD_FNAME_DATE_LEN+1, MCD_FNAME_DATE_FORMAT, tptr);
	
	for(i = 0; i < 10000; i++) {
		sprintf(npath, str_format, path, dstr, fname, i, ext);
		if(!FileExists(npath, NULL))
			break;
	}
	
	if(i >= 10000) {
		rv = -3;
		goto error;
	}
	
	char *buff = malloc(flen);
	strcpy(buff, fname);
	sprintf(fname, MCD_FNAME_FORMAT, dstr, buff, i-((next)?0:1)); // The output    	
	
	free(buff);
	
	error:
	if(npath != NULL) { free(npath); }
	if(dstr != NULL) { free(dstr); }
	if(str_format != NULL) { free(str_format); }

	return rv; 
}

char *get_full_path(char *path, char *fname, char *ext, int *ev) {
	// Gets the full path from the fname. Output is malloced, so
	// must be freed. Input path cannot have a file separator
	// at the end.
	
	int rv = 0;
	char *npath = NULL;
	if(ext == NULL) {
		ext = MCD_EXTENSION; 
	}
	
	if(path == NULL || !file_exists(path)) { rv = MCD_ERR_NOPATH; goto error; }
	if(fname == NULL) { rv = MCD_ERR_NOFILENAME; goto error; }
	
	// Path + / + fname + . + ext + \0
	int nplen = strlen(path) + 1 + strlen(fname) + 1 +  strlen(ext) + 1;
	npath = malloc(nplen);
	
	sprintf(npath, "%s\\%s.%s", path, fname, ext);
	
	error:
	if(rv != 0 && npath != NULL) {
		free(npath);
		npath = NULL;
	}
	
	if(ev != NULL) { *ev = rv; }
	
	return npath;
}

int get_devices () {		
	// Updates the device ring control.
	int nd = 0, bs, i, len, anc = uipc.max_anum;
	int rv = 0;
	char *devices = NULL, *device_name = NULL, *old_dev = NULL, **old_aodevs = NULL;
	
	bs = DAQmxGetSystemInfoAttribute(DAQmx_Sys_DevNames, "", NULL); // Returns max char size.
	
	// Store the old device name and index
	// First do the input devices
	int nl;
	GetNumListItems(pc.dev[1], pc.dev[0], &nl);
	if(nl > 0) {
		GetCtrlValStringLength(pc.dev[1], pc.dev[0], &len);
		old_dev = malloc(len+1);
		GetCtrlVal(pc.dev[1], pc.dev[0], old_dev);
		
		GetCtrlIndex(pc.dev[1], pc.dev[0], &uidc.devindex);
	}
	
	DeleteListItem(pc.dev[1], pc.dev[0], 0, -1);	// Delete devices if they exist.

	// Now the output devices.
	if(anc && uipc.ao_devs != NULL && uipc.adev_true != NULL) {
		int ind;
		old_aodevs = malloc(anc*sizeof(char*));
		
		for(i = 0; i < anc; i++) {
			ind = uipc.ao_devs[i];
			if(ind >= 0 && ind < uipc.anum_devs) {
				old_aodevs[i] = malloc(strlen(uipc.adev_true[ind])+1);
				strcpy(old_aodevs[i], uipc.adev_true[ind]);
			} else {
				old_aodevs[i] = NULL;
			}
		}
	}
	
	uipc.adev_display = free_string_array(uipc.adev_display, uipc.anum_devs);
	uipc.adev_true = free_string_array(uipc.adev_true, uipc.anum_devs);
	
	uipc.anum_devs = 0;
	
	if(bs <= 0) {
		rv = -1; // No devices found.
		goto error;
	}
	
	// Get the actual device names
	devices = malloc(bs);
	int *output = calloc(3, sizeof(int)), aoind;
	
	DAQmxGetSystemInfoAttribute(DAQmx_Sys_DevNames, devices, bs);

	// Figure out how many devices are available while populating the ring
	char *p = devices, *p2 = p;	// Pointer for searching.
	device_name = malloc(bs);
	
	while(p != NULL) { 
		nd++;			// There's 1 more than there are commas.
		p2 = strchr(p, ',');
		if(p2 == NULL)
			len = strlen(devices)-(p-devices)+1;
		else
			len = p2-p+1;
		
		strncpy(device_name, p, len);	// Copy over the name
		device_name[len-1] = '\0';		// Null terminate the string
		
		InsertListItem(pc.dev[1], pc.dev[0], -1, device_name, device_name); // Add it to the ring
		
		DAQmxGetDeviceAttribute(device_name, DAQmx_Dev_AO_SupportedOutputTypes, output, 3);
		
		// If it supports voltage output, add it to the list of devices.
		for(i = 0; i < 3; i++) {
			if(output[i] == DAQmx_Val_Voltage) {
				aoind = uipc.anum_devs++;
				uipc.adev_true = realloc(uipc.adev_true, uipc.anum_devs*sizeof(char*));
				uipc.adev_true[aoind] = malloc(len);
				strcpy(uipc.adev_true[aoind], device_name);
				output[i] = 0;
				break;
			} else if(output[i] == 0) {
				break;
			} else {
				output[i] = 0;
			}
		}

		
		p = (p2 == NULL)?NULL:p2+2; // Increment, making sure we don't go past the end.
	}
	
	free(device_name);
	free(output);
	
	// Get the index as close to what it used to be as we can.
	int default_adev = (uipc.anum_devs >0)?0:-1;
	
	if(nd >= 1) {
		if(old_dev != NULL) {
			int ind;
			GetIndexFromValue(pc.dev[1], pc.dev[0], &ind, old_dev);
			if(ind >= 0)
				uidc.devindex = ind;
		}
		
		if(uidc.devindex >= nd)
			uidc.devindex = nd-1;
		
		SetCtrlIndex(pc.dev[1], pc.dev[0], uidc.devindex);
		
		// Make the selected device default.
		if(uipc.anum_devs > 0) { 
			GetValueLengthFromIndex(pc.dev[1], pc.dev[0], uidc.devindex, &len);
			char *cdev = malloc(len+1);
			GetValueFromIndex(pc.dev[1], pc.dev[0], uidc.devindex, cdev);
			
			int ind = string_in_array(uipc.adev_true, cdev, uipc.anum_devs);
			free(cdev);
			if(ind >= 0) { default_adev = ind; }
		}
	} else {
		MessagePopup("Error", "No devices available!\n");
		rv = -1;
	}
	
	// At the moment, true and display are the same, so we'll just copy it over.
	if(uipc.anum_devs > 0) {
		uipc.adev_display = malloc(sizeof(char*)*uipc.anum_devs);
		for(i = 0; i < uipc.anum_devs; i++) {
			uipc.adev_display[i] = malloc(strlen(uipc.adev_true[i])+1);
			strcpy(uipc.adev_display[i], uipc.adev_true[i]);
		}
	}

	// Try and get the channels where they should be.
	uipc.default_adev = default_adev;
	int *old_aoinds = NULL;
	
	if(uipc.ao_devs != NULL) { 
		old_aoinds = malloc(sizeof(int)*anc);
		memcpy(old_aoinds, uipc.ao_devs, sizeof(int)*anc);
	}
	
	if(old_aodevs != NULL) {
		if(uipc.anum_devs > 0) {
			int *spots = strings_in_array(uipc.adev_true, old_aodevs, uipc.anum_devs, anc);
			
			for(i = 0; i < anc; i++) {
				if(spots != NULL && spots[i] >= 0) {
					uipc.ao_devs[i] = spots[i];
				} else {
					// We can't reliably use the channel in this case.
					if(uipc.ao_chans != NULL) {
						uipc.ao_chans[i] = -1;
					}
					
					if (old_aoinds != NULL) {
						if(old_aoinds[i] < anc) {
							uipc.ao_devs[i] = old_aoinds[i];
						} else {
							uipc.ao_devs[i] = anc-1;	
						}
					}
				}
			}
			
			free(spots);
		}   
	} else if(anc > 0) {
		if(uipc.ao_devs == NULL) { uipc.ao_devs = malloc(sizeof(int)*anc); }
		
		for(i = 0; i < anc; i++) {
			uipc.ao_devs[i] = default_adev;	
		}
	}
	
	if(old_aoinds != NULL) { free(old_aoinds); }

	error:
	if(devices != NULL) { free(devices); }
	if(old_dev != NULL) { free(old_dev); }
	
	if(old_aodevs != NULL) { free_string_array(old_aodevs, anc); }
	
	load_AO_info();
	
	return rv;
}

int get_devices_safe() {
	CmtGetLock(lock_uidc);
	CmtGetLock(lock_uipc);
	CmtGetLock(lock_DAQ);
	
	int rv = get_devices();
	
	CmtReleaseLock(lock_DAQ);
	CmtReleaseLock(lock_uipc);
	CmtReleaseLock(lock_uidc);
	
	return rv;
}

int load_AO_info() {
	// Populates the uipc with information from the DAQs and such.
	// This should probably be called in load_DAQ_info().
	
	int rv = 0, nl, len, i, j, anc = uipc.max_anum;
	char **old_ao_chans = NULL;
	
	// Store old string values from analog outputs
	if(anc > 0 && uipc.ao_chans != NULL && uipc.ao_all_chans != NULL) {
		int ind, dev;
		
		old_ao_chans = malloc(sizeof(char*)*anc);
		for(i = 0; i < anc; i++) {
			dev = uipc.ao_devs[i];
			ind = uipc.ao_chans[i];
			old_ao_chans[i] = NULL;
			
			if(dev < 0) { 
				continue;
			}
			
			if(ind >= 0 && ind < uipc.anum_all_chans[dev]) {
				old_ao_chans[i] = malloc(strlen(uipc.ao_all_chans[dev][ind])+1);
				strcpy(old_ao_chans[i], uipc.ao_all_chans[dev][ind]);
			}
		}
	}
	
	// Clear old values (will refresh later)
	uipc.ao_avail_chans = free_ints_array(uipc.ao_avail_chans, uipc.anum_devs);
	
	if(uipc.ao_all_chans != NULL) {
		for(i = 0; i < uipc.anum_devs; i++) {
			uipc.ao_all_chans[i] = free_string_array(uipc.ao_all_chans[i], uipc.anum_all_chans[i]);
		}
		
		free(uipc.ao_all_chans);
		uipc.ao_all_chans = NULL;
	}
	
	if(uipc.anum_all_chans != NULL) { 
		free(uipc.anum_all_chans);
		uipc.anum_all_chans = NULL;
	}
	
	if(uipc.anum_avail_chans != NULL) { 
		free(uipc.anum_avail_chans);
		uipc.anum_avail_chans = NULL;
	}

	if(uipc.anum_devs < 1)
		goto error;
	
	// Build the arrays uipc.ao_all_chans and uipc.ao_avail_chans. 
	// Null initialize some new arrays.
	uipc.ao_all_chans = calloc(uipc.anum_devs, sizeof(char*));
	uipc.ao_avail_chans = calloc(uipc.anum_devs, sizeof(int*));
	uipc.anum_all_chans = calloc(uipc.anum_devs, sizeof(int));
	uipc.anum_avail_chans = calloc(uipc.anum_devs, sizeof(int));

	int buff_size, c_size = 0, nc, devlen;
	char *chans = NULL, *chan_name = NULL;
	char *p = NULL, *p2 = NULL;
	

	// This is the bit that gets the channels.
	for(i = 0; i < uipc.anum_devs; i++) {
		if(uipc.adev_true[i] == NULL)
			continue;

		// This figures out how much to allocate from the list.
		buff_size = DAQmxGetDeviceAttribute(uipc.adev_true[i], DAQmx_Dev_AO_PhysicalChans, "", NULL);
		
		if(buff_size++ < 1)
			continue;
		
		if(chans == NULL) {
			chans = malloc(buff_size);
		} else if (c_size < buff_size) {
			chans = realloc(chans, buff_size);
			c_size = buff_size;
		}
		
		// Now parse the string. It should be a comma-delimited list.
		DAQmxGetDeviceAttribute(uipc.adev_true[i], DAQmx_Dev_AO_PhysicalChans, chans, buff_size);
		devlen = strlen(uipc.adev_true[i])+1; // Offset to strip out the device name.
		
		p = strtok(chans, ", ");
		nc = 0;
		while(p != NULL) {
			if(uipc.ao_all_chans[i] == NULL) {
				uipc.ao_all_chans[i] = malloc((nc+1)*sizeof(char*));
			} else { 
				uipc.ao_all_chans[i] = realloc(uipc.ao_all_chans[i], (nc+1)*sizeof(char*));
			}
			
			chan_name = p+devlen;
			
			uipc.ao_all_chans[i][nc] = malloc(strlen(chan_name)+1);
			strcpy(uipc.ao_all_chans[i][nc], chan_name);
			
			nc++;
			p = strtok(NULL, ", ");
		}
		
		uipc.anum_all_chans[i] = nc;
	}
	
	// Now we want to start allocating channels to the existing channels.
	uipc.anum_avail_chans = memcpy(uipc.anum_avail_chans, uipc.anum_all_chans, sizeof(int)*uipc.anum_devs);
	int *aac = uipc.anum_avail_chans;
	int **oac = uipc.ao_avail_chans;
	
	// Initialize the available chans to show all chans available.
	for(i = 0; i < uipc.anum_devs; i++) {
		oac[i] = malloc(aac[i]*sizeof(int*));
		for(j = 0; j < aac[i]; j++) {
			oac[i][j] = j;
		}
	}
	
	if(anc >= 1) {
		if(uipc.ao_chans == NULL)
			uipc.ao_chans = malloc(anc*sizeof(int));
		
		uipc.ao_chans = memset(uipc.ao_chans, -1, sizeof(int)*anc);
		
		if(old_ao_chans != NULL) {
			int spot, dev, avail, nac = 0;
			for(i = 0; i < anc; i++) {
				if(old_ao_chans[i] == NULL)
					continue;
			
				dev = uipc.ao_devs[i];
				spot = string_in_array(uipc.ao_all_chans[dev], old_ao_chans[i], uipc.anum_all_chans[i]);
			
				if(spot >= 0) {
					avail = int_in_array(oac[dev], spot, aac[dev]);
				
					if(avail >= 0) {
						uipc.ao_chans[i] = spot;
					
						if(i < uipc.anum) { 
							remove_array_item(uipc.ao_avail_chans[dev], avail, aac[dev]--);
						}
					} else if(aac[dev] > 0) {
						uipc.ao_chans[i] = oac[dev][aac[dev]-1];
					
						if(i < uipc.anum) {
							remove_array_item(uipc.ao_avail_chans[dev], aac[dev]-1, aac[dev]--); 
						}
					
					}
				}
			}
		}
	}
	
	// Reallocate the space for ao_avail_chans. This can only ever go down in size.
	for(i = 0; i < uipc.anum_devs; i++) {
		if(uipc.ao_avail_chans[i] != NULL) { 
			if(aac[i] < 1) {
				free(uipc.ao_avail_chans[i]);
				uipc.ao_avail_chans[i] = NULL;
			} else {
				uipc.ao_avail_chans[i] = realloc(uipc.ao_avail_chans[i], aac[i]*sizeof(int));
			}
		}
	}
	
	// Finally, let's refresh the analog outputs.
	int ind, dev, cind;
	for(i = 0; i < anc; i++) {
		populate_ao_dev(i);
		populate_ao_chan(i);
	}
	
	error:
	if(old_ao_chans != NULL) { free_string_array(old_ao_chans, anc); }
	if(chans != NULL) { free(chans); }
	
	return rv;
}

int load_AO_info_safe(void) {
	CmtGetLock(lock_DAQ);
	CmtGetLock(lock_uipc);
	int rv = load_AO_info();
	CmtReleaseLock(lock_uipc);
	CmtReleaseLock(lock_DAQ);

	return rv;
 }

int load_DAQ_info() {
	// Loads information about the DAQ to the UI controls.
	// Return values are errors from function calls, mostly.
	//
	// Otherwise:
	// -2: No devices found.
	
	// Variable declarations
	int rv = 0, nl, len, old_ti = 0, old_ci = 0, curr_chan, i;
	char *old_trig = NULL, *old_count = NULL;

	// Get the previous values of what we need if it's not stored in uidc.
	// Get the current channel index.
	GetCtrlIndex(pc.curchan[1], pc.curchan[0], &curr_chan);
	
	// First the trigger
	GetNumListItems(pc.trigc[1], pc.trigc[0], &nl);
	if(nl) {
		GetCtrlValStringLength(pc.trigc[1], pc.trigc[0], &len);
		old_trig = malloc(len+1);
		GetCtrlVal(pc.trigc[1], pc.trigc[0], old_trig);
		GetCtrlIndex(pc.trigc[1], pc.trigc[0], &old_ti);
	}
	
	// Finally the counter channel.
	GetNumListItems(pc.cc[1], pc.cc[0], &nl);
	if(nl) {
		GetCtrlValStringLength(pc.cc[1], pc.cc[0], &len);
		old_count = malloc(len+1);
		GetCtrlVal(pc.cc[1], pc.cc[0], old_count);
		GetCtrlIndex(pc.cc[1], pc.cc[0], &old_ci);
	}
	
	// Delete all the lists now.
	DeleteListItem(pc.ic[1], pc.ic[0], 0, -1);
	DeleteListItem(pc.trigc[1], pc.trigc[0], 0, -1);
	DeleteListItem(pc.cc[1], pc.cc[0], 0, -1);
	DeleteListItem(pc.curchan[1], pc.curchan[0], 0, -1);
 
	// Load devices.
	if(get_devices() < 0) {
		rv = -1;
		nl = 0;
	} else {
		GetNumListItems(pc.dev[1], pc.dev[0], &nl);
	}
	
	if(nl < 1){
		rv = -2;
		// Dim some controls, y'all.
		SetCtrlAttribute(pc.trigc[1], pc.trigc[0], ATTR_DIMMED, 1);
		SetCtrlAttribute(pc.trige[1], pc.trige[0], ATTR_DIMMED, 1);
		SetCtrlAttribute(pc.curchan[1], pc.curchan[0], ATTR_DIMMED, 1);
		SetCtrlAttribute(pc.range[1], pc.range[0], ATTR_DIMMED, 1);
		
		if (old_trig != NULL) { free(old_trig); }
		if (old_count != NULL) { free(old_count); }
		return rv;
	}
	
	// Get the current device name
	int nc = 0, devlen;
	GetCtrlValStringLength(pc.dev[1], pc.dev[0], &devlen);
	char *device = malloc(++devlen);
	GetCtrlVal(pc.dev[1], pc.dev[0], device);
	
	int bsi = DAQmxGetDeviceAttribute(device, DAQmx_Dev_AI_PhysicalChans, "", NULL); // Length of result
	
	// Get the list of input channels.
	char *input_chans = malloc(bsi), *channel_name = NULL;
	DAQmxGetDeviceAttribute(device, DAQmx_Dev_AI_PhysicalChans, input_chans, bsi);
	
	// Find out how many channels there are going to be.
	char *p = input_chans, *p2;
	while(p != NULL && nc < 1024) { 	// Max 1024 channels! (Protection against infinite loops)
		nc++;
		p = strchr(++p, ',');
	}
	
	// Change the uidc variable as appropriate.
	if(nc != uidc.nchans) {
		if(uidc.chans == NULL) {
			uidc.chans = malloc(sizeof(int)*nc);
			uidc.nchans = 1;
			uidc.chans[0] = 1;	// Turn on one channel.
		} else
			uidc.chans = realloc(uidc.chans, sizeof(int)*nc);
		
		for(i = uidc.nchans; i < nc; i++)
			uidc.chans[i] = 0;
		
		uidc.nchans = nc;
	}
	
	// Build an array of the names of the input channels
	// Note: channel names are Device/Channel in this case, so we need to offset by devlen
	// for UI purposes, otherwise the ring controls would look crowded.
	uidc.onchans = 0;
	channel_name = malloc(bsi);
	char *buff_string = malloc(bsi);
	p = p2 = input_chans;
	
	for(i = 0; i < nc; i++) {
		p2 = strchr(p, ',');
		if(p2 == NULL) {
			len = strlen(input_chans)-(p-input_chans)+2;
		} else { 
			len = (p2 - p)+2;
		}
		
		// The channel names either have a trailing space or a mark.
		channel_name[0] = uidc.chans[i]?149:' '; 	// If it's on, char 149 is a mark
		strncpy(buff_string, p, len-2);
		strncpy(&channel_name[1], &p[devlen], len-devlen-2);
		channel_name[len-devlen-1] = '\0';								// Null terminate the string.
		buff_string[len-2] = '\0';
		
		InsertListItem(pc.ic[1], pc.ic[0], -1, channel_name, buff_string);
		
		if(uidc.chans[i]) {
			InsertListItem(pc.curchan[1], pc.curchan[0], -1, &buff_string[devlen], i);
			uidc.onchans++;
		}
		
		
		p = (p2 == NULL)?NULL:p2+2;	// Increment if necessary. Skip the ", "
	}
	
	// Fix up the current channel setup while we're at it.
	if(uidc.onchans) {
		GetNumListItems(pc.curchan[1], pc.curchan[0], &nl);
		if(curr_chan < 0) {
			curr_chan = 0;
		}
		
		SetCtrlIndex(pc.curchan[1], pc.curchan[0], (curr_chan>=uidc.onchans)?uidc.onchans-1:curr_chan);
	}
	
	SetCtrlAttribute(pc.curchan[1], pc.curchan[0], ATTR_DIMMED, uidc.onchans?0:1);
	SetCtrlAttribute(pc.range[1], pc.range[0], ATTR_DIMMED, uidc.onchans?0:1);
	free(input_chans);	// Don't need this anymore
	
	// Get a list of all the trigger channels, which are the digital DAQmx_Dev_Terminals
	int bst = DAQmxGetDeviceAttribute(device, DAQmx_Dev_Terminals, "", 0);
	
	char *trig_chans = malloc(bst);
	DAQmxGetDeviceAttribute(device, DAQmx_Dev_Terminals, trig_chans, bst);
	
	// We're not going to bother with an array this time, we'll just read it right into the ring
	p = p2 = trig_chans;
	devlen++; 	// For some reason, the trigger channels are all /Device/, not Device/
	if(bst > bsi) {
		channel_name = realloc(channel_name, bst);
		buff_string = realloc(buff_string, bst);
	}
	
	while(p != NULL) {
		p2 = strchr(p, ',');	// Find the next comma.
		if(p2 == NULL)
			len = strlen(trig_chans)-(p-trig_chans)+1;
		else
			len = p2-p+1;
		
		strncpy(channel_name, &p[devlen], len-devlen);
		strncpy(buff_string, p, len);
		channel_name[len-devlen-1] = '\0';
		buff_string[len-1] = '\0';
		
		InsertListItem(pc.trigc[1], pc.trigc[0], -1, channel_name, buff_string);
		
		p = (p2 == NULL)?NULL:p2+2;
	}
	
	int dimmed = (trig_chans == NULL)?1:0;
	SetCtrlAttribute(pc.trigc[1], pc.trigc[0], ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pc.trige[1], pc.trige[0], ATTR_DIMMED, dimmed);
	
	// Set it to as close to the previous value as we can get
	int ind;
	GetNumListItems(pc.trigc[1], pc.trigc[0], &nl);
	if(nl && old_trig != NULL) {
		GetIndexFromValue(pc.trigc[1], pc.trigc[0], &ind, old_trig);
		if(ind >= 0)
			SetCtrlIndex(pc.trigc[1], pc.trigc[0], ind);
		else if (old_ti >= nl)
			SetCtrlIndex(pc.trigc[1], pc.trigc[0], nl-1);
		else
			SetCtrlIndex(pc.trigc[1], pc.trigc[0], old_ti);
	}
	
	if(old_trig != NULL) { free(old_trig); }
	free(trig_chans);
	
	// Now we move on to the counter channels, basically the same deal as before.
	int bsc = DAQmxGetDeviceAttribute(device, DAQmx_Dev_CO_PhysicalChans, "", 0);
	
	char *counter_chans = malloc(bsc);
	DAQmxGetDeviceAttribute(device, DAQmx_Dev_CO_PhysicalChans, counter_chans, bsc);
	
	if(bsc > bst) {
		channel_name = realloc(channel_name, bsc);
		buff_string = realloc(buff_string, bsc);
	}
	
	devlen--; // Now we're back to Device/Channel.
	p = p2 = counter_chans;
	while(p != NULL) {
		p2 = strchr(p, ',');
		if(p2 == NULL) {
			len = strlen(counter_chans)-(p-counter_chans)+1;
		} else {
			len = p2-p+1;
		}
		
		strncpy(buff_string, p, len);
		strncpy(channel_name, &p[devlen], len-devlen);
		channel_name[len-devlen-1] = '\0';
		buff_string[len-1] = '\0';
		
		InsertListItem(pc.cc[1], pc.cc[0], -1, channel_name, buff_string);
		
		p = (p2 == NULL)?NULL:p2+2;
	}
	
	// Again, set it as close to the previous value as we can get.
	GetNumListItems(pc.cc[1], pc.cc[0], &nl);
	if(nl && old_count != NULL) {
		GetIndexFromValue(pc.cc[1], pc.cc[0], &ind, old_count);
		if(ind >= 0) {
			SetCtrlIndex(pc.cc[1], pc.cc[0], ind);
		} else if (old_ci >= nl) {
			SetCtrlIndex(pc.cc[1], pc.cc[0], nl-1);
		} else {
			SetCtrlIndex(pc.cc[1], pc.cc[0], old_ci);
		}
	}
	
	if(old_count != NULL) { free(old_count); }
	free(counter_chans);
	free(channel_name);
	free(buff_string);
	
	// Make sure that the trigger edge attributes are set up correctly.
	ReplaceListItem(pc.trige[1], pc.trige[0], 0, "Rising", DAQmx_Val_Rising);
	ReplaceListItem(pc.trige[1], pc.trige[0], 1, "Falling", DAQmx_Val_Falling);
	
	free(device);
	return 0;
}

int load_DAQ_info_safe(int UIDC_lock, int UIPC_lock, int DAQ_lock) {
	if(UIDC_lock) { CmtGetLock(lock_uidc); }
	if(UIPC_lock) { CmtGetLock(lock_uipc); }
	if(DAQ_lock) { CmtGetLock(lock_DAQ); }
	
	int rv = load_DAQ_info();
	
	if(DAQ_lock) { CmtReleaseLock(lock_DAQ); }
	if(UIPC_lock) { CmtReleaseLock(lock_uipc); }
	if(UIDC_lock) { CmtReleaseLock(lock_uidc); }
	
	return rv;
}

int toggle_ic_ind(int ind) {
	// Does the back-end work for toggling on a given channel.
	int nl, rv = 0;
	
	char *label = NULL;
	char *value = NULL;
	
	GetNumListItems(pc.ic[1], pc.ic[0], &nl);
	if(ind < 0 || ind >= nl) {
		rv = MCD_ERR_NOINPUTCHANS;
		goto error;
	}
	
	// Get the old label and value   
	int lablen, len;
	GetLabelLengthFromIndex(pc.ic[1], pc.ic[0], ind, &lablen);
	GetValueLengthFromIndex(pc.ic[1], pc.ic[0], ind, &len);
	
	label = malloc(lablen+1);
	value = malloc(len+1);
	
	GetLabelFromIndex(pc.ic[1], pc.ic[0], ind, label);
	GetValueFromIndex(pc.ic[1], pc.ic[0], ind, value);
	
	label[0] = uidc.chans[ind]?' ':149; // 149 is a mark
	
	// Replace the list item to replace the label
	ReplaceListItem(pc.ic[1], pc.ic[0], ind, label, value);
	
	if(uidc.chans[ind]) {
		remove_chan(ind);
	} else {
		add_chan(&label[1], ind);
	}
	
	// Update the uidc var now.
	uidc.chans[ind] = !uidc.chans[ind];		// Toggle its place in the var
	
	error:
	
	if(label != NULL) { free(label); }
	if(value != NULL) { free(value); }
	
	return rv;
}

int toggle_ic_ind_safe(int ind) {
	CmtGetLock(lock_uidc);
	int rv = toggle_ic_ind(ind);
	CmtReleaseLock(lock_uidc);
	
	return rv;
}

void toggle_ic() {
	// Toggles a given input channel. If you are turning it off, this
	// function will jump you back to the previous location. If the previous
	// location is the same as the new location, we'll jump to the first
	// known index.
	
	// Find out what it is and what it used to be.
	int val, oval;
	GetCtrlIndex(pc.ic[1], pc.ic[0], &val);
	GetCtrlAttribute(pc.ic[1], pc.ic[0], ATTR_DFLT_INDEX, &oval);
	
	if(oval < 0) {
		oval = 0;
		SetCtrlAttribute(pc.ic[1], pc.ic[0], ATTR_DFLT_INDEX, 0);	
	}
	
	// Check if we can turn this on or not - if so do the back end
	// On error in the back-end, jump back, clearly.
	if(!uidc.chans[val] && uidc.onchans == 8 || toggle_ic_ind(val)) {
		SetCtrlIndex(pc.ic[1], pc.ic[0], oval); // Jump back to where we were
		return;
	}
	
	// Now we're going to insert into the current channels ring in the right place
	// Either delete the val when we find it, or insert this before the first one
	// that's bigger than it.
	int i, ind;

	// Finally, we'll do the bit where we set this to the right value.
	if(!uidc.chans[val]) {
		if(oval != val) {
			SetCtrlIndex(pc.ic[1], pc.ic[0], oval);
			val = oval;
		} else if (uidc.onchans > 0) {
			for(i = val-1; i >= 0; i--) {
				if(uidc.chans[i])
					break;
			}
			
			if(i < 0) {
				for(i = val+1; i < uidc.nchans; i++) {
					if(uidc.chans[i])
						break;
				}
			}
			val = i;
		}
	}
	
	SetCtrlVal(pc.nc[1], pc.nc[0], uidc.onchans);
	SetCtrlIndex(pc.ic[1], pc.ic[0], val);
	SetCtrlAttribute(pc.ic[1], pc.ic[0], ATTR_DFLT_INDEX, uidc.onchans?val:0);
	
	// If there are no channels left, dim the current chan and range controls
	int dimmed = (uidc.onchans > 0)?0:1;
	SetCtrlAttribute(pc.curchan[1], pc.curchan[0], ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pc.range[1], pc.range[0], ATTR_DIMMED, dimmed);
}

void toggle_ic_safe() {
	CmtGetLock(lock_uidc);
	toggle_ic();
	CmtReleaseLock(lock_uidc);
}

void add_chan(char *label, int val) {
	// Adds a channel to the current channel control in a sorted fashion.
	// This also increments uidc.onchans.
	
	// Find the first control that is bigger than this. That's where you add
	int i, ind;
	for(i = 0; i < uidc.onchans; i++) {
		GetValueFromIndex(pc.curchan[1], pc.curchan[0], i, &ind);
		if(ind > val) { break; }	
	}
	
	if(i >= uidc.onchans) {
		ind = -1;
		i = uidc.onchans;
	} else {
		ind = i;
	}
	
	InsertListItem(pc.curchan[1], pc.curchan[0], ind, label, val);
	
	// Now update the range controls
	for(int j = uidc.onchans; j > i; j--)
		uidc.range[j] = uidc.range[j-1];

	GetCtrlAttribute(pc.range[1], pc.range[0], ATTR_DFLT_VALUE, &uidc.range[i]);
	uidc.onchans++;
}

void add_chan_safe(char *label, int val) {
	CmtGetLock(lock_uidc);
	add_chan(label, val);
	CmtReleaseLock(lock_uidc);
}

void remove_chan(int val) {
	// Removes a channel from the current channels control and decrements onchans
	
	// First find us in the control.
	int i, ind;
	for(i = 0; i < uidc.onchans; i++) {
		GetValueFromIndex(pc.curchan[1], pc.curchan[0], i, &ind);
		if(ind == val) { break; }
	}
	
	if(i >= uidc.onchans) { return; }	// It's not there
	
	DeleteListItem(pc.curchan[1], pc.curchan[0], i, 1);
	
	// Move over all the range values
	for(i; i < uidc.onchans-1; i++) 
		uidc.range[i] = uidc.range[i+1];
	
	uidc.onchans--;
	
	change_chan();	// In case we deleted the current channel.
}

void remove_chan_safe(int val) {
	CmtGetLock(lock_uidc);
	remove_chan(val);
	CmtReleaseLock(lock_uidc);
}

void change_chan() {
	// Function called when you change the current channel.
	// Updates the range control with values from uidc.
	//
	// Single uidc readout call, no race condition, so it
	// doesn't have to be thread-safe by itself.
	
	int chan;
	GetCtrlIndex(pc.curchan[1], pc.curchan[0], &chan);
	
	if(chan >= 0 && chan < 8) { 
		SetCtrlVal(pc.range[1], pc.range[0], uidc.range[chan]);
	}
}

void change_range() {
	// Called whenever you change the range. Simply updates
	// the uidc variable with the new value.
	
	int chan;
	float val;
	GetCtrlIndex(pc.curchan[1], pc.curchan[0], &chan);
	GetCtrlVal(pc.range[1], pc.range[0], &val);
	
	if(chan >= 0 && chan < 8) {
		uidc.range[chan] = val;
	}
}

void change_range_safe() {
	CmtGetLock(lock_uidc);
	change_range();
	CmtReleaseLock(lock_uidc);
}

//////////////////////////////////////////////////////////////
// 															//
//				    Device Interaction						//	
// 															//
//////////////////////////////////////////////////////////////

int setup_DAQ_task() {
	// Given the current configuration of the UI, this sets up the DAQ task.
	// Two channels are created - a counter channel and an analog input channel
	// The counter channel generates a finite sequence of pulses upon receiving
	// a trigger, which is used as the clock for the analog input channel.
	
	int rv = 0;
	char *tname = NULL, *ic_name = NULL, *cc_name = NULL, *cc_out_name = NULL;
	char *tc_name = NULL;
	
	if(rv = clear_DAQ_task()) { goto error; }
	
	if(ce.p == NULL) {		// This must be set.
		rv = -1;
		goto error;
	}
	
	PPROGRAM *p = ce.p;			// Convenience.

	// First we create the tasks
	ce.atname = "AcquireTask";
	ce.ctname = "CounterTask";

	ce.aTask = NULL;
	ce.cTask = NULL;
	
	ce.atset = 0;
	ce.ctset = 0;
	
	if(rv = DAQmxCreateTask(ce.atname, &ce.aTask))  { goto error; }
	ce.atset = 1;	// We've created the counter task
	
	if(rv = DAQmxCreateTask(ce.ctname, &ce.cTask)) { goto error; }
	ce.ctset = 1;	// We've created the counter task
	
	// Convenience
	int np = p->np;
	double sr = p->sr;
	
	// Create the analog input voltage channels
	int i, ic, nc = uidc.onchans, insize = 25, len;
	tname = malloc(strlen("VoltInput00")+1);
	ic_name = malloc(insize);

	if(ce.icnames != NULL && ce.nchan > 0) {
		for(i = 0; i < ce.nchan; i++) { free(ce.icnames[i]); }
	}
	
	if(ce.icnames != NULL) { free(ce.icnames); }
	
	ce.nchan = nc;
	ce.icnames = malloc(sizeof(char*)*nc);
	
	for(i = 0; i < nc; i++) {
		GetValueFromIndex(pc.curchan[1], pc.curchan[0], i, &ic); // Channel index
		
		// Make sure there's enough room for the channel name.
		GetValueLengthFromIndex(pc.ic[1], pc.ic[0], ic, &len);
		if(len > insize) {
			insize = insize+len;
			ic_name = realloc(ic_name, insize);
		}
		
		ce.icnames[i] = malloc(sizeof(char)*insize+1);
		
		GetValueFromIndex(pc.ic[1], pc.ic[0], ic, ic_name); 	// Input channel name
		
		strcpy(ce.icnames[i], ic_name);
		
		sprintf(tname, "VoltInput%02d", i+1);	// Channel name in our task.
		
		// Create the channel
		if(rv = DAQmxCreateAIVoltageChan(ce.aTask, ic_name, tname, DAQmx_Val_Cfg_Default,
			(float64)(-uidc.range[i]), (float64)(uidc.range[i]), DAQmx_Val_Volts, NULL)) {
			goto error;
		}
	}
	
	// Create the counter channel
	GetCtrlValStringLength(pc.cc[1], pc.cc[0], &len);
	cc_name = malloc(len+1);
	cc_out_name = malloc(strlen("Ctr0InternalOutput")+1);
	
	// Figure out what the ouput channel is - I can't find a way to programmatically detect
	// this, so for now it's just a switch based on this specific application.
	GetCtrlVal(pc.cc[1], pc.cc[0], cc_name);

	if(strstr(cc_name, "ctr0") != NULL) {
		sprintf(cc_out_name, "Ctr0InternalOutput");
	} else if (strstr(cc_name, "ctr1") != NULL) { 
		sprintf(cc_out_name, "Ctr1InternalOutput");
	} else {
		MessagePopup("Counter Error", "Unsupported counter channel - choose ctr0 or ctr1\n\
						If these options do not exist, you are fucked. Sorry, mate.\n");
		rv = -150;
		goto error;
	}
	
	if(ce.ccname != NULL) { free(ce.ccname); }
	ce.ccname = cc_out_name;
	cc_out_name = NULL;
	
	/*****TODO*****
	Update this to work with windowed acquisitions - should be easy
	**************/

	// Create the output, when this task is started and triggered, it will generate
	// a train of pulses at frequency sr, until np pulses have occured.
	if(rv = DAQmxCreateCOPulseChanFreq(ce.cTask, cc_name, "Counter", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, sr, 0.5)) { goto error; }
	
	// Set the trigger for the counter and make it retriggerable
	int edge;
	GetCtrlValStringLength(pc.trigc[1], pc.trigc[0], &len);
	tc_name = malloc(len+1);
	
	GetCtrlVal(pc.trigc[1], pc.trigc[0], tc_name);	// Trigger channel name
	GetCtrlVal(pc.trige[1], pc.trige[0], &edge);	// Trigger edge (rising or falling)
	
	// Set up the trigger - use a 6.425 microsecond trigger.
	int ctype;
	if(rv = DAQmxCfgDigEdgeStartTrig(ce.cTask, tc_name, (int32)edge)) { goto error; }
	if(rv = DAQmxCfgImplicitTiming(ce.cTask, DAQmx_Val_FiniteSamps, np)) { goto error; }
	if(rv = DAQmxSetTrigAttribute(ce.cTask, DAQmx_StartTrig_Retriggerable, TRUE)) { goto error; }
	
	// Only available for digital inputs.
	if(strncmp(tc_name, "PFI", 3) == 0) { 
		if(rv = DAQmxSetTrigAttribute(ce.cTask, DAQmx_DigEdge_StartTrig_DigFltr_Enable, 1)) { goto error; }
		if(rv = DAQmxSetTrigAttribute(ce.cTask, DAQmx_DigEdge_StartTrig_DigFltr_MinPulseWidth, 6.425e-6)) { goto error; }
	}
	
	// Set the counter output channel as the clock foro the analog task
	
	if(rv = DAQmxCfgSampClkTiming(ce.aTask, ce.ccname, (float64)sr, DAQmx_Val_Rising, DAQmx_Val_ContSamps, np)) { goto error;}
	
	// Set the buffer for the input. Currently there is no programmatic protection against overruns
	if(rv = DAQmxSetBufferAttribute(ce.aTask, DAQmx_Buf_Input_BufSize, (np*nc)+1000)) { goto error; }  // Extra 1000 in case
	
	error:
	
	if(tname != NULL) { free(tname); }
	if(ic_name != NULL) { free(ic_name); }
	if(cc_name != NULL) { free(cc_name); }
	if(cc_out_name != NULL) { free(cc_out_name); }
	if(tc_name != NULL) { free(tc_name); }
	if(rv != 0) {
		if(ce.atset) { DAQmxClearTask(ce.aTask); }
		if(ce.ctset) { DAQmxClearTask(ce.cTask); }
	}
	return rv;
}

int setup_DAQ_task_safe(int DAQ_lock, int UIDC_lock, int CE_lock) {
	if(DAQ_lock) { CmtGetLock(lock_DAQ); }
	if(UIDC_lock) { CmtGetLock(lock_uidc); }
	if(CE_lock) { CmtGetLock(lock_ce); }
	
	int rv = setup_DAQ_task();
	
	if(CE_lock) { CmtReleaseLock(lock_ce);  }
	if(UIDC_lock) { CmtReleaseLock(lock_uidc); }
	if(DAQ_lock) { CmtReleaseLock(lock_DAQ);  }
	
	return rv;
}

int setup_DAQ_aouts() {
	// This will create the relevant analog output tasks. Currently this
	// is only set up to work with NI DAQs, when more devices are supported
	// the nature of this may need to change.
	//
	// There will be one task, ce.oTask, which contains all the relevant
	// analog output channels. Currently no triggering will be available
	//
	// Returns negative value on failure.
	// -1: ce.p is null.
	
	int rv = 0, *chans_set = NULL;
	
	if(ce.p == NULL) { return -1; }
	
	PPROGRAM *p = ce.p;
	if(!p->nAout) { return 0; }
	
	// Weed out the channels that aren't set.
	int i, dev = -1, chan = -1, nout = 0;
	chans_set = calloc(p->nAout, sizeof(int));
	
	for(i = 0; i < p->nAout; i++) {
		get_ao_dev_chan(p->ao_chans[i], &dev, &chan);
		
		if(dev >= 0 && chan >= 0) { 
			chans_set[i] = 1;
			nout++;
		}
	}
	
	ce.nochans = nout;
	
	if(nout < 1) { return 0; } // No channels on.
	
	// Start by creating the task.
	ce.otname = "AOutTask";
	ce.oTask = NULL;
	ce.otset = 0;
	
	if(rv = DAQmxCreateTask(ce.otname, &ce.oTask)) { goto error; }
	ce.otset = 1;	// Success!
	
	// Free the old memory if necessary
	if(ce.nochans > 0) {
		ce.ocnames = free_string_array(ce.ocnames, ce.nochans);	
	}

	if(ce.ochanson != NULL) { free(ce.ochanson); }
	ce.nochans = nout;
	
	// Create the analog output voltage channels.
	int steps, j;
	float64 val;
	int k = 0, len = strlen("AnalogOutput00")+1;
	ce.ochanson = malloc(sizeof(int)*nout);
	ce.ocnames = malloc(sizeof(char*)*nout);

	float64 min_val, max_val;
	
	for(i = 0; i < p->nAout; i++) {
		if(!chans_set[i]) { continue; }
		
		// Get the names
		ce.ochanson[k] = i;			   
		ce.ocnames[k] = malloc(len);
		sprintf(ce.ocnames[k], "AnalogOutput%02d", k);
		
		// Find minimum and maximum values.
		min_val = max_val = p->ao_vals[i][0];
		
		if(p->ao_dim[i] >= 0) {
			steps = p->maxsteps[p->nCycles+p->ao_dim[i]];
			for(j = 0; j < steps; j++) {
				val = p->ao_vals[i][j];
				
				// Since the float comparison operation is abelian,
				// it's not possible for the value to simultaneously 
				// be less than the minimum value and greater than 
				// the maximum value, so we don't have to check both.
				
				if(min_val > val) {
					min_val = val;	
				} else if(max_val < val) {
					max_val = val;	
				}
			}
		}
		
		// Replace this with a comparison to actual minimum and maximum values for device, not this hard-coded thing.
		if(min_val == max_val) {
			if(min_val > -9.99) { min_val -= 0.01; }
			if(max_val < 9.99) { max_val += 0.01; }
		}
		
		// Create the channel.
		if(rv = DAQmxCreateAOVoltageChan(ce.oTask, p->ao_chans[i], ce.ocnames[k],
				min_val, max_val, DAQmx_Val_Volts, NULL)) { goto error; }
			
		k++;
		
		if(k == nout) { break; }	// We're done.
	}
	
	error:
	
	if(rv != 0) {
		// Clear the task if we can.
		if(ce.otset) {
			if(!DAQmxClearTask(ce.oTask)) { 
				ce.otset = 0; 
				ce.oTask = NULL;
			}
		}
		
		if(ce.ochanson != NULL) { 
			free(ce.ochanson);
			ce.ochanson = NULL;
		}
		
		ce.ocnames = free_string_array(ce.ocnames, ce.nochans);
	
	}
	
	if(chans_set != NULL) { free(chans_set); }
	
	return rv;
}

int setup_DAQ_aouts_safe(int DAQ_lock, int UIPC_lock, int CE_lock) {
	if(DAQ_lock) { CmtGetLock(lock_DAQ); }
	if(CE_lock) { CmtGetLock(lock_ce); }
	if(UIPC_lock) { CmtGetLock(lock_uipc); }
	
	int rv = setup_DAQ_aouts();
	
	if(UIPC_lock) { CmtReleaseLock(lock_uipc); }
	if(CE_lock) { CmtReleaseLock(lock_ce); }
	if(DAQ_lock) { CmtReleaseLock(lock_DAQ); }
	
	return rv;
}

int update_DAQ_aouts() {
	// Update the DAQ aouts based on the ce function.
	//
	// Returns 0 on success, negative number on failure.
	// -1: Task not active.
	
	int rv = 0;
	
	if(!ce.nochans) { return 0; } // No channels
	if(!ce.otset || ce.oTask == NULL) { return -1; }
	
	PPROGRAM *p = ce.p;
	
	// If it's the first time, you need to create ce.ao_vals
	if(ce.ao_vals == NULL) {
		ce.ao_vals = malloc(ce.nochans*sizeof(float64)); 
		
		// Initialize this array.
		for(int i = 0; i < ce.nochans; i++) {
			ce.ao_vals[i] = (float64)p->ao_vals[ce.ochanson[i]][0];		
		}
	}
	
	// Update the ao_vals array
	if(p->n_ao_var) {
		int chan, step;
		for(int i = 0; i < ce.nochans; i++) {
			int chan = ce.ochanson[i];
			
			if(p->ao_varied[chan]) {
				step = ce.cstep[p->nCycles+p->ao_dim[chan]];
				
				ce.ao_vals[i] = (float64)p->ao_vals[chan][step];
			}
		}
	}
	
	// Write the ao_vals array to the DAQ.
	int32 ns = 0;
	if(rv = DAQmxWriteAnalogF64(ce.oTask, 1, 1, -1, DAQmx_Val_GroupByChannel, ce.ao_vals, &ns, NULL)) { goto error; } 
	
	error:
	
	return rv;
}

int update_DAQ_aouts_safe(int DAQ_lock, int CE_lock) {
	if(DAQ_lock) { CmtGetLock(lock_DAQ); } 
	if(CE_lock) { CmtGetLock(lock_ce); }
	
	int rv = update_DAQ_aouts();
	
	if(CE_lock) { CmtReleaseLock(lock_ce); }
	if(DAQ_lock) { CmtReleaseLock(lock_DAQ); }
	
	return rv;
}


int clear_DAQ_task() {
	int rv = 0, rv2;

	// Acquisition task
	if(ce.atset) {
		if(!(rv = DAQmxClearTask(ce.aTask))) {
			ce.atset = 0;
			ce.aTask = NULL;
		}
	}
	
	// Counter task
	if(ce.ctset) {
		rv2 = rv;
		if(!(rv = DAQmxClearTask(ce.cTask))) {
			ce.ctset = 0;
			ce.cTask = NULL;
			rv = rv2;
		}
	}
	
	// Analog outputs.
	if(ce.otset) {
		rv2 = rv;
		if(!(rv = DAQmxClearTask(ce.oTask))) { 
			ce.otset;
			ce.oTask = NULL;
			rv = rv2;
		}	
	}
	
	return rv;
}

int clear_DAQ_task_safe() {
	CmtGetLock(lock_DAQ);
	CmtGetLock(lock_ce);
	
	int rv = clear_DAQ_task();
	
	CmtReleaseLock(lock_ce);
	CmtReleaseLock(lock_DAQ);
	
	return rv;
}








