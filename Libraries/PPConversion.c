//////////////////////////////////////////////////////////////////////
// 																	//
//	      	   		 PPConversion Library							//
//					Paul Ganssle, 03/05/2012						//
//																	//
//	This library is designed for converting older versions of the	//
// pulse program and/or data saving libraries to the newer version. //
// Currently only supports v1.0 and above.							//
//																	//
//////////////////////////////////////////////////////////////////////
#include <utility.h>
#include "toolbox.h"
#include <ansi_c.h>
#include <PulseProgramTypes.h>
#include <FileSave.h>
#include <MathParserLib.h>
#include <PulseProgramLib.h>
#include <PPConversion.h>
#include <UIControls.h>
#include <General.h>
#include <MCUserDefinedFunctions.h>
#include <Version.h>

int SavePulseProgramDDC(char *filename, int safe, PPROGRAM *ip) {
	// Feed this a filename and and a PPROGRAM and it will create a pulse program
	// in the netCDF file with base variable name ppname. If NULL is passed to group
	// then it is assumed that this is a PPROGRAM-only file, and no base filename will
	// be used. Additionally, if group is not NULL, the netcdf file will be assumed 
	// to already exist, and as such it will not be created. 
	
	int rv = 0;			// For error checking in the netCDF functions
	char *fnbuff = NULL, *fname = NULL;
	PPROGRAM *p = NULL;
	int locked = 0;
	
	if(filename == NULL)
		return 0;
	
	if(ip == NULL) { 
			p = safe?get_current_program_safe():get_current_program();
		if(p == NULL) {
			rv = -247;
			goto error;
		}
	} else {
		p = ip;
	}
								
	// Make sure that it has a .tdm ending.
	int i;
		
	if(strcmp(".tdm", &filename[strlen(filename)-4]) != 0) { 
		// Add the TDM if it's not there.
		fnbuff = malloc(strlen(filename)+strlen(".tdm")+1);
		sprintf(fnbuff, "%s%s", filename, ".tdm");
		fname = fnbuff;
	} else {
		fname = filename;
	}
	
	int type = MCTD_TDM;
	
	char *title = NULL;
	title = malloc(MAX_FILENAME_LEN);
	get_name(filename, title, ".tdm");
	
	if(safe) { 
		CmtGetLock(lock_tdm);    
		locked = 1;
	}
	
	// Create seems to freak out if the file already exists. 
	if(FileExists(filename, 0))   { DeleteFile(filename); }
	
	char *index = malloc(strlen(filename)+7);
	if(type == MCTD_TDMS) {
		sprintf(index, "%s%s", filename, "_index");
		if(FileExists(index, NULL)) { DeleteFile(index); }
	} else if (type == MCTD_TDM) {
		strcpy(index, filename);
		index[strlen(index)-1] = 'x';
		if(FileExists(index, NULL)) { DeleteFile(index); }
	}
	
	free(index);

	DDCFileHandle p_file;
	DDCChannelGroupHandle pcg;
	
	if(rv = DDC_CreateFile(fname, NULL, title, "", title, MC_VERSION_STRING, &p_file)) { goto error; }
	
	if(rv = DDC_AddChannelGroup(p_file, MCTD_PGNAME, "The group containing the program", &pcg)) {
		goto error;
	}
	
	if(rv = save_programDDC(pcg, p)) { goto error; }
	
	 if(rv = DDC_SaveFile(p_file))  { goto error; }
	
	error:
	DDC_CloseFile(p_file);
	
	if(ip == NULL && p != NULL) { free_pprog(p); }
	if(title != NULL) { free(title); }
	if(fnbuff != NULL) { free(fnbuff); }
	if(rv != 0) { 
		DeleteFile(filename);
		
		// Depending on when the error happened, we may have left the index
		// file still hanging around. Clean up if necessary.
		char *buff = malloc(strlen(filename)+strlen("_index")+1);
		sprintf(buff, "%s%s", filename, "_index");
		
		if(FileExists(buff, NULL))
			DeleteFile(buff);
		
		free(buff);
	}
	
	if(safe && locked) {
		CmtReleaseLock(lock_tdm);
	}

	return rv;
}

PPROGRAM *LoadPulseProgramDDC(char *filename, int safe, int *err_val) {
	// Loads a netCDF file to a PPROGRAM. p is dynamically allocated
	// as part of the function, so make sure to free it when you are done with it.
	// Passing NULL to group looks for a program in the root group, otherwise
	// the group "group" in the root directory is used.
	// p is freed on error.
	
	// Returns 0 if successful, positive integers for errors.
	
	int ev = err_val[0] = 0, ng, locked = 0;
	DDCFileHandle p_file = NULL;
	DDCChannelGroupHandle pcg, *groups = NULL;
	PPROGRAM *p = NULL;
	char *name_buff = NULL;
	int tdms = 0;
	
	if(strcmp(&filename[strlen(filename)-5], ".tdms") == 0)
		tdms = 1;
	else if (strcmp(&filename[strlen(filename)-4], ".tdm") == 0)
		tdms = 0;
	else {
		ev = -250;
		goto error;
	}
	
	if(safe) {
		CmtGetLock(lock_tdm);
		locked = 1;
	}
	
	if(ev = DDC_OpenFileEx(filename, tdms?"TDMS":"TDM", 1, &p_file))
		goto error;
	
	// Need to get a list of the groups available.
	if(ev = DDC_GetNumChannelGroups(p_file, &ng))
		goto error;
	else if (ng < 1) {
		ev = -249;
		goto error;
	}
	
	groups = malloc(sizeof(TDMSChannelGroupHandle)*ng);
	
	if(ev = DDC_GetChannelGroups(p_file, groups, ng))
		goto error;
	
	// Search for the program group in the list of groups.
	int i, len;
	int g = 40, s = 40;
	name_buff = malloc(g);
	for(i = 0; i < ng; i++) {
		if(ev = DDC_GetChannelGroupStringPropertyLength(groups[i], TDMS_CHANNELGROUP_NAME, &len))
			goto error;
		
		if(g < ++len)
			name_buff = realloc(name_buff, g+=((int)((len-g)/s)+1)*s);
		
		if(ev = DDC_GetChannelGroupProperty(groups[i], TDMS_CHANNELGROUP_NAME, name_buff, len))
			goto error;
		
		if(strcmp(name_buff, MCTD_PGNAME) == 0) {
			pcg = groups[i];
			break;
		}
	}
	
	free(name_buff);
	free(groups);
	groups = NULL;
	name_buff = NULL;

	if(i >= ng) {
		ev = -248;
		goto error;
	}
	
	p = load_programDDC(pcg, &ev);
	
	error:

	if(p_file != NULL) { DDC_CloseFile(p_file); }
	if(groups != NULL) { free(groups); }
	if(name_buff != NULL) { free(name_buff); }
	
	if(safe && locked) { CmtReleaseLock(lock_tdm); }
	
	err_val[0] = ev;
	
	return p;
}

int save_programDDC(DDCChannelGroupHandle pcg, PPROGRAM *p) {
	// Save program as either a TDM or TDMS. 

	int i;
	int *idata = NULL;
	double *ddata = NULL;
	unsigned char *skip_locs = NULL;
	char *data_exprs = NULL, *delay_exprs = NULL, *ao_exprs = NULL, *ao_chans = NULL;
	
	// We're going to start out with a bunch of properties										
	DDC_CreateChannelGroupProperty(pcg, MCTD_PNP, DDC_Int32, p->np);						// Number of points
	DDC_CreateChannelGroupProperty(pcg, MCTD_PSR, DDC_Double, p->sr);						// Sampling Rate
	DDC_CreateChannelGroupProperty(pcg, MCTD_PNT, DDC_Int32, p->nt);						// Number of transients
	DDC_CreateChannelGroupProperty(pcg, MCTD_PTMODE, DDC_Int32, (p->tmode>=0)?p->tmode:-1);	// Transient acquisition mode.
	DDC_CreateChannelGroupProperty(pcg, MCTD_PSCAN, DDC_Int32, p->scan);					// Whether or not there's a scan
	DDC_CreateChannelGroupProperty(pcg, MCTD_PTRIGTTL, DDC_Int32, p->trigger_ttl);    		// Trigger TTL
	DDC_CreateChannelGroupProperty(pcg, MCTD_PTOTTIME, DDC_Double, p->total_time);			// Total time
	DDC_CreateChannelGroupProperty(pcg, MCTD_PNINSTRS, DDC_Int32, p->n_inst);    			// Base number of instructions
	DDC_CreateChannelGroupProperty(pcg, MCTD_PNUINSTRS, DDC_Int32, p->nUniqueInstrs);  		// Total number of unique instructions
	DDC_CreateChannelGroupProperty(pcg, MCTD_PVAR, DDC_Int32, p->varied);					// Whether or not it's varied
	DDC_CreateChannelGroupProperty(pcg, MCTD_PNVAR, DDC_Int32, p->nVaried);					// Number of varied instructions
	DDC_CreateChannelGroupProperty(pcg, MCTD_PNDIMS, DDC_Int32, p->nDims);					// Number of dimensions
	DDC_CreateChannelGroupProperty(pcg, MCTD_PNCYCS, DDC_Int32, p->nCycles);				// Number of phase cycles
	DDC_CreateChannelGroupProperty(pcg, MCTD_PSKIP, DDC_Int32, p->skip);					// Whether or not there are skips
	DDC_CreateChannelGroupProperty(pcg, MCTD_PMAX_N_STEPS, DDC_Int32, p->max_n_steps);		// Total number of steps in the experiment
	DDC_CreateChannelGroupProperty(pcg, MCTD_PREAL_N_STEPS, DDC_Int32, p->real_n_steps);	// Total number of steps - those skipped
	DDC_CreateChannelGroupProperty(pcg, MCTD_NFUNCS, DDC_Int32, p->nFuncs);					// Number of functions actually used
	DDC_CreateChannelGroupProperty(pcg, MCTD_TFUNCS, DDC_Int32, p->tFuncs);					// Total number of functions available
	DDC_CreateChannelGroupProperty(pcg, MCTD_NAOUT, DDC_Int32, p->nAout);					// Number of analog outputs
	DDC_CreateChannelGroupProperty(pcg, MCTD_NAOVAR, DDC_Int32, p->n_ao_var);				// Number of analog outputs which vary.
	
	// Now we'll save the main instructions
	DDCChannelHandle flags_c, time_c, ins_c, insd_c, us_c ; // Instruction channels.

	// Now create and populate the channels for the instructions.
	int nui = p->nUniqueInstrs;
	idata = malloc(sizeof(int)*nui);
	ddata = malloc(sizeof(double)*nui);
	
	// Flags
	DDC_AddChannel(pcg, DDC_Int32, MCTD_PROGFLAG, "Binary flags for each unique instruction", "", &flags_c);
	for(i = 0 ; i < nui; i++)
		idata[i] = p->instrs[i]->flags;	// Get the flags as an array of ints.
	DDC_AppendDataValues(flags_c, idata, nui); // Save the flags

	// Instruction delay
	DDC_AddChannel(pcg, DDC_Double, MCTD_PROGTIME, "Instruction delay times for each instruction", "ns", &time_c);
	for(i = 0 ; i < nui; i++)
		ddata[i] = p->instrs[i]->instr_time;	
	DDC_AppendDataValues(time_c, ddata, nui);
	
	// Instruction
	DDC_AddChannel(pcg, DDC_Int32, MCTD_PROGINSTR, "The instruction value for each unique instruction", "", &ins_c);
	for(i = 0; i < nui; i++)
		idata[i] = p->instrs[i]->instr;
	DDC_AppendDataValues(ins_c, idata, nui);
	
	// Instruction Data
	DDC_AddChannel(pcg, DDC_Int32, MCTD_PROGID, "Instruction data for each unique instruction", "", &insd_c);
	for(i = 0; i < nui; i++)
		idata[i] = p->instrs[i]->instr_data;
	DDC_AppendDataValues(insd_c, idata, nui);
	
	// Units and scan
	DDC_AddChannel(pcg, DDC_Int32, MCTD_PROGUS, "Bit 0 is whether or not to scan, the remaining bits are units for each instr", "", &us_c);
	for(i = 0; i < nui; i++) { 
		idata[i] = p->instrs[i]->trigger_scan + 2*(p->instrs[i]->time_units);
	}
	
	DDC_AppendDataValues(us_c, idata, nui);
	
	free(idata);
	free(ddata);
	
	// Now add the analog outputs
	if(p->nAout) {
		// Figure out if we want to save any expressions.
		int save_exprs = 0;
		
		for(i = 0; i < p->nAout; i++) {
			if(p->ao_varied[i] == 2) {
				save_exprs = 1;
				break;
			}
		}
		
		// All the channel handles
		DDCChannelHandle ao_var_c, ao_dim_c, ao_vals_c;
		
		// Create all the channels we'll need.
		DDC_AddChannel(pcg, DDC_Int32, MCTD_AOVAR, "Array containing variation state of each Analog output.", "", &ao_var_c);
		DDC_AddChannel(pcg, DDC_Int32, MCTD_AODIM, "Dimension along which each output varies.", "", &ao_dim_c);
		DDC_AddChannel(pcg, DDC_Double, MCTD_AOVALS, "Linearly-indexed array of analog output voltages.", "V", &ao_vals_c);
		
		// Save the expressions as a property of MCTD_AOVAR if necessary.
		if(save_exprs) {
			ao_exprs = generate_nc_string(p->ao_exprs, p->nAout, NULL);
			
			DDC_CreateChannelProperty(ao_var_c, MCTD_AOEXPRS, DDC_String, ao_exprs);
			
			free(ao_exprs);
			ao_exprs = NULL;
		}
		
		// Save the simple arrays now.
		DDC_AppendDataValues(ao_var_c, p->ao_varied, p->nAout);
	 	DDC_AppendDataValues(ao_dim_c, p->ao_dim, p->nAout);
		
		// Channels are saved as a string property on ao_var_c
		ao_chans = generate_nc_string(p->ao_chans, p->nAout, NULL);
		DDC_CreateChannelProperty(ao_var_c, MCTD_AOCHANS, DDC_String, ao_chans);
		free(ao_chans);
		ao_chans = NULL;
		
		// Now we need to save the values array. This is just sequential (linear indexing)
		int nv;
		for(i = 0; i < p->nAout; i++) {
			if(p->ao_dim[i] >= 0) {
				nv = p->maxsteps[p->ao_dim[i]+p->nCycles];
			} else {
				nv = 1;
			}
			DDC_AppendDataValues(ao_vals_c, p->ao_vals[i], nv);	
		}
	}
	
	// Save the maxsteps if necessary
	if(p->varied) {
		DDCChannelHandle msteps_c;
		
		// Maxsteps
		DDC_AddChannel(pcg, DDC_Int32, MCTD_PMAXSTEPS, "Size of the acquisition space", "", &msteps_c);
		DDC_AppendDataValues(msteps_c, p->maxsteps, (p->nDims+p->nCycles));
	}
	
	// The next bit only has to happen if this is a varied experiment
	if(p->nVaried) {
		// All the channel handles
		DDCChannelHandle v_ins_c, v_ins_dims_c, v_ins_mode_c; 
		DDCChannelHandle v_ins_locs_c = NULL, delay_exprs_c, data_exprs_c;
		
		// Again, we create a bunch of channels here and populate them as necessary
		// Varied instructions, dims and modes
		DDC_AddChannel(pcg, DDC_Int32, MCTD_PVINS, "List of the varied instructions", "", &v_ins_c);
		DDC_AddChannel(pcg, DDC_Int32, MCTD_PVINSDIM, "What dimension(s) the instrs vary along", "", &v_ins_dims_c);
		DDC_AddChannel(pcg, DDC_Int32, MCTD_PVINSMODE, "Variation mode", "", &v_ins_mode_c);
		
		DDC_AppendDataValues(v_ins_c, p->v_ins, p->nVaried);
		DDC_AppendDataValues(v_ins_dims_c, p->v_ins_dim, p->nVaried);
		DDC_AppendDataValues(v_ins_mode_c, p->v_ins_mode, p->nVaried);
		
		// Save the delay and data expressions as a single string property on the v_ins channel.
		delay_exprs = generate_nc_string(p->delay_exprs, p->nVaried, NULL);
		data_exprs = generate_nc_string(p->data_exprs, p->nVaried, NULL);
	  
		DDC_CreateChannelProperty(v_ins_c, MCTD_PDELEXPRS, DDC_String, delay_exprs);
		DDC_CreateChannelProperty(v_ins_c, MCTD_PDATEXPRS, DDC_String, data_exprs);
		
		free(delay_exprs);
		free(data_exprs);
		
		// This is a bit trickier - TDMS doesn't seem to support multi-dimensional arrays, so we're going to
		// do this in a linear-indexed fashion.
		DDC_AddChannel(pcg, DDC_Int32, MCTD_PVINSLOCS, "Instruction indexes, multidimensional linear-indexed array", "", &v_ins_locs_c);
	
		for(i = 0; i < p->nVaried; i++) 
			DDC_AppendDataValues(v_ins_locs_c, p->v_ins_locs[i], p->max_n_steps); 
		
		// Now we can save the skip stuff, if necessary.
		if(p->skip & 2) {
			// First we add the property, because at this point we just know that we saved a skip expression
			DDC_CreateChannelGroupProperty(pcg, MCTD_PSKIPEXPR, DDC_String, p->skip_expr);
			
			// Now generate the skip locs array.
			if(p->skip & 1 && (p->skip_locs != NULL)) {
				DDCChannelHandle skip_locs_c;
				DDC_AddChannel(pcg, DDC_UInt8, MCTD_PSKIPLOCS, "An array of where the skips occur", "", &skip_locs_c);
				skip_locs = malloc(sizeof(unsigned char)*p->max_n_steps);
				for(i = 0; i < p->max_n_steps; i++)
					skip_locs[i] = (unsigned char)p->skip_locs[i];
				
				DDC_AppendDataValues(skip_locs_c, skip_locs, p->max_n_steps);
				free(skip_locs);
			} else 
				p->skip -= p->skip & 1;
		}
	

	}
	
	return 0;	
}

int save_programDDC_safe(DDCChannelGroupHandle pcg, PPROGRAM *p) {
	CmtGetLock(lock_tdm);
	int rv = save_programDDC(pcg, p);
	CmtReleaseLock(lock_tdm);
	
	return rv;
}

PPROGRAM *load_programDDC(DDCChannelGroupHandle pcg, int *err_val) {
	// Loads programs from the Channel Group "pcg" in a TDM streaming library
	
	// Variable declarations
	PPROGRAM *p = malloc(sizeof(PPROGRAM));
	int on = 0, *idata = NULL, ev = 0, nvi = 0, *vfound = NULL, nchans = 0;
	double *ddata = NULL;
	DDCChannelHandle *handles = NULL;
	char **names = NULL, *delay_exprs = NULL, *data_exprs = NULL;
	char *ao_exprs = NULL, *ao_chans = NULL;
	unsigned char *skip_locs = NULL;
	err_val[0] = ev;
	
	// We're going to start out by getting a bunch of properties
	int si = sizeof(int), sd = sizeof(double);
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PNP, &p->np, si)) {					// Number of points
		if(ev != DDC_PropertyDoesNotExist)
			goto error;
		else 
			GetCtrlVal(pc.np[1], pc.np[0], &p->np);
	}
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PSR, &p->sr, sd)) {					// Sampling Rate
		if(ev != DDC_PropertyDoesNotExist)
			goto error;
		else
			GetCtrlVal(pc.sr[1], pc.sr[0], &p->sr);
	}
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PNT, &p->nt, si)) {					// Number of transients
		if(ev != DDC_PropertyDoesNotExist) { goto error; }
		GetCtrlVal(pc.nt[1], pc.nt[0], &p->nt);
	}
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PTMODE, &p->tmode, si)) {
		if(ev != DDC_PropertyDoesNotExist) { goto error; }
		GetCtrlVal(pc.tfirst[1], pc.tfirst[0], &p->tmode);
	}
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_NAOUT, &p->nAout, si)) 
	{ 
		if(ev != DDC_PropertyDoesNotExist) { goto error; }
		p->nAout = 0;
	}
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_NAOVAR, &p->n_ao_var, si)) {
		if(ev != DDC_PropertyDoesNotExist) { goto error; }
		p->n_ao_var = 0;
	}
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PSCAN, &p->scan, si)){ goto error; } 
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PTRIGTTL, &p->trigger_ttl, si)){ goto error; } 
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PTOTTIME, &p->total_time, sd)){ goto error; } 
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PNINSTRS, &p->n_inst, si)){ goto error; } 
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PNUINSTRS, &p->nUniqueInstrs, si)) { goto error; } 
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PVAR, &p->varied, si)) { goto error; } 
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PNVAR, &p->nVaried, si)) { goto error; }
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PNDIMS, &p->nDims, si)) { goto error; }
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PNCYCS, &p->nCycles, si))	{ goto error; }
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PSKIP, &p->skip, si)) { goto error; }
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PMAX_N_STEPS, &p->max_n_steps, si)) { goto error; }
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PREAL_N_STEPS, &p->real_n_steps, si)) { goto error; }
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_NFUNCS, &p->nFuncs, si)) { goto error; }
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_TFUNCS, &p->tFuncs, si)) { goto error; }
	
	
	// Now we need to go through one by one and get the channel handles.
	int i, j, k = 0;
	int nchan_names = 14; 
	DDCChannelHandle flags_c, time_c, ins_c, us_c, insd_c;
	DDCChannelHandle msteps_c, vins_c, vi_dim_c, vi_mode_c, vi_locs_c, s_locs_c;
	DDCChannelHandle ao_var_c, ao_dim_c, ao_vals_c;
	flags_c = time_c = ins_c = us_c = insd_c = msteps_c = vins_c = vi_dim_c = NULL;
	vi_mode_c = vi_locs_c = s_locs_c = ao_var_c = ao_dim_c = ao_vals_c = NULL;
	
	char *chan_names[] = {MCTD_PROGFLAG, MCTD_PROGTIME, MCTD_PROGINSTR, MCTD_PROGUS, MCTD_PROGID, MCTD_PMAXSTEPS, 
						  MCTD_PVINS, MCTD_PVINSDIM, MCTD_PVINSMODE, MCTD_PVINSLOCS, MCTD_PSKIPLOCS, MCTD_AOVAR, 
						  MCTD_AODIM, MCTD_AOVALS};
	DDCChannelHandle *chan_hs[] = {&flags_c, &time_c, &ins_c, &us_c, &insd_c, &msteps_c, &vins_c, &vi_dim_c, 
								   &vi_mode_c, &vi_locs_c, &s_locs_c, &ao_var_c, &ao_dim_c, &ao_vals_c};
	
	
	// Get the number of channels and a list of the channel handles.
	if(ev = DDC_GetNumChannels(pcg, &nchans)) { goto error; }
	
	handles = malloc(sizeof(DDCChannelHandle)*nchans);

	if(ev = DDC_GetChannels(pcg, handles, nchans)) { goto error; }
	
	// Get the names of all the channels
	names = calloc(nchans, sizeof(char*));	// Null-initialized array of names
	int len;
	
	for(i = 0; i < nchans; i++) {
		if(ev = DDC_GetChannelStringPropertyLength(handles[i], DDC_CHANNEL_NAME, &len)) { goto error; }
		
		names[i] = malloc(++len);
		
		if(ev = DDC_GetChannelProperty(handles[i], DDC_CHANNEL_NAME, names[i], len)) { goto error; }
	}

	// Match the names of the elements to the strings
	int *matches = strings_in_array(names, chan_names, nchans, nchan_names);
	
	// Go through and associate the channels with the matches.
	for(i = 0; i < nchan_names; i++) { 
		if(matches[i] >= 0) { memcpy(chan_hs[i], &handles[matches[i]], sizeof(DDCChannelHandle)); }
	}

	free(handles);
	handles = NULL;
	names = free_string_array(names, nchans);
	
	// At this point, we should have more than enough to complete the allocation of the p file.
	create_pprogram(p);
	on = 1;
	
	// First we'll read out the instructions
	int nui = p->nUniqueInstrs;
	idata = malloc(si*nui);
	ddata = malloc(sd*nui);
	
	// Flags
	if(ev = DDC_GetDataValues(flags_c, 0, nui, idata)) { goto error; }
	for(i = 0; i < nui; i++) { 
		p->instrs[i]->flags = idata[i];
	}
	
	// Instruction Delay
	if(ev = DDC_GetDataValues(time_c, 0, nui, ddata)) { goto error; }
	for(i = 0; i < nui; i++) {
		p->instrs[i]->instr_time = ddata[i];
	}
	
	// Instructions
	if(ev = DDC_GetDataValues(ins_c, 0, nui, idata)) { goto error; }
	for(i = 0; i < nui; i++) {
		p->instrs[i]->instr = idata[i];
	}
	
	// Units and scan
	if(ev = DDC_GetDataValues(us_c, 0, nui, idata)) { goto error; }
	for(i = 0; i < nui; i++) {
		p->instrs[i]->trigger_scan = 1 & idata[i];
		p->instrs[i]->time_units = (int)(idata[i]>>1);
	}
	
	// Instruction Data
	if(ev = DDC_GetDataValues(insd_c, 0, nui, idata)) { goto error; }
	for(i = 0; i < nui; i++) {
		p->instrs[i]->instr_data = idata[i];
	}
	
	free(idata);
	free(ddata);
	idata = NULL;
	ddata = NULL;
	
	// Get the stuff you need to if the experiment's varied.
	nvi = p->nVaried;
	if(p->varied) {
		// MaxSteps  
		if(ev = DDC_GetDataValues(msteps_c, 0, p->nCycles+p->nDims, p->maxsteps)) { goto error; }
	}
	
	// Now if the instructions are varied.
	if(nvi) {
		// VIns, VInsDims, VInsMode
		if(ev = DDC_GetDataValues(vins_c, 0, p->nVaried, p->v_ins)) { goto error; }
		if(ev = DDC_GetDataValues(vi_dim_c, 0, p->nVaried, p->v_ins_dim)) { goto error; }
		if(ev = DDC_GetDataValues(vi_mode_c, 0, p->nVaried, p->v_ins_mode)) { goto error; }
		
		// Delay and data instructions are stored as a property on VIns.
		int ldel, ldat;
		if(ev = DDC_GetChannelStringPropertyLength(vins_c, MCTD_PDELEXPRS, &ldel)) { goto error; }
		if(ev = DDC_GetChannelStringPropertyLength(vins_c, MCTD_PDATEXPRS, &ldat)) { goto error; }
		
		delay_exprs = malloc(++ldel);
		data_exprs = malloc(++ldat);
		
		if(ev = DDC_GetChannelProperty(vins_c, MCTD_PDELEXPRS, delay_exprs, ldel)) { goto error; }
		if(ev = DDC_GetChannelProperty(vins_c, MCTD_PDATEXPRS, data_exprs, ldat)) { goto error; }
		
		// Now we need to get those data expressions as an array.
		int ns = nvi;
		p->delay_exprs = free_string_array(p->delay_exprs, nvi);
		p->delay_exprs = get_nc_strings(delay_exprs, &ns);
		
		if(ns != nvi) {
			ev = -250;
			goto error;
		}
		
		p->data_exprs = free_string_array(p->data_exprs, nvi);
		p->data_exprs = get_nc_strings(data_exprs, &ns);
		if(ns != nvi) {
			ev = -250;
			goto error;
		}
		
		// Now we need to read the v_ins_locs, which has been stored as a single array, into a 2D array.
		for(i = 0; i < nvi; i++) {
			if(ev = DDC_GetDataValues(vi_locs_c, i*p->max_n_steps, p->max_n_steps, p->v_ins_locs[i])) { goto error; }
		}
		
		// Now we get the delay and data expressions
	
		if(p->skip & 2) {
			// First we get the skip expression if it's been saved.
			if(ev = DDC_GetChannelGroupStringPropertyLength(pcg, MCTD_PSKIPEXPR, &len)) { goto error; }
			
			if(p->skip_expr == NULL) {
				p->skip_expr = malloc(++len);
			} else {
				p->skip_expr = realloc(p->skip_expr, ++len);
			}
			
			if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PSKIPEXPR, p->skip_expr, len)) { goto error; }
			
			if(p->skip & 1 && s_locs_c != NULL) {
				skip_locs = malloc(sizeof(unsigned char)*p->max_n_steps);
				if(ev = DDC_GetDataValues(s_locs_c, 0, p->max_n_steps, skip_locs)) { goto error; }
			
				// Copy this over. There's an implicit cast involved, so I'm not using memcpy.
				for(i = 0; i < p->max_n_steps; i++) { p->skip_locs[i] = skip_locs[i]; }
				
				free(skip_locs);
				skip_locs = NULL;
			}
		}
	}
	
	// Get the analog outputs.
	if(p->nAout) {
		if(DDC_GetDataValues(ao_var_c, 0, p->nAout, p->ao_varied)) { goto error; }
		if(DDC_GetDataValues(ao_dim_c, 0, p->nAout, p->ao_dim)) { goto error; }
		
		int get_exprs = 0;
		for(i = 0; i < p->nAout; i++) { 
			if(p->ao_varied[i] == 2) {
				get_exprs = 1;
				break;
			}
		}
		
		int ns = p->nAout;
		
		// Get the channels
		if(ev = DDC_GetChannelStringPropertyLength(ao_var_c, MCTD_AOCHANS, &len)) { goto error; }
		ao_chans = malloc(++len);
		
		if(ev = DDC_GetChannelProperty(ao_var_c, MCTD_AOCHANS, ao_chans, len)) { goto error; }
		
		p->ao_chans = get_nc_strings(ao_chans, &ns);
		if(ns != p->nAout) {
			ev = -250;
			goto error;
		}
		
		// Get expressions if necessary
		if(get_exprs) {
			if(ev = DDC_GetChannelStringPropertyLength(ao_var_c, MCTD_AOEXPRS, &len)) { goto error; }
			
			ao_exprs = malloc(++len);
			
			if(ev = DDC_GetChannelProperty(ao_var_c, MCTD_AOEXPRS, ao_exprs, len)) { goto error; }
			
			p->ao_exprs = get_nc_strings(ao_exprs, &ns);
			free(ao_exprs);
			
			if(ns != p->nAout) {
				ev = -25;
				goto error; 
			}
		}
		
		// Now grab the values array, it's stored sequentially.
		int nv, k = 0;
		for(i = 0; i < p->nAout; i++) { 
			if(p->ao_dim[i] >= 0) {
				nv = p->maxsteps[p->ao_dim[i]+p->nCycles];
			} else {
				nv = 1;
			}
			
			p->ao_vals[i] = malloc(sizeof(double)*nv);
			
			if(ev = DDC_GetDataValues(ao_vals_c, k, nv, p->ao_vals[i])) { goto error; }

			k+=nv;
		}
	}
	
	error:
	
	err_val[0] = ev;
	
	if(ao_chans != NULL) { free(ao_chans); }
	if(ao_exprs != NULL) { free(ao_exprs); }
	if(data_exprs != NULL) { free(data_exprs); }
	if(delay_exprs != NULL) { free(delay_exprs); }
	
	if(idata != NULL) { free(idata); }
	if(ddata != NULL) { free(ddata); }
	
	if(skip_locs != NULL) { free(skip_locs); }

	if(ev) { 
		if(!on) {
			free(p);
		} else {
			free_pprog(p);
		}
		
		p = NULL;
	}
	
	free_string_array(names, nchans);
	
	return p;
}

PPROGRAM *load_programDDC_safe(DDCChannelGroupHandle pcg, int *err_val) {
	CmtGetLock(lock_tdm);
	PPROGRAM *rv = load_programDDC(pcg, err_val);
	CmtReleaseLock(lock_tdm);
	
	return rv;
}

void display_ddc_error(int err_val) {
	// Legacy code -> Will be eliminated
	switch(err_val) {	
		case -247:
			MessagePopup("TDM Error", "Invalid/NULL pulse program.");
		case -248:
			MessagePopup("TDM Error", "Program Channel Group Not Found");
			break;
		case -249:
			MessagePopup("TDM Error", "No Channel groups found in this file");
			break;
		case -250:
			MessagePopup("TDM Error", "File type not valid.");
		default:
			MessagePopup("TDM Error", DDC_GetLibraryErrorDescription(err_val));
	}
}
