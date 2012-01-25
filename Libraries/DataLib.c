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
#include <cvixml.h>  
#include <NIDAQmx.h>
#include <toolbox.h>
#include <userint.h>
#include <ansi_c.h>
#include <spinapi.h>					// SpinCore functions

#include <PulseProgramTypes.h>
#include <cvitdms.h>
#include <UIControls.h>					// For manipulating the UI controls
#include <MathParserLib.h>				// For parsing math
#include <DataLib.h>
#include <MCUserDefinedFunctions.h>		// Main UI functions
#include <PulseProgramLib.h>			// Prototypes and type definitions for this file
#include <SaveSessionLib.h>

//////////////////////////////////////////////////////////////
// 															//
//				Running Experiment Functions				//
// 															//
//////////////////////////////////////////////////////////////
int CVICALLBACK IdleAndGetData (void *functionData) {
	// Main function running in an asynchronous thread for data acquisition, etc.
	
	// Start by getting the current program.
	PPROGRAM *p = get_current_program();
	
	// Run the experiment
	run_experiment(p);
	
	return 0;
}

void CVICALLBACK discardIdleAndGetData (int poolHandle, int functionID, unsigned int event, int value, void *callbackData)
{
	
	if(ce.ctset) {
		CmtGetLock(lock_DAQ);
		DAQmxClearTask(ce.cTask);
		ce.ctset = 0;
		ce.cTask = NULL;
		CmtReleaseLock(lock_DAQ);
	}
	
	if(ce.atset) {
		CmtGetLock(lock_DAQ);
		DAQmxClearTask(ce.aTask);
		ce.atset = 0;
		ce.aTask = NULL;
		CmtReleaseLock(lock_DAQ);
	}
	
	SetCtrlVal(mc.mainstatus[1], mc.mainstatus[0], 0);	// Experiment is done!
	SetCtrlAttribute(mc.mainstatus[1], mc.mainstatus[0], ATTR_LABEL_TEXT, "Stopped");

}

int run_experiment(PPROGRAM *p) {
	// The main thread for running an experiment. This should be called from the main 
	// run-program type thread (IdleAndGetData).
	
	int scan = p->scan;
	int ev = 0, done = 0;
	int cont_mode = 1;		// If cont_mode is on, it is checked in the first iteration of the loop anyway.
							// This will be more relevant when cont_run in ND is implemented
	
	SetCtrlVal(mc.mainstatus[1], mc.mainstatus[0], 1);	// Experiment is running!
	SetCtrlAttribute(mc.mainstatus[1], mc.mainstatus[0], ATTR_LABEL_TEXT, "Running");
	
	// Initialize ce
	ce.ct = 0;
	ce.p = p;
	
	if(ce.cstep != NULL) {			// In case this has been allocated already.
		free(ce.cstep);
		ce.cstep = NULL;
	}
	
	if(p->nVaried) {
		ce.cstep = malloc(sizeof(int)*(p->nCycles+p->nDims));
		get_cstep(0, ce.cstep, p->maxsteps, p->nCycles+p->nDims); 
	}
	
	if(scan) {
		if(setup_DAQ_task() != 0) {
			ev = 1;
			goto error;
		}
		
		// Initialize the TDMS file
		initialize_tdms();
	}
	
	// For the moment, continuous acquisition in ND is not implemented
	// In the future, it will just repeat whatever experiment you tell it to
	// indefinitely.
	GetCtrlVal(pc.rc[1], pc.rc[0], &cont_mode);
	if(cont_mode && p->varied)
		SetCtrlVal(pc.rc[1], pc.rc[0], 0);
	
	// Main loop to keep track of where we're at in the experiment, etc.
	while(!GetQuitIdle() && ce.ct <= p->nt) {
		if(cont_mode)
			GetCtrlVal(pc.rc[1], pc.rc[0], &cont_mode);
		
		if(done = prepare_next_step(p) != 0)
		{
			if(cont_mode && done == 1) {
				ce.ct--;
				p->nt++;
				done = prepare_next_step(p);
			}
			
			if(done < 0)
				ev = done;
			
			if(done != 0)
				break;
		}

		if(!done && scan) {
			// This is the part where data acquisition takes place.
		
			/******* PLACEHOLDER ********
			double *data = malloc(p->np*ce.nchan*sizeof(double));
			for(int i = 0; i< p->np*ce.nchan; i++)
				data[i] = ce.ct+((double)i+4)/347; 
			/***************************/
			
			// Get data from device
			CmtGetLock(lock_DAQ);
			DAQmxStartTask(ce.cTask);
			DAQmxStartTask(ce.aTask);
			CmtReleaseLock(lock_DAQ);
			
			double *data = get_data(ce.p, &ev);
			
			CmtGetLock(lock_DAQ);
			DAQmxStopTask(ce.cTask);
			DAQmxStopTask(ce.aTask);
			CmtReleaseLock(lock_DAQ);
			
			if(ev)
				goto error;
			
			// Plot the data
			plot_data(data, p->np, p->sr, ce.nchan);
			
			int err = save_data(data);
		
			free(data);
		}

	}
	
	error:

	return ev;
}

double *get_data(PPROGRAM *p, int *error) {
	// This is the main function dealing with getting data from the DAQ.
	// ce must be initialized properly and the DAQ task needs to have been started.
	//
	// Errors:
	// 1: Invalid inputs (transients or number of points)
	// 2: Data could not be read
	// 3: Data missing after read
	// 4: Acquisition interrupted by quit command.
	
	
	int32 nsread;
	int j, np = p->np, t = ce.ct, nt = p->nt, nc = ce.nchan, err = 0;
	double sr = p->sr;
	float64 *out = NULL;
	char *errmess = NULL;
	
	// These are malformed program conditions
	if(t > nt || np < 1) {
		err = 1;
		goto error;
	}
	
	// First we need to wait for the DAQ to finish doing its thing. This could take a while, so the function
	// will just loop around waiting for this to happen. That's one big reason why this whole thing should
	// be in a single idle-and-get-data thread. Eventually, we'll calculate the expected amount of time this
	// should take and add a few seconds to it, so that this doesn't time out before it should.
	unsigned int bc;
	int i = 0;
	double time = Timer();
	errmess = malloc(400);
	
	while(i < 402 && !GetDoubleQuitIdle()) {
		CmtGetLock(lock_DAQ);
		DAQmxGetReadAttribute(ce.aTask, DAQmx_Read_AvailSampPerChan, &bc);
		CmtReleaseLock(lock_DAQ);
		
		if(bc >= np) 	// It's ready to be read out
			break;
		
		// Timeout condition.
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
		Delay(0.05);
	}
	
	if(GetDoubleQuitIdle()) {
		err = 4;
		goto error;	
	}
	
	free(errmess);
	errmess = NULL;
	
	// Now we know it's ready to go, so we should read out the values.
	out = malloc(sizeof(float64)*np*nc);
	CmtGetLock(lock_DAQ);
	DAQmxReadAnalogF64(ce.aTask, -1, 0.0, DAQmx_Val_GroupByChannel, out, np*nc, &nsread, NULL);
	CmtReleaseLock(lock_DAQ);

	if(nsread != np)
		err = 3;
	
	error:
	
	if(errmess != NULL)
		free(errmess);
	
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
	if(!p->nVaried) {
		if(++ce.ct <= p->nt) {
			if(ce.ilist == NULL)
				ce.ilist = generate_instructions(p, NULL, &ce.ninst);
			return 0;
		} else
			return 1;
	}

	// Free the old ilist
	if(ce.ilist != NULL) {
		free(ce.ilist);
		ce.ilist = NULL;
	}
	
	// Get linear indexes for the phase cycles and the dimensions, plus the maximum linear indexes of each one.
	int i, cli, dli;
	
	if(++ce.ct > 1)
		dli = p->nDims?get_lindex(&ce.cstep[p->nCycles], &p->maxsteps[p->nCycles], p->nDims):-1;
	else
		dli = 0;
	
	int mcli = p->nCycles?(get_lindex(p->maxsteps, p->maxsteps, p->nCycles)):1, mdli = p->nDims?get_lindex(&p->maxsteps[p->nCycles], &p->maxsteps[p->nCycles], p->nDims):-1;
	
	if(p->nt%mcli != 0)
		return -1;
	
	// First check -> Have we finished all the transients?
	if(ce.ct > p->nt) {
		if(dli >= --mdli)		// Done condition.
			return 1;	
		else {
			// Reset the transients and increase the dimension index.
			ce.ct = 1;
			cli = 0;
			dli++;
		}
	} else 
		cli = (ce.ct-1)%mcli;	// Transients are 1-based index, so we need to convert.
	
	// Update the cstep vars.
	if(p->nCycles) { get_cstep(cli, ce.cstep, p->maxsteps, p->nCycles); }
	if(p->nDims) { get_cstep(dli, &ce.cstep[p->nCycles], &p->maxsteps[p->nCycles], p->nDims); }

	ce.ilist = generate_instructions(p, ce.cstep, &ce.ninst); // Generate the next instructions list.

	if(ce.ilist == NULL)
		return -2;
	
	return 0;
}

//////////////////////////////////////////////////////////////
// 															//
//				File Read/Write Operations					//
// 															//
//////////////////////////////////////////////////////////////

int initialize_tdms() {
	// This is the first thing you should call if you are trying to save data to a TDMS
	// file. It will put all the metadata and set everything up, returning what you need.
	// This assumes that ce is properly initialized.
	
	PPROGRAM *p = ce.p;
	char *filename = ce.path;
	
	if(filename == NULL || p == NULL)
		return -1;
	
	if(ce.nchan < 1)
		return -2;

	int i, len, np, nd, ncyc, varied, lindex;
	int rv = 0;
	char *name = NULL, *desc = NULL, *vname = NULL;
	char *csstr = NULL;
	
	varied = p->varied;
	nd = p->nDims;
	ncyc = p->nCycles;
	np = p->np;
	
	TDMSFileHandle data_file = NULL;
	TDMSChannelGroupHandle mcg = NULL, acg = NULL;

	// This is the case where we are first creating the file - these are stored in the ce.
	name = malloc(strlen(ce.fname)+1);
	desc = malloc(strlen(ce.desc)+1);
	
	strcpy(name, ce.fname);
	name[strlen(name)-4] = '\0';
	strcpy(desc, ce.desc);
	
	// The version name.
	vname = malloc(strlen("Magnetometer Controller Version ")+10);
	sprintf(vname, "Magnetometer Controller Version %0.1f", MC_VERSION);
	
	if(rv = TDMS_CreateFile(filename, TDMS_Streaming, name, desc, name, vname, &data_file)) { goto error; }
	if(rv = TDMS_AddChannelGroup(data_file, MCTD_MAINDATA, "", &mcg)) { goto error; }
	//if(p->nt > 1 && rv = TDMS_AddChannelGroup(data_file, MCTD_AVGDATA, "", &acg)) { goto error; }
	
	free(vname);
	free(desc);
	free(name);
	vname = desc = name = NULL;
	
	// Now since it's the first time, we need to set up the attributes, metadata and program.
	if(rv = TDMS_SetChannelGroupProperty(mcg, MCTD_NP, TDMS_Int32, np)) { goto error; } 
	if(rv = TDMS_SetChannelGroupProperty(mcg, MCTD_SR, TDMS_Double, p->sr)) { goto error; }
	if(rv = TDMS_SetChannelGroupProperty(mcg, MCTD_NT, TDMS_Int32, p->nt)) { goto error; }
	if(rv = TDMS_SetChannelGroupProperty(mcg, MCTD_TIMESTART, TDMS_Timestamp, ce.tstart)) { goto error; }
	if(rv = TDMS_SetChannelGroupProperty(mcg, MCTD_NDIM, TDMS_Int8, nd+1)) { goto error; }
	if(rv = TDMS_SetChannelGroupProperty(mcg, MCTD_NCYCS, TDMS_Int8, ncyc)) { goto error; }
	if(rv = TDMS_SetChannelGroupProperty(mcg, MCTD_NCHANS, TDMS_Int8, ce.nchan)) { goto error; }
	if(rv = TDMS_SetChannelGroupProperty(mcg, MCTD_PTMODE, TDMS_Int32, p->tmode)) { goto error; }
	
	// Now save the program.
	TDMSChannelGroupHandle pcg = NULL;
	TDMS_AddChannelGroup(data_file, MCTD_PGNAME, "", &pcg);
	tdms_save_program(pcg, p);

	// Initialize current experiment.
	if(ce.chans != NULL)
		free(ce.chans);
	
	if(ce.achans != NULL) 
		free(ce.achans);
	
	ce.chans = malloc(sizeof(TDMSChannelHandle)*ce.nchan);
	ce.achans = (p->nt>1)?malloc(sizeof(TDMSChannelHandle)*ce.nchan):NULL;
	
	// Create the data channels
	for(i = 0; i < ce.nchan; i++) {
		if(rv = TDMS_AddChannel(mcg, TDMS_Double, ce.icnames[i], "", "V", &ce.chans[i])) { goto error; }
		if(p->nt > 1)
			if(rv = TDMS_AddChannel(acg, TDMS_Double, ce.icnames[i], "", "V", &ce.achans[i])) { goto error; }
	}
	
	// Save this before we're done.
	TDMS_SaveFile(data_file);
	
	error:
	
	if(rv != 0) {
		ce.td_file = data_file;
		ce.mcg = mcg;
		ce.acg = acg;
	} else {	
		ce.td_file = NULL;
		ce.mcg = NULL;
		if(ce.chans != NULL)
			free(ce.chans);
		ce.chans = NULL;
		
		if(data_file != NULL)
			TDMS_CloseFile(data_file);
	}
	
	if(name != NULL)
		free(name);
	
	if(desc != NULL)
		free(desc);
	
	if(vname != NULL)
		free(vname);
	
	return rv;
}

int save_data(double *data) {
	// In order for this to work, the ce variable needs to be set up appropriately. This function is
	// used to write data to an existing and open file. If for whatever reason 
	
	// Data is stored in one variable per channel. I believe it's indexed, so it should be just as fast to
	// to store it in one big variable as to store it as a bunch of small variables. We can run some tests
	// on that later if we want to see.
	
	PPROGRAM *p = ce.p;
	int rv = 0;
	
	if(p == NULL)
		return -1;
	
	int i, np = p->np, nd = p->nDims;
	char *csstr = NULL;
	double *avg_data = NULL;
	
	int dli;
	// Get the linear index in only the indirect dimension acquisition space
	if(nd)
		dli = get_lindex(&ce.cstep[p->nCycles], p->maxsteps, p->nDims);
	else
		dli = 0;
	
	dli *= p->nt;		// Least significant bit is number of transients.
	dli += ce.ct-1;		// I believe that ct is on a 1-based index.
	
	// Save the current step as a linear index and as a string (for readability)
	if(rv = TDMS_SetChannelGroupProperty(ce.mcg, MCTD_CSTEP, TDMS_Int32, dli)) { goto error; }
	
	csstr = malloc(12*(p->nDims+2)+3); // Signed maxint is 10 digits, plus - and delimiters.
	sprintf(csstr, "[%d", ce.ct-1);
	for(i = 0; i < p->nDims; i++)
		sprintf(csstr, "; %d", ce.cstep[i+p->nCycles]);	
	sprintf(csstr, "%s]", csstr);
	
	if(rv = TDMS_SetChannelGroupProperty(ce.mcg, MCTD_CSTEPSTR, TDMS_String, csstr)) { goto error; }
	free(csstr);
	csstr = NULL;
	
	// Save the time completed
	if(rv = TDMS_SetChannelGroupProperty(ce.mcg, MCTD_TIMEDONE, TDMS_Timestamp, ce.tdone)) { goto error; }

	// Save the data
	for(i = 0; i < ce.nchan; i++) {
		if(rv = TDMS_AppendDataValues(ce.chans[i], &data[i*np], np, 1)) { goto error; }
	}
	
	// Save the average data if needed.
/*	if(p->nt > 1 && ce.achans != NULL) {
		if(ce.ct == 1) {	// First run.
			for(i = 0; i < ce.nchan; i++) {
				if(rv = TDMS_AppendDataValues(ce.achans[i], &data[i*np], np, 1)) { goto error;}
			}
		} else {
			// First get the old data
			avg_data = load_data_tdms(dli, ce.td_file, ce.acg, ce.achs, ce.p, ce.cind, ce.nchan, &rv);
			
			if(rv != 0)
				goto error;
			
			// Now update the average.
			for(i = 0; i < (p->np*ce.nchan); i++) {
				avg_data[i] = ((avg_data[i]*(ce.ct-1))+data[i])/ce.ct;
			}
			
			// And write it to file again.
			if(rv = TDMS_
		}
	
	}
	
*/	
	error:
	if(csstr != NULL)
		free(csstr);
	
	if(avg_data != NULL)
		free(avg_data);
	
	return rv;
}

int load_experiment(char *filename) {
	// Loads the experiment into the uidc file and loads the first experiment to the controls.
	// Filename must end in .tdms
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
	TDMSFileHandle file = NULL;
	TDMSChannelGroupHandle mcg = NULL, pcg = NULL, *groups = NULL;
	TDMSChannelHandle *chs = NULL;
	int ng, g = 40, s = 40, len;
	char *name_buff = NULL;
	char **cnames = NULL;
	int nc = -1;
	
	
	// Check that the filename is conforming.
	if(filename == NULL || strlen(filename) < 5 || sscanf(&filename[strlen(filename)-5], ".tdms") != 0 || !FileExists(filename, NULL))  {
		ev = -248;
		goto error;
	}
	
	// Open the file for reading
	if(ev = TDMS_OpenFile(filename, TRUE, &file))
		goto error;
	
	if(ev = TDMS_GetNumChannelGroups(file, &ng))
		goto error;
	else if (ng < 2) {
		ev = -249;
		goto error;
	}
	
	groups = malloc(sizeof(TDMSChannelGroupHandle)*ng);
	
	if(ev = TDMS_GetChannelGroups(file, groups, ng))
		goto error;
	
	// Search for the program and main groups in the list of groups.
	int mcg_f = 0, pcg_f = 0; // Whether or not the main channel group and program channel group have been found.
	name_buff = malloc(g);
	for(i = 0; i < ng; i++) {
		if(ev = TDMS_GetChannelGroupStringPropertyLength(groups[i], TDMS_CHANNELGROUP_NAME, &len))
			goto error;
		
		if(g < ++len)
			name_buff = realloc(name_buff, g+=((int)((len-g)/s)+1)*s);
		
		if(ev = TDMS_GetChannelGroupProperty(groups[i], TDMS_CHANNEL_NAME, name_buff, len))
			goto error;
		
		if(!pcg_f && strcmp(name_buff, MCTD_PGNAME) == 0) {
			// If we found it, copy from groups to pcg.
			pcg = malloc(sizeof(TDMSChannelGroupHandle));
			memcpy(&pcg, &groups[i], sizeof(TDMSChannelGroupHandle));
			pcg_f = 1;
		}
		
		if(!mcg_f && strcmp(name_buff, MCTD_MAINDATA) == 0) {
			// If we found it, copy from groups to mcg.
			mcg = malloc(sizeof(TDMSChannelGroupHandle));
			memcpy(&mcg, &groups[i], sizeof(TDMSChannelGroupHandle));
			mcg_f = 1;
		}
		
		if(mcg_f && pcg_f)
			break;
	}
	
	if(!mcg_f || !pcg_f) {
		ev = -249;
		goto error;
	}
	
	// We now have both groups, we don't need this list anymore. 
	free(groups);
	groups = NULL;
	
	// Grab the program.
	int err_val;
	p = tdms_load_program(pcg, &ev);
	if(ev || p == NULL)
		goto error;
	
    // Get the current step index.
	if(ev = TDMS_GetChannelGroupProperty(mcg, MCTD_CSTEP, &cind, sizeof(int)))
		goto error;
	
	// Get the number of channels in two ways, and check that they are consistent.
	unsigned char c_buff;
	if(ev = TDMS_GetChannelGroupProperty(mcg, MCTD_NCHANS, &c_buff, sizeof(unsigned char)))
		goto error;
	
	int nchs = (int)c_buff;
	
	if(ev = TDMS_GetNumChannels(mcg, &nc))
		goto error;
	
	if(nc != nchs) {
		ev = -251;
		goto error;
	}
	
	// Allocate the channel array - I'm assuming they are in order.
	chs = malloc(sizeof(TDMSChannelHandle)*nc);
	if(ev = TDMS_GetChannels(mcg, chs, nc))
		goto error;
	
	// Now allocate the array of cnames, then grab them. Currently these are not used, but I could see them
	// being useful in the future and it's easy to implement.
	cnames = malloc(sizeof(double*)*nc);
	for(i = 0; i < nc; i++)
		cnames[i] = NULL;
	
	for(i = 0; i < nc; i++) {
		if(TDMS_GetChannelStringPropertyLength(chs[i], TDMS_CHANNEL_NAME, &len))
			goto error;
		
		cnames[i] = malloc(++len);
		
		if(TDMS_GetChannelProperty(chs[i], TDMS_CHANNEL_NAME, cnames[i], len))
			goto error;
	}
	
	// Finally get the data for the 0th index
	data = load_data_tdms(0, file, mcg, chs, p, cind, nc, &ev);
	if(ev != 0)
		goto error;
	
	// If we're here, then we've finished loading all the data without any errors. We can now safely clear the 
	// old stuff and load the new stuff (hopefully).
	if(uidc.file != NULL)
		TDMS_CloseFile(uidc.file);
	uidc.file = file;
	uidc.mcg = mcg;
	uidc.cind = cind;
	
	if(uidc.chs != NULL) {
		free(uidc.chs);
		uidc.chs = NULL;
	}
	
	uidc.chs = chs;
	
	if(uidc.cnames != NULL) {
		for(i = 0; i < uidc.nch; i++) {
			if(uidc.cnames[i] != NULL) {
				free(uidc.cnames[i]);
				uidc.cnames[i] = NULL;
			}
		}
		free(uidc.cnames);
		uidc.cnames = NULL;
	}
	
	uidc.cnames = cnames;
	uidc.nch = nc; 
	
	if(uidc.p != NULL)
		free_pprog(uidc.p);
	uidc.p = p;
	
	// Now we want to plot the data and set up the experiment navigation.
	plot_data(data, p->np, p->sr, uidc.nch);
	free(data);
	
	update_experiment_nav();
	
	error:
	
	// Free a bunch of memory - some of these things shouldn't be freed if ev wasn't reset,
	// as the pointer has been assigned to a field of uidc.
	if(ev != 0) {
		if(file != NULL)
			TDMS_CloseFile(file);
		
		if(p != NULL)
			free_pprog(p);
		
		if(chs != NULL)
			free(chs);
		
		if(cnames != NULL) {
			for(i = 0; i < nc; i++) {	// Don't reuse nc, or this will break.
				if(cnames[i] != NULL)
					free(cnames[i]);
			}
			
			free(cnames);
		}
	}
	
	// The rest of this should always be freed.
	if(groups != NULL)
		free(groups);
	
	if(name_buff != NULL)
		free(name_buff);
	
	if(data != NULL && ev) {
		free(data);
		data = NULL;
	}

	return ev;
}

double *load_data(int lindex, int *rv) {
	// With uidc.p, uidc.file, uidc.mcg, uidc.chs and uidc.cind already set, 
	// this will load the data corresponding to a given linear index and transient
	// It is important to note that the lindex has dimensionality [nt p->nDims], rather than [p->nCycles p->nDims]
	//
	// Error: -250 => lindex exceeds bounds.
	
	return load_data_tdms(lindex, uidc.file, uidc.mcg, uidc.chs, uidc.p, uidc.cind, uidc.nch, rv);
}

double *load_data_tdms(int lindex, TDMSFileHandle file, TDMSChannelGroupHandle mcg, TDMSChannelHandle *chs, PPROGRAM *p, int cind, int nch, int *rv) {
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
		if(ev = TDMS_GetDataValuesEx(chs[i], lindex*np, np, &data[i*np]))
			goto error;
	}
	
	error:
	if(ev != 0 && data != NULL) {
		free(data);
		data = NULL;
	}				    
	
	if(maxsteps != NULL)
		free(maxsteps);
	
	return data;
}

int write_data() {
	double *data = malloc(200*sizeof(double));
	
	TDMSFileHandle data_file;
	TDMS_CreateFile("DataOut.tdms", TDMS_Streaming, "Test", "", "Test", "Porcelain Kristallnacht", &data_file);
	
	TDMSChannelGroupHandle maingroup;
	TDMSChannelHandle datachannel;
	TDMS_AddChannelGroup(data_file, "Main Group", "Some Description", &maingroup); 
	TDMS_AddChannel(maingroup, TDMS_Double, "Main Channel", "desc", "", &datachannel);
	
	// Write the data.
	SinePattern(200, 1, 0, 1.0, data);
	TDMS_AppendDataValues(datachannel, data, 200, 1); 
	
	return 0;	
}

//////////////////////////////////////////////////////////////
// 															//
//				UI Interaction Functions					//
// 															//
//////////////////////////////////////////////////////////////
void load_data_popup() {
	// The callback called from loading data.
	char *path = malloc(MAX_FILENAME_LEN+MAX_PATHNAME_LEN);
	int rv = FileSelectPopup((uidc.dlpath != NULL)?uidc.dlpath:"", "*.tdms", ".tdms", "Load Experiment from File", VAL_LOAD_BUTTON, 0, 1, 1, 1, path);
	
	if(rv == VAL_NO_FILE_SELECTED) {
		free(path);
		return;
	} else 
		add_data_path_to_recent(path);
	
	int err_val = load_experiment(path);
	
	free(path);
	
	if(err_val != 0)
		display_tdms_error(err_val);
	
}

void add_data_path_to_recent(char *opath) {
	// Adds  data file to the top of the most recent paths list.
	if(opath == NULL) 
		return;
	
	char *path = malloc(strlen(opath)+1);
	strcpy(path, opath);
	
	// TODO: Add to the recent data pulldown in the menu bar and the recent
	// program pulldown on the main page.

	
	// Make this directory the default next time we try and load something.
	char *c = strrchr(path, '\\'); // Finds last instance of the separator.
	c[1] = '\0';				   // Truncate after the last separator.

	if(c != NULL && FileExists(path, NULL)) {
		if(uidc.dlpath == NULL)
			uidc.dlpath = malloc(strlen(path)+1);
		else
			uidc.dlpath = realloc(uidc.dlpath, strlen(path)+1);

		strcpy(uidc.dlpath, path);
	}
	
	free(path);
}

/*********** Plotting Functions ************/
int plot_data(double *data, int np, double sr, int nc) {
	// Given data, this loads the data into the graphs.
	// np = Number of points
	// sr = sampling rate
	// nc = number of channels
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
	if(np <= 0 || sr <= 0.0 || nc <= 0 || data == NULL)
		return 1;
	
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
	for(i = 0; i < npfft/2; i++)
		freq[i] = i*sr/(2*(npfft/2-1)); 
	
	for(i = 0; i < nc; i++) {  // Iterate through the channels
		// Get the data for this channel
		for(j = 0; j < np; j++)
			curr_data[j] = data[j+i*np];
	
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
	if(curr_data != NULL)
		free(curr_data);
	
	if(curr_fft != NULL)
		free(curr_fft);
	
	if(fft_re != NULL)
		free(fft_re);
	
	if(fft_im != NULL)
		free(fft_im);
	
	if(fft_mag != NULL)
		free(fft_mag);
	
	if(phase != NULL)
		free(phase);
	
	if(freq != NULL)
		free(freq);
	
	return ev;
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
	for(int i = 0; i < np; i++) 
		ph[i] = (double)i*uidc.sr/(2*(double)np-1);	// This should be the frequency domain.
	
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

void update_experiment_nav() {
	// Call this to update things like the number of transients available, the number of
	// dimensions, etc. This should be called whenever you switch points in an indirect
	// dimension. This does not change the position you are in, it just updates the ring
	// controls and un-hides the relevant controls.
	
	int i, j;
	if(uidc.p == NULL || uidc.p->nt < 1){
		// If there is no program saved, no navigation is possible, dim/hide everything. 
		for(j = 0; j < 2; j++) {
			for(i = 0; i < 8; i++)
				SetCtrlAttribute(dc.cloc[j], dc.idrings[i], ATTR_VISIBLE, 0);
			SetCtrlAttribute(dc.cloc[j], dc.ctrans, ATTR_DIMMED, 1);
			DeleteListItem(dc.cloc[j], dc.ctrans, 0, -1);
		}
		
		return;  // And done - that was easy.
	}
	
	PPROGRAM *p = uidc.p; // Convenience

	// Get an array from the linear index
	size_t size = sizeof(int)*(p->nDims+1);
	int *maxsteps = malloc(size);
	int *steps = malloc(size);

	// The dimensionality of the cstep, which should be [nt, {dim1, ..., dimn}]
	// p->maxstep is stored as {pc1, ... pcn, dim1, ... , dimn}
	maxsteps[0] = p->nt;
	if(p->nDims) 		// It's just a number if it's not multidimensional 
		memcpy(&maxsteps[1], &p->maxsteps[p->nCycles], sizeof(int)*p->nDims);

	if(get_cstep(uidc.cind, steps, maxsteps, p->nDims+1) != 1) { // On error, return
		free(maxsteps);
		free(steps);
		return;
	}
	
	char **c = NULL;
	
	// If it's multidimensional, go through and turn on each of the controls.
	for(j = 0; j < 2; j++) {
		// Update the multidimensional bit
		if(p->nDims) {
			int nl, elems, step;
			for(i = 0; i < p->nDims; i++) { 
				SetCtrlAttribute(dc.cloc[j], dc.idrings[i], ATTR_VISIBLE, 1);
				GetNumListItems(dc.cloc[j], dc.idrings[i], &nl);
				if(nl > 0)
					GetCtrlIndex(dc.cloc[j], dc.idrings[i], &step);
				else
					step = -1;
				
				// Update the number of steps in each dimension.
				if(nl > steps[i+1])
					DeleteListItem(dc.cloc[j], dc.idrings[i], steps[i+1], -1);	
				else if(nl < steps[i + 1]) {
					c = generate_char_num_array(nl, steps[i+1], &elems);
					for(int k = 0; k < elems; k++) {
						InsertListItem(dc.cloc[j], dc.idrings[i], -1, c[k], k+nl);
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
	
	// Now update the transients
	update_transients();
	
	free(maxsteps);
	free(steps);
}

void update_transients() {
	// Call this after you've changed the position in acquisition space, or in the event of a new
	// addition to the number of transients. uidc.p must be set.
	
	if(uidc.p == NULL)
		return;
	
	PPROGRAM *p = uidc.p;
	
	// Define variables so we don't have to do it twice in-line.
	int nt, nl, ind, pan;
	char *tc = malloc(strlen("Transient ")+(int)ceil(log10(p->nt))+1); // Allocate maximum needed.

	// Whole thing needs to be done twice, once for fft and once for fid.
	for(int i = 0; i < 2; i++) {
		pan = dc.cloc[i];
		// How many transients to display
		if(p->nDims) {
			ind = get_selected_ind(pan, 0, NULL);
			nt = calculate_num_transients(uidc.cind, ind, p->nt);
		} else
			nt = uidc.cind;
		
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
				int len;
				GetCtrlValStringLength(pan, dc.ctrans, &len);
				
				char *buff = malloc(len+1);
				GetCtrlVal(pan, dc.ctrans, buff);
				if(strcmp(buff, "Average") != 0) {
					DeleteListItem(pan, dc.ctrans, 0, -1); // Delete them all and start over.
					nl = 0;
				}
				
				free(buff);
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
	// uidc.p must already be set.
	
	PPROGRAM *p = uidc.p;
	if(p == NULL)
		return -1;
	
	int t = 0, nl;
	
	if(t_inc) {
		GetNumListItems(panel, dc.ctrans, &nl);
		if(nl < 1 || p->nt < 1)
			return -2;
		
		GetCtrlVal(panel, dc.ctrans, &t);
	}
	
	// The program is simple if it's not multi-dimensional.
	if(p->nDims < 1) {
		if(!t_inc)	
			return -3;   // Not multidimensional, didn't ask for transients.
		else
			return t;
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
	
	if(ind < 0)
		return -5;
	
	// Add in transient if necessary.
	if(t_inc) {
		ind *= p->nt;
		ind += t;
	}
	
	if(step != NULL) {
		memcpy(&step[t_inc?1:0], cstep, sizeof(int)*p->nDims); // Skip the first index if that one should have the transient in it.
		if(t_inc)
			step[0] = t;
	}
	
	free(cstep); // No longer needed
	return ind;
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
	
	double *data = malloc(sizeof(double)*np), gc = uidc.fgain[c]/oldgain; // Gain change
	
	GetPlotAttribute(dc.fid, dc.fgraph, uidc.fplotids[c], ATTR_PLOT_YDATA, data);
	
	// Alter the gain.
	for(i = 0; i < np; i++) {
		data[i] *= gc;		
	}
	
	// Update the plot.
	SetPlotAttribute(dc.fid, dc.fgraph, uidc.fplotids[c], ATTR_PLOT_YDATA, data);
	
	free(data);
	
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
	
	double *data = malloc(sizeof(double)*np), oc = uidc.foff[c] - oldoff; // Offset change
	
	GetPlotAttribute(dc.fid, dc.fgraph, uidc.fplotids[c], ATTR_PLOT_YDATA, data);
	
	// Alter the gain.
	for(i = 0; i < np; i++) {
		data[i] += oc;		
	}
	
	// Update the plot.
	SetPlotAttribute(dc.fid, dc.fgraph, uidc.fplotids[c], ATTR_PLOT_YDATA, data);
	
	free(data);
	
}

void change_spec_gain(int num) {
	// Updates the gain for the Spectrum.
	if(num < 0 || num >= 8)
		return;
	
	int c = num;
	
	// Get the old gain
	float oldgain = uidc.sgain[c];
	GetCtrlVal(dc.spec, dc.scgain, &uidc.sgain[c]);
	
	if(uidc.splotids[c][0] < 0 || uidc.splotids[c][1] < 0 || uidc.splotids[c][2] < 0)  // If we don't need to update the plot, we're good
		return;
	
	// Get the plot data
	int np, i;
	GetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[c][0], ATTR_NUM_POINTS, &np);
	
	double **data = malloc(sizeof(double*)*3), gc = uidc.sgain[c]/oldgain; // Gain change
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

void change_spec_offset(int num) {
	// Updates the gain for the Spectrum.
	if(num < 0 || num >= 8)
		return;
	
	int c = num;
	
	// Get the old gain
	float oldoff = uidc.soff[c];
	GetCtrlVal(dc.spec, dc.scgain, &uidc.soff[c]);
	
	if(uidc.splotids[c][0] < 0 || uidc.splotids[c][1] < 0 || uidc.splotids[c][2] < 0)  // If we don't need to update the plot, we're good
		return;
	
	// Get the plot data
	int np, i;
	GetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[c][0], ATTR_NUM_POINTS, &np);
	
	if(np < 1)
		return;
	
	double **data = malloc(sizeof(double*)*3), oc = uidc.soff[c]-oldoff; // Gain change
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

void toggle_fid_chan(int num) {
	// Turns whether or not a given channel is on on or off and
	// updates all the relevant controls.
	
	/****** TO DO *******
	 Once the data loading and such is done, this also needs to hide or show
	 the appropriate data, update the channel preferences, etc.
	 *******************/
	
	if(num < 0 || num >= 8)
		return;
	
	// Update UIDC
	int i, on;
	GetCtrlVal(dc.fid, dc.fchans[num], &on);
	uidc.fchans[num] = on;
	
	if(on) {
		uidc.fnc++;			// One more channel is on	
	} else {
		uidc.fnc--;			// One fewer channel is on
	}
	
	// Turn the data on or off
	if(uidc.fplotids[num] >= 0)
		SetPlotAttribute(dc.fid, dc.fgraph, uidc.fplotids[num], ATTR_TRACE_COLOR, on?uidc.fcol[num]:VAL_TRANSPARENT);
	
	update_chan_ring(dc.fid, dc.fcring, uidc.fchans);
	
	// Set the box's contents to dimmed when no channels are on.
	// Reverse it if any channels are on.
	int c_on = (uidc.fnc < 1);
	
	SetCtrlAttribute(dc.fid, dc.fccol, ATTR_DIMMED, c_on);
	SetCtrlAttribute(dc.fid, dc.fcgain, ATTR_DIMMED, c_on);
	SetCtrlAttribute(dc.fid, dc.fcoffset, ATTR_DIMMED, c_on);
	
	if(!c_on)
		update_fid_chan_box(); // Update the values of the chan box.
}

void toggle_spec_chan(int num) {
	// Turns whether or not a given spectrumc hannel is on or off and updates the relevant controls
	
	if(num < 0 || num >= 8)
		return;
	
	/****** TO DO *******
	 Once the data loading and such is done, this also needs to hide or show
	 the appropriate data, update the channel preferences, etc.
	 *******************/
	
	int i, on;
	GetCtrlVal(dc.spec, dc.schans[num], &on);
	uidc.schans[num] = on;
	
	if(on) {
		uidc.snc++;
	} else {
		uidc.snc--;
	}
	
	// Turn the data on or off
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
	
	if(!c_on)
		update_spec_chan_box(); // Update the values of the chan box.
	
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

void update_spec_fft_chan() {
	// Run this whenever the uidc.schan value has changed.
	
	if(uidc.schan < 0 || uidc.schan >= 3)
		return;
	
	int i, col[3] = {0, 0, 0};
	
	// Go through and turn on or off the plots as appropriate.
	for(i = 0; i < 8; i++) {
		col[uidc.schan] = uidc.schans[i];
		
		if(uidc.splotids[i][0] >= 0)
			SetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[i][0], ATTR_TRACE_COLOR, col[0]?uidc.scol[i]:VAL_TRANSPARENT);
		if(uidc.splotids[i][1] >= 0)
			SetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[i][1], ATTR_TRACE_COLOR, col[1]?uidc.scol[i]:VAL_TRANSPARENT);
		if(uidc.splotids[i][2] >= 0)
			SetPlotAttribute(dc.spec, dc.sgraph, uidc.splotids[i][2], ATTR_TRACE_COLOR, col[2]?uidc.scol[i]:VAL_TRANSPARENT);
	}
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
	

/********* Device Interaction Settings ********/

int get_current_fname (char *path, char *fname) {
	// Make sure that fname has been allocated to the length of the fname + 4 chars.
	// Output is "fname", which is overwritten.
	//
	// Errors are:
	// -1: Path or fname missing
	// -2: Malformed path or fname
	// -3: Maximum number of filenames used
	//
	// Success returns 1;
	
	if(path == NULL || fname == NULL)
		return -1;
	
	// Allocate space for the full pathname.
	char *npath = malloc(strlen(path)+strlen(fname)+11);
	
	strcpy(npath, path);
	
	// Make sure that there's no path separator at the end of path
	// or at the beginning or end of fname.
	while(strlen(npath) > 1 && path[strlen(npath)-1] == '\\')
		npath[strlen(npath)-1] = '\0';
	
	if(strlen(npath) < 1 || !FileExists(npath, NULL)) {
		free(npath);
		return -2;
	}
	
	while(strlen(fname) > 1 && fname[strlen(fname)-1] == '\\')
		fname[strlen(fname)-1] = '\0';
	
	if(strlen(fname) < 1) {
		free(npath);
		return -2;
	}
	
	int i = 0, l = strlen(fname);
	while(fname[i] == '\\' && i < l)
		i++;
	
	if(i > 0) {
		strcpy(fname, &fname[i]);
		fname[l-i+1] = '\0';
	}
	
	strcpy(path, npath);
	
	// Check which ones are available.
	for(i = 0; i < 10000; i++) {
		sprintf(npath, "%s\\%s%04d.tdms", path, fname, i);
		if(!FileExists(npath, NULL))
			break;
	}
	
	if(i >= 10000) {
		free(npath);
		return -3;
	}
	
	sprintf(fname, "%s%04d", fname, i); // The output
	return 1; 
}

int get_devices () {		// Updates the device ring control.
	int nd = 0, bs, i, len;
	int rv = 0;
	char *devices = NULL, *device_name = NULL, *old_dev = NULL;
	
	CmtGetLock(lock_DAQ);	// For multithreading reasons, we need to lock all DAQ functions
	bs = DAQmxGetSystemInfoAttribute(DAQmx_Sys_DevNames, "", NULL); // Returns max char size.
	CmtReleaseLock(lock_DAQ);
	
	// Store the old device name and index
	int nl;
	GetNumListItems(pc.dev[1], pc.dev[0], &nl);
	if(nl > 0) {
		GetCtrlValStringLength(pc.dev[1], pc.dev[0], &len);
		old_dev = malloc(len+1);
		GetCtrlVal(pc.dev[1], pc.dev[0], old_dev);
		GetCtrlIndex(pc.dev[1], pc.dev[0], &uidc.devindex);
	}
	
	DeleteListItem(pc.dev[1], pc.dev[0], 0, -1);	// Delete devices if they exist.
	
	if(bs <= 0) {
		rv = -1; // No devices found.
		goto error;
	}
	
	// Get the actual device names
	devices = malloc(bs);
	CmtGetLock(lock_DAQ);
	DAQmxGetSystemInfoAttribute(DAQmx_Sys_DevNames, devices, bs);
	CmtReleaseLock(lock_DAQ);
	
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
		
		p = (p2 == NULL)?NULL:p2+2; // Increment, making sure we don't go past the end.
	}
	
	free(device_name);
	
	// Get the index as close to what it used to be as we can.
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
	} else {
		MessagePopup("Error", "No devices available!\n");
		rv = -1;
	}
	
	error:
	if(devices != NULL)
		free(devices);

	if(old_dev != NULL)
		free(old_dev);
	
	return rv;
}

int load_DAQ_info() {
	// Loads information about the DAQ to the UI controls.
	
	// Variable declarations
	int rv = 0, nl, len, old_ti = 0, old_ci = 0, curr_chan;
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

	if(get_devices() < 0) {
		rv = -1;
		goto err1;
	}
	
	GetNumListItems(pc.dev[1], pc.dev[0], &nl);
	if(nl < 1){
		rv = -2;
		
		err1:
		// Dim some controls, y'all.
		SetCtrlAttribute(pc.trigc[1], pc.trigc[0], ATTR_DIMMED, 1);
		SetCtrlAttribute(pc.trige[1], pc.trige[0], ATTR_DIMMED, 1);
		SetCtrlAttribute(pc.curchan[1], pc.curchan[0], ATTR_DIMMED, 1);
		SetCtrlAttribute(pc.range[1], pc.range[0], ATTR_DIMMED, 1);
		
		if (old_trig != NULL)
			free(old_trig);
		if (old_count != NULL)
			free(old_count);
		return rv;
	}
	
	CmtGetLock(lock_DAQ);
	
	// Get the current device name
	int i, nc = 0, devlen;
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
		if(p2 == NULL)
			len = strlen(input_chans)-(p-input_chans)+2;
		else 
			len = (p2 - p)+2;
		
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
	if(uidc.onchans)
		SetCtrlIndex(pc.curchan[1], pc.curchan[0], (curr_chan>=uidc.onchans)?uidc.onchans-1:curr_chan);
	
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
	
	if(old_trig != NULL)
		free(old_trig);
	
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
		if(p2 == NULL)
			len = strlen(counter_chans)-(p-counter_chans)+1;
		else
			len = p2-p+1;
		
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
		if(ind >= 0)
			SetCtrlIndex(pc.cc[1], pc.cc[0], ind);
		else if (old_ci >= nl)
			SetCtrlIndex(pc.cc[1], pc.cc[0], nl-1);
		else
			SetCtrlIndex(pc.cc[1], pc.cc[0], old_ci);
	}
	
	if(old_count != NULL)
		free(old_count);
	free(counter_chans);
	free(channel_name);
	free(buff_string);
	
	// Make sure that the trigger edge attributes are set up correctly.
	ReplaceListItem(pc.trige[1], pc.trige[0], 0, "Rising", DAQmx_Val_Rising);
	ReplaceListItem(pc.trige[1], pc.trige[0], 1, "Falling", DAQmx_Val_Falling);
	
	free(device);
	CmtReleaseLock(lock_DAQ);
	return 0;
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
	
	// Check if we can turn this on or not.
	if(!uidc.chans[val] && uidc.onchans == 8) {
		SetCtrlIndex(pc.ic[1], pc.ic[0], oval); // Jump back to where we were
		return;
	}
	
	// Get the old label and value
	int lablen, len;
	char *label, *value;
	GetLabelLengthFromIndex(pc.ic[1], pc.ic[0], val, &lablen);
	GetValueLengthFromIndex(pc.ic[1], pc.ic[0], val, &len);
	
	// Need to free these later
	label = malloc(lablen+1);
	value = malloc(len+1);
	
	GetLabelFromIndex(pc.ic[1], pc.ic[0], val, label);
	GetValueFromIndex(pc.ic[1], pc.ic[0], val, value);
	
	label[0] = uidc.chans[val]?' ':149; // 149 is a mark
	
	// Replace the list item to replace the label
	ReplaceListItem(pc.ic[1], pc.ic[0], val, label, value);
	
	
	// Now we're going to insert into the current channels ring in the right place
	// Either delete the val when we find it, or insert this before the first one
	// that's bigger than it.
	int i, ind;
	
	if(uidc.chans[val])
		remove_chan(val);
	else
		add_chan(&label[1], val);

	free(label);
	free(value);
	
	// Update the uidc var now.
	uidc.chans[val] = !uidc.chans[val];		// Toggle its place in the var

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

void add_chan(char *label, int val) {
	// Adds a channel to the current channel control in a sorted fashion.
	// This also increments uidc.onchans.
	
	// Find the first control that is bigger than this. That's where you add
	int i, ind;
	for(i = 0; i < uidc.onchans; i++) {
		GetValueFromIndex(pc.curchan[1], pc.curchan[0], i, &ind);
		if(ind > val)
			break;	
	}
	
	if(i >= uidc.onchans) {
		ind = -1;
		i = uidc.onchans;
	} else
		ind = i;
	
	InsertListItem(pc.curchan[1], pc.curchan[0], ind, label, val);
	
	// Now update the range controls
	for(int j = uidc.onchans; j > i; j--)
		uidc.range[j] = uidc.range[j-1];

	GetCtrlAttribute(pc.range[1], pc.range[0], ATTR_DFLT_VALUE, &uidc.range[i]);
	uidc.onchans++;
}

void remove_chan(int val) {
	// Removes a channel from the current channels control and decrements onchans
	
	// First find us in the control.
	int i, ind;
	for(i = 0; i < uidc.onchans; i++) {
		GetValueFromIndex(pc.curchan[1], pc.curchan[0], i, &ind);
		if(ind == val)
			break;
	}
	
	if(i >= uidc.onchans)
		return;				// It's not there
	
	DeleteListItem(pc.curchan[1], pc.curchan[0], i, 1);
	
	// Move over all the range values
	for(i; i < uidc.onchans-1; i++) 
		uidc.range[i] = uidc.range[i+1];
	
	uidc.onchans--;
	
	change_chan();	// In case we deleted the current channel.
}

void change_chan() {
	// Function called when you change the current channel.
	// Updates the range control with values from uidc.
	
	int chan;
	GetCtrlIndex(pc.curchan[1], pc.curchan[0], &chan);
	
	if(chan >= 0 && chan < 8) 
		SetCtrlVal(pc.range[1], pc.range[0], uidc.range[chan]);
}

void change_range() {
	// Called whenever you change the range. Simply updates
	// the uidc variable with the new value.
	
	int chan;
	float val;
	GetCtrlIndex(pc.curchan[1], pc.curchan[0], &chan);
	GetCtrlVal(pc.range[1], pc.range[0], &val);
	
	if(chan >= 0 && chan < 8)
		uidc.range[chan] = val;
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
	
	if(rv = clear_DAQ_task())
		goto error;
	
	CmtGetLock(lock_DAQ);
	
	// First we create the tasks
	ce.atname = "AcquireTask";
	ce.ctname = "CounterTask";

	ce.aTask = NULL;
	ce.cTask = NULL;
	
	ce.atset = 0;
	ce.ctset = 0;
	
	if(rv = DAQmxCreateTask(ce.atname, &ce.aTask)) 
		goto error;
	ce.atset = 1;	// We've created the counter task
	
	if(rv = DAQmxCreateTask(ce.ctname, &ce.cTask))
		goto error;
	ce.ctset = 1;	// We've created the counter task
	
	// Get some stuff from the UI
	int np;
	double sr;
	GetCtrlVal(pc.np[1], pc.np[0], &np);
	GetCtrlVal(pc.sr[1], pc.sr[0], &sr);
	
	// Create the analog input voltage channels
	int i, ic, nc = uidc.onchans, insize = 25, len;
	tname = malloc(strlen("VoltInput00")+1);
	ic_name = malloc(insize);

	if(ce.icnames != NULL && ce.nchan > 0) {
		for(i = 0; i < ce.nchan; i++)
			free(ce.icnames[i]);
	}
	
	if(ce.icnames != NULL)
		free(ce.icnames);
	
	ce.nchan = nc;
	ce.icnames = malloc(sizeof(char*)*nc);
	
	for(i = 0; i < nc; i++) {
		GetValueFromIndex(pc.curchan[1], pc.curchan[0], i, &ic); // Channel index
		
		// Make sure there's enough room for the channel name.
		GetValueLengthFromIndex(pc.ic[1], pc.ic[0], i, &len);
		if(len > insize) {
			insize = insize+len;
			ic_name = realloc(ic_name, insize);
		}
		
		ce.icnames[i] = malloc(sizeof(char)*insize+1);
		
		GetValueFromIndex(pc.ic[1], pc.ic[0], i, ic_name); 	// Input channel name
		
		strcpy(ce.icnames[i], ic_name);
		
		sprintf(tname, "VoltInput%02d", i+1);	// Channel name in our task.
		
		// Create the channel
		if(rv = DAQmxCreateAIVoltageChan(ce.aTask, ic_name, tname, DAQmx_Val_Cfg_Default,
			(float64)(-uidc.range[i]), (float64)(uidc.range[i]), DAQmx_Val_Volts, NULL))
			goto error;
	}
	
	// Create the counter channel
	GetCtrlValStringLength(pc.cc[1], pc.cc[0], &len);
	cc_name = malloc(len+1);
	cc_out_name = malloc(strlen("Ctr0InternalOutput")+1);
	
	// Figure out what the ouput channel is - I can't find a way to programmatically detect
	// this, so for now it's just a switch based on this specific application.
	GetCtrlVal(pc.cc[1], pc.cc[0], cc_name);

	if(strstr(cc_name, "ctr0") != NULL)
		sprintf(cc_out_name, "Ctr0InternalOutput");
	else if (strstr(cc_name, "ctr1") != NULL) 
		sprintf(cc_out_name, "Ctr1InternalOutput");
	else {
		MessagePopup("Counter Error", "Unsupported counter channel - choose ctr0 or ctr1\n\
						If these options do not exist, you are fucked. Sorry, mate.\n");
		rv = -150;
		goto error;
	}
	
	if(ce.ccname != NULL) {
		free(ce.ccname);
		ce.ccname = NULL;	
	}
	
	ce.ccname = cc_out_name;
	
	/*****TODO*****
	Update this to work with windowed acquisitions - should be easy
	**************/

	// Create the output, when this task is started and triggered, it will generate
	// a train of pulses at frequency sr, until np pulses have occured.
	DAQmxCreateCOPulseChanFreq(ce.cTask, cc_name, "Counter", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, sr, 0.5);
	
	// Set the trigger for the counter and make it retriggerable
	int edge;
	GetCtrlValStringLength(pc.trigc[1], pc.trigc[0], &len);
	tc_name = malloc(len+1);
	
	GetCtrlVal(pc.trigc[1], pc.trigc[0], tc_name);	// Trigger channel name
	GetCtrlVal(pc.trige[1], pc.trige[0], &edge);	// Trigger edge (rising or falling)
	
	if(rv = DAQmxCfgDigEdgeStartTrig(ce.cTask, tc_name, (int32)edge))
		goto error;
	
	if(rv = DAQmxSetTrigAttribute(ce.cTask, DAQmx_StartTrig_Retriggerable, TRUE))
		goto error;
	
	// Set the counter output channel as the clock foro the analog task
	if (rv = DAQmxCfgSampClkTiming(ce.aTask, cc_out_name, (float64)sr, DAQmx_Val_Rising, 
									DAQmx_Val_ContSamps, np)) { goto error;}
	
	// Set the buffer for the input. Currently there is no programmatic protection against overruns
	rv = DAQmxSetBufferAttribute(ce.aTask, DAQmx_Buf_Input_BufSize, (np*nc)+1000); // Extra 1000 in case
	
	error:
	if(tname != NULL)
		free(tname);
	
	if(ic_name != NULL)
		free(ic_name);

	if(cc_name != NULL)
		free(cc_name);
	
	if(cc_out_name != NULL) 
		free(cc_out_name);
	
	if(ce.ccname != NULL)
		ce.ccname = NULL;
	
	if(tc_name != NULL)
		free(tc_name);
	
	if(rv != 0) {
		if(ce.atset)
			DAQmxClearTask(ce.aTask);
		if(ce.ctset)
			DAQmxClearTask(ce.cTask);
	}
	
	CmtReleaseLock(lock_DAQ);
	return rv;
}

int clear_DAQ_task() {
	int rv = 0;
	CmtGetLock(lock_DAQ);
	
	// Acquisition task
	if(ce.atset) {
		if(!(rv = DAQmxClearTask(ce.aTask)))
			ce.atset = 0;
	}
	
	// Counter task
	if(ce.ctset) {
		int rv2 = rv;
		if(!(rv = DAQmxClearTask(ce.cTask))) {
			ce.ctset = 0;
			rv = rv2;
		}
	}
	
	CmtReleaseLock(lock_DAQ);
	return rv;
	
}

//////////////////////////////////////////////////////////////
// 															//
//				   PulseBlaster Interaction					//	
// 															//
//////////////////////////////////////////////////////////////










