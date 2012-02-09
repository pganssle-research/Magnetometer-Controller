//////////////////////////////////////////////////////////////////////
// 																	//
//				  Pulse Program Parsing Utility v1.0				//
//					Paul Ganssle, 07/15/2011						//
//																	//
//	This is a library intended for use with pulse programs as used	//
//	in the Magnetometer Controller software for the zero-field		//
//	magnetometer programs.											//
// 																	//
//////////////////////////////////////////////////////////////////////


/***************************  Version History  ******************************

1.0 	This is the first version of this as a library separate from the 
		main source code. It implements UIControls to make it easier to 
		move things around on panels, by storing each control as a 2-int
		array containing both the panel it's on and the control index.
		
		(This does not apply to cinst or inst instructions)
		
		This version also implements a new netCDF serialization format for
		saving pulse programs, and implements a more versatile pulse
		programming language including skip conditions in sampling space,
		variation along arbitrary (non-linear) dimensions, and more robust
		phase cycling.
		
*****************************************************************************/

/***************************         To Do       *****************************
Project	: Save/Load Programs and functions
Progress: 90%
Priority: High
Descrip.: Self-explanatory. Load a PPROGRAM from a netcdf file or save a
	PPROGRAM to a netcdf file. Do the second one first, obviously.
	
	Milestones:
	Save Program interface for PPROGRAMs done  	(Done, not debugged)
	Load Program interface for PPROGRAMS done   (Done, not debugged)
	Save Function interface (pfuncs) done		(Core function done and not debugged, wrapper not started)
	Load Program interface (pfuncs) done		(Core function done and not debugged, wrapper not started)

	
Project	: Tweaks to PPROGRAM
Progress: 0%
Priority: Low-Moderate
Desrip	: Need to add a few things to PPROGRAM that I did not consider at
		the outset of the project and add those things in to get_current_program
		as well as the save/load functions, and in some cases some UI tweaks
		
		Milestones:
		Comments -> May also want to implement this in pfuncs. Add UI interface as well
		Expressions for instr_data -> Add UI interface and add to pprogram.
		Expression strings -> Save the expressions used to generate instr_data and delays if necessary
	
******************************************************************************/

#include "toolbox.h"
#include <userint.h>
#include <ansi_c.h>
#include <spinapi.h>					// SpinCore functions
#include <NIDAQmx.h>

#include <PulseProgramTypes.h>
#include <cvitdms.h>
#include <cviddc.h>
#include <UIControls.h>					// For manipulating the UI controls
#include <MathParserLib.h>				// For parsing math
#include <MCUserDefinedFunctions.h>		// Main UI functions
#include <PulseProgramLib.h>			// Prototypes and type definitions for this file
#include <SaveSessionLib.h>
#include <MC10.h>
#include <General.h>

//////////////////////////////////////////////////////////////
// 															//
//				File Read/Write Operations					//
// 															//
//////////////////////////////////////////////////////////////

int SavePulseProgram(char *filename, PPROGRAM *ip) {
	// Feed this a filename and and a PPROGRAM and it will create a pulse program
	// in the netCDF file with base variable name ppname. If NULL is passed to group
	// then it is assumed that this is a PPROGRAM-only file, and no base filename will
	// be used. Additionally, if group is not NULL, the netcdf file will be assumed 
	// to already exist, and as such it will not be created. 
	
	int rv = 0;			// For error checking in the netCDF functions
	char *fnbuff = NULL, *fname = NULL;
	PPROGRAM *p = NULL;
	
	if(filename == NULL)
		return 0;
	
	if(ip == NULL) { 
		p = get_current_program();
		if(p == NULL) {
			rv = -247;
			goto error;
		}
	} else
		p = ip;
								
	// Make sure that it has a .tdm ending.
	int i;
		
	if(strcmp(".tdm", &filename[strlen(filename)-4]) != 0) { 
				fnbuff = malloc(strlen(filename)+strlen(".tdm")+1);
		sprintf(fnbuff, "%s%s", filename, ".tdm");
		fname = fnbuff;
	} else
		fname = filename;
	
	int type = MCTD_TDM;
	
	char *title = NULL;
	title = malloc(MAX_FILENAME_LEN);
	get_name(filename, title, ".tdm");
	
	if(FileExists(filename, 0)) 	  // Create seems to freak out if the file already exists.
		DeleteFile(filename);
	
	char *index = malloc(strlen(filename)+7);
	if(type == MCTD_TDMS) {
		sprintf(index, "%s%s", filename, "_index");
		if(FileExists(index, NULL))
			DeleteFile(index);
	} else if (type == MCTD_TDM) {
		strcpy(index, filename);
		index[strlen(index)-1] = 'x';
		if(FileExists(index, NULL))
			DeleteFile(index);
	}
	
	free(index);

	DDCFileHandle p_file;
	DDCChannelGroupHandle pcg;
	CmtGetLock(lock_tdm);
	if(rv = DDC_CreateFile(fname, NULL, title, "", title, "Magnetometer Controller v1.0", &p_file))
		return rv;
	
	if(rv = DDC_AddChannelGroup(p_file, MCTD_PGNAME, "The group containing the program", &pcg))
		goto error;
	
	CmtGetLock(lock_pb);
	if(rv = save_program(pcg, p))
		goto error;
	CmtReleaseLock(lock_pb);
	
	 if(rv = DDC_SaveFile(p_file))
		 goto error;
	
	error:
	DDC_CloseFile(p_file);
	CmtReleaseLock(lock_tdm);
	
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
	
	
	return rv;
}

PPROGRAM *LoadPulseProgram(char *filename, int *err_val) {
	// Loads a netCDF file to a PPROGRAM. p is dynamically allocated
	// as part of the function, so make sure to free it when you are done with it.
	// Passing NULL to group looks for a program in the root group, otherwise
	// the group "group" in the root directory is used.
	// p is freed on error.
	
	// Returns 0 if successful, positive integers for errors.
	
	int ev = err_val[0] = 0, ng;
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
	
	CmtGetLock(lock_tdm);
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
	
	p = load_program(pcg, &ev);
	
	error:

	if(p_file != NULL)
		DDC_CloseFile(p_file);
	CmtReleaseLock(lock_tdm);
	
	if(groups != NULL)
		free(groups);
	
	if(name_buff != NULL)
		free(name_buff);
	
	err_val[0] = ev;
	
	return p;
}

int save_program(DDCChannelGroupHandle pcg, PPROGRAM *p) {
	// Save program as either a TDM or TDMS. 

	int i;
	int *idata = NULL;
	double *ddata = NULL;
	char *data_exprs = NULL, *delay_exprs = NULL;
	
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
	for(i = 0; i < nui; i++)
		idata[i] = p->instrs[i]->trigger_scan + 2*(p->instrs[i]->time_units);
	DDC_AppendDataValues(us_c, idata, nui);
	
	free(idata);
	free(ddata);
	
	// The next bit only has to happen if this is a varied experiment
	if(p->varied) {
		// All the channel handles
		DDCChannelHandle msteps_c, v_ins_c, v_ins_dims_c, v_ins_mode_c; 
		DDCChannelHandle v_ins_locs_c = NULL, delay_exprs_c, data_exprs_c;
		
		// Again, we create a bunch of channels here and populate them as necessary
		// Maxsteps
		DDC_AddChannel(pcg, DDC_Int32, MCTD_PMAXSTEPS, "Size of the acquisition space", "", &msteps_c);
		DDC_AppendDataValues(msteps_c, p->maxsteps, (p->nDims+p->nCycles));
		
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
				unsigned char *skip_locs = malloc(sizeof(unsigned char)*p->max_n_steps);
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

int tdms_save_program(TDMSChannelGroupHandle pcg, PPROGRAM *p) {
	// Writes the values of the PPPROGRAM to the specified channel group.
	// There will be a few channels within this.
	int i;
	int *idata = NULL;
	double *ddata = NULL;
	char *data_exprs = NULL, *delay_exprs = NULL;
	
	// We're going to start out with a bunch of properties										
	TDMS_SetChannelGroupProperty(pcg, MCTD_PNP, TDMS_Int32, p->np);							// Number of points
	TDMS_SetChannelGroupProperty(pcg, MCTD_PSR, TDMS_Double, p->sr);						// Sampling Rate
	TDMS_SetChannelGroupProperty(pcg, MCTD_PNT, TDMS_Int32, p->nt);							// Number of transients
	TDMS_SetChannelGroupProperty(pcg, MCTD_PTMODE, TDMS_Int32, (p->tmode>=0)?p->tmode:-1);	// Transient acquisition mode.
	TDMS_SetChannelGroupProperty(pcg, MCTD_PSCAN, TDMS_Int32, p->scan);						// Whether or not there's a scan
	TDMS_SetChannelGroupProperty(pcg, MCTD_PTRIGTTL, TDMS_Int32, p->trigger_ttl);    		// Trigger TTL
	TDMS_SetChannelGroupProperty(pcg, MCTD_PTOTTIME, TDMS_Double, p->total_time);			// Total time
	TDMS_SetChannelGroupProperty(pcg, MCTD_PNINSTRS, TDMS_Int32, p->n_inst);    			// Base number of instructions
	TDMS_SetChannelGroupProperty(pcg, MCTD_PNUINSTRS, TDMS_Int32, p->nUniqueInstrs);  		// Total number of unique instructions
	TDMS_SetChannelGroupProperty(pcg, MCTD_PVAR, TDMS_Int32, p->varied);					// Whether or not it's varied
	TDMS_SetChannelGroupProperty(pcg, MCTD_PNVAR, TDMS_Int32, p->nVaried);					// Number of varied instructions
	TDMS_SetChannelGroupProperty(pcg, MCTD_PNDIMS, TDMS_Int32, p->nDims);					// Number of dimensions
	TDMS_SetChannelGroupProperty(pcg, MCTD_PNCYCS, TDMS_Int32, p->nCycles);					// Number of phase cycles
	TDMS_SetChannelGroupProperty(pcg, MCTD_PSKIP, TDMS_Int32, p->skip);						// Whether or not there are skips
	TDMS_SetChannelGroupProperty(pcg, MCTD_PMAX_N_STEPS, TDMS_Int32, p->max_n_steps);		// Total number of steps in the experiment
	TDMS_SetChannelGroupProperty(pcg, MCTD_PREAL_N_STEPS, TDMS_Int32, p->real_n_steps);		// Total number of steps - those skipped
	TDMS_SetChannelGroupProperty(pcg, MCTD_NFUNCS, TDMS_Int32, p->nFuncs);					// Number of functions actually used
	TDMS_SetChannelGroupProperty(pcg, MCTD_TFUNCS, TDMS_Int32, p->tFuncs);					// Total number of functions available
	
	// Now we'll save the main instructions
	TDMSChannelHandle flags_c, time_c, ins_c, insd_c, us_c ; // Instruction channels.

	// Now create and populate the channels for the instructions.
	int nui = p->nUniqueInstrs;
	idata = malloc(sizeof(int)*nui);
	ddata = malloc(sizeof(double)*nui);
	
	// Flags
	TDMS_AddChannel(pcg, TDMS_Int32, MCTD_PROGFLAG, "Binary flags for each unique instruction", "", &flags_c);
	for(i = 0 ; i < nui; i++)
		idata[i] = p->instrs[i]->flags;	// Get the flags as an array of ints.
	TDMS_AppendDataValues(flags_c, idata, nui, 0); // Save the flags
	
	// Instruction delay
	TDMS_AddChannel(pcg, TDMS_Double, MCTD_PROGTIME, "Instruction delay times for each instruction", "ns", &time_c);
	for(i = 0 ; i < nui; i++)
		ddata[i] = p->instrs[i]->instr_time;	
	TDMS_AppendDataValues(time_c, ddata, nui, 0);
	
	// Instruction
	TDMS_AddChannel(pcg, TDMS_Int32, MCTD_PROGINSTR, "The instruction value for each unique instruction", "", &ins_c);
	for(i = 0; i < nui; i++)
		idata[i] = p->instrs[i]->instr;
	TDMS_AppendDataValues(ins_c, idata, nui, 0);
	
	// Instruction Data
	TDMS_AddChannel(pcg, TDMS_Int32, MCTD_PROGID, "Instruction data for each unique instruction", "", &insd_c);
	for(i = 0; i < nui; i++)
		idata[i] = p->instrs[i]->instr_data;
	TDMS_AppendDataValues(insd_c, idata, nui, 0);
	
	// Units and scan
	TDMS_AddChannel(pcg, TDMS_Int32, MCTD_PROGUS, "Bit 0 is whether or not to scan, the remaining bits are units for each instr", "", &us_c);
	for(i = 0; i < nui; i++)
		idata[i] = p->instrs[i]->trigger_scan + 2*(p->instrs[i]->time_units);
	TDMS_AppendDataValues(us_c, idata, nui, 0);
	
	free(idata);
	free(ddata);
	
	// The next bit only has to happen if this is a varied experiment
	if(p->varied) {
		// All the channel handles
		TDMSChannelHandle msteps_c, v_ins_c, v_ins_dims_c, v_ins_mode_c; 
		TDMSChannelHandle v_ins_locs_c = NULL, delay_exprs_c, data_exprs_c;
		
		// Again, we create a bunch of channels here and populate them as necessary
		// Maxsteps
		TDMS_AddChannel(pcg, TDMS_Int32, MCTD_PMAXSTEPS, "Size of the acquisition space", "", &msteps_c);
		TDMS_AppendDataValues(msteps_c, p->maxsteps, (p->nDims+p->nCycles), 0);
		
		// Varied instructions, dims and modes
		TDMS_AddChannel(pcg, TDMS_Int32, MCTD_PVINS, "List of the varied instructions", "", &v_ins_c);
		TDMS_AddChannel(pcg, TDMS_Int32, MCTD_PVINSDIM, "What dimension(s) the instrs vary along", "", &v_ins_dims_c);
		TDMS_AddChannel(pcg, TDMS_Int32, MCTD_PVINSMODE, "Variation mode", "", &v_ins_mode_c);
		
		TDMS_AppendDataValues(v_ins_c, p->v_ins, p->nVaried, 0);
		TDMS_AppendDataValues(v_ins_dims_c, p->v_ins_dim, p->nVaried, 0);
		TDMS_AppendDataValues(v_ins_mode_c, p->v_ins_mode, p->nVaried, 0);
		
		// Save the delay and data expressions as a single string property on the v_ins channel.
		delay_exprs = generate_nc_string(p->delay_exprs, p->nVaried, NULL);
		data_exprs = generate_nc_string(p->data_exprs, p->nVaried, NULL);
	  
		TDMS_SetChannelProperty(v_ins_c, MCTD_PDELEXPRS, TDMS_String, delay_exprs);
		TDMS_SetChannelProperty(v_ins_c, MCTD_PDATEXPRS, TDMS_String, data_exprs);
		
		free(delay_exprs);
		free(data_exprs);
		
		// This is a bit trickier - TDMS doesn't seem to support multi-dimensional arrays, so we're going to
		// do this in a linear-indexed fashion.
		TDMS_AddChannel(pcg, TDMS_Int32, MCTD_PVINSLOCS, "Instruction indexes, multidimensional linear-indexed array", "", &v_ins_locs_c);
	
		for(i = 0; i < p->nVaried; i++) 
			TDMS_AppendDataValues(v_ins_locs_c, p->v_ins_locs[i], p->max_n_steps, 0); 
		
		// Now we can save the skip stuff, if necessary.
		if(p->skip & 2) {
			// First we add the property, because at this point we just know that we saved a skip expression
			TDMS_SetChannelGroupProperty(pcg, MCTD_PSKIPEXPR, TDMS_String, p->skip_expr);
			
			// Now generate the skip locs array.
			if(p->skip & 1 && (p->skip_locs != NULL)) {
				TDMSChannelHandle skip_locs_c;
				TDMS_AddChannel(pcg, TDMS_Boolean, MCTD_PSKIPLOCS, "An array of where the skips occur", "", &skip_locs_c);
				TDMS_AppendDataValues(skip_locs_c, p->skip_locs, p->max_n_steps, 0);
			} else 
				p->skip -= p->skip & 1;
		}
	

	}
	
	return 0;	
}

PPROGRAM *load_program(DDCChannelGroupHandle pcg, int *err_val) {
	// Loads programs from the Channel Group "pcg" in a TDM streaming library
	
	// Variable declarations
	PPROGRAM *p = malloc(sizeof(PPROGRAM));
	int on = 0, *idata = NULL, ev = 0, nvi = 0, *vfound = NULL;
	double *ddata = NULL;
	DDCChannelHandle *handles = NULL;
	char *name_buff = NULL, *delay_exprs = NULL, *data_exprs = NULL;
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
		if(ev != DDC_PropertyDoesNotExist)
			goto error;
		else
			GetCtrlVal(pc.nt[1], pc.nt[0], &p->nt);
	}
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PTMODE, &p->tmode, si)) {			// Transient acquisition mode
		if(ev != DDC_PropertyDoesNotExist)
			goto error;
		else
			GetCtrlVal(pc.tfirst[1], pc.tfirst[0], &p->tmode);
	}
		
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PSCAN, &p->scan, si))				// Whether or not there's a scan
		goto error;
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PTRIGTTL, &p->trigger_ttl, si))    	// Trigger TTL
		goto error;
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PTOTTIME, &p->total_time, sd))		// Total time
		goto error;
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PNINSTRS, &p->n_inst, si))    		// Base number of instructions
		goto error;
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PNUINSTRS, &p->nUniqueInstrs, si))	// Total number of unique instructions
		goto error;
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PVAR, &p->varied, si))				// Whether or not it's varied
		goto error;

	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PNVAR, &p->nVaried, si))				// Number of varied instructions
		goto error;

	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PNDIMS, &p->nDims, si))				// Number of dimensions
		goto error;

	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PNCYCS, &p->nCycles, si))			// Number of phase cycles
		goto error;
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PSKIP, &p->skip, si))				// Whether or not there are skips
		goto error;
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PMAX_N_STEPS, &p->max_n_steps, si))	// Total number of steps in the experiment
		goto error;
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PREAL_N_STEPS, &p->real_n_steps, si))// Total number of steps - those skipped
		goto error;
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_NFUNCS, &p->nFuncs, si))				// Number of functions actually used
		goto error;
	
	if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_TFUNCS, &p->tFuncs, si))				// Total number of functions available
		goto error;
	
	// Now we need to go through one by one and get the channel handles.
	int i, j, k = 0;
	int nchans, nchan_names = 5, nvchan_names = 6;
	DDCChannelHandle flags_c, time_c, ins_c, us_c, insd_c;
	DDCChannelHandle msteps_c, vins_c, vi_dim_c, vi_mode_c, vi_locs_c, s_locs_c;
	flags_c = time_c = ins_c = us_c = insd_c = msteps_c = vins_c = vi_dim_c = vi_mode_c = vi_locs_c = s_locs_c = NULL;
	
	char *chan_names[] = {MCTD_PROGFLAG, MCTD_PROGTIME, MCTD_PROGINSTR, MCTD_PROGUS, MCTD_PROGID};
	DDCChannelHandle *chan_hs[] = {&flags_c, &time_c, &ins_c, &us_c, &insd_c};
	
	char *v_chan_names[] = {MCTD_PMAXSTEPS, MCTD_PVINS, MCTD_PVINSDIM, MCTD_PVINSMODE, MCTD_PVINSLOCS, MCTD_PSKIPLOCS};
	DDCChannelHandle *v_chan_hs[] = {&msteps_c, &vins_c, &vi_dim_c, &vi_mode_c, &vi_locs_c, &s_locs_c};
	
	// Get the number of channels and a list of the channel handles.
	if(ev = DDC_GetNumChannels(pcg, &nchans))
		goto error;
	
	handles = malloc(sizeof(DDCChannelHandle)*nchans);
	
	if(ev = DDC_GetChannels(pcg, handles, nchans))
		goto error;
	
	// Go through each channel and get its name, and if it matches something, set that value.
	int len, g = 20, s = 20;
	name_buff = malloc(g);
	
	// Search for all the 1D bits first.
	int found = 0, *cfound = calloc(si*nchans, si);
	for(i = 0; i < nchans; i++) {
		if(ev = DDC_GetChannelStringPropertyLength(handles[i], DDC_CHANNEL_NAME, &len))
			goto error;
		
		if(g < ++len)	// If there's not enough room, allocate more room 
			name_buff = realloc(name_buff, g+=(((int)((len-g)/s)+1)*s));
		
		// Get the actual channel name
		if(ev = DDC_GetChannelProperty(handles[i], DDC_CHANNEL_NAME, name_buff, len))
			goto error;
		
		for(j = 0; j < nchan_names; j++) {
			if(1<<j & found)	// Don't search if we've already found it.
				continue;
			
			if(strcmp(chan_names[j], name_buff) == 0)
				break;
		}
		
		if(j < nchan_names) {		// In this case, we found it.
			memcpy(chan_hs[j], &handles[i], sizeof(DDCChannelHandle));
			k++;
			found += 1<<j;
			cfound[i] = 1;
			continue;
		} else if (k >= nchan_names)
			break;	
	}
	
	// Now if we need to, to through all the variation bits
	if(!p->varied)
		nvchan_names = 0;
	
	k = 0;
	found = 0;
	for(i = 0; i < nchans; i++) {
		if(cfound[i])	// Skip this one if we found it the first time around.
			continue;
		
		if(ev = DDC_GetChannelStringPropertyLength(handles[i], DDC_CHANNEL_NAME, &len))
			goto error;
		
		if(g < ++len)
			name_buff = realloc(name_buff, g+=(((int)((len-g)/s)+1)*2));
		
		if(ev = DDC_GetChannelProperty(handles[i], DDC_CHANNEL_NAME, name_buff, len))
			goto error;
		
		for(j = 0; j < nvchan_names; j++) {
			if(1<<j & found)
				continue;
			
			if(strcmp(v_chan_names[j], name_buff) == 0)
				break;
		}
		
		if(j < nvchan_names) {
			memcpy(v_chan_hs[j], &handles[i], sizeof(DDCChannelHandle));
			k++;
			found += 1<<j;
			cfound[i] = 1;
			continue;
		} else if (k >= nvchan_names)
			break;
	}
	
	free(cfound);
	free(handles);
	free(name_buff);
	handles = NULL;
	name_buff = NULL;
	
	// At this point, we should have more than enough to complete the allocation of the p file.
	create_pprogram(p);
	on = 1;
	
	// First we'll read out the instructions
	int nui = p->nUniqueInstrs;
	idata = malloc(si*nui);
	ddata = malloc(sd*nui);
	
	// Flags
	if(ev = DDC_GetDataValues(flags_c, 0, nui, idata))
		goto error;
	for(i = 0; i < nui; i++)
		p->instrs[i]->flags = idata[i];
	
	// Instruction Delay
	if(ev = DDC_GetDataValues(time_c, 0, nui, ddata))
		goto error;
	for(i = 0; i < nui; i++)
		p->instrs[i]->instr_time = ddata[i];
	
	// Instructions
	if(ev = DDC_GetDataValues(ins_c, 0, nui, idata))
		goto error;
	for(i = 0; i < nui; i++)
		p->instrs[i]->instr = idata[i];
	
	// Units and scan
	if(ev = DDC_GetDataValues(us_c, 0, nui, idata))
		goto error;
	for(i = 0; i < nui; i++) {
		p->instrs[i]->trigger_scan = 1 & idata[i];
		p->instrs[i]->time_units = (int)(idata[i]>>1);
	}
	
	// Instruction Data
	if(ev = DDC_GetDataValues(insd_c, 0, nui, idata))
		goto error;
	for(i = 0; i < nui; i++)
		p->instrs[i]->instr_data = idata[i];
	
	free(idata);
	free(ddata);
	idata = NULL;
	ddata = NULL;
	
	// Now go through and get the varied instructions
	nvi = p->nVaried;
	if(p->varied) {
		// MaxSteps
		if(ev = DDC_GetDataValues(msteps_c, 0, p->nCycles+p->nDims, p->maxsteps))
			goto error;
		
		// VIns, VInsDims, VInsMode
		if(ev = DDC_GetDataValues(vins_c, 0, p->nVaried, p->v_ins))
			goto error;
		
		if(ev = DDC_GetDataValues(vi_dim_c, 0, p->nVaried, p->v_ins_dim))
			goto error;
		
		if(ev = DDC_GetDataValues(vi_mode_c, 0, p->nVaried, p->v_ins_mode))
			goto error;
		
		// Delay and data instructions are stored as a property on VIns.
		int ldel, ldat;
		if(ev = DDC_GetChannelStringPropertyLength(vins_c, MCTD_PDELEXPRS, &ldel))
			goto error;
		
		if(ev = DDC_GetChannelStringPropertyLength(vins_c, MCTD_PDATEXPRS, &ldat))
			goto error;
		
		delay_exprs = malloc(++ldel);
		data_exprs = malloc(++ldat);
		
		if(ev = DDC_GetChannelProperty(vins_c, MCTD_PDELEXPRS, delay_exprs, ldel))
			goto error;
		
		if(ev = DDC_GetChannelProperty(vins_c, MCTD_PDATEXPRS, data_exprs, ldat))
			goto error;
		
		// Now we need to get those data expressions as an array.
		if(p->delay_exprs != NULL) {
			for(i = 0; i < nvi; i++) {
				if(p->delay_exprs[i] != NULL)
					free(p->delay_exprs[i]);
			}
			free(p->delay_exprs);
		}
		
		int ns = nvi;
		p->delay_exprs = get_nc_strings(delay_exprs, &ns);
		
		if(ns != nvi) {
			ev = -250;
			goto error;
		}
		
		if(p->data_exprs != NULL) {
			for(i = 0; i < nvi; i++) {
				if(p->data_exprs[i] != NULL)
					free(p->data_exprs[i]);
			}
			free(p->data_exprs);
		}
		
		p->data_exprs = get_nc_strings(data_exprs, &ns);
		if(ns != nvi) {
			ev = -250;
			goto error;
		}
		
		// Now we need to read the v_ins_locs, which has been stored as a single array, into a 2D array.
		for(i = 0; i < nvi; i++) {
			if(ev = DDC_GetDataValues(vi_locs_c, i*p->max_n_steps, p->max_n_steps, p->v_ins_locs[i]))
				goto error;
		}
		
		// Now we get the delay and data expressions
	
		if(p->skip & 2) {
			// First we get the skip expression if it's been saved.
			if(ev = DDC_GetChannelGroupStringPropertyLength(pcg, MCTD_PSKIPEXPR, &len))
				goto error;
			
			if(p->skip_expr == NULL)
				p->skip_expr = malloc(++len);
			else
				p->skip_expr = realloc(p->skip_expr, ++len);
			
			if(ev = DDC_GetChannelGroupProperty(pcg, MCTD_PSKIPEXPR, p->skip_expr, len))
				goto error;
			
			if(p->skip & 1 && s_locs_c != NULL) {
				skip_locs = malloc(sizeof(unsigned char)*p->max_n_steps);
				if(ev = DDC_GetDataValues(s_locs_c, 0, p->max_n_steps, skip_locs)) { goto error; }
				
				for(i = 0; i < p->max_n_steps; i++)
					p->skip_locs[i] = skip_locs[i];
				
				free(skip_locs);
				skip_locs = NULL;
			}
		}
	}
	
	error:
	
	err_val[0] = ev;
	
	if(data_exprs != NULL) { free(data_exprs); }
	if(delay_exprs != NULL) { free(delay_exprs); }
	
	if(idata != NULL) { free(idata); }
	if(ddata != NULL) { free(ddata); }
	
	if(skip_locs != NULL) { free(skip_locs); }

	if(ev) { 
		if(!on)
			free(p);
		else
			free_pprog(p);
		p = NULL;
	} 
	
	return p;
	
}

PPROGRAM *tdms_load_program(TDMSChannelGroupHandle pcg, int *err_val) {
	// Loads programs from the Channel Group "pcg" in a TDM streaming library
	
	// Variable declarations
	PPROGRAM *p = malloc(sizeof(PPROGRAM));
	int on = 0, *idata = NULL, ev = 0, nvi = 0, *vfound = NULL;
	double *ddata = NULL;
	TDMSChannelHandle *handles = NULL;
	char *name_buff = NULL, *delay_exprs = NULL, *data_exprs = NULL;
	err_val[0] = ev;
	
	// We're going to start out by getting a bunch of properties
	int si = sizeof(int), sd = sizeof(double);
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PNP, &p->np, si)) {					// Number of points
		if(ev != TDMS_PropertyDoesNotExist)
			goto error;
		else
			GetCtrlVal(pc.np[1], pc.np[0], &p->np);
	}
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PSR, &p->sr, sd)) {					// Sampling Rate
		if(ev != TDMS_PropertyDoesNotExist)
			goto error;
		else
			GetCtrlVal(pc.sr[1], pc.sr[0], &p->sr);
	}
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PNT, &p->nt, si)) {					// Number of transients
		if(ev != TDMS_PropertyDoesNotExist)
			goto error;
		else
			GetCtrlVal(pc.nt[1], pc.nt[0], &p->nt);
	}
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PTMODE, &p->tmode, si)) {			// Transient acquisition mode
		if(ev != TDMS_PropertyDoesNotExist)
			goto error;
		else
			GetCtrlVal(pc.tfirst[1], pc.tfirst[0], &p->tmode);
	}
		
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PSCAN, &p->scan, si))				// Whether or not there's a scan
		goto error;
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PTRIGTTL, &p->trigger_ttl, si))    	// Trigger TTL
		goto error;
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PTOTTIME, &p->total_time, sd))		// Total time
		goto error;
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PNINSTRS, &p->n_inst, si))    		// Base number of instructions
		goto error;
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PNUINSTRS, &p->nUniqueInstrs, si))	// Total number of unique instructions
		goto error;
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PVAR, &p->varied, si))				// Whether or not it's varied
		goto error;

	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PNVAR, &p->nVaried, si))				// Number of varied instructions
		goto error;

	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PNDIMS, &p->nDims, si))				// Number of dimensions
		goto error;

	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PNCYCS, &p->nCycles, si))			// Number of phase cycles
		goto error;
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PSKIP, &p->skip, si))				// Whether or not there are skips
		goto error;
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PMAX_N_STEPS, &p->max_n_steps, si))	// Total number of steps in the experiment
		goto error;
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PREAL_N_STEPS, &p->real_n_steps, si))// Total number of steps - those skipped
		goto error;
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_NFUNCS, &p->nFuncs, si))				// Number of functions actually used
		goto error;
	
	if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_TFUNCS, &p->tFuncs, si))				// Total number of functions available
		goto error;
	
	// Now we need to go through one by one and get the channel handles.
	int i, j, k = 0;
	int nchans, nchan_names = 5, nvchan_names = 6;
	TDMSChannelHandle flags_c, time_c, ins_c, us_c, insd_c;
	TDMSChannelHandle msteps_c, vins_c, vi_dim_c, vi_mode_c, vi_locs_c, s_locs_c;
	flags_c = time_c = ins_c = us_c = insd_c = msteps_c = vins_c = vi_dim_c = vi_mode_c = vi_locs_c = s_locs_c = NULL;
	
	char *chan_names[] = {MCTD_PROGFLAG, MCTD_PROGTIME, MCTD_PROGINSTR, MCTD_PROGUS, MCTD_PROGID};
	TDMSChannelHandle *chan_hs[] = {&flags_c, &time_c, &ins_c, &us_c, &insd_c};
	
	char *v_chan_names[] = {MCTD_PMAXSTEPS, MCTD_PVINS, MCTD_PVINSDIM, MCTD_PVINSMODE, MCTD_PVINSLOCS, MCTD_PSKIPLOCS};
	TDMSChannelHandle *v_chan_hs[] = {&msteps_c, &vins_c, &vi_dim_c, &vi_mode_c, &vi_locs_c, &s_locs_c};
	
	// Get the number of channels and a list of the channel handles.
	if(ev = TDMS_GetNumChannels(pcg, &nchans))
		goto error;
	
	handles = malloc(sizeof(TDMSChannelHandle)*nchans);
	
	if(ev = TDMS_GetChannels(pcg, handles, nchans))
		goto error;
	
	// Go through each channel and get its name, and if it matches something, set that value.
	int len, g = 20, s = 20;
	name_buff = malloc(g);
	
	// Search for all the 1D bits first.
	int found = 0, *cfound = calloc(si*nchans, si);
	for(i = 0; i < nchans; i++) {
		if(ev = TDMS_GetChannelStringPropertyLength(handles[i], TDMS_CHANNEL_NAME, &len))
			goto error;
		
		if(g < ++len)	// If there's not enough room, allocate more room 
			name_buff = realloc(name_buff, g+=(((int)((len-g)/s)+1)*s));
		
		// Get the actual channel name
		if(ev = TDMS_GetChannelProperty(handles[i], TDMS_CHANNEL_NAME, name_buff, len))
			goto error;
		
		for(j = 0; j < nchan_names; j++) {
			if(1<<j & found)	// Don't search if we've already found it.
				continue;
			
			if(strcmp(chan_names[j], name_buff) == 0)
				break;
		}
		
		if(j < nchan_names) {		// In this case, we found it.
			memcpy(chan_hs[j], &handles[i], sizeof(TDMSChannelHandle));
			k++;
			found += 1<<j;
			cfound[i] = 1;
			continue;
		} else if (k >= nchan_names)
			break;	
	}
	
	// Now if we need to, to through all the variation bits
	if(!p->varied)
		nvchan_names = 0;
	
	k = 0;
	found = 0;
	for(i = 0; i < nchans; i++) {
		if(cfound[i])	// Skip this one if we found it the first time around.
			continue;
		
		if(ev = TDMS_GetChannelStringPropertyLength(handles[i], TDMS_CHANNEL_NAME, &len))
			goto error;
		
		if(g < ++len)
			name_buff = realloc(name_buff, g+=(((int)((len-g)/s)+1)*2));
		
		if(ev = TDMS_GetChannelProperty(handles[i], TDMS_CHANNEL_NAME, name_buff, len))
			goto error;
		
		for(j = 0; j < nvchan_names; j++) {
			if(1<<j & found)
				continue;
			
			if(strcmp(v_chan_names[j], name_buff) == 0)
				break;
		}
		
		if(j < nvchan_names) {
			memcpy(v_chan_hs[j], &handles[i], sizeof(TDMSChannelHandle));
			k++;
			found += 1<<j;
			cfound[i] = 1;
			continue;
		} else if (k >= nvchan_names)
			break;
	}
	
	free(cfound);
	free(handles);
	free(name_buff);
	handles = NULL;
	name_buff = NULL;
	
	// At this point, we should have more than enough to complete the allocation of the p file.
	create_pprogram(p);
	on = 1;
	
	// First we'll read out the instructions
	int nui = p->nUniqueInstrs;
	idata = malloc(si*nui);
	ddata = malloc(sd*nui);
	
	// Flags
	if(ev = TDMS_GetDataValuesEx(flags_c, 0, nui, idata))
		goto error;
	for(i = 0; i < nui; i++)
		p->instrs[i]->flags = idata[i];
	
	// Instruction Delay
	if(ev = TDMS_GetDataValuesEx(time_c, 0, nui, ddata))
		goto error;
	for(i = 0; i < nui; i++)
		p->instrs[i]->instr_time = ddata[i];
	
	// Instructions
	if(ev = TDMS_GetDataValuesEx(ins_c, 0, nui, idata))
		goto error;
	for(i = 0; i < nui; i++)
		p->instrs[i]->instr = idata[i];
	
	// Units and scan
	if(ev = TDMS_GetDataValuesEx(us_c, 0, nui, idata))
		goto error;
	for(i = 0; i < nui; i++) {
		p->instrs[i]->trigger_scan = 1 & idata[i];
		p->instrs[i]->time_units = (int)(idata[i]>>1);
	}
	
	// Instruction Data
	if(ev = TDMS_GetDataValuesEx(insd_c, 0, nui, idata))
		goto error;
	for(i = 0; i < nui; i++)
		p->instrs[i]->instr = idata[i];
	
	free(idata);
	free(ddata);
	idata = NULL;
	ddata = NULL;
	
	// Now go through and get the varied instructions
	nvi = p->nVaried;
	if(p->varied) {
		// MaxSteps
		if(ev = TDMS_GetDataValuesEx(msteps_c, 0, p->nCycles+p->nDims, p->maxsteps))
			goto error;
		
		// VIns, VInsDims, VInsMode
		if(ev = TDMS_GetDataValuesEx(vins_c, 0, p->nVaried, p->v_ins))
			goto error;
		
		if(ev = TDMS_GetDataValuesEx(vi_dim_c, 0, p->nVaried, p->v_ins_dim))
			goto error;
		
		if(ev = TDMS_GetDataValuesEx(vi_mode_c, 0, p->nVaried, p->v_ins_mode))
			goto error;
		
		// Delay and data instructions are stored as a property on VIns.
		int ldel, ldat;
		if(ev = TDMS_GetChannelStringPropertyLength(vins_c, MCTD_PDELEXPRS, &ldel))
			goto error;
		
		if(ev = TDMS_GetChannelStringPropertyLength(vins_c, MCTD_PDATEXPRS, &ldat))
			goto error;
		
		delay_exprs = malloc(++ldel);
		data_exprs = malloc(++ldat);
		
		if(ev = TDMS_GetChannelProperty(vins_c, MCTD_PDELEXPRS, delay_exprs, ldel))
			goto error;
		
		if(ev = TDMS_GetChannelProperty(vins_c, MCTD_PDATEXPRS, data_exprs, ldat))
			goto error;
		
		// Now we need to get those data expressions as an array.
		if(p->delay_exprs != NULL) {
			for(i = 0; i < nvi; i++) {
				if(p->delay_exprs[i] != NULL)
					free(p->delay_exprs[i]);
			}
			free(p->delay_exprs);
		}
		
		int ns = nvi;
		p->delay_exprs = get_nc_strings(delay_exprs, &ns);
		
		if(ns != nvi) {
			ev = -250;
			goto error;
		}
		
		if(p->data_exprs != NULL) {
			for(i = 0; i < nvi; i++) {
				if(p->data_exprs[i] != NULL)
					free(p->data_exprs[i]);
			}
			free(p->data_exprs);
		}
		
		p->data_exprs = get_nc_strings(data_exprs, &ns);
		if(ns != nvi) {
			ev = -250;
			goto error;
		}
		
		// Now we need to read the v_ins_locs, which has been stored as a single array, into a 2D array.
		for(i = 0; i < nvi; i++) {
			if(ev = TDMS_GetDataValuesEx(vi_locs_c, i*p->max_n_steps, p->max_n_steps, p->v_ins_locs[i]))
				goto error;
		}
		
		// Now we get the delay and data expressions
	
		if(p->skip & 2) {
			// First we get the skip expression if it's been saved.
			if(ev = TDMS_GetChannelGroupStringPropertyLength(pcg, MCTD_PSKIPEXPR, &len))
				goto error;
			
			if(p->skip_expr == NULL)
				p->skip_expr = malloc(++len);
			else
				p->skip_expr = realloc(p->skip_expr, ++len);
			
			if(ev = TDMS_GetChannelGroupProperty(pcg, MCTD_PSKIPEXPR, p->skip_expr, len))
				goto error;
			
			if(p->skip & 1 && s_locs_c != NULL) {
				if(ev = TDMS_GetDataValuesEx(s_locs_c, 0, p->max_n_steps, p->skip_locs))
					goto error;
			}
		}
	}
	
	return p;
	
	error:
	
	err_val[0] = ev;
	
	if(idata != NULL)
		free(idata);
		
	if(ddata != NULL)
		free(ddata);
	
	if(!on)
		free(p);
	else
		free_pprog(p);
		
	return NULL;
	
}

int get_name(char *pathname, char *name, char *ending) {
	// Gets the base name without the ending. Pass NULL if you don't
	// care about the ending, pass the specified ending (no .) if you 
	// have one in mind, and pass "*" if the ending is not known.
	
	SplitPath(pathname, NULL, NULL, name);
	
	if(ending == NULL)
		return 0;
	
	if(ending[0] == '*') {
		for(int i = strlen(name)-1; i--; i >= 0) {
			if(name[i] == '.') {
				name[i] = '\0';
				break;
			}
		}
		
		return 0;
	}
	
	if(strlen(name) > strlen(ending) && strcmp(&name[strlen(name)-strlen(ending)], ending) == 0)
		name[strlen(name)-strlen(ending)] = '\0';
	
	return 0;
}

char *generate_nc_string(char **strings, int numstrings, int *len) {
	// We're going to store arrays of strings as a single newline-delimited char array. 
	// len returns the full length of the array with newlines and \0. 
	//
	// The array is malloced, so you need to free it when you are done.
	
	int i, l=0;
	for(i = 0; i < numstrings; i++)
		l += strlen(strings[i])+((strlen(strings[i]) > 0)?1:2);	// Add one for the newline.
	
	char *out = malloc(++l);	// Adds one for the null. (Keeping the trailing \n)
	out[0] = '\0';
	if(len != NULL)
		len[0] = l;
	
	// Generate the array
	for(i = 0; i < numstrings; i++) {
		strcat(out, (strlen(strings[i]) > 0)?strings[i]:" ");
		strcat(out, "\n");
	}
	
	return out;
}

char **get_nc_strings(char *string, int *ns) {
	// Performs the reverse operation of generate_nc_string ns should be your best guess as
	// to how many strings there are. If you don't know, a value of 0 or less defaults to 10.
	//
	// TODO: Make it so you can pass -1 or something to ns and it counts the strings.
	
	
	int n = 0, g = ((ns != NULL) && (*ns > 0))?*ns:10, s = g, i;
	char **out = malloc(sizeof(char*)*s), *p = strtok(string, "\n");

	while(p != NULL) {
		i = n++;	// Index is one less than n.
		if(n > g) {	// Increment the counter.
			g += s;		// Get more space.
			out = realloc(out, sizeof(char*)*g);
		}
								 
		// Put the string in the array.
		out[i] = malloc(strlen(p)+1);

		if(p[0] == ' ')
			strcpy(out[i], "");
		else
			strcpy(out[i], p);
			
		p = strtok(NULL, "\n");
	}
	
	if(ns != NULL)
		ns[0] = n;	// This is the number of strings.
	
	return out;
}

void display_ddc_error(int err_val) {
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

void display_tdms_error(int err_val) {
	switch(err_val) {
		case -247:
			MessagePopup("TDMS Error", "Invalid/NULL pulse program.");
		case -248:
			MessagePopup("TDMS Error", "Program Channel Group Not Found");
			break;
		case -249:
			MessagePopup("TDMS Error", "No Channel groups found in this file");
			break;
		default:
			MessagePopup("TDMS Error", TDMS_GetLibraryErrorDescription(err_val));
	}
}

//////////////////////////////////////////////////////////////
// 															//
//				PulseBlaster Interactions					//
// 															//
//////////////////////////////////////////////////////////////

int load_pb_info(int verbose) {
	// Counts the number of pulseblasters in the system and populates the ring control.
	int nd = pb_count_boards();	// Count them up bitches.

	if(nd <= 0 && verbose) 
		MessagePopup("Error Counting Spincore Boards:", pb_get_error());
	
	int nl, pan = pc.pbdev[1], ctrl = pc.pbdev[0];
	int ind = 0;
	
	GetNumListItems(pan, ctrl, &nl);
	if(nl) {
		// Save the old index.
		GetCtrlVal(pan, ctrl, &ind);
		
		DeleteListItem(pan, ctrl, 0, -1);
	}
	
	if(nd < 1) {
		SetCtrlAttribute(pan, ctrl, ATTR_DIMMED, 1);	
		return -2;
	} else
		SetCtrlAttribute(pan, ctrl, ATTR_DIMMED, 0);
	
	int elems;
	char **c = generate_char_num_array(0, nd-1, &elems);
	if(c == NULL)
		return -1;
	
	for(int i = 0; i < elems; i++) {  
		InsertListItem(pan, ctrl, -1, c[i], i); 
		free(c[i]);
	}
	free(c);

	return 0;
}


int program_pulses(PINSTR *ilist, int n_inst, int verbose) {
	// Send a list of instructions to the board for programming
	// ilist is obviously the list
	// n_inst is the number of instructions
	// verbose is whether or not to display error messages

	// Attempt to initialize if necessary
	if(!GetInitialized()) {
		pb_init_safe(verbose);
		
		if(!GetInitialized())
			return -1;
	}
	
	// Check for problems with END_LOOP
	int i;
	for(i = 0; i < n_inst; i++) {
		if(ilist[i].instr == END_LOOP) {
			int id = ilist[i].instr_data;
			if(id >= i || ilist[id].instr != LOOP) {
				if(verbose)
					MessagePopup("Problem Programming Pulses", "END_LOOP instruction data does not refer to a Loop!");
				return -2;
			}
		}
	}
	
	int rv = pb_start_programming_safe(verbose, PULSE_PROGRAM);
	if(rv < 0)  
		return rv;
	
	int *fid = malloc(sizeof(int)*n_inst);
	for(i = 0; i < n_inst; i++)
		fid[i] = -1571573;		// Distinct number.
	
	int in, instr_data; 
	
	CmtGetLock(lock_pb);
	for(i = 0; i < n_inst; i++) {
		in = ilist[i].instr;
		instr_data = ilist[i].instr_data;
		
		// If it's JSR, BRANCH or END_LOOP, you need to point them at the FID they refer to,
		// which is the value returned by programming that instruction, so we'll convert this.
		// (I think they usually end up being the same anyway.
		if(in == JSR || in == BRANCH || in == END_LOOP) {
			if(instr_data >= n_inst || fid[instr_data] == -1571573) {
				if(verbose)
					MessagePopup("Error Programming Board", "Instruction data points where it shouldn't.");
				
				return -2;
			} else
				instr_data = fid[instr_data];
		}
		
		fid[i] = pb_inst_pbonly_safe(verbose, ilist[i].flags, in, instr_data, ilist[i].instr_time);
		if(fid[i] < 0) {
			rv = -3;
			break;
		}
	}
	CmtReleaseLock(lock_pb);
	
	pb_stop_programming_safe(verbose);
	
	// Error processing.
	if(rv == -3) {
		if(verbose) {
			char *err = pb_get_error();
			char *err_str = malloc(strlen(err)+strlen("Error in Instruction 0000: \n\n")+10);
			sprintf(err_str, "Error in Instruction %d:\n\n %s", i, err);
			MessagePopup("Error Programming Board", err_str);
			free(err_str);
		}
		
		return fid[i];
	}

	return 0;
	
}

int update_status(int verbose) {
	// Read the status from the board and update the status controls.
	// Does this in a thread-safe manner. Returns the status value.
	
	int rv = 0;
	
	rv = pb_read_status_safe(verbose);
	
	if(rv < 0)
		return rv;
	
	// Update the controls.
	SetCtrlVal(mc.pbstop[1], mc.pbstop[0], rv & PB_STOPPED);
	SetCtrlVal(mc.pbrun[1], mc.pbrun[0], rv & PB_RUNNING);
	SetCtrlVal(mc.pbwait[1], mc.pbwait[0], rv & PB_WAITING);
	
	// Update the thread-safe variable
	SetStatus(rv);

	return rv;	// Return the value.
}

//////////////////////////////////////////////////////////////
// 															//
//				UI Interaction Functions					//
// 															//
//////////////////////////////////////////////////////////////
/******************** General UI Functions *********************/
PPROGRAM *get_current_program() { // This function gets the current program from the UI controls and dumps it into a PPROGRAM
	PPROGRAM *p = malloc(sizeof(PPROGRAM));	// Dynamically allocate a PPROGRAM first

	ui_cleanup(0); // Clean up the interface and uipc variable so that we can trust it.

	// Grab what statics we can from the uipc variable.
	p->n_inst = uipc.ni;								// Number of instructions
	p->nCycles = uipc.nc;								// Number of cycles
	p->nDims = uipc.nd;									// Number of indirect dimensions
	
	if(p->nDims || p->nCycles) {		   					// If either of these is true, it's a varied experiment
		p->varied = 1;
		p->nVaried = p->nDims?uipc.ndins:0 + uipc.ncins;	// Number of instructions that are varied.
	} else {
		p->varied = 0;
		p->nVaried = 0;
	}
	
	GetCtrlVal(pc.skip[1], pc.skip[0], &p->skip);			// Whether or not skip is on
	GetCtrlVal(pc.nt[1], pc.nt[0], &p->nt);
	GetCtrlVal(pc.tfirst[1], pc.tfirst[0], &p->tmode);	// Transient acquisition mode
	
	GetNumListItems(pc.inst[0], pc.instr, &p->tFuncs);// Number of items in the instruction rings
	p->tFuncs -= 9; // Subtract off atomic functions	// Eventually we'll get this from all_funcs

	// Convenience variables
	int nFuncs = 0, tFuncs = p->tFuncs, nDims=p->nDims, nCycles=p->nCycles;
	int nInst = p->n_inst, nVaried = p->nVaried, i, j;

	// Now we just need max_n_steps , p->nUniqueInstrs and nFuncs before we can create the pprogram.
	int *maxsteps = malloc(sizeof(int)*(nDims+nCycles));
	int max_n_steps = p->max_n_steps = get_maxsteps(maxsteps);
	
	p->nUniqueInstrs = nInst;			// True for a static experiment
	
	// For a varied experiment, we need to update nUniqueInstrs. We'll get some other info as we do that. 
	int *v_ins, *v_ins_dim, *v_ins_mode;
	if(p->varied) {
		// Sort the varying arrays by instruction number so we can more easily merge them.
		int **dim_array = malloc(sizeof(int*)*4);
		int **cyc_array = malloc(sizeof(int*)*2);
		int *index = malloc(sizeof(int)*uipc.ndins);
		for(i = 0; i < uipc.ndins; i++)
			index[i] = i;
		
		dim_array[0] = uipc.dim_ins;
		dim_array[1] = uipc.ins_dims;
		dim_array[2] = uipc.ins_state;
		dim_array[3] = index; 
		
		cyc_array[0] = uipc.cyc_ins;
		cyc_array[1] = uipc.ins_cycs;
	
		// We'll sort the arrays.
		CmtGetLock(lock_uipc);
		sort_linked(dim_array, 4, uipc.ndins);
		sort_linked(cyc_array, 2, uipc.ncins);

		// We also need to sort the nd_delay and nd_data arrays, but since
		// I haven't made a generalized sort_linked function, use the index function.
		// There's probably a memory-cheaper algorithm that runs in the same time, but this
		// is easier. Don't get spooked by the fact that I free uipc.nd_data and uipc.nd_delays
		// These are arrays of pointers, we didn't free the data they point to.
		double **nd_delays = malloc(sizeof(int*)*uipc.ndins);
		int **nd_data = malloc(sizeof(int*)*uipc.ndins);
		
		for(i = 0; i < uipc.ndins; i++) {
			nd_delays[i] = uipc.nd_delays[index[i]];
			nd_data[i] = uipc.nd_data[index[i]];
		}
		
		free(uipc.nd_data);
		free(uipc.nd_delays);
		
		uipc.nd_delays = nd_delays;
		uipc.nd_data = nd_data;
		
		CmtReleaseLock(lock_uipc);
		
		free(index);
		free(dim_array);
		free(cyc_array);
		
		// I have two sorted lists and I want to merge them into a new sorted unique list
		int dinc=0, cinc=0;		// Counters
	
		v_ins = malloc(sizeof(int)*p->nVaried);			// This is the maximum size they can be
		v_ins_dim = malloc(sizeof(int)*p->nVaried);		// They are temporary anyway, so even if
		v_ins_mode = malloc(sizeof(int)*p->nVaried);	// there are a few uninitialized values it's fine.
		
		for(i=0;i<p->nVaried;i++) {
			v_ins_dim[i] = 0;
			v_ins_mode[i] = 0;
			
			if(dinc < uipc.ndins && (cinc >= uipc.ncins || (uipc.dim_ins[dinc] <= uipc.cyc_ins[cinc]))) {
				// Put the next dimension variable in if this is unique and lower than the
				// next item in the queue for the cycle variable.
				v_ins[i] = uipc.dim_ins[dinc];
				v_ins_dim[i] += pow(2, uipc.ins_dims[dinc]+uipc.nc);	// Set the dimension flags

				if(uipc.ins_state[dinc] == 1)
					v_ins_mode[i] = PP_V_ID;
				else if(uipc.ins_state[dinc] == 2)
					v_ins_mode[i] = PP_V_ID|PP_V_ID_EXPR;
				else
					v_ins_mode[i] = PP_V_ID|PP_V_ID_ARB;
			}
			
			if(cinc < uipc.ncins && (dinc >= uipc.ndins || (uipc.cyc_ins[cinc] <= uipc.dim_ins[dinc]))) {
				v_ins[i] = uipc.cyc_ins[cinc];
				v_ins_dim[i] += pow(2, uipc.ins_cycs[cinc]);			// Set the cycle flags
				v_ins_mode[i] += PP_V_PC;
			}
			
							
			// Now add in the unique instructions we'll need.
			if((v_ins_mode[i] & PP_V_ID) && (v_ins_mode[i] & PP_V_PC)) {
				p->nUniqueInstrs += (uipc.cyc_steps[uipc.ins_cycs[cinc++]]*uipc.dim_steps[uipc.ins_dims[dinc++]])-2;
				p->nVaried--;		// Merged two instructions.
			} else if(v_ins_mode[i] & PP_V_ID) 
				p->nUniqueInstrs += uipc.dim_steps[uipc.ins_dims[dinc++]]-1;
			else if(v_ins_mode[i] & PP_V_PC) 
				p->nUniqueInstrs += uipc.cyc_steps[uipc.ins_cycs[cinc++]]-1;
		}
	}
	
	nVaried = p->nVaried;

	// Now if there are functions, we should do some stuff with it.
	int *ffound;
	if(tFuncs) {
		// Now we need to check which functions need to be included.
		ffound = malloc(sizeof(int)*p->tFuncs);
		int pcon, f, pcs;
	
		for(i = 0; i<nInst; i++) {
			if(nFuncs < p->tFuncs) {
				// If it's phase cycled, there may be user defined functions in non-visible
				// instructions, so check those as well.
				GetCtrlVal(pc.inst[i], pc.pcon, &pcon);
				if(pcon) {										
					GetCtrlVal(pc.inst[i], pc.pcsteps, &pcs);
					for(j=1; j<=pcs; j++) {
						f = uipc.c_instrs[i][j]->instr-9;
						if(f >= 0 && !ffound[f]) {
							ffound[f] = 1;
							nFuncs++;
						}
					}
				} else {
					GetCtrlVal(pc.inst[i], pc.instr, &f);
					f-=9;	// Subtract off the number of atomic functions
					if(f >= 0 && !ffound[f]) {
						ffound[f] = 1;
						nFuncs++;
					}
				}
			} else 
				break;
		}
	}
	
	p->nFuncs = nFuncs;
	
	
	// Now before we allocate the arrays, just the other statics we know.
	if(p->skip)									
		p->real_n_steps = uipc.real_n_steps;
	else
		p->real_n_steps = p->max_n_steps;
	
	// Whether or not there's a scan.
	for(i = 0; i<uipc.ni; i++) {
		GetCtrlVal(pc.inst[i], pc.scan, &p->scan);
		if(p->scan)
			break;
	}
	
	GetCtrlVal(pc.np[1], pc.np[0], &p->np);
	GetCtrlVal(pc.sr[1], pc.sr[0], &p->sr);
	GetCtrlVal(pc.trig_ttl[1], pc.trig_ttl[0], &p->trigger_ttl);
	p->total_time = uipc.total_time;
	
	
	// Now allocate the arrays.
	create_pprogram(p);
	
	// Now we can assign what arrays we've got already.
	for(i=0; i<nDims+nCycles; i++) 
		p->maxsteps[i] = maxsteps[i];
	
	// Grab the instructions from the UI.
	for(i=0; i<nInst; i++)
		get_instr(p->instrs[i], i);
	
	if(p->varied) {
		// First what we've already gotten when we were getting nUniqueInstrs.
		for(i=0; i<p->nVaried; i++) {
			p->v_ins[i] = v_ins[i];
			p->v_ins_dim[i] = v_ins_dim[i];
			p->v_ins_mode[i] = v_ins_mode[i];
		}
		
		free(v_ins);
		free(v_ins_dim);
		free(v_ins_mode);
		
		// Now we need to generate all the instructions we could need and save them to an array.
		// While we are doing that, we'll generate the v_ins_locs index of those instructions.
		int nextinstr = nInst-1, ins_buff;	// Counter for how many instructions have been generated.
		int k, l, dind, cind, dim = -1, cyc = -1, dlev = -1, dstep, pcstep, num;	
		
		// Initialize some variables used in indexing.
		int *places = malloc(sizeof(int)*(nDims+nCycles)), *cstep = malloc(sizeof(int)*(nDims+nCycles)), *zero_cstep = malloc(sizeof(int)*(nDims+nCycles));
		for(i=0; i<(nDims+nCycles); i++) {
			places[i] = 1;
			cstep[i] = 0;							// The first one we'll want is for linear index 0.
			zero_cstep[i] = 0;
			for(j=1; j<=i; j++)
				places[i]*=maxsteps[j-1];
		}
		
		// Now we're going to go instruction-by-instruction and generate all the unique instructions 
		// stemming from that varied instruction. While we're at it, we'll also generate the index. 
		for(i=0; i<nVaried; i++) {
			num = p->v_ins[i];
			dind = -1;
			cind = -1;
			
			// Start by updating the first instructions to what they should be.
			// This will also get us the place in the index that we need.
			get_updated_instr(p->instrs[num], num, zero_cstep, &cind, &dind, p->v_ins_mode[i]);
			
			if(cind >= 0)
				cyc = uipc.ins_cycs[cind];
			if(dind >= 0)
				dim = uipc.ins_dims[dind];
			
			dlev = dim+uipc.nc;			// The indirect dimensions come after the cycles
			
			// Now we can generate all the unique instructions and the v_ins_locs index.
			if((p->v_ins_mode[i] & PP_V_BOTH) == PP_V_BOTH) {	// The non-linear condition (must be first)
				int placestep = places[dlev]/places[cyc];		// Dim is always bigger.
				int r;											// One more counter
				for(int m = 0; m < uipc.dim_steps[dim]; m++) {
					for(j = 0; j < uipc.cyc_steps[cyc]; j++) {
						// Update the cstep - these two are the only ones that matter.
						if(m != 0 || j != 0) {
							cstep[cyc] = j;
							cstep[dlev] = m;
						
							// Get an updated version of the next instruction and increment nextinstr.
							get_updated_instr(p->instrs[++nextinstr], num, cstep, &cind, &dind, p->v_ins_mode[i]);
							ins_buff = nextinstr;
						} else {
							ins_buff = num;
						}
						
						// Now populate all the parts of the locator index that point to this instruction
						for(k = m*places[dlev]; k < max_n_steps; k += p->maxsteps[dlev]*places[dlev]) {
							for(l = j*places[cyc]; l < places[dlev]; l += (p->maxsteps[cyc]*places[cyc])) {
								for(r = 0; r < places[cyc]; r++)
									p->v_ins_locs[i][k+l+r] = ins_buff;
							}
						}
					}
				}
			} else if(p->v_ins_mode[i] & PP_V_PC) {
				for(j = 0; j < p->maxsteps[cyc]; j++) {
					// Get the updated instruction for each new point in acquisition space
					if(j != 0) {
						cstep[cyc] = j;		//Only need to update this one, everything else is irrelevant.
						get_updated_instr(p->instrs[++nextinstr], num, cstep, &cind, &dind, p->v_ins_mode[i]);
						ins_buff = nextinstr;
					} else {
						ins_buff = num;
					}
					
					// Now populate all parts of the locator index that point to this instruction.
					for(k = j*places[cyc]; k < max_n_steps; k += places[cyc]*p->maxsteps[cyc]) {
						for(l = 0; l < places[cyc]; l++)
							p->v_ins_locs[i][k+l] = ins_buff;
					}
				}
			} else if(p->v_ins_mode[i] & PP_V_ID) {
				for(j = 0; j < uipc.dim_steps[dim]; j++) {
					if(j != 0) {
						// Get the updated instruction for each new point in acquisition space
						cstep[dlev] = j;	// Only need to update this one, everything else is irrelevant.
						get_updated_instr(p->instrs[++nextinstr], num, cstep, &cind, &dind, p->v_ins_mode[i]);
						ins_buff = nextinstr;
					} else {
						ins_buff = num;
					}
					// Now populate all parts of the locator index that point to this instruction.
					for(k = j*places[dlev]; k < max_n_steps; k += places[dlev]*p->maxsteps[dlev]) {
						for(l = 0; l < places[dlev]; l++)
							p->v_ins_locs[i][k+l] = ins_buff;
					}
				}
			}
			
			// I also want to store the expression strings.
			if(p->v_ins_mode[i] & PP_V_ID_EXPR) {
				int len, color;
				
				GetCtrlAttribute(pc.cinst[p->v_ins[i]], pc.cexpr_delay, ATTR_TEXT_BGCOLOR, &color);
				if(color == VAL_GREEN) {
					GetCtrlValStringLength(pc.cinst[p->v_ins[i]], pc.cexpr_delay, &len);
					p->delay_exprs[i] = realloc(p->delay_exprs[i], len+1);
					
					GetCtrlVal(pc.cinst[p->v_ins[i]], pc.cexpr_delay, p->delay_exprs[i]);
				}
				
				GetCtrlAttribute(pc.cinst[p->v_ins[i]], pc.cexpr_data, ATTR_TEXT_BGCOLOR, &color);
				if(color == VAL_GREEN) {
					GetCtrlValStringLength(pc.cinst[p->v_ins[i]], pc.cexpr_data, &len);
					p->data_exprs[i] = realloc(p->data_exprs[i], len+1);
					
					GetCtrlVal(pc.cinst[p->v_ins[i]], pc.cexpr_data, p->data_exprs[i]);
				}
				
				
			}
		}

		// Check to see if there's a skip condition, whether or not skips are on.
		// Flags are 1 = Skipping actually on, 2 = Expression found.
		int d_len, len;
		GetCtrlValStringLength(pc.skiptxt[1], pc.skiptxt[0], &len);
		GetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_DFLT_VALUE_LENGTH, &d_len);
		
		char *skip_txt = NULL;
		skip_txt = malloc(len+1);
		GetCtrlVal(pc.skiptxt[1], pc.skiptxt[0], skip_txt);
		
		// Check if it's the default value.
		if(len == d_len) {
			char *d_skip = malloc(d_len+1);
			GetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_DFLT_VALUE, d_skip);
			
			if(strcmp(skip_txt, d_skip) != 0)
				p->skip += 2;
			
			free(d_skip);
		} else
			p->skip += 2;

		if(p->skip & 2) {
			// Copy over the expression.
			if(p->skip_expr == NULL)
				p->skip_expr = malloc(len+1);
			else
				p->skip_expr = realloc(p->skip_expr, len+1);
			strcpy(p->skip_expr, skip_txt);

			if(p->skip & 1 && uipc.skip_locs != NULL) {
				// Copy over the skip_locs array.
				for(i=0; i<p->max_n_steps; i++)
					p->skip_locs[i] = uipc.skip_locs[i];
			}
			
			if(skip_txt != NULL)
				free(skip_txt);
		}
		
		free(cstep);
		free(places);
		free(zero_cstep);
	}
	
	return p;
}

void set_current_program(PPROGRAM *p) { // Set the current program to the program p
	// Sets the UI and uipc variables as necessary.
	if(p == NULL)
		return;

	int i, j, cyc, dim, dlev, steps, num;
							
	// First we'll clear the previous program, so there are no weird leftovers.
	clear_program();
	
	// Start with the static instructions.
	SetCtrlVal(pc.ninst[1], pc.ninst[0], p->n_inst);
	change_number_of_instructions();
	
	for(i=0; i < p->n_inst; i++)
		set_instr(i, p->instrs[i]);
	
	// Now update the varied instructions
	if(p->nDims) {
		SetCtrlVal(pc.ndims[1], pc.ndims[0], p->nDims+1);
		change_num_dims();
		
		set_ndon(1);
	} else
		set_ndon(0);
	
	if(p->nCycles) {
		SetCtrlVal(pc.numcycles[1], pc.numcycles[0], p->nCycles);
		change_num_cycles();
	}
	
	// Now we need to update the number of steps in each dimension and cycle.
	for(i = 0; i < p->nCycles; i++) {
		change_cycle_num_steps(i, p->maxsteps[i]);	
	}
	
	for(i = 0; i < p->nDims; i++) {
		change_num_dim_steps(i, p->maxsteps[i+p->nCycles]);
	}
	
	// Acquisition parameters
	SetCtrlVal(pc.nt[1], pc.nt[0], p->nt);
	
	SetCtrlVal(pc.sr[1], pc.sr[0], p->sr);
	SetCtrlVal(pc.np[1], pc.np[0], p->np);
	change_np_or_sr(0);
	
	if(p->tmode >= 0)
		SetCtrlVal(pc.tfirst[1], pc.tfirst[0], p->tmode);

	// Turn on the instructions if they should be on.
	int *cstep = NULL, len;
	int *nd_data, *nd_units;
	double *nd_delays;
	
	// Generate a cstep;
	cstep = malloc(sizeof(int)*(p->nCycles+p->nDims));
	for(i = 0; i < p->nCycles+p->nDims; i++)
		cstep[i] = 0;
	
	if(p->varied) {
		for(i = 0; i < p->nVaried; i++) {
			num = p->v_ins[i];
			if(p->v_ins_mode[i] & PP_V_PC) { 
				update_pc_state(num, 1);
			
				// Get the cycle
				for(j = 0; j < p->nCycles; j++) {
					if(p->v_ins_dim[j] & (int)pow(2, j))
						break;
				}
			
				cyc = j;
			
				SetCtrlIndex(pc.inst[num], pc.pclevel, cyc);
			
				change_cycle(num);
			
				// Now update the uipc file with the relevant instructions.
				CmtGetLock(lock_uipc);
				if(uipc.c_instrs[num] == NULL || uipc.max_cinstrs[num] < p->maxsteps[cyc]) {
					if(uipc.c_instrs[num] == NULL)
						uipc.c_instrs[num] = malloc(sizeof(PINSTR*)*p->maxsteps[cyc]);
					else
						uipc.c_instrs[num] = realloc(uipc.c_instrs[num], sizeof(PINSTR*)*p->maxsteps[cyc]);
				
					for(j = uipc.max_cinstrs[num]; j < p->maxsteps[cyc]; j++)
						uipc.c_instrs[num][j] = NULL;
				
					uipc.max_cinstrs[num] = p->maxsteps[cyc];
				}

				// Copy the relevant pinstrs into the cache.
				for(j = 0; j < p->maxsteps[cyc]; j++) {
					if(uipc.c_instrs[num][j] == NULL)
						uipc.c_instrs[num][j] = malloc(sizeof(PINSTR));
				
					cstep[cyc] = j;
					copy_pinstr(p->instrs[p->v_ins_locs[i][get_lindex(cstep, p->maxsteps, p->nCycles+p->nDims)]], uipc.c_instrs[num][j]);
				}
			
				// Reset the cstep (probably unnecessary)
				cstep[cyc] = 0;
				CmtReleaseLock(lock_uipc);
			}
		
			if(p->v_ins_mode[i] & PP_V_ID) {
				// Get the dimension
				for(j = 0; j < p->nDims; j++) {
					if(p->v_ins_dim[i] & (int)pow(2, j+p->nCycles))
						break;
				}
				dim = j;
				dlev = dim+p->nCycles;
				steps = p->maxsteps[dim+p->nCycles];
			
				// Generate arrays for the delays and data.
				nd_data = malloc(sizeof(int)*steps);
				nd_delays = malloc(sizeof(double)*steps);
				nd_units = malloc(sizeof(int)*steps);
			
				PINSTR *instr;
			
				for(j = 0; j < steps; j++) {
					cstep[dlev] = j;
					instr = p->instrs[p->v_ins_locs[i][get_lindex(cstep, p->maxsteps, p->nCycles+p->nDims)]];
				
					if(takes_instr_data(instr->instr)) 
						nd_data[j] = instr->instr_data; 
					else 
						nd_data[j] = 0;
					nd_delays[j] = instr->instr_time;
					nd_units[j] = instr->time_units;
				}

				// Reset cstep
				cstep[dlev] = 0;
			
				// Determine if delay or data is on.
				int del = 1, dat = 1;
				if(constant_array_int(nd_data, steps))
					dat = 0;
				if(constant_array_double(nd_delays, steps))
					del = 0;
			
				// Set the initial and final controls.
				if(del) {
					// Initial delay
					SetCtrlIndex(pc.cinst[num], pc.delu_init, nd_units[0]);
					SetCtrlVal(pc.cinst[num], pc.del_init, nd_delays[0]/pow(1000, nd_units[0]));
				
					// Final delay
					SetCtrlIndex(pc.cinst[num], pc.delu_fin, nd_units[steps-1]);
					SetCtrlVal(pc.cinst[num], pc.del_fin, nd_delays[steps-1]/pow(1000, nd_units[steps-1]));
				}
			
				if(dat) {
					SetCtrlVal(pc.cinst[num], pc.dat_init, nd_data[0]);
					SetCtrlVal(pc.cinst[num], pc.dat_fin, nd_data[steps-1]);
				}
		
				if(p->v_ins_mode[i] & PP_V_ID_EXPR) {
					update_nd_state(num, 2);
				
					// Load the expressions
					len = strlen(p->data_exprs[i]);
					if(len > 0)
						SetCtrlVal(pc.cinst[num], pc.cexpr_data, p->data_exprs[i]);
				
					len = strlen(p->delay_exprs[i]);
					if(len > 0)
						SetCtrlVal(pc.cinst[num], pc.cexpr_delay, p->delay_exprs[i]);
				
					update_nd_from_exprs(num);
				} else {
					update_nd_state(num, 1);
				}
			
				// Update the dimension (increments and such)
				SetCtrlIndex(pc.cinst[num], pc.dim, dim);
				change_dimension(num);
			
				// Now if this was arbitrary, copy over the nd_data and nd_delay tables.
				if(p->v_ins_mode[i] & PP_V_ID_ARB) {
					for(j = 0; j < uipc.ndins; j++) {
						if(uipc.dim_ins[j] == num)
							break;
					}
				
					int ind = j;
					
					CmtGetLock(lock_uipc);
					if(del) {
						for(j = 0; j < steps; j++)
							uipc.nd_delays[ind][j] = nd_delays[j];
					}
			
					if(dat) {
						for(j = 0; j < steps; j++)
							uipc.nd_data[ind][j] = nd_data[j];
					}
					CmtReleaseLock(lock_uipc);
				}
				
				free(nd_data);
				free(nd_delays);
				free(nd_units);
			}
		}
		
		// Load up the skip expression
		if(p->skip && p->skip_expr != NULL) {
			SetCtrlVal(pc.skiptxt[1], pc.skiptxt[0], p->skip_expr);
			update_skip_condition();
				
			if(!uipc.skip_err && (p->skip & 1))
				SetCtrlVal(pc.skip[1], pc.skip[0], 1);
			else
				SetCtrlVal(pc.skip[1], pc.skip[0], 0);
			
		}
	}
	
	if(cstep != NULL)
		free(cstep);
}

void load_prog_popup() {
	// Function that generates a popup and allows the user to load a program from file.
	char *path = malloc(MAX_FILENAME_LEN+MAX_PATHNAME_LEN);
	int rv = FileSelectPopup((uipc.ppath != NULL)?uipc.ppath:"Programs", "*.tdm", ".tdm;.tdms", "Load Program From File", VAL_LOAD_BUTTON, 0, 0, 1, 1, path);

	if(rv == VAL_NO_FILE_SELECTED) {
		free(path);
		return;
	} else 
		add_prog_path_to_recent(path);

	int err_val;
	PPROGRAM *p = LoadPulseProgram(path, &err_val);

	if(err_val != 0)
		display_ddc_error(err_val);

	if(p != NULL) {
		set_current_program(p);
		free_pprog(p);
	}

	free(path);
}

void save_prog_popup() {
	char *path = malloc(MAX_FILENAME_LEN+MAX_PATHNAME_LEN);
	int rv = FileSelectPopup((uipc.ppath != NULL)?uipc.ppath:"Programs", "*.tdm", ".tdm", "Save Program To File", VAL_SAVE_BUTTON, 0, 0, 1, 1, path);

	if(rv == VAL_NO_FILE_SELECTED) {
		free(path);
		return;
	} else
		add_prog_path_to_recent(path);

	PPROGRAM *p = get_current_program();

	int err_val = SavePulseProgram(path, p);

	if(err_val != 0) 
		display_ddc_error(err_val);
	
	if(p != NULL) { free_pprog(p); }
	free(path);
}

void add_prog_path_to_recent(char *opath) {
	// Adds 'opath' to the recent programs menus and such. opath must end in .tmds.
	if(opath == NULL)
		return;
	
	// TODO: Add to the recent program pulldown in the menu bar and the recent
	// program pulldown on the main page.
	
	char *path = malloc(strlen(opath)+1);
	strcpy(path, opath);
	
	// Make this directory the default next time we try and load something.
	char *c = strrchr(path, '\\'); // Finds last instance of the separator.
	c[1] = NULL;				   // Truncate after the last separator.

	if(c != NULL && FileExists(path, NULL)) {
		
		CmtGetLock(lock_uipc);
		if(uipc.ppath == NULL)
			uipc.ppath = malloc(strlen(path)+1);
		else
			uipc.ppath = realloc(uipc.ppath, strlen(path)+1);
		
		strcpy(uipc.ppath, path);
		CmtReleaseLock(lock_uipc);
	}
	
	free(path);
}

void clear_program() {
	// Deletes all instructions for a new program.
	
	// Clear all the instructions
	for(int i = 0; i < uipc.max_ni; i++)
		clear_instruction(i);
	
	// Update the number of instructions
	SetCtrlVal(pc.ninst[1], pc.ninst[0], 1);
	change_number_of_instructions();
	
	// Update the ND information
	SetCtrlVal(pc.numcycles[1], pc.numcycles[0], 0);
	change_num_cycles();
	
	// Free up the uipc stuff
	CmtGetLock(lock_uipc);
	if(uipc.c_instrs != NULL)
		free(uipc.c_instrs);	// The values should have been freed by clear_instruction
	if(uipc.max_cinstrs != NULL)
		free(uipc.max_cinstrs);
	if(uipc.cyc_pans != NULL)
		free(uipc.cyc_pans);
	
	// Reset the values so we're not pointing to free memory.
	uipc.n_cyc_pans = 0;
	uipc.c_instrs = NULL;
	uipc.max_cinstrs = NULL;
	uipc.cyc_pans = NULL;
	CmtReleaseLock(lock_uipc);
	
	// Turn off the skips if necessary.
	SetCtrlVal(pc.skip[1], pc.skip[0], 0);
	
	SetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_TEXT_BGCOLOR, VAL_WHITE);
	SetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_TEXT_COLOR, VAL_BLACK);
	SetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_TEXT_BOLD, 0);
	SetCtrlAttribute(pc.skip[1], pc.skip[0], ATTR_CTRL_MODE, VAL_INDICATOR);
	
	char *def_val;
	int def_len;
	GetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_DFLT_VALUE_LENGTH, &def_len);
	def_val = malloc(def_len+1);
	
	GetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_DFLT_VALUE, def_val);
	SetCtrlVal(pc.skiptxt[1], pc.skiptxt[0], def_val);
	free(def_val);
}

void get_pinstr_array(PINSTR **ins_ar, PPROGRAM *p, int *cstep) {
	// Give this the current step and the PPROGRAM and it returns the array of instructions
	// that you will be using at that step.
	int i;
	
	// Copy over all the initial instructions to start with.
	for(i = 0; i < p->n_inst; i++)
		copy_pinstr(p->instrs[i], ins_ar[i]);
	
	if(p->varied)
		return;			// If there's no variation, we're done.
	
	int ins_ind, lindex = get_lindex(cstep, p->maxsteps, p->nDims+p->nCycles);
	
	// Now we just need to copy the instructions from p->intrs. The locations for each
	// stage in acquisition space is stored in p->v_ins_locs.
	for(i = 0; i < p->nVaried; i++)
		copy_pinstr(ins_ar[p->v_ins[i]], p->instrs[p->v_ins_locs[i][lindex]]);
	
}

int ui_cleanup(int verbose) {
	// Function for cleaning up the input so it's consistent - i.e. turning off ND if there are no varied instrs.
	// We'll systematically go through and check the uipc variable against the user interface. If there is an 
	// inconsistency, favor the UI. Additionally, we will go through and check that things which "vary" are not
	// actually the same thing.
	//
	// If you set the "verbose" flag, a popup is displayed after the cleanup indicating that a change has occured
	// and giving the user the option to cancel the operation that called it. In that case, ui_cleanup returns 1,
	// otherwise it returns 0.
	
	int change = 0;		// Flag if there's a change
	int i, j, state, steps, dim, cyc, ind, step;
	int data_on, delay_on;
	double time;
	
	// First go through and find any problems within the multidimensional acquisition, if applicable.
	if(uipc.nd) {
		if(uipc.ndins == 0) {			// No instructions actually vary.
			change = 1;
			set_ndon(0);
		}
		
		// Iterate through the instructions which do (nominally) vary, and turn them off if they don't really.
		for(i = 0; i<uipc.ndins; i++) {
			ind = uipc.dim_ins[i];
			state = get_nd_state(ind);
			steps = uipc.dim_steps[uipc.ins_dims[i]];
			// First, if either of these is null, it may be that it was never evaluated, so check that.
			if(uipc.nd_delays == NULL || uipc.nd_delays[i] == NULL || uipc.nd_data == NULL || uipc.nd_data[i] == NULL) {
				if(state == 1) {
					update_nd_increment(ind, MC_INC);	
				} else if(state == 2) {
					update_nd_from_exprs(ind);
				}
			}
			
			// Check if delay variation is on.
			if(uipc.nd_delays == NULL || uipc.nd_delays[i] == NULL || constant_array_double(uipc.nd_delays[i], steps))
				delay_on = 0;
			else
				delay_on = 1;
			
			// Check if data variation is on
			if(uipc.nd_data == NULL || uipc.nd_data[i] == NULL || constant_array_int(uipc.nd_data[i], steps))
				data_on = 0;
			else
				data_on = 1;
			
			// If neither is actually on, turn variation off.
			if(!data_on && !delay_on) {
				change = 1;
				update_nd_state(ind, 0);	
				i--;					// Need to decrement i because we're removing an instruction.
			}
		}
		
		// Now check that there aren't more dimensions than are actually being varied.
		for(i = 0; i<uipc.nd; i++) {
			for(j=0; j<uipc.ndins; j++) {
				if(uipc.ins_dims[j] == i)
					break;			// We've found an example of this dimension, so break
			}
			
			if(j < uipc.ndins)
				continue;
			
			// If we're at this point, we need to remove this dimension from the experiment
			change = 1;
			
			CmtGetLock(lock_uipc);
			remove_array_item(uipc.dim_steps, i, uipc.nd); // Remove the steps.
			for(j=0; j<uipc.ndins; j++) {
				if(uipc.ins_dims[j] > i)
					uipc.ins_dims[j]--;
			}
			uipc.nd--;				// Update number of dimensions
			i--;					// We removed one, so drop it back one.
			CmtReleaseLock(lock_uipc);
		}
		if(uipc.nd)
			populate_dim_points();		// Update the UI
	} 
	
	if(!uipc.nd) 
		set_ndon(0);

	// Now fix anything related to the phase cycling, if applicable.
	if(uipc.nc) {
		int step;				 
		
		if(uipc.ncins == 0 || uipc.c_instrs == NULL) {
			change = 1;
			SetCtrlVal(pc.numcycles[1], pc.numcycles[0], 0);
			change_num_cycles();
		}
		
		save_all_expanded_instructions(); // Update the uipc with info from the expanded instructions.

		// Iterate through the instructions which nominally vary and turn them off it they don't really.
		for(i = 0; i<uipc.ncins; i++) {
			// Retrieve the specific variables.
			ind = uipc.cyc_ins[i];
			cyc = uipc.ins_cycs[i];
			steps = uipc.cyc_steps[cyc];
			
			// Now other phase cycling stuff.
			if(uipc.c_instrs[ind] == NULL) {
				change = 1;
				update_pc_state(ind, 0);
				i--;									// If we turn off an instruction, we need to decrement i
			} else {
				if(uipc.c_instrs[ind][0] == NULL) { 	// The only way that this is possible is if you never
					change = 1;						 	// changed the step away from the first one.
					update_pc_state(ind, 0);
					i--;
					continue;
				}
				
				// Before we move on, we need to update the uipc from the current UI.
				GetCtrlVal(pc.inst[ind], pc.pcstep, &step);
				PINSTR *instr = malloc(sizeof(PINSTR));
				get_instr(instr, ind);
				update_cyc_instr(ind, instr, step);
				free(instr);
				
				// If a user doesn't define a step in a cycle, just repeat the last cycle that they defined. 
				for(j = 1; j<steps; j++) {
					if(uipc.c_instrs[ind][j] == NULL)
						update_cyc_instr(ind, uipc.c_instrs[ind][j-1], j);
				}

				// Now just to be sure, check if the array is constant.
				if(constant_array_pinstr(uipc.c_instrs[ind], steps)) {
					change = 1;
					update_pc_state(ind, 0);
					i--;
				}
			}
		}
		
		// Now that we've turned off all the instructions that we don't need, we just need to fix the number of cycles
		for(i=0; i<uipc.nc; i++) {
			for(j=0; j<uipc.ncins; j++) {
				if(uipc.ins_cycs[j] == i)
					break;					
			}
			
			if(j < uipc.ncins)		   // We've found an example of this cycle, so we're good.
				continue;
			
			// If we're at this point, we need to remove the cycle from the expriment
			change = 1;
			CmtGetLock(lock_uipc);
			remove_array_item(uipc.cyc_steps, i, uipc.nc);	// Remove the steps from the array
			for(j=0; j<uipc.ncins; j++) {
				if(uipc.ins_cycs[j] > i)
					uipc.ins_cycs[j]--;
			}
			uipc.nc--;			// Update number of cycles
			i--;				// We removed one, so decrement the counter.
			CmtReleaseLock(lock_uipc);
		}
		
		populate_cyc_points();	// Update the UI.
	} else {
		// Probably not necessary, but just in case.
		int nc;
		GetCtrlVal(pc.numcycles[1], pc.numcycles[0], &nc);
		if(nc) {
			SetCtrlVal(pc.numcycles[1], pc.numcycles[0], 0);
			change_num_cycles();
		}
	}

	// Finally, if we're in verbose mode, give us a popup.
	if(verbose && change) {
		return change;
	}
	
	// Return this anyway. This allows for two different non-verbose modes, based on what you
	// do with the return value. You can either ignore it or cancel the operation even without
	// the user's input. We can implement a user-defined preference to decide on a case-by-case
	// basis what to do in the various situations that ui_cleanup() is called.
	
	return change;
} 

void change_instr_units(int panel) {
	// Function for changing the units of a given instruction.
	// Old units can be inferred from max value.
	int newunits;
	
	// Minimum step size for spincore is 10ns, and apparently the number
	// of steps is stored in 31 bits, so the maximum single delay value
	// is (2^31)*10.
	const double m_val = 21474836480.0; 
	GetCtrlIndex(panel, pc.delayu, &newunits);
	double max = m_val/pow(1000, newunits);
	double min = 100/pow(1000, newunits); 	// Minimum is 100ns
	double oldmax, oldmin;
	
	GetCtrlAttribute(panel, pc.delay, ATTR_MAX_VALUE, &oldmax);
	GetCtrlAttribute(panel, pc.delay, ATTR_MIN_VALUE, &oldmin);
	
	// Deal with overflows.
	double delay;
	GetCtrlVal(panel, pc.delay, &delay);
	SetCtrlAttribute(panel, pc.delay, ATTR_MAX_VALUE, m_val/pow(1000, newunits));  
	SetCtrlAttribute(panel, pc.delay, ATTR_MIN_VALUE, 100/pow(1000, newunits));
	
	// If it would be coerced, just keep the actual value as represented by
	// the original units.
	if (delay == oldmax || delay == oldmin) {
		GetCtrlAttribute(panel, pc.delay, ATTR_DFLT_VALUE, &delay);
		SetCtrlVal(panel, pc.delay, delay);
	} else if(delay > max || delay < min)
		SetCtrlAttribute(panel, pc.delay, ATTR_DFLT_VALUE, delay); 
	
	change_instr_delay(panel);
}

/********************* Basic Program Setup *********************/
void change_np_or_sr(int np_sr_at)
{
	//Tell this function which to change (number of points, sample rate or acquisition time)
	//and it will update the controls based on that.
	//0 = AT
	//1 = SR
	//2 = NP

	// Error condition - no change
	if(np_sr_at < 0  || np_sr_at > 2)
		return;
	
	double sr, at;
	int np;
	
	// Get all three values.
	GetCtrlVal(pc.sr[1], pc.sr[0], &sr);
	GetCtrlVal(pc.at[1], pc.at[0], &at);
	GetCtrlVal(pc.np[1], pc.np[0], &np);
	
	// Convert acquisition time to seconds.
	at /= 1000.0;

	if(np_sr_at == 1)
		sr = ((double)((int)((np/at)*1000.0+0.5)))/1000.0; // Round to the nearest mHz.
	else if(np_sr_at == 2)
		np = (int)((at*sr)+0.5);	// Round the number of points to the nearest integer.
	
	// Since there's rounding, in all cases we recalculate acquisition time.
	at = 1000.0*((double)np/sr);
	
	// Min/Max values
	if(np < 1 || sr < 1.0 || sr > 250000.0) {
	
		if(np < 1) 
			SetCtrlVal(pc.np[1], pc.np[0], 1);
	
		if(sr < 1.0)
			SetCtrlVal(pc.sr[1], pc.sr[0], 1.0);
		else if(sr > 250000.0) 
			SetCtrlVal(pc.sr[1], pc.sr[0], 250000.0);
		
		change_np_or_sr(0);
		return;
	}
	
	// Set all three controls.
	SetCtrlVal(pc.sr[1], pc.sr[0], sr);
	SetCtrlVal(pc.at[1], pc.at[0], at);
	SetCtrlVal(pc.np[1], pc.np[0], np);
}

void change_nt() {
	// Called to make sure that we always have a number divisible by the number of transients for all 
	// cycles. We always round down, since that's what we're more likely to want.
	
	int nt, inc;
	GetCtrlVal(pc.nt[1], pc.nt[0], &nt);
	GetCtrlAttribute(pc.nt[1], pc.nt[0], ATTR_INCR_VALUE, &inc);

	int newval = ((int)(nt/inc));
	
	SetCtrlVal(pc.nt[1], pc.nt[0], (newval < 1)?inc:(inc*newval));
}
/***************** Get Instruction Parameters ******************/  
void get_instr(PINSTR *instr, int num) {
	// Get the PINSTR represented by the num-th instruction.
	// This function will break if you pass it an instruction that shouldn't exist.
	// Output: instr.
	
	int panel = pc.inst[num];
	get_instr_panel(instr, panel);
}

void get_instr_panel(PINSTR *instr, int panel) {
	// Given a panel (rather than an instruction number), this gets the instruction
	instr->flags = get_flags_panel(panel);								// Get flags
	
	GetCtrlVal(panel, pc.instr, &instr->instr);				// The instruction
	GetCtrlVal(panel, pc.instr_d, &instr->instr_data);			// The instruction data
	
	GetCtrlVal(panel, pc.delay, &instr->instr_time);			// Instruction time
	GetCtrlIndex(panel, pc.delayu, &instr->time_units);		// Time units

	GetCtrlVal(panel, pc.scan, &instr->trigger_scan);
	
	//Instr_time should always be in units of nanoseconds for behind the scenes stuff.
	if(instr->time_units)
		instr->instr_time *= pow(1000, instr->time_units);
}


int get_flags(int num) {
	// Convenience function for getting the flags from a given instruction number.
	int panel = pc.inst[num];
	return get_flags_panel(panel);
}

int get_flags_panel(int panel) {
	// Function for getting the flags for a given panel.   
	return get_flags_range(panel, 0, 23);	
}

int get_flags_range(int panel, int start, int end) {
	int flags = 0, ttlon;
	if(start > end) {
		int buff = end;
		end = start;
		start = buff;
	}
	
	for(int i = start; i <= end; i++) {
		GetCtrlVal(panel, pc.TTLs[i], &ttlon);
		flags = flags|(ttlon<<i);
	}
	
	return flags;
}

/***************** Set Instruction Parameters ******************/ 
int set_instr(int num, PINSTR *instr) {
	// Pass this the program controls, instruction number and the instr and it sets the controls appropriately
	
	int ni;
	GetCtrlVal(pc.ninst[1], pc.ninst[0], &ni);			// For error checking
	if(num >= ni)											// Can't set an instruction if it's not there
		return -1;											// Error -> Instruction invalid
	
	int panel = pc.inst[num];					
	set_instr_panel(panel, instr);
	change_instruction(num);
	return 1;												// Success
}

void set_instr_panel(int panel, PINSTR *instr) {
	int i;
	
	PINSTR ins = {											// The default instruction. Must not be 
		.flags = 0,											// declared in an if loop and arrays can
		.instr = 0,											// only be set this way while declaring.
		.instr_data = 0,
		.trigger_scan = 0,
		.instr_time = 100000000.0,
		.time_units = 2
	};
	
	if(instr == NULL)									// If you pass a NULL pointer it sets it 
		instr = &ins;
	
	// Set the flags
	set_flags_panel(panel, instr->flags);
	
	SetCtrlIndex(panel, pc.instr, instr->instr);			// Set the instruction
	if(instr->instr >= 9) { 								// This is a compound function, not an atomic function
		int r_flags = get_reserved_flags(instr->instr); 	// Get the reserved flags
		SetCtrlAttribute(panel, pc.delay, ATTR_DIMMED, 1);	// Dim the unnecessary controls
		SetCtrlAttribute(panel, pc.delayu, ATTR_DIMMED, 1); 

		for(i = 0; i<24; i++) {
			if(r_flags & (int)pow(2, i))					// Dim the reserved TTLs
				SetCtrlAttribute(panel, pc.TTLs[i], ATTR_DIMMED, 1);
		}
	} else {
		SetCtrlAttribute(panel, pc.delay, ATTR_DIMMED, 0);	// Undim them if it's an atomic function
		SetCtrlAttribute(panel, pc.delay, ATTR_DIMMED, 0);
	}
	
	// Undim the instruction data control if it needs it, otherwise, set to 0 and dim it.
	if(takes_instr_data(instr->instr)) { 
		SetCtrlAttribute(panel, pc.instr_d, ATTR_DIMMED, 0);
		SetCtrlVal(panel, pc.instr_d, instr->instr_data);
		change_instr_data(panel);
	} else {
		SetCtrlAttribute(panel, pc.instr_d, ATTR_DIMMED, 1);
		SetCtrlVal(panel, pc.instr_d, 0);
	}
	
	// Set up the delay time - it's always saved in nanoseconds, so before you set the UI, adjust for units.
	double instr_time = instr->instr_time;
	if(instr->time_units)
		instr_time /= pow(1000, instr->time_units);
	
	SetCtrlVal(panel, pc.delay, instr_time);
	SetCtrlIndex(panel, pc.delayu, instr->time_units);
	change_instr_delay(panel);
	
	// Deal with whether or not a scan is triggered now
	int trig_ttl;
	GetCtrlVal(pc.trig_ttl[1], pc.trig_ttl[0], &trig_ttl);// Get the trigger TTL.
		
	if(instr->trigger_scan) {
		SetCtrlVal(panel, pc.scan, 1);					
		SetCtrlVal(panel, pc.TTLs[trig_ttl], 1);			// Both need to be on or both off.
	} else {
		SetCtrlVal(panel, pc.scan, 0);
		SetCtrlVal(panel, pc.TTLs[trig_ttl], 0);
	}
}

void set_scan(int num, int state) {
	int panel = pc.inst[num];
	set_scan_panel(panel, state);
}

void set_scan_panel(int panel, int state) {
	// Called when you try and toggle the scan button. Sets the val and the trigger ttl
	SetCtrlVal(panel, pc.scan, state);
	SetCtrlVal(panel, pc.TTLs[uipc.trigger_ttl], state);
}

void change_instr_delay(int panel) {
	double val;
	int units, ndunits;

	// Gets the values we need
	GetCtrlVal(panel, pc.delay, &val);
	GetCtrlVal(panel, pc.delayu, &units);
	SetCtrlAttribute(panel, pc.delay, ATTR_PRECISION, get_precision(val, MCUI_DEL_PREC));
	
	int num;
	GetCtrlVal(panel, pc.ins_num, &num);

	GetCtrlVal(pc.cinst[num], pc.delu_init, &ndunits);
	
	// Convert units if necessary.
	if(ndunits != units) {
		val *= pow(1000, units);
		int new_units = calculate_units(val);
		val /= pow(1000, units);
		SetCtrlVal(pc.cinst[num], pc.del_init, val);
		
		SetCtrlVal(pc.cinst[num], pc.delu_init, new_units);
	}

	SetCtrlVal(pc.cinst[num], pc.del_init, val);
	SetCtrlAttribute(pc.cinst[num], pc.del_init, ATTR_PRECISION, get_precision(val, MCUI_DEL_PREC));
	
	
	int state = get_nd_state(num);
	if(state == 0 || state == 1)
		update_nd_increment(num, MC_FINAL);
}

void change_instr_data(int panel) {
	// Updates the ND controls in response to a change in the instruction data.
	int num, val;
	
	GetCtrlVal(panel, pc.instr_d, &val);
	GetCtrlVal(panel, pc.ins_num, &num);
	
	SetCtrlVal(pc.cinst[num], pc.dat_init, val);
	int state = get_nd_state(num);
	if(state == 0 || state == 1) {
		int inc, steps;
		GetCtrlVal(pc.cinst[num], pc.nsteps, &steps);
		GetCtrlVal(pc.cinst[num], pc.dat_inc, &inc);
		SetCtrlVal(pc.cinst[num], pc.dat_fin, val+inc*steps);
	}
}
void change_instruction(int num) {
	// Convenience function for change_instruction_panel
	// Also syncs with the cinstr.
	int ind, panel = pc.inst[num];
	GetCtrlVal(panel, pc.instr, &ind);
	change_instruction_panel(panel);

	// Sync with the corresponding ND instruction
	SetCtrlVal(pc.cinst[num], pc.cinstr, ind);
	
	int nd_ctrls[4] = {pc.dat_init, pc.dat_inc, pc.dat_fin, pc.cexpr_data};
	int ndnum = 4;
	
	if(takes_instr_data(ind)) {
		if(get_nd_state(num))
			SetCtrlAttribute(panel, pc.instr_d, ATTR_DIMMED, 1);
		change_control_mode(pc.cinst[num], nd_ctrls, ndnum, VAL_HOT);
	} else {
		if(get_nd_state(num))
			SetCtrlAttribute(panel, pc.instr_d, ATTR_DIMMED, 0);
		change_control_mode(pc.cinst[num], nd_ctrls, ndnum, VAL_INDICATOR);
	}
}

void change_instruction_panel(int panel) {
	int ind;
	// Dims instr_data if necessary and sets up the minimum
	GetCtrlVal(panel, pc.instr, &ind);
	
	// Set up the minimum
	int min = instr_data_min(ind);
	SetCtrlAttribute(panel, pc.instr_d, ATTR_MIN_VALUE, min);
	
	if(!takes_instr_data(ind))
		SetCtrlVal(panel, pc.instr_d, 0);
	
	SetCtrlAttribute(panel, pc.instr_d, ATTR_DIMMED, !takes_instr_data(ind));
}

void swap_ttl(int to, int from) {
	// Swaps two TTLs in the program.
	
	if(to == from)
		return;
	
	int i, j, cp, valto, valfrom, expanded;
	
	// Start with the visible instructions.
	for(i = 0; i < uipc.ni; i++) {
		cp = pc.inst[i];
		GetCtrlVal(cp, pc.TTLs[to], &valto);
		GetCtrlVal(cp, pc.TTLs[from], &valfrom);
		
		SetCtrlVal(cp, pc.TTLs[from], valto);
		SetCtrlVal(cp, pc.TTLs[to], valfrom);
	}
	
	// Now the instructions in phase cycles
	int num, cyc, flags, toh = to>from;
	for(i = 0; i < uipc.ncins; i++) {
		num = uipc.cyc_ins[i];
		cyc = uipc.ins_cycs[i];
		
		GetCtrlAttribute(pc.inst[num], pc.collapsepc, ATTR_VISIBLE, &expanded);
		if(expanded) {
			for(j = 0; j < uipc.cyc_steps[cyc]; j++) {
				cp = uipc.cyc_pans[i][j];
			
				GetCtrlVal(cp, pc.TTLs[to], &valto);
				GetCtrlVal(cp, pc.TTLs[from], &valfrom);
		
				SetCtrlVal(cp, pc.TTLs[from], valto);
				SetCtrlVal(cp, pc.TTLs[to], valfrom);
			}
		} else {
			if(uipc.c_instrs[num] == NULL)
				continue;
			
			for(j = 0; j < uipc.cyc_steps[cyc]; j++) {
				if(uipc.c_instrs[num][j] != NULL) {
					flags = uipc.c_instrs[num][j]->flags;
					
					// Read the relevant flags into the buffer
					valto = flags & 1<<to;
					valfrom = flags & 1<<from;
					
					flags -= (valto|valfrom);	// Remove the old flags.
					
					// Swap the positions
					valto = toh?valto>>(to-from):valto<<(from-to);
					valfrom = toh?valfrom<<(to-from):valto>>(from-to);
					
					flags += (valto|valfrom);	// Add the old flags back in.
				}
			}
		}
	}
}

void move_ttl(int num, int to, int from) {
	// Move a TTL from "from" to "to", and shift all the others in response.
	// This is a "safe" function, and will skip broken ttls. If the initial or
	// final value is marked broken, the operation will fail and return -1.
	int panel = pc.inst[num];
	
	move_ttl_panel(panel, to, from);
}

void move_ttl_panel(int panel, int to, int from) {
	// Moves a TTL from one place to another, skipping over broken TTLs.
	// The algorithm is simple - move the "from" TTL into a buffer, then
	// iterate towards to and move everything over one. When you get to the
	// end, everything will be moved over 1 and "to" will be duplicated at the end.
	// Then you can overwrite the "to" value with the buffer.
	
	if(to == from)
		return;
	
	// Slightly slower, but more general.
	int flags = get_flags_range(panel, to, from);
	flags = move_bit_skip(flags, uipc.broken_ttls, to, from);
	
	set_flags_range(panel, flags, to, from);
	return; 
	
	int toh = to>from; 	// High if to is the higher one.
	int low = toh?from:to, high = toh?to:from;
	
	int broken_ttls = get_bits(uipc.broken_ttls, low, high)*((int)pow(2, low));
	
	if(broken_ttls && ((int)pow(2, low) | (int)pow(2, high)) & broken_ttls)
		return;
	
	int buff1, buff2;
	GetCtrlVal(panel, pc.TTLs[from], &buff1);
	int i, next = i = from;
	int inc = toh?1:-1;
	
	while(toh?i < to:i > to) {
		next += inc;
		while(broken_ttls && broken_ttls & (int)pow(2, next)) { // Skip over any ttls that are broken.
			next+= inc;
			if(!(toh?next < to: next >= to))
				break;
		}
		
		GetCtrlVal(panel, pc.TTLs[next], &buff2);
		SetCtrlVal(panel, pc.TTLs[i], buff2);
		i = next;
	}
	
	SetCtrlVal(panel, pc.TTLs[to], buff1);
}

void change_trigger_ttl() {
	// Called by the trigger TTL control on a change in value
	
	int i, cp, nt, ot = uipc.trigger_ttl; // New trigger, old trigger.
	GetCtrlVal(pc.trig_ttl[1], pc.trig_ttl[0], &nt);
	
	if(nt == ot)
		return;	// No change
	
	// If they are trying to move to a broken TTL, find the next non-broken one and give that instead.
	int toh = nt>ot, inc = toh?1:-1;
	int skip = uipc.broken_ttls;
	if((1<<nt) & skip) {
		// Find the next available TTL.
		for(i = nt+inc; toh?(i < 24):(i >= 0); i+= inc) {
			if(!((1<<i) & skip))
				break;
		}
		
		// If we didn't find a TTL that will work for us, find
		// the next available one in the other direction.
		if(toh?(i >= 24):(i < 0)) {
			for(i = nt-inc; toh?(i > ot):(i < ot); i -= inc) {
				if(!((1<<i) & skip))
					break;
			}
		}

		if(i == ot)
			return;	// No Change.
		else { 
			nt = i;
			SetCtrlVal(pc.trig_ttl[1], pc.trig_ttl[0], nt);
		}
	}
	
	// Whatever we do, we do it for each instruction
	int expanded;
	for(i = 0; i < uipc.max_ni; i++) {
		cp = pc.inst[i];
		// Set the old trigger TTL to be a normal TTL and the new one to be a trigger
		set_ttl_trigger(cp, ot, 0);
		set_ttl_trigger(cp, nt, 1);

		// If there are instructions hidden, find them and move the TTLs around.
		CmtGetLock(lock_uipc);
		if(uipc.c_instrs != NULL && uipc.c_instrs[i] != NULL && uipc.max_cinstrs[i] > 0) {
			for(int j = 1; j < uipc.max_cinstrs[i]; j++)
				uipc.c_instrs[i][j]->flags = move_bit_skip(uipc.c_instrs[i][j]->flags, skip, nt, ot);
		}
		CmtReleaseLock(lock_uipc);

		// Check if it's expanded.
		GetCtrlAttribute(cp, pc.collapsepc, ATTR_VISIBLE, &expanded);
		if(expanded) {
			int cyc;
			GetCtrlVal(cp, pc.pclevel, &cyc);
			for(int j = 1; j < uipc.cyc_steps[cyc]; j++) {
				set_ttl_trigger(uipc.cyc_pans[i][j], nt, 1);
				set_ttl_trigger(uipc.cyc_pans[i][j], ot, 0);
				move_ttl_panel(uipc.cyc_pans[i][j], nt, ot);
			}
		}
		
		// Finally we move the trigger TTL to its new home.
		move_ttl(i, nt, ot);
	}
	
	CmtGetLock(lock_uipc);
	uipc.trigger_ttl = nt;
	CmtReleaseLock(lock_uipc);
}

void set_ttl_trigger(int panel, int ttl, int on) {
	// Pass -1 to ttl to set it to whatever the current trigger is.
	if(ttl < 0 || ttl >= 24)
		ttl = uipc.trigger_ttl;
	
	// Call this when you first create a panel to initialize the TTL state
	if(on) {
		SetCtrlAttribute(panel, pc.TTLs[ttl], ATTR_ON_COLOR, VAL_BLUE);
		SetCtrlAttribute(panel, pc.TTLs[ttl], ATTR_OFF_COLOR, (3*VAL_DK_GRAY+VAL_BLACK)/4);
		SetCtrlAttribute(panel, pc.TTLs[ttl], ATTR_CTRL_MODE, VAL_INDICATOR);
	} else {
		SetCtrlAttribute(panel, pc.TTLs[ttl], ATTR_ON_COLOR, VAL_RED);
		SetCtrlAttribute(panel, pc.TTLs[ttl], ATTR_OFF_COLOR, VAL_BLACK);
		SetCtrlAttribute(panel, pc.TTLs[ttl], ATTR_CTRL_MODE, VAL_HOT);
	}
}

void set_flags(int num, int flags) {
	// Convenience function.
	int panel = pc.inst[num];
	set_flags_panel(panel, flags);
}

void set_flags_panel(int panel, int flags) {
	// Sets the flags of panel to be the appropriate thing.
	
	set_flags_range(panel, flags, 0, 23);
}

void set_flags_range(int panel, int flags, int start, int end) {
	// Sets all the flags in a given range.
	if(start > end) {
		int buff = end;
		end = start;
		start = buff;
	}
	
	for(int i = start; i <= end; i++)
		SetCtrlVal(panel, pc.TTLs[i], (flags&(1<<i))?1:0);
		
}

int ttls_in_use() {
	// Gets the flags for all TTLs currently in use by the program.
	int i, j, ttls = 0;
	
	// Start with the visible instructions
	for(i = 0; i < uipc.ni; i++)
		ttls = ttls|get_flags(i);
	
	// Now if there are any phase cycling instructions, get those now.
	save_all_expanded_instructions();	// Make sure that the expanded instructions are included. 
	
	for(i = 0 ; i < uipc.nc; i++) {
		for(j = 0; j < uipc.cyc_steps[i]; j++)
			ttls = ttls|uipc.c_instrs[uipc.cyc_ins[i]][j]->flags;
	}
	
	return ttls;
}

/************* Get ND and Phase Cycling Parameters *************/  
int get_nd_state(int num) {
	// If you don't know the state of the ND instruction, this function gets it for you.
	// State 0 = Off
	// State 1 = On/Red    	=> Linear delay/instr increment
	// State 2 = On/Blue   	=> Expression mode
	// State 3 = On/Green	=> List edit mode
	
	int val, state;
	GetCtrlVal(pc.cinst[num], pc.vary, &val);

	if(!val) {
		state = 0;
	} else { 
		int color;
		GetCtrlAttribute(pc.cinst[num], pc.vary, ATTR_ON_COLOR, &color);  
		if (color == VAL_RED) {
			state = 1;
		} else if (color == VAL_BLUE) {
			state = 2;
		} else {
			state = -1;
		}
	}

	return state;
}

int get_pc_state(int num) {
	// Gets the state of phase cycling. At the moment there are only two states
	// but I'll probably add more, so it's easier this way.
	
	int val;
	GetCtrlVal(pc.inst[num], pc.pcon, &val);
	
	return val;
}

void get_updated_instr(PINSTR *instr, int num, int *cstep, int *cind, int *dind, int mode) {
	// Updates instr based on the values in uipc to whatever it should be at
	// the current step (cstep). This assumes that the uipc variables have
	// already been sorted.
	//
	// cind and dind must be initialized variables pointing to the indices of
	// the instruction in the respective uipc fields. If a negative number is
	// passed, the indices will be found and cind and dind set for future use.
	
	if(mode == 0)
		return;
	
	// Get the place of num in each of the indices if necessary.
	int i, ci = -1, di = -1;
		
	if(cind != NULL)
		ci = *cind;
	
	if(dind != NULL)
		di = *dind;
	
	if(ci < 0 && (mode & PP_V_PC)) {
		for(i=0; i<uipc.ncins; i++) {
			if(uipc.cyc_ins[i] == num) {
				ci = i;
				break;
			} else if (uipc.cyc_ins[i] > num)
				break;
		}
		
		if(cind != NULL)
			cind[0] = ci;
	} else if (!(mode & PP_V_PC)) {
		ci = -1;	
	}
	
	if(di < 0 && (mode & PP_V_ID)) {
		for(i=0; i<uipc.ndins; i++) {
			if(uipc.dim_ins[i] == num) {
				di = i;
				break;
			} else if (uipc.dim_ins[i] > num)
				break;
		}
		if(dind != NULL)
			dind[0] = di;
	} else if (!(mode & PP_V_ID)) {
		di = -1;
	}
	
	// If it's phase cycled, get the relevant instruction.
	if(ci >= 0) 
		copy_pinstr(uipc.c_instrs[num][cstep[uipc.ins_cycs[ci]]], instr);
	else
		get_instr(instr, num);
	
	// If it varies in some dimension, change either the data or delay or both.
	if(di >= 0) {
		int step = cstep[uipc.ins_dims[di]+uipc.nc];	// The step we're on.
		if(uipc.nd_data != NULL && uipc.nd_data[di] != NULL)
			instr->instr_data = uipc.nd_data[di][step];
		
		if(uipc.nd_delays != NULL && uipc.nd_delays[di] != NULL)
			instr->instr_time = uipc.nd_delays[di][step];
	}
}

/**************** Set ND Instruction Parameters ****************/  
void set_ndon(int ndon) {		// Toggles whether or not multi-dimensional instructions are on.
	// Sets whether it's a multidimensional experiment

	int dimmed, val = ndon, nd, i;

	if(val) {
		dimmed = 0;
		change_num_dims();
	} else {
		dimmed = 1;
		CmtGetLock(lock_uipc);
		uipc.nd = 0;
		CmtReleaseLock(lock_uipc);
	}
	
	// Set the controls appropriately.
	SetCtrlVal(pc.ndon[1], pc.ndon[0], val);
	SetPanelAttribute(pc.PPConfigCPan, ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pc.ndims[1], pc.ndims[0], ATTR_DIMMED, dimmed);
	for(i = 0; i<uipc.max_ni; i++) {
		SetPanelAttribute(pc.cinst[i], ATTR_DIMMED, dimmed);
		set_instr_nd_mode(i, ndon?get_nd_state(i):0);
	}
	
	if(!uipc.nc) {
		SetCtrlAttribute(pc.skip[1], pc.skip[0], ATTR_DIMMED,dimmed);
		SetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_DIMMED, dimmed);
	}
}

void update_nd_state(int num, int state) {	// Changes the state of a given ND control.   
	// State 0 = Off
	// State 1 = On/Red    	=> Linear delay/instr increment
	// State 2 = On/Blue   	=> Expression mode
	// State == -1 -> Figure out what state we're in for us.
	
	int panel = pc.cinst[num];	// Convenience
	
	// Get the state if we need it
	if(state < 0) 
		state = get_nd_state(num);

	// Set up the uipc variable.
	int ind = -1, i;			
	for(i=0; i<uipc.ndins; i++) {		// Get the place in the dim_ins array
		if(uipc.dim_ins[i] == num) {   // -1 if it's not there.
			ind = i;
			break;
		}
	}
	
	if(!state) {
		if(ind >=0) {
			// Remove the index from the lists and decrement the number of instructions
			CmtGetLock(lock_uipc);
			remove_array_item(uipc.dim_ins, ind, uipc.ndins);
			remove_array_item(uipc.ins_state, ind, uipc.ndins);
			remove_array_item(uipc.ins_dims, ind, uipc.ndins);
			
			if(uipc.nd_data[ind] != NULL)
				free(uipc.nd_data[ind]);
			if(uipc.nd_delays[ind] != NULL)
				free(uipc.nd_delays[ind]);
			
			remove_array_item_void(uipc.nd_data, ind, uipc.ndins, 2);
			remove_array_item_void(uipc.nd_delays, ind, uipc.ndins, 3);
			
			// Now reallocate the size of the arrays
			if(--uipc.ndins == 0) {
				free(uipc.dim_ins);
				free(uipc.ins_state);
				free(uipc.ins_dims);
				free(uipc.nd_data);
				free(uipc.nd_delays);
				
				uipc.dim_ins = NULL;
				uipc.ins_state = NULL;
				uipc.ins_dims = NULL;
				uipc.nd_data = NULL;
				uipc.nd_delays = NULL;
			} else {
				uipc.dim_ins = realloc(uipc.dim_ins, sizeof(int)*uipc.ndins);
				uipc.ins_dims = realloc(uipc.ins_dims, sizeof(int)*uipc.ndins);
				uipc.ins_state = realloc(uipc.ins_state, sizeof(int)*uipc.ndins);
				uipc.nd_data = realloc(uipc.nd_data, sizeof(int*)*uipc.ndins);
				uipc.nd_delays = realloc(uipc.nd_delays, sizeof(double*)*uipc.ndins);	
			}
			CmtReleaseLock(lock_uipc);
		}
	} else {
		if(ind < 0) {				// only need to update if we're not already there.
			CmtGetLock(lock_uipc);
			uipc.ndins++;			// Increment the number of instructions varied 
			
			// Memory allocation is important.
			if(uipc.dim_ins == NULL)
				uipc.dim_ins = malloc(sizeof(int)*uipc.ndins);
			else
				uipc.dim_ins = realloc(uipc.dim_ins, sizeof(int)*uipc.ndins);
			
			if(uipc.ins_dims == NULL)
				uipc.ins_dims = malloc(sizeof(int)*uipc.ndins);
			else
				uipc.ins_dims = realloc(uipc.ins_dims, sizeof(int)*uipc.ndins);
			
			if(uipc.ins_state == NULL)
				uipc.ins_state = malloc(sizeof(int)*uipc.ndins);
			else
				uipc.ins_state = realloc(uipc.ins_state, sizeof(int)*uipc.ndins);
			
			if(uipc.nd_data == NULL)
				uipc.nd_data = malloc(sizeof(int*)*uipc.ndins);
			else
				uipc.nd_data = realloc(uipc.nd_data, sizeof(int*)*uipc.ndins);
			
			if(uipc.nd_delays == NULL)
				uipc.nd_delays = malloc(sizeof(double*)*uipc.ndins);
			else
				uipc.nd_delays = realloc(uipc.nd_delays, sizeof(double*)*uipc.ndins);
	
			// Need to update the list controls
			int nl;
			GetNumListItems(panel, pc.dim, &nl);
			if(nl < uipc.nd) {
				int elements;
				char **c = generate_char_num_array(1, uipc.nd, &elements);
				
				for(i=nl; i<uipc.nd; i++) 
					InsertListItem(panel, pc.dim, -1, c[i], i);
				
				if(c != NULL) {
					for(i = 0; i<elements; i++)
						free(c[i]);
					free(c);
				}
				
			} else if(nl > uipc.nd) 
				DeleteListItem(panel, pc.dim, uipc.nd, -1);
			
			int dim, nstep;
			GetCtrlVal(panel, pc.dim, &dim);
			GetCtrlVal(panel, pc.nsteps, &nstep);
			
			// Now insert our instruction at the end.
			uipc.ins_dims[uipc.ndins-1] = dim;
			uipc.dim_ins[uipc.ndins-1] = num;
			uipc.ins_state[uipc.ndins-1] = state;
			uipc.nd_data[uipc.ndins-1] = NULL; 
			uipc.nd_delays[uipc.ndins-1] = NULL;
			
			SetCtrlVal(panel, pc.nsteps, uipc.dim_steps[dim]);
			CmtReleaseLock(lock_uipc);
			if(state == 1)
				update_nd_increment(num, MC_INC);
			else
				update_nd_from_exprs(num);
		}
	}
	
	
	// Set up the controls that need to be dimmed or hidden
	int inc_ctrls[4] = {pc.del_inc, pc.delu_inc,
						pc.dat_inc, pc.disp_inc};
	int exprs_ctrls[2] = {pc.cexpr_data, pc.cexpr_delay};
	int all_ctrls[10] = {pc.del_init, pc.delu_init, pc.dat_init, 
						 pc.disp_init, pc.disp_fin, pc.del_fin, 
						 pc.delu_fin, pc.dat_fin, pc.nsteps, pc.dim};
	int inc_num = 4, exprs_num = 2, all_num = 10;
	int exprs = MC_HIDDEN, inc = MC_HIDDEN, main = MC_DIMMED, all = 0;
	int color = VAL_RED;	// This will change as necessary
	
	if(state > 0)		// A varied state
		SetCtrlVal(panel, pc.vary, 1);
	
	// Dim/Hide the controls that need to be dimmed and/or hidden
 	if(state == 0) {
		// In the 0 state, we're off, so that should be dimmed time increment stuff
		all = MC_DIMMED;
		inc = MC_DIMMED;
	} else if(state == 1) {
		inc = 0;		   // No color change needed
	} else if(state == 2) {
		exprs = 0;
		color = VAL_BLUE;
	}
	
	// Set up the visibility modes
	change_visibility_mode(panel, all_ctrls, all_num, all);
	change_visibility_mode(panel, inc_ctrls, inc_num, inc);
	change_visibility_mode(panel, exprs_ctrls, exprs_num, exprs);
	
	SetCtrlAttribute(panel, pc.vary, ATTR_ON_COLOR, color); // Change color to red.
	SetCtrlVal(panel, pc.vary, state);
	
	set_instr_nd_mode(num, state);
}

void set_instr_nd_mode(int num, int nd) {
	// Sets the mode for the 1D instruction based to "ND" if nd > 0
	// otherwise to the normal 1D mode.
	// Modes:
	// 0 -> All instructions undimmed, border is normal.
	// 1 -> Border red, instruction and instr_data are dimmed
	// 2 -> Border blue, instruction and instr_data are dimmed.
	
	// First update the dim controls
	int main_num = 3;
	int main_ctrls[3] = {pc.delay, pc.instr_d, pc.delayu};
	change_visibility_mode(pc.inst[num], main_ctrls, main_num, nd?MC_DIMMED:0);
	
	if(nd) {
		SetCtrlAttribute(pc.inst[num], pc.ins_num, ATTR_DISABLE_PANEL_THEME, 1);
		SetCtrlAttribute(pc.inst[num], pc.ins_num, ATTR_FRAME_COLOR, (nd == 2)?VAL_BLUE:VAL_RED);
	} else { 
		SetCtrlAttribute(pc.inst[num], pc.ins_num, ATTR_FRAME_COLOR, 14737379);	// The default color, for whatever reason it's 0xE0DFE3
		SetCtrlAttribute(pc.inst[num], pc.ins_num, ATTR_DISABLE_PANEL_THEME, 0);
		
		int instr; 
		GetCtrlVal(pc.inst[num], pc.instr, &instr);
		if(!takes_instr_data(instr))
			SetCtrlAttribute(pc.inst[num], pc.instr_d, ATTR_DIMMED, 1);	
	}
	
	
}

void change_num_dims() { 	// Updates the number of dimensions in the experiment
	int i, j, nd;
	GetCtrlVal(pc.ndims[1], pc.ndims[0], &nd);
	if(--nd == uipc.nd)
		return;
	
	// Update the UI elements
	if(uipc.ndins) {
		char **c = NULL;
		int elements = 0;
		if(nd > uipc.nd)
		c = generate_char_num_array(1, nd, &elements);
		
		// We are going to be updating uipc as we go, so we want a local copy so that as we
		// iterate through it we don't end up skipping instructions and such
		int ndins = uipc.ndins;
		int *ins_dims = malloc(sizeof(int)*ndins), *dim_ins = malloc(sizeof(int)*ndins);
		for(i = 0; i<ndins; i++) {
			ins_dims[i] = uipc.ins_dims[i];
			dim_ins[i] = uipc.dim_ins[i];
		}
		
		for(i = 0; i<ndins; i++) {
			if(ins_dims[i] >= nd) {
				update_nd_state(dim_ins[i], 0);
				continue;
			}
		
			if(nd < uipc.nd) {
				DeleteListItem(pc.cinst[dim_ins[i]], pc.dim, nd, -1); // Delete the rest of them.
			} else {
				for(j=uipc.nd; j<nd; j++)
					InsertListItem(pc.cinst[dim_ins[i]], pc.dim, -1, c[j], j);
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
	
	CmtGetLock(lock_uipc);
	uipc.nd = nd;
	if(uipc.max_nd < nd) {
		if(uipc.dim_steps == NULL)
			uipc.dim_steps = malloc(sizeof(int)*nd);
		else
			uipc.dim_steps = realloc(uipc.dim_steps, sizeof(int)*nd);
		
		for(i=uipc.max_nd; i<nd; i++)
			uipc.dim_steps[i] = 2;
		
		uipc.max_nd = nd;
	}
	CmtReleaseLock(lock_uipc);
}

void change_num_dim_steps(int dim, int steps) { // Updates number of steps in the given dimension
	int i, j, nl, cp;
	
	if (steps == uipc.dim_steps[dim])
		return;	// No change
	
	// Update the controls
	for(i = 0; i<uipc.ndins; i++) {
		if(uipc.ins_dims[i] == dim)
			change_nd_steps_instr(uipc.dim_ins[i], steps);
	}
	
	// Now update the uipc variable and we're done.
	CmtGetLock(lock_uipc);
	uipc.dim_steps[dim] = steps;
	CmtReleaseLock(lock_uipc);
 
}

void change_nd_steps_instr(int num, int steps) { // Changes the number of steps for a given instr
	int panel = pc.cinst[num];
	
	SetCtrlVal(panel, pc.nsteps, steps);
	
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
	int panel = pc.cinst[num];
	
	// Pretty much all you need to do is update uipc (for now)
	int nl;
	GetNumListItems(panel, pc.dim, &nl);
	if(nl)
		GetCtrlVal(panel, pc.dim, &dim);		// Get the new dimension
	else
		dim = 0;
	
	for(i=0; i<uipc.ndins; i++) {
		if(uipc.dim_ins[i] == num)
			break;
	}
	
	if(i == uipc.ndins) 
		return;				// Something's wrong, return.
	
	
	ind = i;
	// Update ins_dims and change the number of steps for the control.
	uipc.ins_dims[ind] = dim;

	change_nd_steps_instr(num, uipc.dim_steps[dim]);
}

void populate_dim_points() {	// Function for updating the UI with the values from the uipc var
	int j, nl, dim;
	
	// A char array of labels
	int elements;
	char **c = generate_char_num_array(1, uipc.nd, &elements);
	
	int panel;
	for(int i = 0; i<uipc.ndins; i++) {
		panel = pc.cinst[uipc.dim_ins[i]];

		// First make the number of dimensions per control correct
		GetNumListItems(panel, pc.dim, &nl);
		if(nl < uipc.nd) {
			for(j=nl; j<uipc.nd; j++)
				InsertListItem(panel, pc.dim, -1, c[j], j);
		} else if (nl > uipc.nd) {
			DeleteListItem(panel, pc.dim, uipc.nd, -1);
		}
		
		// Now update the number of steps.
		dim = uipc.ins_dims[i];
		SetCtrlIndex(panel, pc.dim,dim);		// In case the dimension has changed
		SetCtrlVal(panel, pc.nsteps, uipc.dim_steps[dim]);	
	}
	
	for(j=0; j<elements; j++)
		free(c[j]);
	free(c);
	
	// Now update the number of dimensions
	int nd;
	GetCtrlVal(pc.ndims[1], pc.ndims[0], &nd);
	if(uipc.nd < 1) {
		SetCtrlVal(pc.ndims[1], pc.ndims[0], 2);
		set_ndon(0);
	} else if (--nd != uipc.nd) {
		SetCtrlVal(pc.ndims[1], pc.ndims[0], uipc.nd+1);
		change_num_dims();
	}
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
	int panel = pc.cinst[num]; // Convenience
	
	// Get the values that are there now.
	GetCtrlVal(panel, pc.del_init, &init);
	GetCtrlVal(panel, pc.del_inc, &inc);
	GetCtrlVal(panel, pc.del_fin, &fin);
	GetCtrlVal(panel, pc.delu_init, &initu);
	GetCtrlVal(panel, pc.delu_inc, &incu);
	GetCtrlVal(panel, pc.delu_fin, &finu);
	GetCtrlVal(panel, pc.cinstr, &instr);
	
	// Convert everything to nanoseconds.
	init *= pow(1000, initu);
	inc *= pow(1000, incu);
	fin *= pow(1000, finu);
	
	// Get the dimension it varies along
	GetCtrlVal(panel, pc.nsteps, &steps);

	 // First the delays
	steps--; 	// Temporary decrement, because the step indexing is 0-based.
	switch(mode) {
		case MC_INIT:
			init = fin-(inc*steps);
			
			if(init < 0) {		// Init can't be negative.
				init = 0;
				fin = steps*inc;
			}
			
			initu = calculate_units(init);
			SetCtrlVal(pc.cinst[num], pc.delu_init, initu);
			break;
		case MC_INC:
			inc = (fin-init)/steps;
			
			incu = calculate_units(inc);
			SetCtrlVal(pc.cinst[num], pc.delu_inc, incu);
			
			break;
		case MC_FINAL:
			fin = init+(inc*steps);
			
			if(fin < 0) {		// Only possible when inc is negative.
				fin = 0;
				init = -inc*steps;
			}
			
			finu = calculate_units(fin);
			SetCtrlVal(pc.cinst[num], pc.delu_fin, finu);
			break;
	}
	steps++;
	
	// Update the uipc variable.
	int ind, i;
	for(i=0; i<uipc.ndins; i++) {
		if(uipc.dim_ins[i] == num)
			break;
	}
	if(i < uipc.ndins)
		ind = i;
	else
		ind = -1;
	
	CmtGetLock(lock_uipc);
	if(ind >= 0) {
		if(uipc.nd_delays[ind] == NULL) 
			uipc.nd_delays[ind] = malloc(sizeof(double)*steps);
		else
			uipc.nd_delays[ind] = realloc(uipc.nd_delays[ind], sizeof(double)*steps);
	
		for(i=0; i<steps; i++) 
			uipc.nd_delays[ind][i] = init+inc*i;
	}
	CmtReleaseLock(lock_uipc);
	
	// Convert back to the appropriate units
	init /= pow(1000, initu);
	inc /= pow(1000, incu);
	fin /= pow(1000, finu);
	
	// Set the controls to the new values
	SetCtrlVal(panel, pc.del_init, init);
	SetCtrlVal(panel, pc.del_inc, inc);
	SetCtrlVal(panel, pc.del_fin, fin);
	
	SetCtrlAttribute(panel, pc.del_init, ATTR_PRECISION, get_precision(init, MCUI_DEL_PREC));
	SetCtrlAttribute(panel, pc.del_inc, ATTR_PRECISION, get_precision(inc, MCUI_DEL_PREC));
	SetCtrlAttribute(panel, pc.del_fin, ATTR_PRECISION, get_precision(fin, MCUI_DEL_PREC));
	
	// Now if necessary, update the data increment controls
	if(takes_instr_data(instr)) {
		int initd, incd, find;	// Data

		GetCtrlVal(panel, pc.dat_init, &initd);
		GetCtrlVal(panel, pc.dat_inc, &incd);
	 	GetCtrlVal(panel, pc.dat_fin, &find);
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
		CmtGetLock(lock_uipc);
		if(ind >= 0) {
			if(uipc.nd_delays[ind] == NULL) 
				uipc.nd_data[ind] = malloc(sizeof(int)*steps);
			else
				uipc.nd_data[ind] = realloc(uipc.nd_data[ind], sizeof(int)*steps);
	
			for(int i=0; i<steps; i++) 
				uipc.nd_data[ind][i] = initd+incd*i;
		}
		CmtReleaseLock(lock_uipc);
		
		SetCtrlVal(panel, pc.dat_init, initd);
		SetCtrlVal(panel, pc.dat_inc, incd);
	 	SetCtrlVal(panel, pc.dat_fin, find);
	}
	
	// Update the 1D controls.
	int dat, delu, delu1d;
	double del;
	GetCtrlVal(panel, pc.dat_init, &dat);
	GetCtrlVal(panel, pc.del_init, &del);
	GetCtrlVal(panel, pc.delu_init, &delu);
	GetCtrlVal(pc.inst[num], pc.delayu, &delu1d);
	
	del /= pow(1000, delu)/pow(1000, delu1d);	// Get new value for old units.
	
	// Set the controls as appropriate.
	SetCtrlVal(pc.inst[num], pc.delay, del);
	SetCtrlVal(pc.inst[num], pc.instr_d, dat);
}


void update_nd_from_exprs(int num) {// Generate the experiment from the exprs (state == 2)
	// Return value is 0 or a positive error.
	int state = get_nd_state(num);
	int i, *cstep = NULL;
	int panel = pc.cinst[num];
	int dim, ind;
	
	for(i=0; i<uipc.ndins; i++) {
		if(uipc.dim_ins[i] == num)
			break;
	}
	
	if(i== uipc.ndins) // Index not found
		return;
	ind = i;
	
	int eval_data = 0, eval_delay = 0; // Whether or not to evaluate these things.
	int instr, len_data, len_delays, len_default;
	char *expr_delay, *expr_data, *default_val;
	
	GetCtrlVal(pc.inst[num], pc.instr, &instr);   // The instruction
	GetCtrlVal(panel, pc.dim, &dim);				// What dimension we're varying along.
	if(takes_instr_data(instr)) {
		GetCtrlValStringLength(panel, pc.cexpr_data, &len_data);
		GetCtrlAttribute(panel, pc.cexpr_data, ATTR_DFLT_VALUE_LENGTH, &len_default);
		
		expr_data = malloc(len_data+1);
		GetCtrlVal(panel, pc.cexpr_data, expr_data);
		
		default_val = malloc(len_default+1);
		GetCtrlAttribute(panel, pc.cexpr_data, ATTR_DFLT_VALUE, default_val);
		
		if(strcmp(default_val, expr_data) != 0)
			eval_data = 1;
		else
			free(expr_data);
		
		free(default_val);
	}
	
	GetCtrlValStringLength(panel, pc.cexpr_delay, &len_delays);
	GetCtrlAttribute(panel, pc.cexpr_delay, ATTR_DFLT_VALUE_LENGTH, &len_default);
	
	expr_delay = malloc(len_delays+1);
	GetCtrlVal(panel, pc.cexpr_delay, expr_delay);
	
	default_val = malloc(len_default+1);
	GetCtrlAttribute(panel, pc.cexpr_delay, ATTR_DFLT_VALUE, default_val);
	
	if(len_delays != 0 && strcmp(default_val, expr_delay) != 0)
		eval_delay = 1;
	else
		free(expr_delay);
	
	free(default_val);
	
	// At this point we know which one(s) to evaluate, and we have the expressions.
	
	if(!eval_delay && !eval_data)
		return;	// Nothing to do.
	
	// Set up the uipc var if necessary.
	int steps;
	GetCtrlVal(panel, pc.nsteps, &steps);
	
	CmtGetLock(lock_uipc);
	if(eval_delay) {
		if(uipc.nd_delays[ind] == NULL)
			uipc.nd_delays[ind] = malloc(sizeof(double)*steps);
		else
			uipc.nd_delays[ind] = realloc(uipc.nd_delays[ind], sizeof(double)*steps);
	}
	
	if(eval_data) {
		if(uipc.nd_data[ind] == NULL)
			uipc.nd_data[ind] = malloc(sizeof(int)*steps);
		else
			uipc.nd_data[ind] = realloc(uipc.nd_data[ind], sizeof(int)*steps);
	}
	CmtReleaseLock(lock_uipc);
	
	// Build the basic cstep
	cstep = malloc(sizeof(int)*(uipc.nc+uipc.nd));
	for(i = 0; i<(uipc.nc+uipc.nd); i++)
		cstep[i] = 0;	// Just set everything we don't care about to the first one, it's not important
	
	
	int err_del = 0, err_dat = 0;			// Error values
	constants *c = setup_constants(); 		// Initializes the static constants
	
	// Now some static constants unique to this type of evaluation.
	double init, fin;
	int initu, finu;
	GetCtrlVal(panel, pc.del_init, &init);
	GetCtrlVal(panel, pc.delu_init, &initu);
	GetCtrlVal(panel, pc.del_fin, &fin);
	GetCtrlVal(panel, pc.delu_fin, &finu);
	
	init *= pow(1000, initu);
	fin *= pow(1000, finu);
	
	add_constant(c, "init", C_DOUBLE, &init);
	add_constant(c, "fin", C_DOUBLE, &fin);
	
	CmtGetLock(lock_uipc);
	for(i=0; i<steps; i++) {
		cstep[dim]++;
		update_constants(c, cstep);
		
		// We also will add both "x" and "step" here.
		change_constant(c, "x", C_INT, &i);
		change_constant(c, "step", C_INT, &i);
		
		if(eval_delay && !err_del) {
			uipc.nd_delays[ind][i] = parse_math(expr_delay, c, &err_del, 0);
			
			if(uipc.nd_delays[ind][i] < 0) 
				err_del = get_parse_error(-1, NULL)+1;		// The parse errors have higher precedence
			
			if(err_del) {
				uipc.err_del_size = uipc.nc+uipc.nd;
				if(uipc.err_del_pos != NULL) // Save the cstep
					uipc.err_del_pos = realloc(uipc.err_del_pos, sizeof(int)*uipc.err_del_size);
				else
					uipc.err_del_pos = malloc(sizeof(int)*uipc.err_del_size);
				
				for(int j=0; j<uipc.err_del_size; j++)
					uipc.err_del_pos[j] = cstep[j];
				
				if(!eval_data || err_dat)
					break;
			}
		}
		
		if(eval_data && !err_dat) {
			uipc.nd_data[ind][i] = (int)parse_math(expr_data, c, &err_dat, 0);
			
			if(uipc.nd_data[ind][i] < 0) 
				err_dat = get_parse_error(-1, NULL)+1;
			
			if(err_dat) {
				uipc.err_dat_size = uipc.nc+uipc.nd;
				if(uipc.err_dat_pos != NULL) // Save the cstep
					uipc.err_dat_pos = realloc(uipc.err_dat_pos, sizeof(int)*uipc.err_dat_size);
				else
					uipc.err_dat_pos = malloc(sizeof(int)*uipc.err_dat_size);
				
				for(int j=0; j<uipc.err_dat_size; j++)
					uipc.err_dat_pos[j] = cstep[j];

				if(!eval_delay || err_del)
					break;
			}
		}
	}
	
	CmtReleaseLock(lock_uipc);
	
	free_constants(c);
	
	// If the evaluation was unsuccessful, the border should turn red.
	// If it was successful, the border should turn green, otherwise it is
	// switched back to the windows style with no specific border color.
	
	int dat_bgcolor = VAL_WHITE, del_bgcolor = VAL_WHITE;
	int dat_bold = 0, del_bold = 0;
	int dat_textcolor = VAL_BLACK, del_textcolor = VAL_BLACK;
	if(eval_delay) {
		free(expr_delay);
		
		del_bold = 1;

		double iv = uipc.nd_delays[ind][0], fv = uipc.nd_delays[ind][steps-1];
		int initu = calculate_units(iv), finu = calculate_units(fv);
		iv /= pow(1000, initu);
		fv /= pow(1000, finu);
		
		SetCtrlAttribute(pc.cinst[num], pc.del_init, ATTR_PRECISION, get_precision(iv, 6));
		SetCtrlAttribute(pc.cinst[num], pc.del_fin, ATTR_PRECISION, get_precision(fv, 6));
		
		SetCtrlIndex(pc.cinst[num], pc.delu_init, initu);
		SetCtrlIndex(pc.cinst[num], pc.delu_fin, finu);
		
		SetCtrlVal(pc.cinst[num], pc.del_init, (uipc.nd_delays[ind][0])/pow(1000, initu));
		SetCtrlVal(pc.cinst[num], pc.del_fin, (uipc.nd_delays[ind][steps-1])/pow(1000, finu));
		
		CmtGetLock(lock_uipc);
		if(uipc.err_del = err_del) {			// Not a typo, I'm actually setting the
			del_bgcolor = VAL_RED;			    // value of the uipc error field here.
			del_textcolor = VAL_OFFWHITE;
		} else 
			del_bgcolor = VAL_GREEN;
		CmtReleaseLock(lock_uipc);
	} 
	
	if(eval_data) {
		free(expr_data);
		
		dat_bold = 1;
		
		SetCtrlVal(pc.cinst[num], pc.dat_init, uipc.nd_data[ind][0]);
		SetCtrlVal(pc.cinst[num], pc.dat_fin, uipc.nd_data[ind][steps-1]);
		
		CmtGetLock(lock_uipc);
		if(uipc.err_dat = err_dat) {			// Again, not a typo.
			dat_bgcolor = VAL_RED;
			dat_textcolor = VAL_OFFWHITE;
		} else 
			dat_bgcolor = VAL_GREEN;
		CmtReleaseLock(lock_uipc);
	}
	
	free(cstep);
	
	// Update the UI Controls
	SetCtrlAttribute(panel, pc.cexpr_delay, ATTR_TEXT_BGCOLOR, del_bgcolor);
	SetCtrlAttribute(panel, pc.cexpr_delay, ATTR_TEXT_BOLD, del_bold);
	SetCtrlAttribute(panel, pc.cexpr_delay, ATTR_TEXT_COLOR, del_textcolor);
	
	SetCtrlAttribute(panel, pc.cexpr_data, ATTR_TEXT_BGCOLOR, dat_bgcolor);
	SetCtrlAttribute(panel, pc.cexpr_data, ATTR_TEXT_BOLD, dat_bold);
	SetCtrlAttribute(panel, pc.cexpr_data, ATTR_TEXT_COLOR, dat_textcolor);
	
	// Update the 1D controls.
	int dat, delu, delu1d;
	double del;
	GetCtrlVal(panel, pc.dat_init, &dat);
	GetCtrlVal(panel, pc.del_init, &del);
	GetCtrlVal(panel, pc.delu_init, &delu);
	GetCtrlVal(pc.inst[num], pc.delayu, &delu1d);
	
	del /= pow(1000, delu)/pow(1000, delu1d);	// Get new value for old units.
	
	// Set the controls as appropriate.
	SetCtrlVal(pc.inst[num], pc.delay, del);
	SetCtrlVal(pc.inst[num], pc.instr_d, dat);
	
	return;
}

void update_skip_condition() { 		// Generate uipc.skip_locs from the UI.
	if(uipc.nd == 0 && uipc.nc == 0)
		return;						// Nothing to skip
									   
	char *expr;
	int len, i, size=uipc.nd+uipc.nc;
	int pan = pc.skiptxt[1];
	int ctrl = pc.skiptxt[0];
	
	// Get the expression string.
	GetCtrlValStringLength(pan, ctrl, &len);
	
	if(len == 0)
		return;						// Nothing to evaluate
	
	ui_cleanup(0);					// Clean up before evaluation.
	
	expr = malloc(len+1);
	GetCtrlVal(pan, ctrl, expr);
	
	// Get max_n_steps and maxsteps;
	int *maxsteps = malloc(sizeof(int)*size);
	
	CmtGetLock(lock_uipc);
	uipc.max_n_steps = 1;
	for(i=0; i<uipc.nc; i++) {
		uipc.max_n_steps*=uipc.cyc_steps[i];
		maxsteps[i] = uipc.cyc_steps[i];
	}
	
	for(i=0; i<uipc.nd; i++) { 
		uipc.max_n_steps*=uipc.dim_steps[i];
		maxsteps[i+uipc.nc] = uipc.dim_steps[i];
	}
	CmtReleaseLock(lock_uipc);
	
	// Allocate space for skip_locs. One for each step. We'll use an
	// unsigned char array since bools aren't a thing in C
	int max_n_steps = uipc.max_n_steps;
	if(max_n_steps == 0) {
		free(maxsteps);
		free(expr);
		return;
	}
	
	CmtGetLock(lock_uipc);
	if(uipc.skip_locs == NULL)
		uipc.skip_locs = malloc(sizeof(unsigned char)*max_n_steps);
	else
		uipc.skip_locs = realloc(uipc.skip_locs, sizeof(unsigned char)*max_n_steps);
	
	// Set up the static constants
	constants *c = setup_constants();
	
	// Iterate through each step in the experiment and generate the skips
	int err;
	uipc.real_n_steps = max_n_steps;				// Assume we didn't skip anything at first.
	int *cstep = malloc(sizeof(int)*size);
	
	for(i=0; i<max_n_steps; i++) {
		get_cstep(i, cstep, maxsteps, size); 	// Convert from linear index to cstep.
		
		update_constants(c, cstep);				// Update the dynamic constants (variables)
		
		double val = parse_math(expr, c, &err, 0);
		
		if(val) {
			uipc.skip_locs[i] = 1;
			uipc.real_n_steps--;
		} else
			uipc.skip_locs[i] = 0;
		
		if(err)
			break;
	}
	
	free_constants(c);
	
	// The new values for the UI controls.
	int bg = VAL_GREEN, txt = VAL_BLACK;
	
	// Update the control if there was an error.
	if(uipc.skip_err = err) {		// Not a typo, I'm setting uipc.skip_err to err.
		bg = VAL_RED;
		txt = VAL_OFFWHITE;
		
		uipc.skip_err_size = size;
		if(uipc.skip_err_pos == NULL)
			uipc.skip_err_pos = malloc(sizeof(int)*size);
		else
			uipc.skip_err_pos = realloc(uipc.skip_err_pos, sizeof(int)*size);
		
		for(i=0; i<size; i++)
			uipc.skip_err_pos[i] = cstep[i];
		
		uipc.real_n_steps = max_n_steps; 	// If we fucked it up, there will be no skipping.
	}
	
	CmtReleaseLock(lock_uipc);
	
	SetCtrlAttribute(pan, ctrl, ATTR_TEXT_BOLD, 1);
	SetCtrlAttribute(pan, ctrl, ATTR_TEXT_BGCOLOR, bg);
	SetCtrlAttribute(pan, ctrl, ATTR_TEXT_COLOR, txt);
	
	if(err)
		SetCtrlAttribute(pc.skip[1], pc.skip[0], ATTR_CTRL_MODE, VAL_INDICATOR);
	else
		SetCtrlAttribute(pc.skip[1], pc.skip[0], ATTR_CTRL_MODE, VAL_HOT);
	
	free(expr);
	free(maxsteps);
	free(cstep);
	
}

char *get_tooltip(int skip) {
	// Generates the text of a tooltip explaining the variables available
	// for the skip expressions. Does not currently need to be freed at the
	// moment.
	//
	// Pass TRUE to skip for vars for the skip expression
	// Pass FALSE to skip for the nd expressions.
	
	if(skip) {
		return 	"Variables\n"
				"------------------\n"
				"nd: Number of dimensions\n"
				"nc: Number of cycles\n"
				"\n"
				"cds#: Current dimension step (cds0, cds1,...)\n"
				"mds#: Max dimension step in dimension \"#\"\n"
				"ccs#: Current cycle step (ccs0, ccs1, ...)\n"
				"mcs#: Maximum cycle step in cycle \"#\"\n"
				"\n"
				"us: Microseconds\n"
				"ms: Milliseconds\n"
				"s: Seconds\n"
				"------------------\n";
	} else {
		return 	"Variables\n"
				"------------------\n"
				"nd: Number of dimensions\n"
				"nc: Number of cycles\n"
				"\n"
				"x or step: The current step in this dimension."
				"\n"
				"cds#: Current dimension step (cds0, cds1,...)\n"
				"mds#: Max dimension step in dimension \"#\"\n"
				"ccs#: Current cycle step (ccs0, ccs1, ...)\n"
				"mcs#: Maximum cycle step in cycle \"#\"\n"
				"\n"
				"us: Microseconds\n"
				"ms: Milliseconds\n"
				"s: Seconds\n"
				"------------------\n";
	}
}

/**************** Set Phase Cycling Parameters *****************/ 

void update_pc_state(int num, int state) {
	// Right now there are only two states, on and off.
	int all_ctrls[3] = {pc.pcsteps, pc.pclevel, pc.pcstep};
	int all_num = 3, all;
	int panel = pc.inst[num];
	int i, ind = -1;
	int left = 1106;	// This is the "PC off" state.
	
	// Find out if it's already in the list.
	if(uipc.cyc_ins != NULL) {
		for(i = 0; i<uipc.ncins; i++) {
			if(uipc.cyc_ins[i] == num) {
				ind = i;
				break;
			}
		}
	}
								
	if(!state) {
		all = MC_DIMMED;
		int up_visible;
		GetCtrlAttribute(panel, pc.collapsepc, ATTR_VISIBLE, &up_visible);
			
		// If up is visible, we need to collapse it first.
		if(up_visible)
			set_phase_cycle_expanded(num, 0);
		
		if(ind>=0) {
			// Remove the index from the lists
			CmtGetLock(lock_uipc);
			remove_array_item(uipc.cyc_ins, ind, uipc.ncins);
			remove_array_item(uipc.ins_cycs, ind, uipc.ncins);
			
			// Now we're going to decrement ncins and free the memory from those lists.
			if(--uipc.ncins <= 0) {
				free(uipc.cyc_ins);
				free(uipc.ins_cycs);
				
				uipc.cyc_ins = NULL;
				uipc.ins_cycs = NULL;
			} else {
				uipc.cyc_ins = realloc(uipc.cyc_ins, sizeof(int)*uipc.ncins);
				uipc.ins_cycs = realloc(uipc.ins_cycs, sizeof(int)*uipc.ncins);
			}
			CmtReleaseLock(lock_uipc);
			
			// Update the controls
			SetCtrlAttribute(pc.cinst[num], pc.cins_num, ATTR_FRAME_COLOR, 14737379);	// The default color, for whatever reason
			SetCtrlAttribute(pc.cinst[num], pc.cins_num, ATTR_DISABLE_PANEL_THEME, 0);
			
			// Hide the expander button.
			SetCtrlAttribute(panel, pc.expandpc, ATTR_VISIBLE, 0);
			SetCtrlAttribute(panel, pc.xbutton, ATTR_LEFT, left);
		}
	} else {
		all = 0;
		// If we haven't turned on phase cycling yet, add a cycle
		if(!uipc.nc) {
			SetCtrlVal(pc.numcycles[1], pc.numcycles[0], 1);
			change_num_cycles();
		}
		
		if(ind < 0) {  	// If this isn't true, that's an error (for now).
			// Populate the rings.
			int nl;
			GetNumListItems(panel, pc.pclevel, &nl);
			int elements;
			char **c;
			
			if(uipc.nc > nl) {
				c = generate_char_num_array(1, uipc.nc, &elements);
			
				for(i = nl; i<uipc.nc; i++) 
					InsertListItem(panel, pc.pclevel, -1, c[i], i); 	// Insert the cycles
				
				if(c != NULL) {
					for(i = 0; i<elements; i++)
						free(c[i]);
					free(c);
				}
			} else if(uipc.nc < nl) 
				DeleteListItem(panel, pc.pclevel, uipc.nc, -1);
			
			int cyc;
			GetCtrlVal(panel, pc.pclevel, &cyc);
		
			GetNumListItems(panel, pc.pcstep, &nl);
			if(nl > uipc.cyc_steps[cyc]) 
				DeleteListItem(panel, pc.pcstep, nl, -1);
			else if(nl < uipc.cyc_steps[cyc]) {
				c = generate_char_num_array(1, uipc.cyc_steps[cyc], &elements);

				for(i = nl; i<uipc.cyc_steps[cyc]; i++) 
					InsertListItem(panel, pc.pcstep, -1, c[i], i);	
				
				if(c != NULL) {
					for(i = 0; i<elements; i++)
						free(c[i]);	
					free(c);
				}
			}
			
			// Now we need to update the uipc var.
			CmtGetLock(lock_uipc);
			uipc.ncins++;
			if(uipc.cyc_ins == NULL)
				uipc.cyc_ins = malloc(uipc.ncins*sizeof(int));
			else
				uipc.cyc_ins = realloc(uipc.cyc_ins, uipc.ncins*sizeof(int));
			
			if(uipc.ins_cycs == NULL)
				uipc.ins_cycs = malloc(uipc.ncins*sizeof(int));
			else
				uipc.ins_cycs = realloc(uipc.ins_cycs, uipc.ncins*sizeof(int));
			
			uipc.ins_cycs[uipc.ncins-1] = cyc;
			uipc.cyc_ins[uipc.ncins-1] = num;
			CmtReleaseLock(lock_uipc);
		} else {
			all = MC_DIMMED;
		}
		
		SetCtrlAttribute(pc.cinst[num], pc.cins_num, ATTR_FRAME_COLOR, VAL_RED);	// The default color, for whatever reason
		SetCtrlAttribute(pc.cinst[num], pc.cins_num, ATTR_DISABLE_PANEL_THEME, 1);
		
		// Show the expander button
		SetCtrlAttribute(panel, pc.expandpc, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, pc.xbutton, ATTR_LEFT, left + 10);
	}
	
	// Update transient mode.
	// If not, uppdate the transient acquisition value.
	int niv = 1, oiv;
	
	if(uipc.cyc_steps != NULL && uipc.ins_cycs != NULL) {
		int cycs_found = 0;
		int cyc, cbit;
		for(i = 0; i < uipc.ncins; i++) {
			cyc = uipc.ins_cycs[i];
			cbit = 1<<cyc;
			if(!(cbit&cycs_found)) {
				niv *= uipc.cyc_steps[cyc];
				cycs_found = cbit|cycs_found;
			}
		}
	}
	
	GetCtrlAttribute(pc.nt[1], pc.nt[0], ATTR_INCR_VALUE, &oiv);
	SetCtrlAttribute(pc.nt[1], pc.nt[0], ATTR_INCR_VALUE, niv);
	
	int nt;
	GetCtrlVal(pc.nt[1], pc.nt[0], &nt);
	SetCtrlVal(pc.nt[1], pc.nt[0], (int)((nt/oiv)*niv));
	
	change_visibility_mode(panel, all_ctrls, all_num, all);
	SetCtrlVal(panel, pc.pcon, state);
}

void change_num_cycles() {
	// Function called if the cycle number control is changed.
	int nc, i, j;
	GetCtrlVal(pc.numcycles[1], pc.numcycles[0], &nc);
	
	if(nc == uipc.nc)
		return;
	
	// Update the UI elements
	if(uipc.ncins) {
		char **c = NULL;	   
		int elements = 0;
		if(nc > uipc.nc)
		c = generate_char_num_array(1, nc, &elements);

		// We are going to be updating uipc as we go, so we want a local copy so that as we
		// iterate through it we don't end up skipping instructions and such
		int ncins = uipc.ncins;
		int *ins_cycs = malloc(sizeof(int)*ncins), *cyc_ins = malloc(sizeof(int)*ncins);
		for(i=0; i<ncins; i++){
			ins_cycs[i] = uipc.ins_cycs[i];
			cyc_ins[i] = uipc.cyc_ins[i];
		}
	
		for(j = 0; j<ncins; j++) {
			if(ins_cycs[j] >= nc) {
				update_pc_state(cyc_ins[j], 0);
				continue;
			}
		
			if(nc < uipc.nc) {
				DeleteListItem(pc.inst[cyc_ins[j]], pc.pclevel, nc, -1);	// Delete the rest of them	
			} else {
				for(i=uipc.nc; i<nc; i++) 
					InsertListItem(pc.inst[cyc_ins[j]], pc.pclevel, -1, c[i], i);
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
	CmtGetLock(lock_uipc);
	if(uipc.c_instrs == NULL) {
		uipc.c_instrs = malloc(sizeof(PINSTR**)*uipc.max_ni);
		uipc.max_cinstrs = malloc(sizeof(int)*uipc.max_ni);
		for(i = 0; i<uipc.max_ni; i++) {
			uipc.c_instrs[i] = NULL;	// Null initialization
			uipc.max_cinstrs[i] = 0;	// -1 initialization
		}
	}
	
	// Update the uipc variable
	uipc.nc = nc;
	if(uipc.max_nc < nc) {								// We keep everything allocated as a form
		if(uipc.cyc_steps == NULL)						// of history, so there's no need to do  
			uipc.cyc_steps = malloc(sizeof(int)*nc);	// this unless you go past the highest
		else											// value we've ever assigned
			uipc.cyc_steps = realloc(uipc.cyc_steps, sizeof(int)*nc);

		for(i=uipc.max_nc; i<nc; i++) 
			uipc.cyc_steps[i] = 2;

		uipc.max_nc = nc;
	}
	CmtReleaseLock(lock_uipc);
	
	int dimmed = 0;
	if(!uipc.nc && !uipc.nd) 
		dimmed = 1;
	
	SetCtrlAttribute(pc.skip[1], pc.skip[0], ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_DIMMED, dimmed);
	
}

void change_cycle(int num) {
	// Updates controls and such when you switch from one cycle to another
	int nl, cyc;
	int panel = pc.inst[num];

	GetNumListItems(panel, pc.pclevel, &nl);
	if(nl < 1)
		return;	// Some kind of weird error.
	
	// First we'll update the uipc.
	GetCtrlVal(panel, pc.pclevel, &cyc);		// Should be 0-based index
	int i;
	CmtGetLock(lock_uipc);
	for(i = 0; i<uipc.ncins; i++) {
		if(uipc.cyc_ins[i] == num) {			// Find the place in the cyc_ins array
			uipc.ins_cycs[i] = cyc;
			break;
		}
	}
	CmtReleaseLock(lock_uipc);
	
	// Update the number of steps control.
	SetCtrlVal(panel, pc.pcsteps, uipc.cyc_steps[cyc]);
	
	// Update the ring controls
	GetNumListItems(panel, pc.pcstep, &nl);	// Need to know how many to add or remove
	int steps = uipc.cyc_steps[cyc];
	if(steps == nl)								// It's all good.
		return;
	
	// The easy one, since -1 deletes everything from steps on.
	if(steps < nl) {
		DeleteListItem(panel, pc.pcstep, steps, -1);
	}
	
	// The harder one, need to add some instructions
	if(steps > nl) {
		int elements;
		char **c = generate_char_num_array(nl+1, steps, &elements);
		for(i=0; i<elements; i++) {
			InsertListItem(panel, pc.pcstep, -1, c[i], i+nl);
			free(c[i]);
		}
		free(c);
	}
	
	if(nl == 0) {
		SetCtrlIndex(panel, pc.pcstep, 0);
		SetCtrlAttribute(panel, pc.pcstep, ATTR_DFLT_INDEX, 0);
	}
}

void change_cycle_step(int num) {
	// Feed this an instruction number and it will update uipc and
	// update the phase cycle accordingly.
	
	int to, from, nl;
	int panel = pc.inst[num];
	
	GetNumListItems(panel, pc.pcstep, &nl);
	if(nl == 0)
		return;		// Weird error.
	
	// The old value is stored as the default value so we know where it came from
	GetCtrlVal(panel, pc.pcstep, &to);
	GetCtrlAttribute(panel, pc.pcstep, ATTR_DFLT_INDEX, &from);
	
	if(from < 0)
		from = 0;
	
	// Now we need to make sure that the uipc.c_instrs array is up to date.
	int cyc = -1, i;
	for(i = 0; i<uipc.ncins; i++) {
		if(uipc.cyc_ins[i] == num) 
			cyc = uipc.ins_cycs[i];
	}
	if(cyc < 0)
		return;	// Error.
	
	// Once you commit, we allocate you an array in c_instrs. The array size
	// is conservative, so it's only initially as big as it needs to be, but
	// it's also persistent for history reasons, so it never gets any smaller
	// than the biggest it's ever been.
	
	PINSTR *instr = malloc(sizeof(PINSTR));
	get_instr(instr, num); 						// Get the current instruction
	update_cyc_instr(num, instr, from);			// Save it as the old instruction before we change it.
	free(instr);
	
	// If it's already been set, change the phase cycle instruction to what it used to be
	if(uipc.max_cinstrs[num] > to && uipc.c_instrs[num][to] != NULL)
		set_instr(num, uipc.c_instrs[num][to]);
	
	// Now it's all done, so we just set the default index to the new index.
	SetCtrlAttribute(panel, pc.pcstep, ATTR_DFLT_INDEX, to);
	
	/************TO DO***************
	TODO: I should also implement a "check box" for what phase cycles have already
	been set up. Do this later.
	********************************/
}

void change_cycle_num_steps(int cyc, int steps) {
	// Function for changing the number of steps in a phase cycle.
	int i, j, nl, cp;

	if(steps == uipc.cyc_steps[cyc])
		return;	// No change.
	
	CmtGetLock(lock_uipc);
	int oldsteps = uipc.cyc_steps[cyc];
	uipc.cyc_steps[cyc] = steps;
	CmtReleaseLock(lock_uipc);
	
	int elements;
	char **c = NULL;
	if(steps > oldsteps)
		c = generate_char_num_array(1, steps, &elements);
	
	for(i=0; i < uipc.ncins; i++) {
		if(uipc.ins_cycs[i] != cyc)
			continue;
		
		cp = pc.inst[uipc.cyc_ins[i]];
		
		if(steps < oldsteps) {
			GetNumListItems(cp, pc.pcstep, &nl);
			if(nl >= steps)
				DeleteListItem(cp, pc.pcstep, steps, -1);
		} else {
			GetNumListItems(cp, pc.pcstep, &nl);
			for(j=nl; j<steps; j++)
				InsertListItem(cp, pc.pcstep, -1, c[j], j);
		}
		
		// Now if something's expanded, we need to deal with that
		if(uipc.n_cyc_pans > i) {
			CmtGetLock(lock_uipc);
			int num = uipc.cyc_ins[i];
			if(uipc.cyc_pans[num] != NULL) {
				if(steps > oldsteps) { 	// Adding steps
					uipc.cyc_pans[num] = realloc(uipc.cyc_pans[num], sizeof(int)*steps);
					for(j = oldsteps; j < steps; j++)
						uipc.cyc_pans[num][j] = setup_expanded_instr(num, j);
				} else {							// Removing steps
					for(j = steps; j < oldsteps; j++)
						DiscardPanel(uipc.cyc_pans[num][j]);
					uipc.cyc_pans[num] = realloc(uipc.cyc_pans[num], sizeof(int)*steps);
				}
				
				resize_expanded(num, steps);
			}
			CmtReleaseLock(lock_uipc);
		}
		
		SetCtrlVal(cp, pc.pcsteps, steps);
	}
	
	if(c != NULL) {
		for(i = 0; i < elements; i++)
			free(c[i]);
		free(c);
	}
	
	// Finally, we update the minimum possible value for increasing transients.
	int nt, oiv, niv = 1;
	GetCtrlVal(pc.nt[1], pc.nt[0], &nt);
	GetCtrlAttribute(pc.nt[1], pc.nt[0], ATTR_INCR_VALUE, &oiv);

	// It's multiplicative.
	for(i = 0; i < uipc.nc; i++) 
		niv *= uipc.cyc_steps[i];
	
	// Recalculate the number of transients.
	nt = (nt/oiv)*niv;
	
	SetCtrlAttribute(pc.nt[1], pc.nt[0], ATTR_INCR_VALUE, niv);
	SetCtrlVal(pc.nt[1], pc.nt[0], nt);
}

void update_cyc_instr(int num, PINSTR *instr, int step) {
	// Updates the uipc variable for instruction number "num" at step "step"
	// with the instruction you feed it. Make sure that max_cinstrs is set
	// before you use this function. 
	int i;
   											  
	// Make this array the right size.
	CmtGetLock(lock_uipc);
	if(uipc.c_instrs[num] == NULL)
		uipc.c_instrs[num] = malloc(sizeof(PINSTR*)*(step+1));
	else if (uipc.max_cinstrs[num] <= step) 
		uipc.c_instrs[num] = realloc(uipc.c_instrs[num], sizeof(PINSTR*)*(step+1));
	
	if(step >= uipc.max_cinstrs[num]) {					// Matches exclusively both of
		for(i = uipc.max_cinstrs[num]; i<=step; i++)   	// the above conditions.
			uipc.c_instrs[num][i] = NULL;
		
		uipc.max_cinstrs[num] = step+1;
	}
	
	// If it's never been allocated, allocate it.
	if(uipc.c_instrs[num][step] == NULL)
		uipc.c_instrs[num][step] = malloc(sizeof(PINSTR));
	
	// Finally just copy the instr into the uipc array.
	copy_pinstr(instr, uipc.c_instrs[num][step]);
	CmtReleaseLock(lock_uipc);
}

void populate_cyc_points()
{	 
	// Function for updating the UI with the values from the uipc var
	int j, on, nl, cyc;
	
	// A char array of labels
	int elements;
	char **c = generate_char_num_array(1, uipc.nc, &elements); 

	int panel;
	for(int i = 0; i<uipc.ncins; i++) {
		panel = pc.inst[uipc.cyc_ins[i]];
		
		// First make the number of dimensions per control correct
		GetNumListItems(panel, pc.pclevel, &nl);
		if(nl < uipc.nc) {
			for(j=nl; j<uipc.nc; j++)
				InsertListItem(panel, pc.pclevel, -1, c[j], j);
		} else if (nl > uipc.nc) {
			DeleteListItem(panel, pc.pclevel, uipc.nc, -1);
		}
		
		// Now update the number of steps.
		cyc = uipc.ins_cycs[i];				// In case the cycle changed
		SetCtrlIndex(panel, pc.pclevel, cyc);
		SetCtrlVal(panel, pc.pcsteps, uipc.cyc_steps[cyc]);
	}
	
	for(j=0; j<elements; j++)
		free(c[j]);
	free(c);
	
	// Now update the number of cycles
	int nc;
	GetCtrlVal(pc.numcycles[1], pc.numcycles[0], &nc);
	if(uipc.nc <0) {	// Error check
		uipc.nc = 0;
	}
	
	if (nc != uipc.nc) {
		SetCtrlVal(pc.numcycles[1], pc.numcycles[0], uipc.nc);
		change_num_cycles();
	}
}

/*************** Expanded Phase Cycle Functions ****************/ 
void set_phase_cycle_expanded(int num, int state) {
	// Toggles between having a phase cycle expanded and not having it expanded.
	int panel = pc.inst[num], cyc;
	int steps, top, left, i;
	
	
	GetCtrlIndex(panel, pc.pclevel, &cyc);  
	
	GetPanelAttribute(pc.inst[num], ATTR_TOP, &top);
	GetPanelAttribute(pc.inst[num], ATTR_LEFT, &left);
	
	GetCtrlVal(panel, pc.pcsteps, &steps);
	
	if(state) {
		// Now we want to create the panels.
		CmtGetLock(lock_uipc);
		if(uipc.cyc_pans == NULL) {
			uipc.cyc_pans = malloc(sizeof(int*)*(num+1));
		} else if(uipc.n_cyc_pans <= num) {
			uipc.cyc_pans = realloc(uipc.cyc_pans, sizeof(int*)*(num+1));
		}
		
		for(i = uipc.n_cyc_pans; i <= num; i++)
			uipc.cyc_pans[i] = NULL;
		
		uipc.cyc_pans[num] = malloc(sizeof(int)*uipc.cyc_steps[cyc]);
		uipc.n_cyc_pans = num+1;
		
		// Set up the new subpanels
		uipc.cyc_pans[num][0] = pc.inst[num];
		for(i = 1; i < uipc.cyc_steps[cyc]; i++)
			uipc.cyc_pans[num][i] = setup_expanded_instr(num, i);
		CmtReleaseLock(lock_uipc);
		
		// Do the actual expansion.
		SetPanelAttribute(panel, ATTR_FRAME_STYLE, VAL_OUTLINED_FRAME);
		SetPanelAttribute(panel, ATTR_FRAME_THICKNESS, 1);
		
		resize_expanded(num, steps);
		
		// Change the callback function for the step-changer
		int info = num*1024;
		InstallCtrlCallback(panel, pc.pcstep, ChangePhaseCycleStepExpanded, (void*)info);

	} else {
		int ind = get_index(uipc.cyc_ins, num, uipc.ncins);
		if(ind < 0)
			return;
		
		SetPanelAttribute(panel, ATTR_FRAME_STYLE, VAL_HIDDEN_FRAME);
		SetPanelAttribute(panel, ATTR_FRAME_THICKNESS, 0);

		resize_expanded(num, 1);
		save_expanded_instructions(ind);
		
		for(i = uipc.cyc_steps[cyc]-1; i > 0; i--)
			DiscardPanel(uipc.cyc_pans[num][i]);
		
		CmtGetLock(lock_uipc);
		free(uipc.cyc_pans[num]); 
		uipc.cyc_pans[num] = NULL;
		CmtReleaseLock(lock_uipc);
		
		// Change back the callback function for the step-changer
		InstallCtrlCallback(panel, pc.pcstep, ChangePhaseCycleStep, NULL);
	}
	
	SetCtrlAttribute(panel, pc.expandpc, ATTR_VISIBLE, !state);
	SetCtrlAttribute(panel, pc.collapsepc, ATTR_VISIBLE, state);
}

int setup_expanded_instr(int num, int step) {
	// Creates an instruction as part of the list of expanded phase cycle instructions
	if(step >= 1024)
		return -1;
	
	int subp = LoadPanel(pc.inst[num], pc.uifname, pc.pulse_inst); // Start with the panel
	set_ttl_trigger(subp, -1, 1);
	
	// Place the panel
	SetPanelPos(subp, (INSTR_HEIGHT+INSTR_GAP)*(step), 0);
	
	// Delete controls we don't need
	DiscardCtrl(subp, pc.ins_num);
	DiscardCtrl(subp, pc.pclevel);
	DiscardCtrl(subp, pc.pcon);
	DiscardCtrl(subp, pc.pcsteps);
	DiscardCtrl(subp, pc.collapsepc);
	DiscardCtrl(subp, pc.expandpc);
	
	// Move the arrow buttons over
	SetCtrlAttribute(subp, pc.uparrow, ATTR_LEFT, 970);
	SetCtrlAttribute(subp, pc.downarrow, ATTR_LEFT, 970);
	SetCtrlAttribute(subp, pc.xbutton, ATTR_LEFT, 990);
	
	// Reprogram the callbacks
	// Callback data stores data as follows: lowest bit of move_info is whether or
	// not it's the up button. The next 10 bits encode the step (obviously this limits
	// the number of steps to 1024, no big loss). The remainder encodes the instruction 
	// number. The other stuff (info) is the same, but without the first bit set.
	int move_info = 1 + step*2 + num*2048;
	InstallCtrlCallback(subp, pc.uparrow, MoveInstButtonExpanded, (void *)move_info);
	move_info--;
	InstallCtrlCallback(subp, pc.downarrow, MoveInstButtonExpanded, (void *)move_info);
	
	int info = step+num*1024;
	InstallCtrlCallback(subp, pc.pcstep, ChangePhaseCycleStepExpanded, (void*)info);
	InstallCtrlCallback(subp, pc.xbutton, DeleteInstructionCallbackExpanded, (void*)info);
	InstallCtrlCallback(subp, pc.instr, InstrCallbackExpanded, (void*)info);
	InstallCtrlCallback(subp, pc.delay, ChangeInstDelayExpanded, (void*)info);
	InstallCtrlCallback(subp, pc.delayu, ChangeTUnitsExpanded, (void*)info);
	InstallCtrlCallback(subp, pc.instr_d, InstrDataCallbackExpanded, (void*)info); 

	// Now populate the ring control
	int elements, cyc;
	GetCtrlIndex(pc.inst[num], pc.pclevel, &cyc);
	char **c = generate_char_num_array(1, uipc.cyc_steps[cyc], &elements);
	for(int i = 0; i < uipc.cyc_steps[cyc]; i++) 
		InsertListItem(subp, pc.pcstep, -1, c[i], i);
	
	SetCtrlAttribute(subp, pc.pcstep, ATTR_DIMMED, 0);
	SetCtrlIndex(subp, pc.pcstep, step);
	
	if(c != NULL) {
		for(int i = 0; i < elements; i++)
			free(c[i]);
		free(c);
	}
	
	// Now we want to check if there's an instruction waiting for us, and otherwise
	// grab the most recent instruction and duplicate it.
	if(uipc.c_instrs != NULL && uipc.max_cinstrs[num] > step && uipc.c_instrs[num][step] != NULL) {
		set_instr_panel(subp, uipc.c_instrs[num][step]);	
	} else {
		PINSTR instr;
		get_instr_panel(&instr, uipc.cyc_pans[num][step-1]);
		set_instr_panel(subp, &instr);
	}
	
	// Show us that sweet, sweet panel.
	DisplayPanel(subp);
	
	return subp;
}

void resize_expanded(int num, int steps) {
	// Changes the size of a panel to the appropriate size and moves all the
	// other panels around to accomodate.
	
	if(steps < 1)
		return;
	
	int top;
	GetPanelAttribute(pc.inst[num], ATTR_TOP, &top);
		
	SetPanelAttribute(pc.inst[num], ATTR_HEIGHT, ((INSTR_HEIGHT+INSTR_GAP)*steps)-INSTR_GAP);
	
	for(int i = num+1; i < uipc.max_ni; i++)
		SetPanelAttribute(pc.inst[i], ATTR_TOP, top+((INSTR_HEIGHT+INSTR_GAP)*(steps+i-num-1)));
}

void save_all_expanded_instructions() {
	// Saves all currently expanded instructions to the uipc variable.
	int i, expanded;
	
	for(i = 0; i < uipc.ncins; i++) {
		GetCtrlAttribute(pc.inst[uipc.cyc_ins[i]], pc.collapsepc, ATTR_VISIBLE, &expanded);
		if(expanded)	// Save only if we need to.
			save_expanded_instructions(i);
	}
}

void save_expanded_instructions(int ind) {
	// For convenience, this saves all the instructions at index "ind" in the uipc
	// phase cycling arrays (cyc_ins, ins_cycs, etc)
	
	// Start at the end and move backwards, so you don't have to realloc every time.
	for(int i = uipc.cyc_steps[uipc.ins_cycs[ind]]-1; i >= 0; i--)
		save_expanded_instruction(uipc.cyc_ins[ind], i);
}

void save_expanded_instruction(int num, int step) {
	// Saves an expanded information to the uipc.c_instrs field.

	// Array allocation stuff
	CmtGetLock(lock_uipc);
	if(uipc.c_instrs[num] == NULL) {
		uipc.c_instrs[num] = malloc(sizeof(PINSTR*)*(step+1));
		uipc.max_cinstrs[num] = 0;
	} else if(uipc.max_cinstrs[num] < step+1) {
		uipc.c_instrs[num] = realloc(uipc.c_instrs[num], sizeof(PINSTR*)*(step+1));
	}
	
	if(uipc.max_cinstrs[num] < step+1){
		for(int i = uipc.max_cinstrs[num]; i <= step; i++)
			uipc.c_instrs[num][i] = NULL;

		uipc.max_cinstrs[num] = step+1;
	}
	
	// Now we have a proper uipc variable, just add in the instruction.
	if(uipc.c_instrs[num][step] == NULL)
		uipc.c_instrs[num][step] = malloc(sizeof(PINSTR));
	
	get_instr_panel(uipc.c_instrs[num][step], uipc.cyc_pans[num][step]);
	CmtReleaseLock(lock_uipc);
}

void move_expanded_instruction(int num, int to, int from) {
	// Moves one of the expanded instructions from one position to another.
	// Since there's nothing fancy going on here, we can just grab the instruction
	// from one spot and move it to another.
	int cyc;
	GetCtrlVal(pc.inst[num], pc.pclevel, &cyc);
	
	if(to == from || to < 0 || to >= uipc.cyc_steps[cyc])
		return;
	
	int i, diff = (int)fabs(to-from) + 1;
	PINSTR *inst_buff1 = malloc(sizeof(PINSTR)), *inst_buff2 = malloc(sizeof(PINSTR));
	
	
	// Get the from instruction
	get_instr_panel(inst_buff1, uipc.cyc_pans[num][from]);
	
	int panel;
	if(to > from) {
		for(i = from; i < to; i++) {
			get_instr_panel(inst_buff2, uipc.cyc_pans[num][i+1]);
			set_instr_panel(uipc.cyc_pans[num][i], inst_buff2);
		}
	} else {
		for(i = from; i > to; i--) {
			get_instr_panel(inst_buff2, uipc.cyc_pans[num][i-1]);
			set_instr_panel(uipc.cyc_pans[num][i], inst_buff2);
		}
	}
	
	// Move the from instruction to the to instruction
	set_instr_panel(uipc.cyc_pans[num][to], inst_buff1);
	free(inst_buff1);
	free(inst_buff2);
}

void delete_expanded_instruction(int num, int step) {
	int cyc, cn, i, j;
	GetCtrlIndex(pc.inst[num], pc.pclevel, &cyc);
	
	set_instr_panel(uipc.cyc_pans[num][step], NULL);				// Clear the instruction.

	PINSTR *in_buff;
	// First we move all the instructions we need to to the end, since
	// all the things varying on the same cycle are linked.
	for(i = 0; i < uipc.ncins; i++) {
		cn = uipc.cyc_ins[i];
		if(cn != cyc)
			continue;
	
		if(cn < uipc.n_cyc_pans && uipc.cyc_pans[cn] != NULL)
			move_expanded_instruction(num, uipc.cyc_steps[cyc]-1, step);	// Move it to the end.
		else if(uipc.max_cinstrs[num] > step) {
			// Move our instruction to the end and move everything over.
			CmtGetLock(lock_uipc);
			if(uipc.max_cinstrs[num] < uipc.cyc_steps[cyc]) {
				uipc.c_instrs[num] = realloc(uipc.c_instrs[num], sizeof(PINSTR*)*uipc.cyc_steps[cyc]);
				uipc.max_cinstrs[num] = uipc.cyc_steps[cyc];
			}
			
			in_buff = uipc.c_instrs[num][step];
			for(j =	step; j < uipc.cyc_steps[cyc]-1; i++) {
				uipc.c_instrs[num][j] = uipc.c_instrs[num][j+1];	
			}
			uipc.c_instrs[num][j] = in_buff;
			CmtReleaseLock(lock_uipc);
		}
	}
	
	// Now we just change the number of instructions in the cycle.
	change_cycle_num_steps(cyc, uipc.cyc_steps[cyc]-1);
}

/****************** Instruction Manipulation *******************/ 
int move_instruction(int to, int from)
{
	// Moves an instruction somewhere else in the list and shifts everything around
	// accordingly.
	int diff = (int)fabs(to-from) + 1, inst_buffer[diff], inst_top, inst_height[diff], cinst_buffer[diff], cinst_top[diff], i, start;
	
	if(to == from)
		return 0;
	
	if(to<from)
	{
		inst_buffer[0] = pc.inst[from];
		cinst_buffer[0] = pc.cinst[from];
		start = to;
		
		for (i = 1; i<diff; i++)
		{
			inst_buffer[i] = pc.inst[to+i-1];
			cinst_buffer[i] = pc.cinst[to+i-1];
		}
	}
	else
	{
		inst_buffer[diff-1] = pc.inst[from];
		cinst_buffer[diff-1] = pc.cinst[from];
		start = from;
														;
		for (i = 0; i<diff-1; i++)
		{
			inst_buffer[i] = pc.inst[from+i+1];
			cinst_buffer[i] = pc.cinst[from+i+1];
		}
	}
	
	// Check if it's in a loop before and after, and if there's a change, then you shoul
	int ninstructions = uipc.ni;
	int loop_locations[ninstructions][3]; // An array with all the loop locations and their corresponding end-points.
	int j = 0, ins, end_ins;
	for(i = 0; i<ninstructions; i++) {
		GetCtrlVal(pc.inst[i], pc.instr, &ins);
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
			SetCtrlVal(pc.inst[c_e], pc.instr_d, to);
		} else if (c_l > to) {
			SetCtrlVal(pc.inst[c_e], pc.instr_d, c_l+1);
		} else if (c_l > from) {
			SetCtrlVal(pc.inst[c_e], pc.instr_d, c_l-1);
		}

	}
	
	// Update the uipc var.
	int end = start+diff;

	// ND Instructions
	CmtGetLock(lock_uipc);
	for(i = 0; i<uipc.ndins; i++) {
		ins = uipc.dim_ins[i];
		if(ins < start || ins > end)
			continue;
		
		if(ins == from)
			uipc.dim_ins[i] = to;
		else if (to > from)
			uipc.dim_ins[i]++;
		else if (from < to)
			uipc.dim_ins[i]--;
	}
	
	// Phase cycled instructions
	for(i = 0; i<uipc.ncins; i++) {
		ins = uipc.cyc_ins[i];
		if(ins < start || ins > end)
			continue;
		
		if(ins == from)
			uipc.cyc_ins[i] = to;
		else if (to > from)
			uipc.cyc_ins[i]--;
		else if (to < from)
			uipc.cyc_ins[i]++;
	}
	CmtReleaseLock(lock_uipc);

	// Now the actual moving and updating of things.
	GetPanelAttribute(pc.inst[start], ATTR_TOP, &inst_top);
	
	for (i = 0; i<diff; i++)
	{
		GetPanelAttribute(inst_buffer[i], ATTR_HEIGHT, &inst_height[i]);
		GetPanelAttribute(pc.cinst[start+i], ATTR_TOP, &cinst_top[i]);
	}
	
	for(i = 0; i<diff; i++)
	{ 
		pc.inst[start+i] = inst_buffer[i];
		pc.cinst[start+i] = cinst_buffer[i];
		
		SetPanelAttribute(pc.inst[start+i], ATTR_TOP, inst_top);
		SetPanelAttribute(pc.cinst[start+i], ATTR_TOP, cinst_top[i]);
		SetCtrlVal(pc.inst[start+i], pc.ins_num, start+i);
		SetCtrlVal(pc.cinst[start+i], pc.cins_num, start+i);
		
		// Update the top.
		inst_top += inst_height[i]+5;
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
			//	update_instr_data_nd(pc.cinst[c_l], 2); // This will update the times.
			}
		}
	}
	
	return 0; 
}

void clear_instruction(int num) {
	// Takes an instruction and restores it to defaults.
	
	set_instr(num, NULL);		// This sets everything to defaults
	
	// If this was a phase cycled instruction, this is the only time we want
	// to clear that, so we're going to free up the memory and such.
	if(get_pc_state(num))
		update_pc_state(num, 0);
	
	CmtGetLock(lock_uipc);
	if(uipc.max_cinstrs != NULL)
	{
		if(uipc.c_instrs[num] != NULL) {
			for(int i = 0; i<uipc.max_cinstrs[num]; i++) {
				if(uipc.c_instrs[num][i] != NULL) {
					free(uipc.c_instrs[num][i]);
				}
			}
			free(uipc.c_instrs[num]);
			uipc.c_instrs[num] = NULL;
		}
		uipc.max_cinstrs[num] = 0;
	}
	CmtReleaseLock(lock_uipc);
	
	SetCtrlVal(pc.inst[num], pc.pcsteps, 2);
	
	// We also want to clear stored ND information.
	if(get_nd_state(num))
		update_nd_state(num, 0);
	
	// Clear the expression controls
	char *def_val;
	int def_len;

	// Reset to default values.
	GetCtrlAttribute(pc.cinst[num], pc.cexpr_data, ATTR_DFLT_VALUE_LENGTH, &def_len);
	def_val = malloc(def_len+1);
	
	GetCtrlAttribute(pc.cinst[num], pc.cexpr_data, ATTR_DFLT_VALUE, def_val);
	SetCtrlVal(pc.cinst[num], pc.cexpr_data, def_val);
	free(def_val);
	
	GetCtrlAttribute(pc.cinst[num], pc.cexpr_delay, ATTR_DFLT_VALUE_LENGTH, &def_len);
	def_val = malloc(def_len+1);
	
	GetCtrlAttribute(pc.cinst[num], pc.cexpr_delay, ATTR_DFLT_VALUE, def_val);
	SetCtrlVal(pc.cinst[num], pc.cexpr_delay, def_val);
	free(def_val);
	
	// Now reset the colors and boldness.
	SetCtrlAttribute(pc.cinst[num], pc.cexpr_delay, ATTR_TEXT_BGCOLOR, VAL_WHITE);
	SetCtrlAttribute(pc.cinst[num], pc.cexpr_data, ATTR_TEXT_BGCOLOR, VAL_WHITE);
	
	SetCtrlAttribute(pc.cinst[num], pc.cexpr_delay, ATTR_TEXT_COLOR, VAL_BLACK);
	SetCtrlAttribute(pc.cinst[num], pc.cexpr_data, ATTR_TEXT_COLOR, VAL_BLACK);
	
	SetCtrlAttribute(pc.cinst[num], pc.cexpr_delay, ATTR_TEXT_BOLD, 0);
	SetCtrlAttribute(pc.cinst[num], pc.cexpr_data, ATTR_TEXT_BOLD, 0);
	
	// Clear out the ring controls
	int nl;
	GetNumListItems(pc.inst[num], pc.pcstep, &nl);
	if(nl)							   
		DeleteListItem(pc.inst[num], pc.pcstep, 0, -1);
	
	GetNumListItems(pc.inst[num], pc.pclevel, &nl);
	if(nl)							   
		DeleteListItem(pc.inst[num], pc.pclevel, 0, -1);
	
	GetNumListItems(pc.cinst[num], pc.dim, &nl);
	if(nl)							   
		DeleteListItem(pc.cinst[num], pc.dim, 0, -1);
}

void change_number_of_instructions() {
	// Gets "num" from pc.ninst and changes the number of instructions. If num < uipc.ni, t
	// the instructions at the end are hidden. if uipc.max_ni > num > uipc > uipc.ni, the 
	// old instructions are made visible again. If num > uipc.max_ni, a new instruction is 
	// created at the end and max_ni is incremented.
	
	int num, i;
	GetCtrlVal(pc.ninst[1], pc.ninst[0], &num);
	
	if(num == uipc.ni)
		return;			// No change needed
	
	if(num<uipc.ni) {						// In this case, we just hide the panels
		for(i = num; i<uipc.max_ni; i++) {
			HidePanel(pc.inst[i]);
			HidePanel(pc.cinst[i]);
		}
		CmtGetLock(lock_uipc);
		uipc.ni = num;
		CmtReleaseLock(lock_uipc);
		return;
	}
	
	if(num>uipc.max_ni) {		// The only really important part is to make new panels if there aren't any left		
		int top, left, height, ctop, cleft, cheight; 	// Getting the GUI values if we need them. 
		GetPanelAttribute(pc.inst[uipc.max_ni-1], ATTR_TOP, &top);	     		// Need these vals for both the
		GetPanelAttribute(pc.cinst[uipc.max_ni-1], ATTR_TOP, &ctop); 		 	// ND instrs and the pulse ones
		GetPanelAttribute(pc.inst[uipc.max_ni-1], ATTR_LEFT, &left);		 
		GetPanelAttribute(pc.cinst[uipc.max_ni-1], ATTR_LEFT, &cleft);
		GetPanelAttribute(pc.inst[uipc.max_ni-1], ATTR_HEIGHT, &height);
		GetPanelAttribute(pc.cinst[uipc.max_ni-1], ATTR_HEIGHT, &cheight);

		pc.inst = realloc(pc.inst, sizeof(int)*num);
		pc.cinst = realloc(pc.cinst, sizeof(int)*num);
		for(i=uipc.max_ni; i<num; i++) {
			pc.inst[i] = LoadPanel(pc.PProgCPan, pc.uifname, pc.pulse_inst);	// Make a new instruction
			SetPanelPos(pc.inst[i], top+=height+5, left);		// Place it and increment "top"
			SetCtrlAttribute(pc.inst[i], pc.xbutton, ATTR_DISABLE_PANEL_THEME, 1);
			
			// Set the precision initially.
			double del;
			GetCtrlVal(pc.inst[i], pc.delay, &del);
			SetCtrlAttribute(pc.inst[i], pc.delay, ATTR_PRECISION, get_precision(del, MCUI_DEL_PREC));
			
			pc.cinst[i] = LoadPanel(pc.PPConfigCPan, pc.uifname, pc.md_inst);	// Make a new ND instr
			SetPanelPos(pc.cinst[i], ctop+=cheight+5, cleft);
		
			GetCtrlVal(pc.cinst[i], pc.del_init, &del);
			SetCtrlAttribute(pc.cinst[i], pc.del_init, ATTR_PRECISION, get_precision(del, MCUI_DEL_PREC));
			
			GetCtrlVal(pc.cinst[i], pc.del_inc, &del);
			SetCtrlAttribute(pc.cinst[i], pc.del_inc, ATTR_PRECISION, get_precision(del, MCUI_DEL_PREC));
	
			GetCtrlVal(pc.cinst[i], pc.del_fin, &del);
			SetCtrlAttribute(pc.cinst[i], pc.del_fin, ATTR_PRECISION, get_precision(del, MCUI_DEL_PREC));
			
			// Update the instruction numbers
			SetCtrlVal(pc.inst[i], pc.ins_num, i);
			SetCtrlVal(pc.cinst[i], pc.cins_num, i);
			
			// Set up the trigger ttl.
			set_ttl_trigger(pc.inst[i], -1, 1);
		}

		CmtGetLock(lock_uipc);
		if(uipc.c_instrs != NULL) {
			uipc.c_instrs = realloc(uipc.c_instrs, sizeof(PINSTR**)*num);
			uipc.max_cinstrs = realloc(uipc.max_cinstrs, sizeof(int)*num);
			for(i=uipc.max_ni; i<num; i++) {
				uipc.c_instrs[i] = NULL;
				uipc.max_cinstrs[i] = 0;
			}
		}

		uipc.max_ni = num;		// We've increased the max number of instructions, so increment it.
		CmtReleaseLock(lock_uipc);
		setup_broken_ttls();
	}
	
	int ndon;
	GetCtrlVal(pc.ndon[1], pc.ndon[0], &ndon);
	
	for(i=uipc.ni; i<num; i++) {	// Now we can show the remaining panels.
		DisplayPanel(pc.inst[i]);
		DisplayPanel(pc.cinst[i]);
		SetPanelAttribute(pc.cinst[i], ATTR_DIMMED, !ndon);
	}
	
	CmtGetLock(lock_uipc);
	uipc.ni = num;	// And update the uipc value.
	CmtReleaseLock(lock_uipc);
}

void delete_instruction(int num) {
	// Deletes an instruction by clearing the value, moving it to the end and
	// reducing the number of instructions available.
	
	clear_instruction(num);

	if(uipc.ni == 1)
		return;			// Can't have 0 instructions
	
	move_instruction(uipc.max_ni-1, num);
	SetCtrlVal(pc.ninst[1], pc.ninst[0], uipc.ni-1);
	change_number_of_instructions();
}

/****************** Analog Output Manipulation *******************/ 
void change_num_aouts() {
	// Gets "num" from pc.anum and changes the number of analog output channels.
	// Same behavior as change_number_of_instructions.
	unsigned short int num, i;
	GetCtrlVal(pc.anum[1], pc.anum[0], &num);
	
	if(num == uipc.anum)
		return;		// We're done.
	
	SetPanelAttribute(pc.ainst[0], ATTR_DIMMED, (num == 0)?1:0); 
	
	if(num < uipc.anum) {
		//Hide the panels.
		for(i = num+((num==0)?1:0); i<uipc.max_anum; i++) {
			HidePanel(pc.ainst[i]);
		}
		
		CmtGetLock(lock_uipc);
		uipc.anum = num;
		CmtReleaseLock(lock_uipc);
		return;
	}
	
	// Make some more panels if necessary.
	if(num > uipc.max_anum) {
		int top, left, height, mi = uipc.max_anum-1;
		GetPanelAttribute(pc.ainst[mi], ATTR_TOP, &top);
		GetPanelAttribute(pc.ainst[mi], ATTR_LEFT, &left);
		GetPanelAttribute(pc.ainst[mi], ATTR_HEIGHT, &height);
		
		pc.ainst = realloc(pc.ainst, sizeof(int)*num);
		
		for(i=uipc.max_anum; i<num; i++) {
			pc.ainst[i] = LoadPanel(pc.AOutCPan, pc.uifname, pc.a_inst);
			SetPanelPos(pc.ainst[i], top+=height+5, left);
		}
		
		CmtGetLock(lock_uipc);
		uipc.max_anum = num;
		CmtReleaseLock(lock_uipc);
	}
	
	
	for(i = uipc.anum; i<num; i++) {
		DisplayPanel(pc.ainst[i]);	
	}

	// Finally update uipc for later.
	CmtGetLock(lock_uipc);
	uipc.anum = num;
	CmtReleaseLock(lock_uipc);
	
}

void delete_aout(int num) {
	clear_aout(num);		// Clear the values.
	
	// Move it to the end.
	int instr = pc.ainst[num];
	int top, btop;
	GetPanelAttribute(pc.ainst[num], ATTR_TOP, &top);
	for(int i = num; i < uipc.max_anum-1; i++) {
		GetPanelAttribute(pc.ainst[i+1], ATTR_TOP, &btop);	// Move it.
		SetPanelAttribute(pc.ainst[i+1], ATTR_TOP, top);
		top = btop;
		
		pc.ainst[i] = pc.ainst[i+1];
	}
	
	SetPanelAttribute(instr, ATTR_TOP, top);
	pc.ainst[uipc.max_anum-1]= instr;
	
	SetCtrlVal(pc.anum[1], pc.anum[0], uipc.anum-1);
	change_num_aouts();
}

void clear_aout(int num) {
																		   
	// Set values to 0.
	SetCtrlVal(pc.ainst[num], pc.ainitval, 0.0);
	SetCtrlVal(pc.ainst[num], pc.aincval, 0.0);
	SetCtrlVal(pc.ainst[num], pc.afinval, 0.0);
	SetCtrlVal(pc.ainst[num], pc.andon, 0);	// Don't forget to actually toggle this later.
	
	// Clear the expression control
	SetCtrlVal(pc.ainst[num], pc.aincexpr, "");
	SetCtrlAttribute(pc.ainst[num], pc.aincexpr, ATTR_VISIBLE, 0);
	SetCtrlAttribute(pc.ainst[num], pc.aincexpr, ATTR_TEXT_BGCOLOR, VAL_WHITE);
	
	// Set channel, dev to default value.
	int	val, nchans;
	
	GetNumListItems(pc.ainst[num], pc.aodev, &nchans);
	if(nchans > 0) {
		GetCtrlAttribute(pc.ainst[num], pc.aodev, ATTR_DFLT_VALUE, &val);
		SetCtrlVal(pc.ainst[num], pc.aodev, val);
	}
	
	GetNumListItems(pc.ainst[num], pc.aochan, &nchans);
	if(nchans) {
		GetCtrlAttribute(pc.ainst[num], pc.aochan, ATTR_DFLT_VALUE, &val);  
		SetCtrlVal(pc.ainst[num], pc.aochan, val);
	}
	
}


/************** User Defined Function Interfaces ***************/ 
pfunc *get_pfunc(int func_index) {
	// Gets the user-defined function at index func_index-9;
	
	
	return NULL;
}

int get_reserved_flags(int func_index) {
	// Pass this the index of the function in the list and it returns the list of flags that are reserved.
	
	func_index -= 9;		// Not the atomic functions obviously
	if(func_index < 0)
		return 0;			// If it's an atomic function, nothing's reserved.
	
	return 0;
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
			return 1;
		case LONG_DELAY:
			return 2;
			break;
	}
	
	// TODO: Update this for user-defined functions.
	return 0;
}

/********************** Math Evaluation ************************/
int get_update_error(int err_code, char *err_message) {
	// Retrieves the error message for a given error code passed from
	// update_nd_from_exprs. Pass NULL to err_message to retrieve the
	// length (including null) of the string.
	
	// Pass -1 to err_code to get the highest index err code.
	
	int max_parse_error = get_parse_error(-1, NULL);
	
	if(err_code < 0)
		return max_parse_error+1;
	
	if(err_code <= max_parse_error) 
		return get_parse_error(err_code, err_message);
	
	char *err_buff;
	switch(err_code-max_parse_error) {
		case 1:
			err_buff = "Negative value found.";
			break;
		default:
			err_buff = "No error";
			break;
	}
	
	if(err_message != NULL)
		strcpy(err_message, err_buff);
	
	return strlen(err_buff)+1;
}

constants *setup_constants() {				// Setup all the static constants in the experiment
	constants *c = malloc_constants();		// First create the constants variable
		
	// We'll initialize the constants now.
	// Eventually we'll read these from a preferences file, but for now set them in stone
	add_constant(c, "nd", C_INT, &uipc.nd);				// Number of dimensions
	add_constant(c, "nc", C_INT, &uipc.nc);				// Number of ccles.
		
	// Now the steps per dimension and the steps per cycle
	char *current_var = malloc(5);
	int i;
	
	for(i=0;i<uipc.nc; i++) {
		sprintf(current_var, "mcs%d", i);				// Maximum cycle step for cycle i
		add_constant(c, current_var, C_INT, &uipc.cyc_steps[i]);
	}
		
	for(i=0;i<uipc.nd; i++){
		sprintf(current_var, "mds%d", i);				// Max dim step for dimension i
		add_constant(c, current_var, C_INT, &uipc.dim_steps[i]);
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
	// cstep must be a position in acqusition space of size uipc.nd+uipc.nc
	char *current_var = malloc(5);
	int i;
	
	// change_constant redirects to add_constant if the constant is not found.
	for(i=0;i<uipc.nc; i++) {
		sprintf(current_var, "ccs%d", i);					// Current cycle step for cycle i
		change_constant(c, current_var, C_INT, &cstep[i]);
	}
	
	for(i=0;i<uipc.nd; i++) {
		sprintf(current_var, "cds%d", i);					// Current dim step for dimension i
		change_constant(c, current_var, C_INT, &cstep[i+uipc.nc]);
	}
	
	free(current_var);
}

//////////////////////////////////////////////////////////////
// 															//
//					 Struct Manipulation					//
// 															//
//////////////////////////////////////////////////////////////

void create_pprogram(PPROGRAM *p) { // Initially allocates the PPROGRAM space
	// Give us a pre-malloc'ed PPROGRAM for this function
	// It needs the following fields already assigned:
	// 	p->n_inst		p->max_n_steps	p->nUniqueInstrs
	//	p->varied		p->nFuncs
	//	p->skip			p->nDims
	//	p->nVaried	   	p->nCycles
	
	if(p->varied) {
		p->maxsteps = malloc(sizeof(int)*(p->nDims+p->nCycles)); 				// Allocate the maximum position point
		p->v_ins = malloc(sizeof(int)*p->nVaried); 			// Allocate the array of varied instruction locations.
		p->v_ins_dim = malloc(sizeof(int)*p->nVaried);		// Allocate the array of varied dimension per instr
		p->v_ins_mode = malloc(sizeof(int)*p->nVaried);		// Allocate the array of variation modes
		
		p->delay_exprs = malloc(sizeof(char*)*p->nVaried);	// Allocate array of delay expression strings
		p->data_exprs = malloc(sizeof(char*)*p->nVaried);	// Allocate array of data expression strings
		
		for(int i = 0; i < p->nVaried; i++) {						// Initialize these to null-terminated strings.
			p->delay_exprs[i] = malloc(1);
			p->data_exprs[i] = malloc(1);
			
			p->delay_exprs[i][0] = '\0';
			p->data_exprs[i][0] = '\0';
		}
			
		
		if(p->skip) { 
			p->skip_locs = malloc(sizeof(int)*p->max_n_steps);
			p->skip_expr = malloc(1);						// Allocate the expression
			p->skip_expr[0] = '\0';							// Make it a null-terminated string
		} else {
			p->skip_locs = NULL;
			p->skip_expr = NULL;
		}
	} else {
		p->maxsteps = NULL;
		p->v_ins = NULL;
		p->v_ins_dim = NULL;
		p->v_ins_mode = NULL;
		
		p->delay_exprs = NULL;
		p->data_exprs = NULL;
	}
	
	if(p->nFuncs)
		p->func_locs = malloc(sizeof(int)*p->nFuncs);			// Allocate the array of function indexes

	// Allocate the arrays of pointers.
	p->v_ins_locs = malloc(sizeof(int**)*p->nVaried); 		// Array of pointers to arrays of ints.
	p->funcs = malloc(sizeof(pfunc*)*p->nFuncs);			// Array of pointers to functions
	p->instrs = malloc(sizeof(PINSTR*)*p->nUniqueInstrs); 	// Array of pointers to instructions
	
	// Allocate the instruction array and function arrays
	int i;
	for(i = 0; i < p->nUniqueInstrs; i++)
		p->instrs[i] = malloc(sizeof(PINSTR));
	for(i = 0; i < p->nVaried; i++)
		p->v_ins_locs[i] = malloc(sizeof(int)*p->max_n_steps);
	for(i = 0; i < p->nFuncs; i++) 
		p->funcs[i]	= malloc(sizeof(pfunc));
}

void free_pprog(PPROGRAM *p) {
	// The main thing to do here is to free all the arrays and the arrays of arrays.
	if(p == NULL)
		return;
	
	if(p->varied) {
		free(p->maxsteps);
		free(p->v_ins);
		free(p->v_ins_dim);
		free(p->v_ins_mode);

		if(p->skip) {
			free(p->skip_locs);
			free(p->skip_expr);
		}
	}
	// Free all the dynamically allocated pointers in these arrays of pointers
	for(int i = 0; i<p->nFuncs; i++)			// The condition will never be met if nFuncs == 0 
		free(p->funcs[i]);

	for(int i = 0; i<p->nUniqueInstrs; i++)
		free(p->instrs[i]);

	if(p->varied) {
		for(int i = 0; i<p->nVaried; i++) {
			free(p->v_ins_locs[i]);
			free(p->data_exprs[i]);
			free(p->delay_exprs[i]);
		}
	}

	// Free the arrays of pointers.
	if(p->varied) {
		free(p->v_ins_locs);
		free(p->data_exprs);
		free(p->delay_exprs);
	}
	if(p->nFuncs) {
		free(p->funcs);
		free(p->func_locs);
	}
	free(p->instrs);
	
	free(p);
}

int pinstr_cmp(PINSTR *pi1, PINSTR *pi2) {
	// By analogy to strcmp, this tells you if two pinstrs are the same
	// However, this returns 0 if they are not the same and 1 otherwise.
	
	// Can't be NULL, so that has to be the first check
	if(pi1 == NULL || pi2 == NULL) {
		if(pi1 == NULL && pi2 == NULL)
			return 1;
		else
			return 0;
	}
	
	// Now go through and compare each field, return on the first difference
	if(pi1->flags != pi2->flags)
		return 0;
	
	if(pi1->instr != pi2->instr)
		return 0;
	
	if(pi1->instr_data != pi2->instr_data)
		return 0;
	
	if(pi1->trigger_scan != pi2->trigger_scan)
		return 0;
	
	if(pi1->instr_time != pi2->instr_time)
		 return 0;
	
	if(pi1->time_units != pi2->time_units)
		return 0;
	
	return 1;
}

PINSTR *copy_pinstr(PINSTR *instr_in, PINSTR *instr_out) {
	// Function which copies a PINSTR. If NULL is passed to instr_out, a new PINSTR is
	// dynamically allocated (and must be freed if not in use!), this will be returned.
	
	if(instr_out == NULL) {
		instr_out = malloc(sizeof(PINSTR));		
	}
	
	if(instr_in == NULL)
		return NULL;
	
	// Copy over element by element
	instr_out->flags = instr_in->flags;
	instr_out->instr = instr_in->instr;
	instr_out->instr_data = instr_in->instr_data;
	instr_out->trigger_scan = instr_in->trigger_scan;
	instr_out->instr_time = instr_in->instr_time;
	instr_out->time_units = instr_in->time_units;
	
	return instr_out;
}

PINSTR *generate_instructions(PPROGRAM *p, int *cstep, int *n_inst) {
	// Generates the list of instructions that you want for the given cstep.
	// Return value is a list of instructions. Outputs the new number of instructions
	// to n_inst, which need not be set to the right initial value. The return value
	// is dynamically allocated, and must be freed.
	
	if(p == NULL)
		return NULL;
	
	int lindex = get_lindex(cstep, p->maxsteps, p->nCycles+p->nDims);
	int ni = p->n_inst;
	
	PINSTR *ilist = malloc(sizeof(PINSTR)*ni), *inst;
	int i, j = 0, k = 0;
	
	// The instructions in p->v_ins are sorted low to high, so we can iterate
	// through p->instrs 0->p->n_inst and check one by one. First we generate the
	// full list of instructions. We'll go through and clean up later.
	for(i = 0; i < ni; i++) {
		// If it's a varied instruction, replace it with the appropriate one.
		if(j < p->nVaried && p->v_ins[j] == i && lindex >= 0) {
			inst = p->instrs[p->v_ins_locs[j++][lindex]];
		} else {
			inst = p->instrs[i];
		}
		
	 	copy_pinstr(inst, &ilist[i]);	// Copy over the relevant instruction
	}
	
	// Now there may be some instructions here that shouldn't be sent to the board
	int removed, num;
	for(i = 0; i < ni; i++) {
		// Conditions for removal.
		if((ilist[i].instr_time < 100.0) || ilist[i].instr_data < instr_data_min(ilist[i].instr)) {
			if(ilist[i].instr == LOOP) {
				num = find_end_loop_instrs(i, ilist, ni);
				if(num <= i)
					goto error;
				
				removed = (num-i)+1;
			} else {
				num = i;
				removed = 1;
			}
			
			// Moves everything over one.
			for(j = num+1; j < ni; j++) {
				copy_pinstr(&ilist[j], &ilist[i+j-num-1]);
				
				// Fix instructions which point to other places in the program.
				if((ilist[j].instr == END_LOOP 
					|| ilist[j].instr == BRANCH
					|| ilist[j].instr == JSR) 
					&& ilist[j].instr_data >= i) { ilist[i+j-num-1].instr_data -= removed; }
			}
		
			ni -= removed;
		}
	}
	
	n_inst[0] = ni;
	
	return ilist;
	
	error:
	free(ilist);
	
	return NULL;
}


//////////////////////////////////////////////////////////////
// 															//
//					General Utilities						//
// 															//
//////////////////////////////////////////////////////////////

/*********************** Linear Indexing ***********************/ 

int get_maxsteps(int *maxsteps) {
	// Retrieves the maxsteps array as well as max_n_steps. Return value 
	// is max_n_steps. maxsteps has size uipc.nc+uipc.nd;
	int i, max_n_steps = 1;
	for(i=0; i<uipc.nc; i++) {
		max_n_steps *= uipc.cyc_steps[i];
		maxsteps[i] = uipc.cyc_steps[i];
	}
	
	for(i = 0; i<uipc.nd; i++) {
		max_n_steps *= uipc.dim_steps[i];
		maxsteps[i+uipc.nc] = uipc.dim_steps[i];
	}
	
	return max_n_steps;
}

int get_lindex(int *cstep, int *maxsteps, int size)
{
	// Give this a position in space maxsteps, which is an array of size "size"
	// It returns the linear index of cstep
	// We'll assume you can figure out whether or not this is the end of the array
	
	if(cstep == NULL)
		return -3;			// Not varied.
	
	if(size == 0)
		return -2;			// Size of array is 0.
	
	int lindex = cstep[0];	// Start with the least signficant bit.
	int place = 1;
	for(int i=1; i<size; i++) {
		place*=maxsteps[i-1];		// This is the "place" place. In decimal, you have [10 10 10] = 1s, 10s, 100s.
		lindex+=cstep[i]*place;		// The same thing goes here. [3 4 2] = 1s place, 3s places, 12s place.
	}
	
	return lindex;
}

int get_cstep(int lindex, int *cstep, int *maxsteps, int size) {
	// Give this a linear index value and it will return an array of size "size"
	// containing the place in sampling space maxsteps that the lienar index points to
	// cstep must already be allocated.
	if(size-- == 0)
		return -2; 				// Size cannot be 0.
	
	int i, place = 1;
	for(i = 0; i<size; i++)
		place*=maxsteps[i];		
	
	if(place == 0 || lindex < 0)
		return -2; 				// Size is 0 or other issue

	if(lindex == maxsteps[size]*place)
		return -1;				// End of array condition met
	
	int remainder = lindex, step;
	for(i=size; i>= 0; --i) {		// Start at the end and work backwards
		step = remainder;	// Initialize step to whatever's left. When we first get started, it's everything
		step /= place;		// Divide by the place. This number is by definition bigger than the product 
		cstep[i] = step;	// of all the numbers before it, so rounding down gets the most signficant digit

		remainder -= place*step;	// Get the remainder for the next iteration
		if(i>0)
			place /= maxsteps[i-1];	// Update the place
	}
	
	return 1;	// Successful.
}

/********************** Loop Manipulation **********************/ 

int find_end_loop (int instr) {
	// Feed this an instruction which is a loop, it finds the corresponding END_LOOP instruction.
	// If no such instruction is found, returns -1;
	
	int ins;
	GetCtrlVal(pc.inst[instr], pc.instr, &ins);
	
	if(ins != LOOP) {
		return -1;
	}
	
	// Search for the END_LOOP, check if it matches. Simple.
	int e_i_d, ni;
	GetCtrlVal(pc.ninst[1], pc.ninst[0], &ni);
	for(int i = instr+1; i<ni; i++)
	{
		GetCtrlVal(pc.inst[i], pc.instr, &ins);
		if(ins == END_LOOP)
		{
			GetCtrlVal(pc.inst[i], pc.instr_d, &e_i_d);
			if(e_i_d == instr)
			{
				return i;
			}
		}
	}
	
	return -1;
}

int find_end_loop_instrs(int instr, PINSTR *instrs, int n_instrs) {
	// Same as find_end_loop, but it works from a list of PINSTRs
	// rather than from the UI controls. Returns negative on failure.
	
	if(instr >= n_instrs)
		return -1;
	
	for(int i = instr+1; i < n_instrs; i++) {
		if(instrs[i].instr == END_LOOP && instrs[i].instr_data == instr)
			return i;
	}
	
	return -2;
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

/******************** Control Manipulation *********************/ 

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

/********************* Array Manipulation **********************/

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

int constant_array_double(double *array, int size) {
	// Give this an array of doubles and the size of the array
	// and it checks to see if the array is the same everywhere.
	
	for(int i=1; i<size; i++) {
		if(array[i] != array[0])	// If it's not the same as the first one
			return 0;				// it's not the same everywhere.
	}

	return 1;
}

int constant_array_int(int *array, int size) {
	// Give this an array of ints and the size of the array
	// and it checks to see if the array is the same everywhere.
	
	for(int i=1; i<size; i++) {
		if(array[i] != array[0])	// If it's not the same as the first one
			return 0;				// it's not the same everywhere.
	}

	return 1;
}

int constant_array_pinstr(PINSTR **array, int size) {
	// Give this an array of PINSTRs and the size of the array
	// and it checks to see if the array is the same everywhere.
	
	for(int i=1; i<size; i++) {
		if(!pinstr_cmp(array[i], array[0]))	// If it's not the same as the first one
			return 0;						// it's not the same everywhere.
	}

	return 1;
}

int get_index(int *array, int val, int size) {
	// Returns the index where "val" occurs in array. If it's not there, 
	// return -1. Size should be the number of elements in the array.
	
	int i;
	
	for(i = 0; i < size; i++) {
		if(array[i] == val)
			break;
	}
	
	if(i >= size)
		return -1;
	else
		return i;
}

int realloc_if_needed(char *array, int len, int new_len, int inc) {
	// Reallocates "array" in chunks of size "inc" such that it is
	// greater than or equal to new_len. Return value is the new
	// length of the array. No change if new_len < len;
	
	if(new_len <= len)
		return len;
	
	len += ((int)((new_len-len)/inc)+1)*inc;
	array = realloc(array, len);
	return len;
}

/*************************** Sorting ***************************/ 

void sort_linked(int **array, int num_arrays, int size) {
	// Feed this an array of arrays of ints and it sorts by the first array.
	// Uses insertion sort, which is efficient for small data sets.
	// Sorts so that the first element is the smallest.
	// Pass an array of size size*sizeof(int) to *index if you'd like the index
	// from the sort, otherwise pass NULL.

	int i, j, k;
	
	int *buff = malloc(sizeof(int)*num_arrays); 	// Buffer array for the integers we're moving around.
	
	// The insertion sort.
	for(i=0; i<size; i++) {
		for(j=0; j<num_arrays; j++)					// Move the element into the buffer
			buff[j] = array[j][i];
		
		for(j=i; j>0; j--) {
			if(buff[0] >= array[0][j-1])
				break;
			else {
				for(k=0; k<num_arrays; k++) 
					array[k][j] = array[k][j-1];	// Everything moves over one
			}
		}
		
		// Put the element into the array now that it's in its proper spot
		for(k=0; k<num_arrays; k++) {
			array[k][j] = buff[k];
		}
	}
}

/********************* String Manipulation *********************/

char **generate_char_num_array(int first, int last, int *elems) {
	// Generates an array of strings that are just numbers, starting with
	// first, ending with last.
	// *c needs to be freed. *elems is the number of items in
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

char *generate_expression_error_message(char *err_message, int *pos, int size) { 
	// Pass this an error message and a cstep and it makes a nice little message
	// that displays the position of the error and the reason for it.
	// The output of this is dynamically allocated, so you need to free it.
	
	// Error checking
	if(err_message == NULL)
		return NULL;
	
	// Need to do some stuff to allocate the size of the message
	int max_num = 0;
	for(int i=0; i<size; i++) {
		if(pos[i] > max_num)
			max_num = pos[i];
	}
	
	int num_char_len = ((int)log10(max_num)+1)*size;	// Characters to allocate per number.
	int commas = 2*(size-1);				// How many commas to generate.
	char *text;
	
	if(size == 1)
		text = "\n\nGenerated at step ";
	else 
		text = "\n\nGenerated at point [";
	
	int total_len = num_char_len+commas+strlen(text)+strlen(err_message)+2;	// One for the null, one for either ] or .
	
	// Build the full output string.
	char *output = malloc(total_len);
	sprintf(output, "%s%s%d", err_message, text, pos[0]);
	
	for(int i=1; i<size; i++)
		sprintf(output, "%s%s%d", output, ", ", pos[i]);
	
	if(size == 1)
		strcat(output, "]");
	else
		strcat(output, ".");
	
	return output;
	
}

/**************************** Math *****************************/
int calculate_units(double val) {
	// Returns the correct units.
	if(val == 0)
		return 0;
	
	int a = (int)((log10(fabs(val)))/3);
	if(a < 0)
		return 0;
	if(a > 3)
		return 3;
	else
		return a;
}

int get_precision (double val, int total_num) {
	// Feed this a value, then tell you how many numbers you want to display total.
	// This will give you the number of numbers to pad out after the decimal place.
	// If the number to be displayed has more digits after the decimal point than 
	// total_num, it returns 0.

	if(--total_num <= 1)  	// Always need one thing before the decimal place.
		return 0;
	
	if(val == 0)
		return total_num;

	int a = log10(fabs(val)); // The number of digits is the base-10 log, plus 1 - since we subtracted off the 1 from total_num, we're good..
	
	if(a >= total_num) // If we're already at capacity, we need nothing after the 0.
		return 0;
	
	return total_num-a; // Subtract off the number being used by the value, that's your answer.
}

int get_bits(int in, int start, int end) {
	// Get the bits in the span from start to end in integer in. Zero-based index.
	
	if(start > end || start < 0)
		return -1;
	
	if(end > 31)
		return -2; // Maxint
	
	return get_bits_in_place(in, start, end)>>start;
}

int get_bits_in_place(int in, int start, int end) {
	// Gets the bits you want and preserves them in place.
	if(start > end || start < 0)
		return -1;
	
	if(end > 31)
		return -2; // Maxint

	return (in & (1<<end+1)-(1<<start));
}

int move_bit_skip(int in, int skip, int to, int from) {
	// Same as move_bit, but this also skips anything high in "flags"
	if(((1<<to)|(1<<from)) & skip)
		return -1; // Error.
	
	int bits = move_bit(in, to, from); // Start off by not skipping anything.
	int i, toh = to>from, inc = toh?-1:1;

	// Now we go through bit by bit and swap each broken bit into its original
	// place. If to>from, that involves swapping with the bit 1 size bigger, 
	// if from > to, you swap with the bit 1 size smaller.
	for(i = to; toh?(i > from):(i < from); i+=inc) {
		if((1<<i) & skip)
			bits = move_bit(bits, i, i+inc); // Swap with the appropriate adjacent bit.
	}

	return bits;
}

int move_bit(int in, int to, int from) {
	// Move a bit from one place to another, shifting everything as you go
	// Two examples: 1101101, from 1->end or from end->1 (big-endian)
	
	if(to == from)
		return in;
	
	int toh = to > from;
	int high = toh?to:from, low = toh?from:to;
	
	// Start by getting only the stuff we care about.
	int bits = get_bits_in_place(in, low, high); // 1101101 -> 110110
	int mask = in - bits;
	bits = bits >> low;
	
	// Put the "from" flag into a buffer
	int from_flag = bits & 1<<(from-low); // 1->end: 0, end->1: 100000  
	
	// Subtract off the "from" flag
	bits -= from_flag; // 1e: 110110, e1: 010110
	
	// If the from flag is high, we need to multiply by 2, otherwise subtract
	bits = (toh ? bits/2 : bits*2); // 1e: 011011, e1: 101100
	
	// Now we transform from_flag to be a flag in the right spot.
	from_flag = toh?from_flag<<(to-from):from_flag>>(from-to); // 1e: 0->0, e1: 100000 -> 1
	
	// Finally we add it back in.
	bits += from_flag; 	// 1e: 011011->011011, e1: 101100->101101
	
	// Then we need to put it back in place.
	bits = bits << low;  // 1e: 0110110, e1: 1011010
	bits = mask|bits; // 1e: 0110111, e1: 1011011
	
	return bits; // Final results: 1101101 -> 1e: 0110111, e1: 1011011
}

//////////////////////////////////////////////////////////////
// 															//
//						Vestigial							//
// 															//
//////////////////////////////////////////////////////////////

int get_vdim(int panel, int varyctrl, int dimctrl) {
	// Get the dimension along which this instruction varies.
	// If panel is an MDInstr, varyctrl should be pc.vary and dimctrl should be pc.dim
	// If panel is a PulseInstP, varyctrl should be pc.pcon and dimctrl should be pc.pclevel
	int dim, nl, von;

	GetCtrlVal(panel, varyctrl, &von); 	// Determine if it's even on.
	if(!von)
		return 0;						// Dimension = 0 means it's not on.
	
	GetNumListItems(panel, dimctrl, &nl);
	if(nl <= 0)
		return -1; 						// Something's wrong.
	
	GetCtrlVal(panel, dimctrl, &dim);	// Everything's good
	return dim;
}

int get_vsteps(int panel, int varyctrl, int nsteps) {
	// Get the number of steps across which this instruction varies
	// If panel is an MDInstr, varyctrl should be pc.vary and nsteps should be pc.nsteps
	// If panel is a PulseInstP, varyctrl should be pc.pcon and dimctrl should be pc.pcsteps
	int steps, von;

	GetCtrlVal(panel, varyctrl, &von); 	// Determine if it's even on.
	if(!von)
		return 0;						// Dimension = 0 means it's not on.
	
	GetCtrlVal(panel, nsteps, &steps);	// Everything's good
	return steps;
}


