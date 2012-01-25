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

#include <PulseProgramTypes.h>
#include <UIControls.h>					// For manipulating the UI controls
#include <MathParserLib.h>				// For parsing math
#include <MCUserDefinedFunctions.h>		// Main UI functions
#include <PulseProgramLib.h>			// Prototypes and type definitions for this file

//////////////////////////////////////////////////////////////
// 															//
//				File Read/Write Operations					//
// 															//
//////////////////////////////////////////////////////////////

int SavePulseProgram(char *filename, PPROGRAM *p, char *group) {
	// Feed this a filename and and a PPROGRAM and it will create a pulse program
	// in the netCDF file with base variable name ppname. If NULL is passed to group
	// then it is assumed that this is a PPROGRAM-only file, and no base filename will
	// be used. Additionally, if group is not NULL, the netcdf file will be assumed 
	// to already exist, and as such it will not be created. 
	
	int rv = 0;			// For error checking in the netCDF functions
	int *v_ins_dids = NULL, *var_ids = NULL;
	
	if(filename == NULL)
		return 0;
	
	int ncid, gid;
	
	if(FileExists(filename, 0)) {	  // nc_create seems to freak out if the file already exists.
		int df_err = DeleteFile(filename);
		if(df_err) {
			// We're going to have to try a bit harder this time.
			FILE *fid = fopen(filename, "w+");
			fclose(fid);
			DeleteFile(filename);
		}
	}
	
	
	// Create the netCDF file if it doesn't already exist.
	if(group == NULL) {
		if(rv = nc_create(filename, NC_NETCDF4|NC_CLOBBER, &ncid))
			goto error;	// Error opening file
		
		gid = ncid;
	} else {
		if(rv = nc_open(filename, NC_WRITE, &ncid))
			goto error;	// Error opening file
		
		if(rv = nc_def_grp(ncid, group, &gid))			// Create the requested group in the file. 
			goto error;
	}
	
	// It may be necessary to call nc_redef here, but I'll try without.
	int ud_funcs = 0;		// Whether or not we need to store user-defined funcs
	if(p->nFuncs)
		ud_funcs = 1;
	
	// Add in the basic attributes -> Error checking seems like it's going to be
	// tedious and probably not all that fruitful at the moment, so skip it for now.
	if(rv = nc_put_att_int(gid, NC_GLOBAL, "np", NC_INT, 1, &p->np))						// Number of points
		goto error;																
	
	if(rv = nc_put_att_double(gid, NC_GLOBAL, "sr", NC_DOUBLE, 1, &p->sr)) 					// Sampling rate
		goto error;
	
	if(rv = nc_put_att_int(gid, NC_GLOBAL, "trig_ttl", NC_INT, 1, &p->trigger_ttl))			// Trigger TTL
		goto error;
	
	if(rv = nc_put_att_int(gid, NC_GLOBAL, "scan", NC_INT, 1, &p->scan))					// Scan bool
		goto error;
	
	if(rv = nc_put_att_int(gid, NC_GLOBAL, "ud_funcs", NC_INT, 1, &ud_funcs))				// User-defined funcs bool
		goto error;
	
	if(rv = nc_put_att_int(gid, NC_GLOBAL, "varied", NC_INT, 1, &p->varied))				// Varied bool
		goto error;
		
	if(rv = nc_put_att_int(gid, NC_GLOBAL, "skip", NC_INT, 1, &p->skip)) 					// Skip bool
		goto error;
	
	if(rv = nc_put_att_double(gid, NC_GLOBAL, "total_time", NC_DOUBLE, 1, &p->total_time))	// Total time
		goto error;
	
	if(rv = nc_put_att_int(gid, NC_GLOBAL, "n_inst", NC_INT, 1, &p->n_inst))				// Number of instructions
		goto error;
		
	// Now write the array of instructions.
	var_ids = malloc(sizeof(int)*6);		// 6 = Number of fields in a pinstr
	int dim_id;
	if(rv = nc_put_pinstr_array(ncid, gid, p->instrs, p->nUniqueInstrs, MC_DEF_MODE, NULL, &dim_id, var_ids))
		goto error;
	
	// Now if it's a varied experiment, set up the arrays of varied stuff.
	int v_ins_id, skip_locs_id, v_del_id, v_dat_id;

	if(p->varied) {
		int nVaried = p->nVaried;
		int *maxsteps = p->maxsteps;
	
		// The dimensions we care about
		v_ins_dids = malloc(sizeof(int)*2);
		if(rv = nc_def_dim(gid, "VariedInstr", nVaried, &v_ins_dids[0]))
			goto error;
		if(rv = nc_def_dim(gid, "LIndexSteps", p->max_n_steps, &v_ins_dids[1]))
			goto error;
		
		// Define the variables (there will be 3)
		if(rv = nc_def_var(gid, "v_ins_locs", NC_INT, 2, v_ins_dids, &v_ins_id))
			goto error;
		
		// Now we'll put a bunch of metadata on v_ins_locs
		// First the scalars
		if (rv = nc_put_att_int(gid, v_ins_id, "nDims", NC_INT, 1, &p->nDims))
			goto error;
		if(rv = nc_put_att_int(gid, v_ins_id, "nCycles", NC_INT, 1, &p->nCycles))
			goto error;
		if(rv = nc_put_att_int(gid, v_ins_id, "nVaried", NC_INT, 1, &p->nVaried))
			goto error;
		if(rv = nc_put_att_int(gid, v_ins_id, "max_n_steps", NC_INT, 1, &p->max_n_steps))
			goto error;
		if(rv = nc_put_att_int(gid, v_ins_id, "real_n_steps", NC_INT, 1, &p->real_n_steps))
			goto error;
		
		// Then the vectors
		if(rv = nc_put_att_int(gid, v_ins_id, "v_ins_num", NC_INT, p->nVaried, p->v_ins))
			goto error;
		if(rv = nc_put_att_int(gid, v_ins_id, "v_ins_dim", NC_INT, p->nVaried, p->v_ins_dim))
			goto error;
		if(rv = nc_put_att_int(gid, v_ins_id, "v_ins_mode", NC_INT, p->nVaried, p->v_ins_mode))
			goto error;
		if(rv = nc_put_att_int(gid, v_ins_id, "maxsteps", NC_INT, p->nDims+p->nCycles, p->maxsteps))
			goto error;
		if(rv = nc_put_att_string(gid, v_ins_id, "delay_exprs", p->nVaried, p->delay_exprs))
			goto error;
		if(rv = nc_put_att_string(gid, v_ins_id, "data_exprs", p->nVaried, p->data_exprs))
			goto error;
		
		// Now if it's necessary, the skip_locs array
		if(p->skip) {
			if(rv = nc_def_var(gid, "skip_locs", NC_INT, 1, &v_ins_dids[1], &skip_locs_id))
				goto error;
			
			size_t exprlen = strlen(p->skip_expr)+1;
			if(rv = nc_put_att_text(gid, skip_locs_id, "skip_expr", exprlen, p->skip_expr))
				goto error;
		}
		
		free(v_ins_dids);
		v_ins_dids = NULL;
	}
	
	nc_enddef(ncid);	//	Leave definition mode, enter data mode.

	// Now the arrays are all defined and such, we need to populate the variables.
	// Starting with the instructions -> The first thing we need to do is build a char array
	// that contains all the data in the appropriate format.
	
	
	// Finally, we need to write the varied arrays (v_ins_locs and skip_locs)
	if(p->varied) {
		// These are going to be way easier -> start with v_ins_locs
		if(rv = nc_put_var_int(gid, v_ins_id, &p->v_ins_locs[0][0]))	// We can just write the array (I hope)
			goto error;
		
		// Now we want to save the array of expressions.
		/*if(rv = nc_put_var_string(gid, v_del_id, p->delay_exprs))
			goto error;
		
		if(rv = nc_put_var_string(gid, v_dat_id, p->data_exprs))
			goto error; */
		
		// Finally the skip variable.
		if(p->skip) {
			if(rv = nc_put_var_int(gid, skip_locs_id, p->skip_locs))	// Same with this one
				goto error;								
		}
	}
	
	int func_gid;
	if(rv = nc_def_grp(ncid, "func_group", &func_gid))
		goto error;
	
	if(ud_funcs) {
	/*	if (rv = nc_put_pfuncs(ncid, gid, p->funcs, p->func_locs, p->nFuncs, p->tFuncs, NC_PINSTR)) 	// Write the functions if need be.
			goto error;
	*/
	}
	
	
	error:
	nc_close(ncid);					// All the values have been written, so we can close up shop.

	if(var_ids != NULL)
		free(var_ids);
	
	if(v_ins_dids != NULL)
		free(v_ins_dids);
	
	return rv;
}

PPROGRAM *LoadPulseProgram(char *filename, int *err_val, char *group) {
	// Loads a netCDF file to a PPROGRAM. p is dynamically allocated
	// as part of the function, so make sure to free it when you are done with it.
	// Passing NULL to group looks for a program in the root group, otherwise
	// the group "group" in the root directory is used.
	// p is freed on error.
	
	// Returns 0 if successful, positive integers for errors.
	
	err_val[0] = 0;
	int retval = 0, pos = 0; 		// For error checking
	int ncid, gid;					// Root value, group id
	
	char **data = NULL, **delay = NULL;
	
	if((retval = nc_open(filename, NC_NOWRITE, &ncid)))
		goto error;						// On errors go to the cleanup function.
	
	if(group == NULL) 
		gid = ncid;
	else {
		if(retval = nc_inq_grp_ncid(ncid, group, &gid))
			goto error;
	}

	// Let's make a PPRogram
	PPROGRAM *p = malloc(sizeof(PPROGRAM));
	pos++;							// We've got something malloced, so we should note that
	
	// Grab those attributes all the kids are raving about
	int ud_funcs;		// Whether or not we're going to use user-defined functions
	if(retval = nc_get_att_int(gid, NC_GLOBAL, "np", &p->np))	
		goto error;
	if(retval = nc_get_att_double(gid, NC_GLOBAL, "sr", &p->sr))
		goto error;
	if(retval = nc_get_att_int(gid, NC_GLOBAL, "trig_ttl", &p->trigger_ttl))
		goto error;
	if(retval = nc_get_att_int(gid, NC_GLOBAL, "scan", &p->scan))
		goto error;
	if(retval = nc_get_att_int(gid, NC_GLOBAL, "ud_funcs", &ud_funcs))
		goto error;
	if(retval = nc_get_att_int(gid, NC_GLOBAL, "varied", &p->varied))
		goto error;
	if(retval = nc_get_att_int(gid, NC_GLOBAL, "skip", &p->skip))
		goto error;
	if(retval = nc_get_att_int(gid, NC_GLOBAL, "n_inst", &p->n_inst))
		goto error;
	
	// Now we need to get nUniqueInstrs and the dimid for the dimensions.
	int instr_dim;
	if(retval = nc_inq_dimid(gid, "Instructions", &instr_dim))
		goto error;
	
	if(retval = nc_inq_dimlen(gid, instr_dim, &p->nUniqueInstrs))
		goto error;
	
	// Now we need to get the varids of the variables and gather the metadata we need to allocate the rest of p
	int v_ins_id, skip_locs_id, func_locs_id, *func_ids;		// Varyids
	
	if(p->varied) {
		// Then, if necessary, the varied instructions index.
		if(retval = nc_inq_varid(gid, "v_ins_locs", &v_ins_id))
			goto error;
		
		if(retval = nc_get_att_int(gid, v_ins_id, "nDims", &p->nDims))
			goto error;
		
		if(retval = nc_get_att_int(gid, v_ins_id, "nCycles", &p->nCycles))
			goto error;
		
		if(retval = nc_get_att_int(gid, v_ins_id, "nVaried", &p->nVaried))
			goto error;
		
		if(retval = nc_get_att_int(gid, v_ins_id, "max_n_steps", &p->max_n_steps))
			goto error;
		
		if(retval = nc_get_att_int(gid, v_ins_id, "real_n_steps", &p->real_n_steps))
			goto error;
		
		if(p->skip) {
			// If we have skips, get this
			if(retval = nc_inq_varid(gid, "skip_locs", &skip_locs_id))
				goto error;
		}
	} else {
		p->nDims = 0;			// This stuff does in fact need to be passed, so we have to 0 it out.
		p->nCycles = 0;
		p->nVaried = 0;
	}
	
	if(ud_funcs) {
		// Finally we need to get nFuncs and tFuncs.
		if(retval = nc_inq_varid(gid, "func_locs", &func_locs_id))	// We'll get func_ids later
			goto error;
		
		if(retval = nc_get_att_int(gid, func_locs_id, "nFuncs", &p->nFuncs))
			goto error;
		
		if(retval = nc_get_att_int(gid, func_locs_id, "tFuncs", &p->tFuncs))
			goto error;
	} else {
		p->nFuncs = 0;
		p->tFuncs = 0;
	}
	
	// Now we have everything we need to dynamically allocate the rest of the pprogram and start reading out the arrays
	create_pprogram(p);
	pos++;		// At this value, use free_pprog(p) rather than free(p)
	
	// Read out the instructions array now
	int i; // Counter
	if(retval = nc_get_pinstr_array(gid, p->instrs, p->nUniqueInstrs, instr_dim))
		goto error;
	
	if(p->varied) {
		// Now we read out the varied arrays. These are a little easier because they are arrays of ints
		if(retval = nc_get_var_int(gid, v_ins_id, &p->v_ins_locs[0][0]))	// Pointer to the first element, that's what this function wants I think.
			goto error;
		
		// Then we should get the attributes we defined.
		if(retval = nc_get_att_int(gid, v_ins_id, "v_ins_num", p->v_ins))
			goto error;
		
		if(retval = nc_get_att_int(gid, v_ins_id, "v_ins_mode", p->v_ins_mode))
			goto error;
		
		if(retval = nc_get_att_int(gid, v_ins_id, "v_ins_dim", p->v_ins_dim))
			goto error;
		
		if(retval = nc_get_att_int(gid, v_ins_id, "maxsteps", p->maxsteps))
			goto error;
		
		// Create an array of strings to get the delay and data expressions.
		data = malloc(p->nVaried*sizeof(char*));
		delay = malloc(p->nVaried*sizeof(char*));
		
		for(i = 0; i < p->nVaried; i++) {
			data[i] = NULL;
			delay[i] = NULL;
		}
		
		if(retval = nc_get_att_string(gid, v_ins_id, "data_exprs", data))
			goto error;
		
		if(retval = nc_get_att_string(gid, v_ins_id, "delay_exprs", delay))
			goto error;
		
		// Now let's go through and copy over the delays to p. We need to do it in two steps
		// so that I can have more control over the memory allocation.
		int len;
		for(i = 0; i < p->nVaried; i++) {
			len = strlen(data[i]);
			if(len > 0) {
				p->data_exprs[i] = realloc(p->data_exprs[i], len+1);
				strcpy(p->data_exprs[i], data[i]);	
			}
			
			len = strlen(delay[i]);
			if(len > 0) {
				p->delay_exprs[i] = realloc(p->delay_exprs[i], len+1);
				strcpy(p->delay_exprs[i], delay[i]);
			}
		}
		
		if(p->skip) {
			// Now the skipped instruction locations and the expresion that generated them
			if(retval = nc_get_var_int(gid, skip_locs_id, p->skip_locs))
				goto error;
			
			if(retval = nc_get_att_text(gid, skip_locs_id, "skip_expr", p->skip_expr))
				goto error;
		}
	}
	
	// Finally we read out the functions if they exist. This is a subfunction because these are sometimes saved independently and not as part of a program.
	if(ud_funcs) {
		if(retval = nc_get_pfuncs(gid, func_locs_id, p->funcs, p->func_locs, p->nFuncs, p->tFuncs))
			goto error;
	}
	
	return p;
	
	error:
	// We need to free memory before we exit if the function didn't complete correctly.
	// "pos" encodes where we were for this purpose.

	if(data != NULL) {
		for(i = 0; i < p->nVaried; i++) {
			if(data[i] != NULL)
				free(data[i]);
		}
		free(data);
	}
	
	if(delay != NULL) {
		for(i = 0; i < p->nVaried; i++) {
			if(delay[i] != NULL)
				free(delay[i]);
		}
		free(delay);
	}
	
	if(pos == 1)
		free(p);

	if(pos >= 2)
		free_pprog(p);
	
	err_val[0] = retval;
	return NULL;
}

void display_netcdf_error(int err_val) {
	MessagePopup("NetCDF Error", nc_strerror(err_val));	
}

int nc_put_pinstr_array(int ncid, int gid, PINSTR **i_ar, int num_instrs, int mode, char *dim_name, int *dim_id, int *var_ids) {
	// Convenience function for putting an array of pinstrs. Give it the relevant
	// Pass either MC_DEF_MODE or MC_DAT_MODE to for define or data mode.
	// This will define the dimension in the group gid as having num_instrs instructions.
	// The dimension will have name, "dim_name". If dim_name is null, default is "Instructions"
	
	if(i_ar == NULL)
		return 10;
	
	if(dim_name == NULL || strlen(dim_name) <= 1)
		dim_name = "Instructions";
	
	if(mode == MC_DEF_MODE)
		nc_redef(ncid);
	
	int rv = 0;
	
	// Define the dimension.
	if(rv = nc_def_dim(gid, dim_name, num_instrs, dim_id))
		return rv;
	
	// Define the variables -> i_flags, i_instr, i_data, i_scan, i_units, i_time;
	if(rv = nc_def_var(gid, "i_flags", NC_INT, 1, dim_id, &var_ids[0]))
		return rv;
	
	if(rv = nc_def_var(gid, "i_instr", NC_INT, 1, dim_id, &var_ids[1]))
		return rv;
	
	if(rv = nc_def_var(gid, "i_data", NC_INT, 1, dim_id, &var_ids[2]))
		return rv;
	
	if(rv = nc_def_var(gid, "i_scan", NC_BYTE, 1, dim_id, &var_ids[3]))
		return rv;
	
	if(rv = nc_def_var(gid, "i_units", NC_BYTE, 1, dim_id, &var_ids[4]))
		return rv;
	
	if(rv = nc_def_var(gid, "i_time", NC_DOUBLE, 1, dim_id, &var_ids[5]))
		return rv;
	
	// Now generate the arrays we're going to need for when we put the values in.
	int si = sizeof(int)*num_instrs, sd = sizeof(double)*num_instrs, sc = num_instrs;
	int *flags = malloc(si), *instrs = malloc(si), *data = malloc(si);
	unsigned char *units = malloc(sc), *scan = malloc(sc);
	double *time = malloc(sd);
	
	for(int i=0; i<num_instrs; i++) {
		flags[i] = i_ar[i]->flags;
		instrs[i] = i_ar[i]->instr;
		data[i] = i_ar[i]->instr_data;
		scan[i] = (signed char)(i_ar[i]->trigger_scan);
		units[i] = (signed char)(i_ar[i]->time_units);
		time[i] = i_ar[i]->instr_time;
	}
	
	// Now we can go through and put the arrays.
	nc_enddef(ncid);
	
	if(rv = nc_put_var_int(gid, var_ids[0], flags))
		goto error;
	if(rv = nc_put_var_int(gid, var_ids[1], instrs))
		goto error;
	
	if(rv = nc_put_var_int(gid, var_ids[2], data))
		goto error;
	
	if(rv = nc_put_var_schar(gid, var_ids[3], scan))
		goto error;
	
	if(rv = nc_put_var_schar(gid, var_ids[4], units))
		goto error;
	
	if(rv = nc_put_var_double(gid, var_ids[5], time))
		goto error;
	
	error:
	
	if(mode == MC_DEF_MODE)
		nc_redef(ncid);
	
	free(flags);
	free(instrs);
	free(data);
	free(units);
	free(scan);
	free(time);
	
	return rv;
}

int nc_get_pinstr_array(int gid, PINSTR **i_ar, int num_instrs, int dimid) {
	// Gets the instruction group with the name passed in dim_name. If NULL is passed
	// to dim_name, "Instructions" is used.
	
	if(i_ar == NULL)
		return 10;
	
	int rv = 0;
	
	// Get the variable ids.
	int flag_id, instr_id, data_id, scan_id, units_id, time_id;
	if(rv = nc_inq_varid(gid, "i_flags", &flag_id))
		return rv;
	
	if(rv = nc_inq_varid(gid, "i_instr", &instr_id))
		return rv;
	
	if(rv = nc_inq_varid(gid, "i_data", &data_id))
		return rv;
	
	if(rv = nc_inq_varid(gid, "i_scan", &scan_id))
		return rv;
	
	if(rv = nc_inq_varid(gid, "i_units", &units_id))
		return rv;
	
	if(rv = nc_inq_varid(gid, "i_time", &time_id))
		return rv;
	
	// Now create some arrays to put the data into load the data into the arrays.
	int si = sizeof(int)*num_instrs, sd = sizeof(double)*num_instrs, sc = num_instrs;
	int *flags = malloc(si), *instrs = malloc(si), *data = malloc(si);
	signed char *scan = malloc(sc), *units = malloc(sc);
	double *time = malloc(sd);
	
	if(rv = nc_get_var_int(gid, flag_id, flags))
		goto error;
	
	if(rv = nc_get_var_int(gid, instr_id, instrs))
		goto error;
	
	if(rv = nc_get_var_int(gid, data_id, data))
		goto error;
	
	if(rv = nc_get_var_schar(gid, scan_id, scan))
		goto error;
	
	if(rv = nc_get_var_schar(gid, units_id, units))
		goto error;
	
	if(rv = nc_get_var_double(gid, time_id, time))
		goto error;
	
	
	// Finally put them into a pinstr array.
	for(int i = 0; i < num_instrs; i++) {
		i_ar[i]->flags = flags[i];
		i_ar[i]->instr = instrs[i];
		i_ar[i]->instr_data = data[i];
		i_ar[i]->time_units = (int)(units[i]);
		i_ar[i]->trigger_scan = (int)(scan[i]);
		i_ar[i]->instr_time = time[i];
	}
	
	
	// And free the arrays we created.
	error:
	free(flags);
	free(instrs);
	free(data);
	free(scan);
	free(units);
	free(time);
	
	return rv;
	
}
 
int nc_get_pfuncs(int gid, int func_locs_id, pfunc **funcs, int *func_locs, int nFuncs, int tFuncs) {
	// Read the array of pfuncs from the group gid.  gid < 0 returns an error.
	// This assumes that you give it the file open and in data mode.
	// On an error, the function passes along the return value of the function that threw the error.
	
	if(gid <0)
		return 100;
	
	int pos = 0, retval;	// Error checking variables
	
	// We need to allocate enough space for an array of function names, so we need the length of the dimension "FuncNames"
	int fndid, maxlen;
	if(retval = nc_inq_dimid(gid, "FuncNames", &fndid))
		goto error;
	
	if(retval = nc_inq_dimlen(gid, fndid, &maxlen))
		goto error;
	
	// Now we can allocate an array of these things.
	char **fname_ar = malloc(sizeof(char*)*nFuncs);
	int i, j; // Counters
	for(i=0; i<nFuncs;i++)
		fname_ar[i] = malloc(maxlen);
	pos++;	// At this point we need this 1D array.
	
	if(retval = nc_get_var_string(gid, func_locs_id, fname_ar))
		goto error;
	
	// We also now need to get func_locs.
	if(retval = nc_get_att_int(gid, func_locs_id, "indices", func_locs))	//Easy, unlike what is to come.
		goto error;
		
	// Now we need to know the maximum number of instructions, which is the size of the dimension "FuncInstruction"
	// We also need to move to the subgroup "func_group" for this bit
	int fgid;
	if(retval = nc_inq_grp_ncid(gid, "func_group", &fgid))
		goto error;
	
	int fdid, max_inst;
	if(retval = nc_inq_dimid(fgid, "FuncInstruction", &fdid))
		goto error;
	
	if(retval = nc_inq_dimlen(fgid, fdid, &max_inst))
		goto error;
	
	// Now let's allocate an array for containing the instructions
	char **fins_ar = malloc(sizeof(char*)*max_inst);
	int *func_ids = malloc(sizeof(int)*nFuncs);
	for(i = 0; i<max_inst;i++)
		fins_ar[i] = malloc(PINSTR_SIZE);
	pos++; 	// Two 2D arrays and 1 1D array to free here.
	
	int f_pos = 0; // Counters for malloc within the func.
	
	for(i=0; i<nFuncs;i++) {
		// Start by getting the varid corresponding the the function name.
		if(retval = nc_inq_varid(fgid, fname_ar[i], &func_ids[i]))
			goto error;
		
		// Now get the attributes, which we need for the rest of this.
		if(retval = nc_get_att_int(fgid, func_ids[i], "rflags", &funcs[i]->r_flags))
			goto error;
		
		if(retval = nc_get_att_int(fgid, func_ids[i], "argmode", &funcs[i]->argmode))
			goto error;
		
		if(retval = nc_get_att_int(fgid, func_ids[i], "n_instr", &funcs[i]->n_instr))
			goto error;
		
		
		funcs[i]->name = malloc(strlen(fname_ar[i]));
		funcs[i]->instrs = malloc(sizeof(PINSTR*)*funcs[i]->n_instr);
		for(j = 0; j<funcs[i]->n_instr; j++)
			funcs[i]->instrs[j] = malloc(sizeof(PINSTR));
	
		f_pos++;	// Counter of how many filenames and instructions have been malloced.    

		strcpy(funcs[i]->name, fname_ar[i]);	// Copy the name over
		
		if(retval = nc_get_var(fgid, func_ids[i], fins_ar))	// Gets the instructions array as a char array.
			goto error;
		
		// Now all we have left is the instructions.
		/*for(j = 0; j<funcs[i]->n_instr; j++) 
			get_nc_pinstr(funcs[i]->instrs[j], fins_ar[j]);*/	// Convert the array to a proper PINSTR
	}
	
	return 0;
	
	error: // Now we need to free the memory that is left over after the error.
	
	if(pos >= 1) {
		for(i = 0; i<nFuncs; i++)
			free(fname_ar[i]);
		free(fname_ar);
	}
	
	if(pos >= 2) {
		for(i = 0; i<max_inst; i++)
			free(fins_ar[i]);
		free(fins_ar);
		free(func_ids);
		
		for(i = 0; i<f_pos; i++) {
			free(funcs[i]->name);
			for(j=0; j<funcs[i]->n_instr; j++)
				free(funcs[i]->instrs[i]);
			free(funcs[i]->instrs);
		}
	}
	
	return retval;
}

int nc_put_pfuncs(int ncid, int gid, pfunc **funcs, int *func_locs, int nFuncs, int tFuncs, nc_type NC_PINSTR) {
	// Writes the array of pfuncs to ncid in the group gid. If gid < 0, gid = ncid;
	// This assumes that you give it the file open and in data mode. The function 
	// does definitions, but will return the file in data mode.
	
	nc_redef(ncid);		// Get into definition mode.
	
	if(gid < 0) 
		gid = ncid;

	// Variable definitions
	int func_gid, func_locs_id, *funcs_ids, maxlen=0, max_inst = 0, i;
	int finstr_did, *flocs_dids;						// Dimensions
	char empty_instr[PINSTR_SIZE];
	
	for(i = 0; i<PINSTR_SIZE; i++)
		empty_instr[i] = 0;							// Blank instruction.
	
	funcs_ids = malloc(sizeof(int)*nFuncs);
	nc_def_grp(gid, "func_group", &func_gid);		// A new group so that user-defined function names don't cause conflicts

	// Define the arrays now - we'll start with the functions themselves
	for(i = 0; i<nFuncs; i++) {					// It's best to just create dimension that are too large than 
		if(max_inst > funcs[i]->n_instr)			// to have some insane proliferation of dimensions, so we'll just
			max_inst = funcs[i]->n_instr;		// make the dimension as long as the longest number of instructions in
													// the functions we're trying to store. Similarly, the string array for
		if(maxlen > strlen(funcs[i]->name))		// the func_locs should be as long as the longest name.
			maxlen = strlen(funcs[i]->name);
	}
	
	nc_def_dim(func_gid, "FuncInstruction", max_inst, &finstr_did);	// It's a vector, so use the 1D version.
	
	for(i = 0; i<nFuncs; i++) {
		nc_def_var(func_gid, funcs[i]->name, NC_PINSTR, 1, &finstr_did, &funcs_ids[i]);		// Function variables are each an array of instrs
	
		nc_def_var_endian(func_gid, funcs_ids[i], NC_ENDIAN_LITTLE);	// Make sure it's little endian
		nc_def_var_fill(func_gid, funcs_ids[i], 0, empty_instr);	// Fill with empty instructions if no value given
		
		nc_put_att_int(func_gid, funcs_ids[i], "rflags", NC_INT, 1, &(funcs[i]->r_flags));
		nc_put_att_int(func_gid, funcs_ids[i], "argmode", NC_INT, 1, &(funcs[i]->argmode));
		nc_put_att_int(func_gid, funcs_ids[i], "n_instr", NC_INT, 1, &(funcs[i]->n_instr));
	}
	
	flocs_dids = malloc(sizeof(int)*2);
	nc_def_dim(gid, "AllFuncs", nFuncs, &flocs_dids[0]);		// Set up the func_locs dimensions
	nc_def_dim(gid, "FuncNames", maxlen, &flocs_dids[1]);
	
	// Then the function locator index, which will be an array of strings with attributes pointing to the func_locs values.
	nc_def_var(gid, "func_locs", NC_CHAR, 2, flocs_dids, &func_locs_id);		// Define the function index variable
	
	nc_put_att_int(gid, func_locs_id, "tFuncs", NC_INT, 1, &tFuncs);	// Store the total number of functions as an attribute
	nc_put_att_int(gid, func_locs_id, "nFuncs", NC_INT, 1, &nFuncs);
	nc_put_att_int(gid, func_locs_id, "indices", NC_INT, tFuncs, func_locs);
	free(flocs_dids);
	
	nc_enddef(ncid);	// Done with definition mode
	
	// First let's build an array of size nFuncs x maxlen containing all the function names
	char **func_names_array = malloc(sizeof(char*)*nFuncs);
	for(i = 0; i<nFuncs; i++) {
		func_names_array[i] = malloc(maxlen);
		strcpy(func_names_array[i], funcs[i]->name);
	}
	
	nc_put_var_string(gid, func_locs_id, func_names_array);			// Write the array
	
	for(i = 0; i<nFuncs; i++)
		free(func_names_array[i]);
	free(func_names_array);
	
	// Now iterate through the variables and generate some PINSTR arrays
	char **func_instrs;
	int j;
	for(i = 0; i<nFuncs; i++) {
		func_instrs = malloc(sizeof(char*)*funcs[i]->n_instr);
		for(j=0;j<funcs[i]->n_instr;j++) {
			func_instrs[j] = malloc(PINSTR_SIZE);			// Allocate space to put a PINSTR here
			
			//make_nc_pinstr(funcs[i]->instrs[j], func_instrs[j]);		// Put a PINSTR there
		}
				
		// We have an array for this particular func, so we can write it now.
		nc_put_var(func_gid, funcs_ids[i], func_instrs);
			
		// Now we need to free that memory
		for(j = 0; j<funcs[i]->n_instr; j++)
			free(func_instrs[j]);
		free(func_instrs);
	}
		
	return 0;

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
	p->n_inst = uipc->ni;								// Number of instructions
	p->nCycles = uipc->nc;								// Number of cycles
	p->nDims = uipc->nd;								// Number of indirect dimensions
	if(p->nDims || p->nCycles)		   					// If either of these is true, it's a varied experiment
		p->varied = 1;
	else
		p->varied = 0;
	
	p->nVaried = uipc->ndins + uipc->ncins;				// Number of instructions that are varied.
	
	GetCtrlVal(pc->skip[1], pc->skip[0], &p->skip);		// Whether or not skip is on
	
	GetNumListItems(pc->inst[0], pc->instr, &p->tFuncs);// Number of items in the instruction rings
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
		int **dim_array = malloc(sizeof(int*)*3);
		int **cyc_array = malloc(sizeof(int*)*2);
	
		dim_array[0] = uipc->dim_ins;
		dim_array[1] = uipc->ins_dims;
		dim_array[2] = uipc->ins_state;
		cyc_array[0] = uipc->cyc_ins;
		cyc_array[1] = uipc->ins_cycs;
	
		sort_linked(dim_array, 3, uipc->ndins);
		sort_linked(cyc_array, 2, uipc->ncins);
	
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
			
			if(dinc < uipc->ndins && (cinc >= uipc->ncins || (uipc->dim_ins[dinc] <= uipc->cyc_ins[cinc]))) {
				// Put the next dimension variable in if this is unique and lower than the
				// next item in the queue for the cycle variable.
				v_ins[i] = uipc->dim_ins[dinc];
				v_ins_dim[i] += pow(2, uipc->ins_dims[dinc]+uipc->nc);	// Set the dimension flags

				if(uipc->ins_state[dinc] == 1)
					v_ins_mode[i] = PP_V_ID;
				else if(uipc->ins_state[dinc] == 2)
					v_ins_mode[i] = PP_V_ID|PP_V_ID_EXPR;
				else
					v_ins_mode[i] = PP_V_ID|PP_V_ID_ARB;
			}
			
			if(cinc < uipc->ncins && (dinc >= uipc->ndins || (uipc->cyc_ins[cinc] <= uipc->dim_ins[dinc]))) {
				v_ins[i] = uipc->cyc_ins[cinc];
				v_ins_dim[i] += pow(2, uipc->ins_cycs[cinc]);			// Set the cycle flags
				v_ins_mode[i] += PP_V_PC;
			}
			
							
			// Now add in the unique instructions we'll need.
			if((v_ins_mode[i] & PP_V_ID) && (v_ins_mode[i] & PP_V_PC)) {
				p->nUniqueInstrs += (uipc->cyc_steps[uipc->ins_cycs[cinc++]]*uipc->dim_steps[uipc->ins_dims[dinc++]])-1;
				p->nVaried--;		// Merged two instructions.
			} else if(v_ins_mode[i] & PP_V_ID) 
				p->nUniqueInstrs += uipc->dim_steps[uipc->ins_dims[dinc++]];
			else if(v_ins_mode[i] & PP_V_PC) 
				p->nUniqueInstrs += uipc->cyc_steps[uipc->ins_cycs[cinc++]];
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
				GetCtrlVal(pc->inst[i], pc->pcon, &pcon);
				if(pcon) {										
					GetCtrlVal(pc->inst[i], pc->pcsteps, &pcs);
					for(j=1; j<=pcs; j++) {
						f = uipc->c_instrs[i][j]->instr-9;
						if(f >= 0 && !ffound[f]) {
							ffound[f] = 1;
							nFuncs++;
						}
					}
				} else {
					GetCtrlVal(pc->inst[i], pc->instr, &f);
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
		p->real_n_steps = uipc->real_n_steps;
	else
		p->real_n_steps = p->max_n_steps;
	
	// Whether or not there's a scan.
	for(i = 0; i<uipc->ni; i++) {
		GetCtrlVal(pc->inst[i], pc->scan, &p->scan);
		if(p->scan)
			break;
	}
	
	GetCtrlVal(pc->np[1], pc->np[0], &p->np);
	GetCtrlVal(pc->sr[1], pc->sr[0], &p->sr);
	GetCtrlVal(pc->trig_ttl[1], pc->trig_ttl[0], &p->trigger_ttl);
	p->total_time = uipc->total_time;
	
	
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
		int *places = malloc(sizeof(int)*(nDims+nCycles)), *cstep = malloc(sizeof(int)*(nDims+nCycles));
		for(i=0; i<(nDims+nCycles); i++) {
			places[i] = 1;
			cstep[i] = 0;							// The first one we'll want is for linear index 0.
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
			get_updated_instr(p->instrs[num], num, cstep, &cind, &dind, p->v_ins_mode[i]);
			
			if(cind >= 0)
				cyc = uipc->ins_cycs[cind];
			if(dind >= 0)
				dim = uipc->ins_dims[dind];
			
			dlev = dim+uipc->nc;			// The indirect dimensions come after the cycles
			
			// Now we can generate all the unique instructions and the v_ins_locs index.
			if((p->v_ins_mode[i] & PP_V_BOTH) == PP_V_BOTH) {	// The non-linear condition (must be first)
				int placestep = places[dlev]/places[cyc];		// Dim is always bigger.
				int r;											// One more counter
				for(int m = 0; m < uipc->dim_steps[dim]; m++) {
					for(j = 0; j < uipc->cyc_steps[cyc]; j++) {
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
				for(j = 0; j < uipc->dim_steps[dim]; j++) {
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
				
				GetCtrlAttribute(pc->cinst[p->v_ins[i]], pc->cexpr_delay, ATTR_TEXT_BGCOLOR, &color);
				if(color == VAL_GREEN) {
					GetCtrlValStringLength(pc->cinst[p->v_ins[i]], pc->cexpr_delay, &len);
					p->delay_exprs[i] = realloc(p->delay_exprs[i], len+1);
					
					GetCtrlVal(pc->cinst[p->v_ins[i]], pc->cexpr_delay, p->delay_exprs[i]);
				}
				
				GetCtrlAttribute(pc->cinst[p->v_ins[i]], pc->cexpr_data, ATTR_TEXT_BGCOLOR, &color);
				if(color == VAL_GREEN) {
					GetCtrlValStringLength(pc->cinst[p->v_ins[i]], pc->cexpr_data, &len);
					p->data_exprs[i] = realloc(p->data_exprs[i], len+1);
					
					GetCtrlVal(pc->cinst[p->v_ins[i]], pc->cexpr_data, p->data_exprs[i]);
				}
				
				
			}
		}

		// If there are skips, get them now.
		if(p->skip) {
			if(uipc->skip_locs != NULL) {	// Probably unnecessary in light of ui_cleanup, but for safety.
				// Get the expression that generated the skip.
				int expr_len;
				GetCtrlValStringLength(pc->skiptxt[1], pc->skiptxt[0], &expr_len);
				p->skip_expr = realloc(p->skip_expr, expr_len+1);
				GetCtrlVal(pc->skiptxt[1], pc->skiptxt[0], p->skip_expr);
			
				// Copy over the skip_locs array.
				for(i=0; i<p->max_n_steps; i++)
					p->skip_locs[i] = uipc->skip_locs[i];
			} else {
				free(p->skip_expr);
				p->skip = 0;
			}
		}
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
	SetCtrlVal(pc->ninst[1], pc->ninst[0], p->n_inst);
	change_number_of_instructions();
	
	for(i=0; i < p->n_inst; i++)
		set_instr(i, p->instrs[i]);
	
	// Now update the varied instructions
	if(p->nDims) {
		SetCtrlVal(pc->ndims[1], pc->ndims[0], p->nDims);
		change_num_dims();
		
		toggle_nd();
	}
	
	if(p->nCycles) {
		SetCtrlVal(pc->numcycles[1], pc->numcycles[0], p->nCycles);
		change_num_cycles();
	}
	
	// Now we need to update the number of steps in each dimension and cycle.
	for(i = 0; i < p->nCycles; i++) {
		change_cycle_num_steps(i, p->maxsteps[i]);	
	}
	
	for(i = 0; i < p->nDims; i++) {
		change_num_dim_steps(i, p->maxsteps[i+p->nCycles]);
	}
	
	// Turn on the instructions if they should be on.
	int *cstep = NULL, len;
	int *nd_data, *nd_units;
	double *nd_delays;
	
	// Generate a cstep;
	cstep = malloc(sizeof(int)*(p->nCycles+p->nDims));
	for(i = 0; i < p->nCycles+p->nDims; i++)
		cstep[i] = 0;
	
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
			
			SetCtrlIndex(pc->inst[num], pc->pclevel, cyc);
			
			change_cycle(num);
			
			// Now update the uipc file with the relevant instructions.
			if(uipc->c_instrs[num] == NULL || uipc->max_cinstrs[num] < p->maxsteps[cyc]) {
				if(uipc->c_instrs[num] == NULL)
					uipc->c_instrs[num] = malloc(sizeof(PINSTR)*p->maxsteps[cyc]);
				else
					uipc->c_instrs[num] = realloc(uipc->c_instrs[num], sizeof(PINSTR*)*p->maxsteps[cyc]);
				
				for(j = uipc->max_cinstrs[num]; j < p->maxsteps[cyc]; j++)
					uipc->c_instrs[num][j] = NULL;
				
				uipc->max_cinstrs[num] = p->maxsteps[cyc];
			}

			// Copy the relevant pinstrs into the cache.
			for(j = 0; j < p->maxsteps[cyc]; j++) {
				if(uipc->c_instrs[num][j] == NULL)
					uipc->c_instrs[num][j] = malloc(sizeof(PINSTR));
				
				cstep[cyc] = j;
				copy_pinstr(p->instrs[p->v_ins_locs[i][get_lindex(cstep, p->maxsteps, p->nCycles+p->nDims)]], uipc->c_instrs[num][j]);
			}
			
			// Reset the cstep (probably unnecessary)
			cstep[cyc] = 0;
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
			
			PINSTR *instr = malloc(sizeof(PINSTR));
			
			for(j = 0; j < steps; j++) {
				cstep[dlev] = j;
				copy_pinstr(p->instrs[p->v_ins_locs[i][get_lindex(cstep, p->maxsteps, p->nCycles+p->nDims)]], instr);
				
				nd_data[j] = instr->instr_data;
				nd_delays[j] = instr->instr_time;
				nd_units[j] = instr->time_units;
			}
			
			free(instr);
			
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
				SetCtrlIndex(pc->cinst[num], pc->delu_init, nd_units[0]);
				SetCtrlVal(pc->cinst[num], pc->del_init, nd_delays[0]/pow(1000, nd_units[0]));
				
				// Final delay
				SetCtrlIndex(pc->cinst[num], pc->delu_fin, nd_units[steps-1]);
				SetCtrlVal(pc->cinst[num], pc->del_fin, nd_delays[steps-1]/pow(1000, nd_units[steps-1]));
			}
			
			if(dat) {
				SetCtrlVal(pc->cinst[num], pc->dat_init, nd_data[0]);
				SetCtrlVal(pc->cinst[num], pc->dat_fin, nd_data[steps-1]);
			}
		
			if(p->v_ins_mode[i] & PP_V_ID_EXPR) {
				update_nd_state(num, 2);
				
				// Load the expressions
				len = strlen(p->data_exprs[i]);
				if(len > 0)
					SetCtrlVal(pc->cinst[num], pc->cexpr_data, p->data_exprs[i]);
				
				len = strlen(p->delay_exprs[i]);
				if(len > 0)
					SetCtrlVal(pc->cinst[num], pc->cexpr_delay, p->delay_exprs[i]);
				
				update_nd_from_exprs(num);
			} else {
				update_nd_state(num, 1);
			}
			
			// Update the dimension (increments and such)
			SetCtrlIndex(pc->cinst[num], pc->dim, dim);
			change_dimension(num);
			
			// Now if this was arbitrary, copy over the nd_data and nd_delay tables.
			if(p->v_ins_mode[i] & PP_V_ID_ARB) {
				for(j = 0; j < uipc->ndins; j++) {
					if(uipc->dim_ins[j] == num)
						break;
				}
				
				int ind = j;
				
				if(del) {
					for(j = 0; j < steps; j++)
						uipc->nd_delays[ind][j] = nd_delays[j];
				}
			
				if(dat) {
					for(j = 0; j < steps; j++)
						uipc->nd_data[ind][j] = nd_data[j];
				}
			}
		}
	}
}


void clear_program() {
	// Deletes all instructions for a new program.
	
	// Clear all the instructions
	for(int i = 0; i < uipc->max_ni; i++)
		clear_instruction(i);
	
	// Update the number of instructions
	SetCtrlVal(pc->ninst[1], pc->ninst[0], 1);
	change_number_of_instructions();
	
	// Turn off the skips if necessary.
	SetCtrlVal(pc->skip[1], pc->skip[0], 0);
	
	SetCtrlAttribute(pc->skiptxt[1], pc->skiptxt[0], ATTR_TEXT_BGCOLOR, VAL_WHITE);
	SetCtrlAttribute(pc->skiptxt[1], pc->skiptxt[0], ATTR_TEXT_COLOR, VAL_BLACK);
	SetCtrlAttribute(pc->skiptxt[1], pc->skiptxt[0], ATTR_TEXT_BOLD, 0);
	SetCtrlAttribute(pc->skip[1], pc->skip[0], ATTR_CTRL_MODE, VAL_INDICATOR);
	
	char *def_val;
	int def_len;
	GetCtrlAttribute(pc->skiptxt[1], pc->skiptxt[0], ATTR_DFLT_VALUE_LENGTH, &def_len);
	def_val = malloc(def_len+1);
	
	GetCtrlAttribute(pc->skiptxt[1], pc->skiptxt[0], ATTR_DFLT_VALUE, def_val);
	SetCtrlVal(pc->skiptxt[1], pc->skiptxt[0], def_val);
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
	if(uipc->nd) {
		if(uipc->ndins == 0) {			// No instructions actually vary.
			change = 1;
			toggle_nd();
		}
		
		// Iterate through the instructions which do (nominally) vary, and turn them off if they don't really.
		for(i = 0; i<uipc->ndins; i++) {
			ind = uipc->dim_ins[i];
			state = get_nd_state(ind);
			steps = uipc->dim_steps[uipc->ins_dims[i]];
			// First, if either of these is null, it may be that it was never evaluated, so check that.
			if(uipc->nd_delays == NULL || uipc->nd_delays[i] == NULL || uipc->nd_data == NULL || uipc->nd_data[i] == NULL) {
				if(state == 1) {
					update_nd_increment(ind, MC_INC);	
				} else if(state == 2) {
					update_nd_from_exprs(ind);
				}
			}
			
			// Check if delay variation is on.
			if(uipc->nd_delays == NULL || uipc->nd_delays[i] == NULL || constant_array_double(uipc->nd_delays[i], steps))
				delay_on = 0;
			else
				delay_on = 1;
			
			// Check if data variation is on
			if(uipc->nd_data == NULL || uipc->nd_data[i] == NULL || constant_array_int(uipc->nd_data[i], steps))
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
		for(i = 0; i<uipc->nd; i++) {
			for(j=0; j<uipc->ndins; j++) {
				if(uipc->ins_dims[j] == i)
					break;			// We've found an example of this dimension, so break
			}
			
			if(j < uipc->ndins)
				continue;
			
			// If we're at this point, we need to remove this dimension from the experiment
			change = 1;
			remove_array_item(uipc->dim_steps, i, uipc->nd); // Remove the steps.
			for(j=0; j<uipc->ndins; j++) {
				if(uipc->ins_dims[j] > i)
					uipc->ins_dims[j]--;
			}
			uipc->nd--;				// Update number of dimensions
			i--;					// We removed one, so drop it back one.
		}
		populate_dim_points();		// Update the UI
	} else {
		// I don't think this is possible, but this is the cleanup function, so it's not a bad idea.
		int ndon;
		GetCtrlVal(pc->ndon[1], pc->ndon[0], &ndon);
		if(ndon)
			toggle_nd();
	}
	
	
	// Now fix anything related to the phase cycling, if applicable.
	if(uipc->nc) {
		int step; 
		
		if(uipc->ncins == 0 || uipc->c_instrs == NULL) {
			change = 1;
			SetCtrlVal(pc->numcycles[1], pc->numcycles[0], 0);
			change_num_cycles();
		}
		
		// Iterate through the instructions which nominally vary and turn them off it they don't really.
		for(i = 0; i<uipc->ncins; i++) {
			// Retrieve the specific variables.
			ind = uipc->cyc_ins[i];
			cyc = uipc->ins_cycs[i];
			steps = uipc->cyc_steps[cyc];
			
			if(uipc->c_instrs[ind] == NULL) {
				change = 1;
				update_pc_state(ind, 0);
				i--;									// If we turn off an instruction, we need to decrement i
			} else {
				if(uipc->c_instrs[ind][0] == NULL) { 	// The only way that this is possible is if you never
					change = 1;						 	// changed the step away from the first one.
					update_pc_state(ind, 0);
					i--;
					continue;
				}
				
				// Before we move on, we need to update the uipc from the current UI.
				GetCtrlVal(pc->inst[ind], pc->pcstep, &step);
				PINSTR *instr = malloc(sizeof(PINSTR));
				get_instr(instr, ind);
				update_cyc_instr(ind, instr, step);
				free(instr);
				
				// If a user doesn't define a step in a cycle, just repeat the last cycle that they defined. 
				for(j = 1; j<steps; j++) {
					if(uipc->c_instrs[ind][j] == NULL)
						update_cyc_instr(ind, uipc->c_instrs[ind][j-1], j);
				}

				// Now just to be sure, check if the array is constant.
				if(constant_array_pinstr(uipc->c_instrs[ind], steps)) {
					change = 1;
					update_pc_state(ind, 0);
					i--;
				}
			}
		}
		
		// Now that we've turned off all the instructions that we don't need, we just need to fix the number of cycles
		for(i=0; i<uipc->nc; i++) {
			for(j=0; j<uipc->ncins; j++) {
				if(uipc->ins_cycs[j] == i)
					break;					
			}
			
			if(j < uipc->ncins)		   // We've found an example of this cycle, so we're good.
				continue;
			
			// If we're at this point, we need to remove the cycle from the expriment
			change = 1;
			remove_array_item(uipc->cyc_steps, i, uipc->nc);	// Remove the steps from the array
			for(j=0; j<uipc->ncins; j++) {
				if(uipc->ins_cycs[j] > i)
					uipc->ins_cycs[j]--;
			}
			uipc->nc--;			// Update number of cycles
			i--;				// We removed one, so decrement the counter.
		}
		
		populate_cyc_points();	// Update the UI.
	} else {
		// Probably not necessary, but just in case.
		int nc;
		GetCtrlVal(pc->numcycles[1], pc->numcycles[0], &nc);
		if(nc) {
			SetCtrlVal(pc->numcycles[1], pc->numcycles[0], 0);
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

/***************** Get Instruction Parameters ******************/  
void get_instr(PINSTR *instr, int num) {
	// Get the PINSTR represented by the i-th instruction.
	// This function will break if you pass it an instruction that shouldn't exist.
	// Output: instr.
	
	instr->flags = get_flags(num);					// Get flags
	
	GetCtrlVal(pc->inst[num], pc->instr, &instr->instr);				// The instruction
	GetCtrlVal(pc->inst[num], pc->instr_d, &instr->instr_data);			// The instruction data
	
	GetCtrlVal(pc->inst[num], pc->delay, &instr->instr_time);			// Instruction time
	GetCtrlIndex(pc->inst[num], pc->delayu, &instr->time_units);		// Time units

	GetCtrlVal(pc->inst[num], pc->scan, &instr->trigger_scan);
	
	//Instr_time should always be in units of nanoseconds for behind the scenes stuff.
	if(instr->time_units)
		instr->instr_time *= pow(1000, instr->time_units);
}


int get_flags(int num) {
	// Function for getting the flags for a given panel.
	int panel = pc->inst[num];
	int flags = 0, ttlon;
	
	for(int i = 0; i<24; i++) {
		GetCtrlVal(panel, pc->TTLs[i], &ttlon); // Figure out if it's on
		ttlon *= (int)pow(2, i); 			// Make this a binary flag
		flags += ttlon;						// Add it in to the flags
	}
	
	return flags;
}

/***************** Set Instruction Parameters ******************/ 
int set_instr(int num, PINSTR *instr) {
	// Pass this the program controls, instruction number and the instr and it sets the controls appropriately
	
	int ni;
	GetCtrlVal(pc->ninst[1], pc->ninst[0], &ni);			// For error checking
	if(num >= ni)											// Can't set an instruction if it's not there
		return -1;											// Error -> Instruction invalid
	
	int i, panel = pc->inst[num];							// Convenience
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
	for(i = 0; i<24; i++) {
		if(instr->flags & (int)pow(2, i))							// The flags are stored as a binary index, so 
			SetCtrlVal(panel, pc->TTLs[i], 1);					// in decimal they are weird, but a bitwise-AND
		else 												// reveals which indies are on.
			SetCtrlVal(panel, pc->TTLs[i], 0);

		SetCtrlAttribute(panel, pc->TTLs[i], ATTR_DIMMED, 0);	// Undim all the TTLs
	}
	
	SetCtrlIndex(panel, pc->instr, instr->instr);			// Set the instruction
	if(instr->instr >= 9) { 								// This is a compound function, not an atomic function
		int r_flags = get_reserved_flags(instr->instr); 	// Get the reserved flags
		SetCtrlAttribute(panel, pc->delay, ATTR_DIMMED, 1);	// Dim the unnecessary controls
		SetCtrlAttribute(panel, pc->delayu, ATTR_DIMMED, 1); 

		for(i = 0; i<24; i++) {
			if(r_flags & (int)pow(2, i))					// Dim the reserved TTLs
				SetCtrlAttribute(panel, pc->TTLs[i], ATTR_DIMMED, 1);
		}
	} else {
		SetCtrlAttribute(panel, pc->delay, ATTR_DIMMED, 0);	// Undim them if it's an atomic function
		SetCtrlAttribute(panel, pc->delay, ATTR_DIMMED, 0);
	}
	
	// Undim the instruction data control if it needs it, otherwise, set to 0 and dim it.
	if(takes_instr_data(instr->instr)) 
		SetCtrlAttribute(panel, pc->instr_d, ATTR_DIMMED, 0);
	else {
		SetCtrlAttribute(panel, pc->instr_d, ATTR_DIMMED, 1);
		SetCtrlVal(panel, pc->instr_d, 0);
	}
	
	// Set up the delay time - it's always saved in nanoseconds, so before you set the UI, adjust for units.
	double instr_time = instr->instr_time;
	if(instr->time_units)
		instr_time /= pow(1000, instr->time_units);
	
	SetCtrlVal(panel, pc->delay, instr_time);
	SetCtrlIndex(panel, pc->delayu, instr->time_units);
	
	// Deal with whether or not a scan is triggered now
	int trig_ttl;
	GetCtrlVal(pc->trig_ttl[1], pc->trig_ttl[0], &trig_ttl);// Get the trigger TTL.
		
	if(instr->trigger_scan) {
		SetCtrlVal(panel, pc->scan, 1);					
		SetCtrlVal(panel, pc->TTLs[trig_ttl], 1);			// Both need to be on or both off.
	} else {
		SetCtrlVal(panel, pc->scan, 0);
		SetCtrlVal(panel, pc->TTLs[trig_ttl], 0);
	}
	
	return 1;												// Success
}

void set_scan(int num, int state) {
	// Called when you try and toggle the scan button. Sets the val and the trigger ttl
	int trig;
	GetCtrlVal(pc->trig_ttl[1], pc->trig_ttl[0], &trig);
	
	SetCtrlVal(pc->inst[num], pc->scan, state);
	SetCtrlVal(pc->inst[num], pc->TTLs[trig], state);
}

void change_instr_units(int num) {
	// Function for changing the units of a given instruction.
	// Old units can be inferred from max value.
	int panel = pc->inst[num];
	int newunits;
	double max;
	
	// Minimum step size for spincore is 10ns, and apparently the number
	// of steps is stored in 31 bits, so the maximum single delay value
	// is (2^31)*10.
	const double m_val = 21474836480.0; 
	
	GetCtrlAttribute(panel, pc->delay, ATTR_MAX_VALUE, &max);
	GetCtrlIndex(panel, pc->delayu, &newunits);

	SetCtrlAttribute(panel, pc->delay, ATTR_MAX_VALUE, m_val/pow(1000, newunits));
}

void change_instr_delay(int num) {
	double val;
	int units, ndunits;
	int panel = pc->inst[num];
	
	GetCtrlVal(panel, pc->delay, &val);
	GetCtrlVal(panel, pc->delayu, &units);
	GetCtrlVal(pc->cinst[num], pc->delu_init, &ndunits);
	
	if(ndunits != units)
		val *= pow(1000, ndunits-units);
	
	SetCtrlVal(pc->cinst[num], pc->del_init, val);
	int state = get_nd_state(num);
	if(state == 1)
		update_nd_increment(num, MC_FINAL);
}

void change_instruction(int num) {
	// Dims instr_data if necessary and sets up the minimum
	int ind;
	int panel = pc->inst[num];
	
	GetCtrlVal(panel, pc->instr, &ind);
	
	// Change the corresponding ring control on the ND side.
	SetCtrlVal(pc->cinst[num], pc->cinstr, ind);
	
	// Set up the minimum
	int min = instr_data_min(ind);
	SetCtrlAttribute(panel, pc->instr_d, ATTR_MIN_VALUE, min);
  
	int nd_ctrls[4] = {pc->dat_init, pc->dat_inc, pc->dat_fin, pc->cexpr_data};
	int ndnum = 4;
	
	if(takes_instr_data(ind)) {
		if(!get_nd_state(num))
			SetCtrlAttribute(panel, pc->instr_d, ATTR_DIMMED, 0);
		change_control_mode(pc->cinst[num], nd_ctrls, ndnum, VAL_HOT);
	} else {
		SetCtrlVal(panel, pc->instr_d, 0);
		if(!get_nd_state(num))
			SetCtrlAttribute(panel, pc->instr_d, ATTR_DIMMED, 1);
		change_control_mode(pc->cinst[num], nd_ctrls, ndnum, VAL_INDICATOR);
	}
}

int move_ttl(int num, int to, int from) {
	// Move a TTL from "from" to "to", and shift all the others in response.
	// This is a "safe" function, and will skip broken ttls. If the initial or
	// final value is marked broken, the operation will fail and return -1.
	int panel = pc->inst[num];
	
	if(to == from)
		return 0;
	
	int high, low, buff1, buff2, i;
	if(to > from) {
		high = to;
		low = from;
	} else {
		high = from;
		low = to;
	}

	int broken_ttls = 0;
	for(i = low; i<high; i++) {
		GetCtrlAttribute(panel, pc->TTLs[i], ATTR_DIMMED, &buff1);
		if(buff1) 
			broken_ttls += (int)pow(2, i);
	}
	
	if(broken_ttls && ((int)pow(2, low) | (int)pow(2, high)) & broken_ttls)
		return -1;
	
	GetCtrlVal(panel, pc->TTLs[low], &buff1);
	i = low;
	int next = i;
	while(i<high) {
		next++;
		while(broken_ttls && broken_ttls & (int)pow(2, next)) { // Skip over any ttls that are broken.
			next++;
			if(next >= high)
				break;
		}
		
		GetCtrlVal(panel, pc->TTLs[next], &buff2);
		SetCtrlVal(panel, pc->TTLs[i], buff2);
		i = next;
	}
	
	return 0;
}
/************* Get ND and Phase Cycling Parameters *************/  
int get_nd_state(int num) {
	// If you don't know the state of the ND instruction, this function gets it for you.
	// State 0 = Off
	// State 1 = On/Red    	=> Linear delay/instr increment
	// State 2 = On/Blue   	=> Expression mode
	// State 3 = On/Green	=> List edit mode
	
	int val, state;
	GetCtrlVal(pc->cinst[num], pc->vary, &val);

	if(!val) {
		state = 0;
	} else { 
		int color;
		GetCtrlAttribute(pc->cinst[num], pc->vary, ATTR_ON_COLOR, &color);  
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
	GetCtrlVal(pc->inst[num], pc->pcon, &val);
	
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
		for(i=0; i<uipc->ncins; i++) {
			if(uipc->cyc_ins[i] == num) {
				ci = i;
				break;
			} else if (uipc->cyc_ins[i] > num)
				break;
		}
		
		if(cind != NULL)
			cind[0] = ci;
	} else {
		ci = -1;	
	}
	
	if(di < 0 && (mode & PP_V_ID)) {
		for(i=0; i<uipc->ndins; i++) {
			if(uipc->dim_ins[i] == num) {
				di = i;
				break;
			} else if (uipc->dim_ins[i] > num)
				break;
		}
		if(dind != NULL)
			dind[0] = di;
	} else {
		di = -1;
	}
	
	// If it's phase cycled, get the relevant instruction.
	if(ci >= 0) 
		copy_pinstr(uipc->c_instrs[num][cstep[uipc->ins_cycs[ci]]], instr);
	else
		get_instr(instr, num);
	
	// If it varies in some dimension, change either the data or delay or both.
	if(di >= 0) {
		int step = cstep[uipc->ins_dims[di]+uipc->nc];	// The step we're on.
		if(uipc->nd_data != NULL && uipc->nd_data[di] != NULL)
			instr->instr_data = uipc->nd_data[di][step];
		
		if(uipc->nd_delays != NULL && uipc->nd_delays[di] != NULL)
			instr->instr_time = uipc->nd_delays[di][step];
	}
}

/**************** Set ND Instruction Parameters ****************/  
void toggle_nd() {		// Toggles whether or not multi-dimensional instructions are on.
	int dimmed, val, nd, i;
	GetCtrlVal(pc->ndon[1], pc->ndon[0], &val);
	val = !val; // This is what it means to toggle something
	
	if(val) {
		dimmed = 0;
		change_num_dims();
	} else {
		dimmed = 1;
		uipc->nd = 0;
	}
	
	// Set the controls appropriately.
	SetCtrlVal(pc->ndon[1], pc->ndon[0], val);
	SetPanelAttribute(pc->PPConfigCPan, ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pc->ndims[1], pc->ndims[0], ATTR_DIMMED, dimmed);
	for(i = 0; i<uipc->max_ni; i++)
		SetPanelAttribute(pc->cinst[i], ATTR_DIMMED, dimmed);
	
	if(!uipc->nc) {
		SetCtrlAttribute(pc->skip[1], pc->skip[0], ATTR_DIMMED,dimmed);
		SetCtrlAttribute(pc->skiptxt[1], pc->skiptxt[0], ATTR_DIMMED, dimmed);
	}
}

void update_nd_state(int num, int state) {	// Changes the state of a given ND control.   
	// State 0 = Off
	// State 1 = On/Red    	=> Linear delay/instr increment
	// State 2 = On/Blue   	=> Expression mode
	// State == -1 -> Figure out what state we're in for us.
	
	int panel = pc->cinst[num];	// Convenience
	
	// Get the state if we need it
	if(state < 0) {
		state = get_nd_state(num);
	}
	
	// Set up the uipc variable.
	int ind = -1, i;			
	for(i=0; i<uipc->ndins; i++) {		// Get the place in the dim_ins array
		if(uipc->dim_ins[i] == num) {   // -1 if it's not there.
			ind = i;
			break;
		}
	}
	
	if(!state) {
		if(ind >=0) {
			// Remove the index from the lists and decrement the number of instructions
			remove_array_item(uipc->dim_ins, ind, uipc->ndins);
			remove_array_item(uipc->ins_state, ind, uipc->ndins);
			remove_array_item(uipc->ins_dims, ind, uipc->ndins);
			
			if(uipc->nd_data[ind] != NULL)
				free(uipc->nd_data[ind]);
			if(uipc->nd_delays[ind] != NULL)
				free(uipc->nd_delays[ind]);
			
			remove_array_item_void(uipc->nd_data, ind, uipc->ndins, 2);
			remove_array_item_void(uipc->nd_delays, ind, uipc->ndins, 3);
			
			// Now reallocate the size of the arrays
			if(--uipc->ndins == 0) {
				free(uipc->dim_ins);
				free(uipc->ins_state);
				free(uipc->ins_dims);
				free(uipc->nd_data);
				free(uipc->nd_delays);
				
				uipc->dim_ins = NULL;
				uipc->ins_state = NULL;
				uipc->ins_dims = NULL;
				uipc->nd_data = NULL;
				uipc->nd_delays = NULL;
			} else {
				uipc->dim_ins = realloc(uipc->dim_ins, sizeof(int)*uipc->ndins);
				uipc->ins_dims = realloc(uipc->ins_dims, sizeof(int)*uipc->ndins);
				uipc->ins_state = realloc(uipc->ins_state, sizeof(int)*uipc->ndins);
				uipc->nd_data = realloc(uipc->nd_data, sizeof(int*)*uipc->ndins);
				uipc->nd_delays = realloc(uipc->nd_delays, sizeof(double*)*uipc->ndins);	
			}
			
		}
	} else {
		if(ind < 0) {	// only need to update if we're not already there.
			uipc->ndins++;			// Increment the number of instructions varied 
			
			// Memory allocation is important.
			if(uipc->dim_ins == NULL)
				uipc->dim_ins = malloc(sizeof(int)*uipc->ndins);
			else
				uipc->dim_ins = realloc(uipc->dim_ins, sizeof(int)*uipc->ndins);
			
			if(uipc->ins_dims == NULL)
				uipc->ins_dims = malloc(sizeof(int)*uipc->ndins);
			else
				uipc->ins_dims = realloc(uipc->ins_dims, sizeof(int)*uipc->ndins);
			
			if(uipc->ins_state == NULL)
				uipc->ins_state = malloc(sizeof(int)*uipc->ndins);
			else
				uipc->ins_state = realloc(uipc->ins_state, sizeof(int)*uipc->ndins);
			
			if(uipc->nd_data == NULL)
				uipc->nd_data = malloc(sizeof(int*)*uipc->ndins);
			else
				uipc->nd_data = realloc(uipc->nd_data, sizeof(int*)*uipc->ndins);
			
			if(uipc->nd_delays == NULL)
				uipc->nd_delays = malloc(sizeof(double*)*uipc->ndins);
			else
				uipc->nd_delays = realloc(uipc->nd_delays, sizeof(double*)*uipc->ndins);
	
			// Need to update the list controls
			int nl;
			GetNumListItems(panel, pc->dim, &nl);
			if(nl < uipc->nd) {
				int elements;
				char **c = generate_char_num_array(1, uipc->nd, &elements);
				
				for(i=nl; i<uipc->nd; i++) 
					InsertListItem(panel, pc->dim, -1, c[i], i);
				
				if(c != NULL) {
					for(i = 0; i<elements; i++)
						free(c[i]);
					free(c);
				}
				
			} else if(nl > uipc->nd) 
				DeleteListItem(panel, pc->dim, uipc->nd, -1);
				  
			int dim;
			GetCtrlVal(panel, pc->dim, &dim);
			
			// Now insert our instruction at the end.
			uipc->ins_dims[uipc->ndins-1] = dim;
			uipc->dim_ins[uipc->ndins-1] = num;
			uipc->ins_state[uipc->ndins-1] = state;
			uipc->nd_data[uipc->ndins-1] = NULL; 
			uipc->nd_delays[uipc->ndins-1] = NULL; 
		}
	}
	
	
	// Set up the controls that need to be dimmed or hidden
	int inc_ctrls[4] = {pc->del_inc, pc->delu_inc,
						pc->dat_inc, pc->disp_inc};
	int exprs_ctrls[2] = {pc->cexpr_data, pc->cexpr_delay};
	int all_ctrls[10] = {pc->del_init, pc->delu_init, pc->dat_init, 
						 pc->disp_init, pc->disp_fin, pc->del_fin, 
						 pc->delu_fin, pc->dat_fin, pc->nsteps, pc->dim};
	int inc_num = 4, exprs_num = 2, all_num = 10, main_num = 3;
	int main_ctrls[3] = {pc->delay, pc->instr_d, pc->delayu};
	int exprs = MC_HIDDEN, inc = MC_HIDDEN, main = MC_DIMMED, all = 0;
	int color = VAL_RED;	// This will change as necessary
	
	if(state > 0) {		// A varied state
		// If it's on, we want to switch the border to blue.
		SetCtrlVal(panel, pc->vary, 1);
		SetCtrlAttribute(pc->inst[num], pc->ins_num, ATTR_DISABLE_PANEL_THEME, 1);
	}
	
	// Dim/Hide the controls that need to be dimmed and/or hidden
 	if(state == 0) {
		// In the 0 state, we're off, so that should be dimmed time increment stuff
		SetCtrlAttribute(pc->inst[num], pc->ins_num, ATTR_FRAME_COLOR, 14737379);	// The default color, for whatever reason
		SetCtrlAttribute(pc->inst[num], pc->ins_num, ATTR_DISABLE_PANEL_THEME, 0);
		SetCtrlVal(panel, pc->vary, 1);
		
		main = 0;
		all = MC_DIMMED;
		inc = MC_DIMMED;
	} else if(state == 1) {
		inc = 0;		   // No color change needed
		SetCtrlAttribute(pc->inst[num], pc->ins_num, ATTR_FRAME_COLOR, VAL_RED);
	} else if(state == 2) {
		exprs = 0;
		color = VAL_BLUE;
		SetCtrlAttribute(pc->inst[num], pc->ins_num, ATTR_FRAME_COLOR, VAL_BLUE);
	}
	
	// Set up the visibility modes
	change_visibility_mode(pc->inst[num], main_ctrls, main_num, main);
	change_visibility_mode(panel, all_ctrls, all_num, all);
	change_visibility_mode(panel, inc_ctrls, inc_num, inc);
	change_visibility_mode(panel, exprs_ctrls, exprs_num, exprs);
	
	SetCtrlAttribute(panel, pc->vary, ATTR_ON_COLOR, color); // Change color to red.
	SetCtrlVal(panel, pc->vary, state);
}

void change_num_dims() { 	// Updates the number of dimensions in the experiment
	int i, j, nd;
	GetCtrlVal(pc->ndims[1], pc->ndims[0], &nd);
	if(--nd == uipc->nd)
		return;
	
	// Update the UI elements
	if(uipc->ndins) {
		char **c = NULL;
		int elements = 0;
		if(nd > uipc->nd)
		c = generate_char_num_array(1, nd, &elements);
		
		// We are going to be updating uipc as we go, so we want a local copy so that as we
		// iterate through it we don't end up skipping instructions and such
		int ndins = uipc->ndins;
		int *ins_dims = malloc(sizeof(int)*ndins), *dim_ins = malloc(sizeof(int)*ndins);
		for(i = 0; i<ndins; i++) {
			ins_dims[i] = uipc->ins_dims[i];
			dim_ins[i] = uipc->dim_ins[i];
		}
		
		for(i = 0; i<ndins; i++) {
			if(ins_dims[i] >= nd) {
				update_nd_state(dim_ins[i], 0);
				continue;
			}
		
			if(nd < uipc->nd) {
				DeleteListItem(pc->cinst[dim_ins[i]], pc->dim, nd, -1); // Delete the rest of them.
			} else {
				for(j=uipc->nd; j<nd; j++)
					InsertListItem(pc->cinst[dim_ins[i]], pc->dim, -1, c[j], j);
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
	uipc->nd = nd;
	if(uipc->max_nd < nd) {
		if(uipc->dim_steps == NULL)
			uipc->dim_steps = malloc(sizeof(int)*nd);
		else
			uipc->dim_steps = realloc(uipc->dim_steps, sizeof(int)*nd);
		
		for(i=uipc->max_nd; i<nd; i++)
			uipc->dim_steps[i] = 2;
		
		uipc->max_nd = nd;
	}
}

void change_num_dim_steps(int dim, int steps) { // Updates number of steps in the given dimension
	int i, j, nl, cp;
	
	if (steps == uipc->dim_steps[dim])
		return;	// No change
	
	// Update the controls
	for(i = 0; i<uipc->ndins; i++) {
		if(uipc->ins_dims[i] == dim)
			change_nd_steps_instr(uipc->dim_ins[i], steps);
	}
	
	// Now update the uipc variable and we're done.
	uipc->dim_steps[dim] = steps;
 
}

void change_nd_steps_instr(int num, int steps) { // Changes the number of steps for a given instr
	int panel = pc->cinst[num];
	
	SetCtrlVal(panel, pc->nsteps, steps);
	
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
	int panel = pc->cinst[num];
	
	// Pretty much all you need to do is update uipc (for now)
	GetCtrlVal(panel, pc->dim, &dim);		// Get the new dimension
	
	for(i=0; i<uipc->ndins; i++) {
		if(uipc->dim_ins[i] == num)
			break;
	}
	
	if(i == uipc->ndins) 
		return;				// Something's wrong, return.
	
	
	ind = i;
	// Update ins_dims and change the number of steps for the control.
	uipc->ins_dims[ind] = dim;

	change_nd_steps_instr(num, uipc->dim_steps[dim]);
}

void populate_dim_points() {	// Function for updating the UI with the values from the uipc var
	int j, nl, dim;
	
	// A char array of labels
	int elements;
	char **c = generate_char_num_array(1, uipc->nd, &elements);
	
	int panel;
	for(int i = 0; i<uipc->ndins; i++) {
		panel = pc->cinst[uipc->dim_ins[i]];

		// First make the number of dimensions per control correct
		GetNumListItems(panel, pc->dim, &nl);
		if(nl < uipc->nd) {
			for(j=nl; j<uipc->nd; j++)
				InsertListItem(panel, pc->dim, -1, c[j], j);
		} else if (nl > uipc->nd) {
			DeleteListItem(panel, pc->dim, uipc->nd, -1);
		}
		
		// Now update the number of steps.
		dim = uipc->ins_dims[i];
		SetCtrlIndex(panel, pc->dim,dim);		// In case the dimension has changed
		SetCtrlVal(panel, pc->nsteps, uipc->dim_steps[dim]);	
	}
	
	for(j=0; j<elements; j++)
		free(c[j]);
	free(c);
	
	// Now update the number of dimensions
	int nd;
	GetCtrlVal(pc->ndims[1], pc->ndims[0], &nd);
	if(uipc->nd < 1) {
		SetCtrlVal(pc->ndims[1], pc->ndims[0], 2);
		toggle_nd();
	} else if (--nd != uipc->nd) {
		SetCtrlVal(pc->ndims[1], pc->ndims[0], uipc->nd+1);
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
	int panel = pc->cinst[num]; // Convenience
	
	// Get the values that are there now.
	GetCtrlVal(panel, pc->del_init, &init);
	GetCtrlVal(panel, pc->del_inc, &inc);
	GetCtrlVal(panel, pc->del_fin, &fin);
	GetCtrlVal(panel, pc->delu_init, &initu);
	GetCtrlVal(panel, pc->delu_inc, &incu);
	GetCtrlVal(panel, pc->delu_fin, &finu);
	GetCtrlVal(panel, pc->cinstr, &instr);
	
	// Convert everything to nanoseconds.
	init *= pow(1000, initu);
	inc *= pow(1000, incu);
	fin *= pow(1000, finu);
	
	// Get the dimension it varies along
	GetCtrlVal(panel, pc->nsteps, &steps);

	 // First the delays
	steps--; 	// Temporary decrement, because the step indexing is 0-based.
	switch(mode) {
		case MC_INIT:
			init = fin-(inc*steps);
			
			if(init < 0) {		// Init can't be negative.
				init = 0;
				fin = steps*inc;
			}
			break;
		case MC_INC:
			inc = (fin-init)/steps;
			break;
		case MC_FINAL:
			fin = init+(inc*steps);
			
			if(fin < 0) {		// Only possible when inc is negative.
				fin = 0;
				init = -inc*steps;
			}
			break;
	}
	steps++;
	
	// Update the uipc variable.
	int ind, i;
	for(i=0; i<uipc->ndins; i++) {
		if(uipc->dim_ins[i] == num)
			break;
	}
	if(i < uipc->ndins)
		ind = i;
	else
		ind = -1;
	
	if(ind >= 0) {
		if(uipc->nd_delays[ind] == NULL) 
			uipc->nd_delays[ind] = malloc(sizeof(double)*steps);
		else
			uipc->nd_delays[ind] = realloc(uipc->nd_delays[ind], sizeof(double)*steps);
	
		for(i=0; i<steps; i++) 
			uipc->nd_delays[ind][i] = init+inc*i;
	}
	
	// Convert back to the appropriate units
	init /= pow(1000, initu);
	inc /= pow(1000, incu);
	fin /= pow(1000, finu);
	
	// Set the controls to the new values
	SetCtrlVal(panel, pc->del_init, init);
	SetCtrlVal(panel, pc->del_inc, inc);
	SetCtrlVal(panel, pc->del_fin, fin);
	
	// Now if necessary, update the data increment controls
	if(takes_instr_data(instr)) {
		int initd, incd, find;	// Data

		GetCtrlVal(panel, pc->dat_init, &initd);
		GetCtrlVal(panel, pc->dat_inc, &incd);
	 	GetCtrlVal(panel, pc->dat_fin, &find);
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
		if(ind >= 0) {
			if(uipc->nd_delays[ind] == NULL) 
				uipc->nd_data[ind] = malloc(sizeof(int)*steps);
			else
				uipc->nd_data[ind] = realloc(uipc->nd_data[ind], sizeof(int)*steps);
	
			for(int i=0; i<steps; i++) 
				uipc->nd_data[ind][i] = initd+incd*i;
		}
		
		SetCtrlVal(panel, pc->dat_init, initd);
		SetCtrlVal(panel, pc->dat_inc, incd);
	 	SetCtrlVal(panel, pc->dat_fin, find);
		
	}
}


void update_nd_from_exprs(int num) {// Generate the experiment from the exprs (state == 2)
	// Return value is 0 or a positive error.
	int state = get_nd_state(num);
	int i, *cstep = malloc(sizeof(int)*(uipc->nc+uipc->nd));
	int panel = pc->cinst[num];
	int dim, ind;
	
	for(i=0; i<uipc->ndins; i++) {
		if(uipc->dim_ins[i] == num)
			break;
	}
	
	if(i== uipc->ndins) // Index not found
		return;
	ind = i;
	
	int eval_data = 0, eval_delay = 0; // Whether or not to evaluate these things.
	int instr, len_data, len_delays, len_default;
	char *expr_delay, *expr_data, *default_val;
	
	GetCtrlVal(pc->inst[num], pc->instr, &instr);   // The instruction
	GetCtrlVal(panel, pc->dim, &dim);				// What dimension we're varying along.
	if(takes_instr_data(instr)) {
		GetCtrlValStringLength(panel, pc->cexpr_data, &len_data);
		GetCtrlAttribute(panel, pc->cexpr_data, ATTR_DFLT_VALUE_LENGTH, &len_default);
		
		expr_data = malloc(len_data+1);
		GetCtrlVal(panel, pc->cexpr_data, expr_data);
		
		default_val = malloc(len_default+1);
		GetCtrlAttribute(panel, pc->cexpr_data, ATTR_DFLT_VALUE, default_val);
		
		if(strcmp(default_val, expr_data) != 0)
			eval_data = 1;
		else
			free(expr_data);
		
		free(default_val);
	}
	
	GetCtrlValStringLength(panel, pc->cexpr_delay, &len_delays);
	GetCtrlAttribute(panel, pc->cexpr_delay, ATTR_DFLT_VALUE_LENGTH, &len_default);
	
	expr_delay = malloc(len_delays+1);
	GetCtrlVal(panel, pc->cexpr_delay, expr_delay);
	
	default_val = malloc(len_default+1);
	GetCtrlAttribute(panel, pc->cexpr_delay, ATTR_DFLT_VALUE, default_val);
	
	if(len_delays != 0 && strcmp(default_val, expr_delay) != 0)
		eval_delay = 1;
	else
		free(expr_delay);
	
	free(default_val);
	
	// At this point we know which one(s) to evaluate, and we have the expressions.
	
	if(!eval_delay && !eval_data)
		return;	// Nothing to do.
	
	// Set up the uipc var if necessary.
	int steps = uipc->dim_steps[dim];
	if(eval_delay) {
		if(uipc->nd_delays[ind] == NULL)
			uipc->nd_delays[ind] = malloc(sizeof(double)*steps);
		else
			uipc->nd_delays[ind] = realloc(uipc->nd_delays[ind], sizeof(double)*steps);
	}
	
	if(eval_data) {
		if(uipc->nd_data[ind] == NULL)
			uipc->nd_data[ind] = malloc(sizeof(int)*steps);
		else
			uipc->nd_data[ind] = realloc(uipc->nd_data[ind], sizeof(int)*steps);
	}
	
	
	// Build the basic cstep.
	for(i = 0; i<(uipc->nc+uipc->nd); i++)
		cstep[i] = 0;	// Just set everything we don't care about to the first one, it's not important
	
	
	int err_del = 0, err_dat = 0;			// Error values
	constants *c = setup_constants(); 		// Initializes the static constants
	
	// Now some static constants unique to this type of evaluation.
	double init, fin;
	int initu, finu;
	GetCtrlVal(panel, pc->del_init, &init);
	GetCtrlVal(panel, pc->delu_init, &initu);
	GetCtrlVal(panel, pc->del_fin, &fin);
	GetCtrlVal(panel, pc->delu_fin, &finu);
	
	init *= pow(1000, initu);
	fin *= pow(1000, finu);
	
	add_constant(c, "init", C_DOUBLE, &init);
	add_constant(c, "fin", C_DOUBLE, &fin);
	
	for(i=0; i<steps; i++) {
		cstep[dim]++;
		update_constants(c, cstep);
		
		// We also will add both "x" and "step" here.
		change_constant(c, "x", C_INT, &i);
		change_constant(c, "step", C_INT, &i);
		
		if(eval_delay && !err_del) {
			uipc->nd_delays[ind][i] = parse_math(expr_delay, c, &err_del, 0);
			
			if(uipc->nd_delays[ind][i] < 0) 
				err_del = get_parse_error(-1, NULL)+1;		// The parse errors have higher precedence
			
			if(err_del) {
				uipc->err_del_size = uipc->nc+uipc->nd;
				if(uipc->err_del_pos != NULL) // Save the cstep
					uipc->err_del_pos = realloc(uipc->err_del_pos, sizeof(int)*uipc->err_del_size);
				else
					uipc->err_del_pos = malloc(sizeof(int)*uipc->err_del_size);
				
				for(int j=0; j<uipc->err_del_size; j++)
					uipc->err_del_pos[j] = cstep[j];

				if(!eval_data || err_dat)
					break;
			}
		}
		
		if(eval_data && !err_dat) {
			uipc->nd_data[ind][i] = (int)parse_math(expr_data, c, &err_dat, 0);
			
			if(uipc->nd_data[ind][i] < 0) 
				err_dat = get_parse_error(-1, NULL)+1;
			
			if(err_dat) {
				uipc->err_dat_size = uipc->nc+uipc->nd;
				if(uipc->err_dat_pos != NULL) // Save the cstep
					uipc->err_dat_pos = realloc(uipc->err_dat_pos, sizeof(int)*uipc->err_dat_size);
				else
					uipc->err_dat_pos = malloc(sizeof(int)*uipc->err_dat_size);
				
				for(int j=0; j<uipc->err_dat_size; j++)
					uipc->err_dat_pos[j] = cstep[j];
				
				if(!eval_delay || err_del)
					break;
			}
		}
	}
	
	// If the evaluation was unsuccessful, the border should turn red.
	// If it was successful, the border should turn green, otherwise it is
	// switched back to the windows style with no specific border color.
	
	int dat_bgcolor = VAL_WHITE, del_bgcolor = VAL_WHITE;
	int dat_bold = 0, del_bold = 0;
	int dat_textcolor = VAL_BLACK, del_textcolor = VAL_BLACK;
	if(eval_delay) {
		free(expr_delay);
		
		del_bold = 1;
		
		if(uipc->err_del = err_del) {			// Not a typo, I'm actually setting the
			del_bgcolor = VAL_RED;			    // value of the uipc error field here.
			del_textcolor = VAL_OFFWHITE;
		} else 
			del_bgcolor = VAL_GREEN;
	} 
	
	if(eval_data) {
		free(expr_data);
		
		dat_bold = 1;
		
		if(uipc->err_dat = err_dat) {			// Again, not a typo.
			dat_bgcolor = VAL_RED;
			dat_textcolor = VAL_OFFWHITE;
		} else 
			dat_bgcolor = VAL_GREEN;
	} 
	
	// Update the UI Controls
	SetCtrlAttribute(panel, pc->cexpr_delay, ATTR_TEXT_BGCOLOR, del_bgcolor);
	SetCtrlAttribute(panel, pc->cexpr_delay, ATTR_TEXT_BOLD, del_bold);
	SetCtrlAttribute(panel, pc->cexpr_delay, ATTR_TEXT_COLOR, del_textcolor);
	
	SetCtrlAttribute(panel, pc->cexpr_data, ATTR_TEXT_BGCOLOR, dat_bgcolor);
	SetCtrlAttribute(panel, pc->cexpr_data, ATTR_TEXT_BOLD, dat_bold);
	SetCtrlAttribute(panel, pc->cexpr_data, ATTR_TEXT_COLOR, dat_textcolor);
	
	return;
}

void update_skip_condition() { 		// Generate uipc->skip_locs from the UI.
	if(uipc->nd == 0 && uipc->nc == 0)
		return;						// Nothing to skip
									   
	char *expr;
	int len, i, size=uipc->nd+uipc->nc;
	int pan = pc->skiptxt[1];
	int ctrl = pc->skiptxt[0];
	
	// Get the expression string.
	GetCtrlValStringLength(pan, ctrl, &len);
	
	if(len == 0)
		return;						// Nothing to evaluate
	
	ui_cleanup(0);					// Clean up before evaluation.
	
	expr = malloc(len+1);
	GetCtrlVal(pan, ctrl, expr);
	
	// Get max_n_steps and maxsteps;
	int *maxsteps = malloc(sizeof(int)*size);
	uipc->max_n_steps = 1;
	for(i=0; i<uipc->nc; i++) {
		uipc->max_n_steps*=uipc->cyc_steps[i];
		maxsteps[i] = uipc->cyc_steps[i];
	}
	
	for(i=0; i<uipc->nd; i++) { 
		uipc->max_n_steps*=uipc->dim_steps[i];
		maxsteps[i+uipc->nc] = uipc->dim_steps[i];
	}
	
	// Allocate space for skip_locs. One for each step. We'll use an
	// unsigned char array since bools aren't a thing in C
	int max_n_steps = uipc->max_n_steps;
	if(max_n_steps == 0) {
		free(maxsteps);
		free(expr);
		return;
	}
	
	if(uipc->skip_locs == NULL)
		uipc->skip_locs = malloc(sizeof(unsigned char)*max_n_steps);
	else
		uipc->skip_locs = realloc(uipc->skip_locs, sizeof(unsigned char)*max_n_steps);
	
	// Set up the static constants
	constants *c = setup_constants();
	
	// Iterate through each step in the experiment and generate the skips
	int err;
	uipc->real_n_steps = max_n_steps;				// Assume we didn't skip anything at first.
	int *cstep = malloc(sizeof(int)*size);
	for(i=0; i<max_n_steps; i++) {
		get_cstep(i, cstep, maxsteps, size); 	// Convert from linear index to cstep.
		
		update_constants(c, cstep);				// Update the dynamic constants (variables)
		
		double val = parse_math(expr, c, &err, 0);
		
		if(val) {
			uipc->skip_locs[i] = 1;
			uipc->real_n_steps--;
		} else
			uipc->skip_locs[i] = 0;
		
		if(err)
			break;
	}
	
	// The new values for the UI controls.
	int bg = VAL_GREEN, txt = VAL_BLACK;
	
	// Update the control if there was an error.
	if(uipc->skip_err = err) {		// Not a typo, I'm setting uipc->skip_err to err.
		bg = VAL_RED;
		txt = VAL_OFFWHITE;
		
		uipc->skip_err_size = size;
		if(uipc->skip_err_pos == NULL)
			uipc->skip_err_pos = malloc(sizeof(int)*size);
		else
			uipc->skip_err_pos = realloc(uipc->skip_err_pos, sizeof(int)*size);
		
		for(i=0; i<size; i++)
			uipc->skip_err_pos[i] = cstep[i];
		
		uipc->real_n_steps = max_n_steps; 	// If we fucked it up, there will be no skipping.
	}
	
	SetCtrlAttribute(pan, ctrl, ATTR_TEXT_BOLD, 1);
	SetCtrlAttribute(pan, ctrl, ATTR_TEXT_BGCOLOR, bg);
	SetCtrlAttribute(pan, ctrl, ATTR_TEXT_COLOR, txt);
	
	if(err)
		SetCtrlAttribute(pc->skip[1], pc->skip[0], ATTR_CTRL_MODE, VAL_INDICATOR);
	else
		SetCtrlAttribute(pc->skip[1], pc->skip[0], ATTR_CTRL_MODE, VAL_HOT);
	
	free(expr);
	free(maxsteps);
	free(cstep);
	
}

/**************** Set Phase Cycling Parameters *****************/ 

void update_pc_state(int num, int state) {
	// Right now there are only two states, on and off.
	int all_ctrls[3] = {pc->pcsteps, pc->pclevel, pc->pcstep};
	int all_num = 3, all;
	int panel = pc->inst[num];
	int i, ind = -1;

	// Find out if it's already in the list.
	if(uipc->cyc_ins != NULL) {
		for(i = 0; i<uipc->ncins; i++) {
			if(uipc->cyc_ins[i] == num) {
				ind = i;
				break;
			}
		}
	}
	
	if(!state) {
		all = MC_DIMMED;
		if(ind>=0) {
			// Remove the index from the lists.
			remove_array_item(uipc->cyc_ins, ind, uipc->ncins);
			remove_array_item(uipc->ins_cycs, ind, uipc->ncins);
			
			// Now we're going to decrement ncins and free the memory from those lists.
			if(--uipc->ncins <= 0) {
				free(uipc->cyc_ins);
				free(uipc->ins_cycs);
				
				uipc->cyc_ins = NULL;
				uipc->ins_cycs = NULL;
			} else {
				uipc->cyc_ins = realloc(uipc->cyc_ins, sizeof(int)*uipc->ncins);
				uipc->ins_cycs = realloc(uipc->ins_cycs, sizeof(int)*uipc->ncins);
			}
		}
	} else {
		all = 0;
		// If we haven't turned on phase cycling yet, add a cycle
		if(!uipc->nc) {
			SetCtrlVal(pc->numcycles[1], pc->numcycles[0], 1);
			change_num_cycles();
		}
		
		if(ind < 0) {  	// If this isn't true, that's an error (for now).
			// Populate the rings.
			int nl;
			GetNumListItems(panel, pc->pclevel, &nl);
			int elements;
			char **c;
			
			if(uipc->nc > nl) {
				c = generate_char_num_array(1, uipc->nc, &elements);
			
				for(i = nl; i<uipc->nc; i++) 
					InsertListItem(panel, pc->pclevel, -1, c[i], i); 	// Insert the cycles
				
				if(c != NULL) {
					for(i = 0; i<elements; i++)
						free(c[i]);
					free(c);
				}
			} else if(uipc->nc < nl) 
				DeleteListItem(panel, pc->pclevel, uipc->nc, -1);
			
			int cyc;
			GetCtrlVal(panel, pc->pclevel, &cyc);
		
			GetNumListItems(panel, pc->pcstep, &nl);
			if(nl > uipc->cyc_steps[cyc]) 
				DeleteListItem(panel, pc->pcstep, nl, -1);
			else if(nl < uipc->cyc_steps[cyc]) {
				c = generate_char_num_array(1, uipc->cyc_steps[cyc], &elements);

				for(i = nl; i<uipc->cyc_steps[cyc]; i++) 
					InsertListItem(panel, pc->pcstep, -1, c[i], i);	
				
				if(c != NULL) {
					for(i = 0; i<elements; i++)
						free(c[i]);	
					free(c);
				}
			}
			
			// Now we need to update the uipc var.
			uipc->ncins++;
			if(uipc->cyc_ins == NULL)
				uipc->cyc_ins = malloc(uipc->ncins*sizeof(int));
			else
				uipc->cyc_ins = realloc(uipc->cyc_ins, uipc->ncins*sizeof(int));
			
			if(uipc->ins_cycs == NULL)
				uipc->ins_cycs = malloc(uipc->ncins*sizeof(int));
			else
				uipc->ins_cycs = realloc(uipc->ins_cycs, uipc->ncins*sizeof(int));
			
			uipc->ins_cycs[uipc->ncins-1] = cyc;
			uipc->cyc_ins[uipc->ncins-1] = num;
		} else {
			all = MC_DIMMED;
		}
	}
	
	change_visibility_mode(panel, all_ctrls, all_num, all);
	SetCtrlVal(panel, pc->pcon, state);
}

void change_num_cycles() {
	// Function called if the cycle number control is changed.
	int nc, i, j;
	GetCtrlVal(pc->numcycles[1], pc->numcycles[0], &nc);
	
	if(nc == uipc->nc)
		return;
	
	// Update the UI elements
	if(uipc->ncins) {
		char **c = NULL;
		int elements = 0;
		if(nc > uipc->nc)
		c = generate_char_num_array(1, nc, &elements);

		// We are going to be updating uipc as we go, so we want a local copy so that as we
		// iterate through it we don't end up skipping instructions and such
		int ncins = uipc->ncins;
		int *ins_cycs = malloc(sizeof(int)*ncins), *cyc_ins = malloc(sizeof(int)*ncins);
		for(i=0; i<ncins; i++){
			ins_cycs[i] = uipc->ins_cycs[i];
			cyc_ins[i] = uipc->cyc_ins[i];
		}
	
		for(j = 0; j<ncins; j++) {
			if(ins_cycs[j] >= nc) {
				update_pc_state(cyc_ins[j], 0);
				continue;
			}
		
			if(nc < uipc->nc) {
				DeleteListItem(pc->inst[cyc_ins[j]], pc->pclevel, nc, -1);	// Delete the rest of them	
			} else {
				for(i=uipc->nc; i<nc; i++) 
					InsertListItem(pc->inst[cyc_ins[j]], pc->pclevel, -1, c[i], i);
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
	if(uipc->c_instrs == NULL) {
		uipc->c_instrs = malloc(sizeof(PINSTR**)*uipc->max_ni);
		uipc->max_cinstrs = malloc(sizeof(int)*uipc->max_ni);
		for(i = 0; i<uipc->max_ni; i++) {
			uipc->c_instrs[i] = NULL;	// Null initialization
			uipc->max_cinstrs[i] = 0;	// -1 initialization
		}
	}

	// Update the uipc variable
	uipc->nc = nc;
	if(uipc->max_nc < nc) {								// We keep everything allocated as a form
		if(uipc->cyc_steps == NULL)						// of history, so there's no need to do  
			uipc->cyc_steps = malloc(sizeof(int)*nc);	// this unless you go past the highest
		else											// value we've ever assigned
			uipc->cyc_steps = realloc(uipc->cyc_steps, sizeof(int)*nc);

		for(i=uipc->max_nc; i<nc; i++) 
			uipc->cyc_steps[i] = 2;

		uipc->max_nc = nc;
	}
	
	int dimmed = 0;
	if(!uipc->nc && !uipc->nd) 
		dimmed = 1;
	
	SetCtrlAttribute(pc->skip[1], pc->skip[0], ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pc->skiptxt[1], pc->skiptxt[0], ATTR_DIMMED, dimmed);
	
}

void change_cycle(int num) {
	// Updates controls and such when you switch from one cycle to another
	int nl, cyc;
	int panel = pc->inst[num];

	GetNumListItems(panel, pc->pclevel, &nl);
	if(nl < 1)
		return;	// Some kind of weird error.
	
	// First we'll update the uipc.
	GetCtrlVal(panel, pc->pclevel, &cyc);		// Should be 0-based index
	int i;
	for(i = 0; i<uipc->ncins; i++) {
		if(uipc->cyc_ins[i] == num) {			// Find the place in the cyc_ins array
			if(uipc->ins_cycs[i] == cyc)
				return;							// No change
			uipc->ins_cycs[i] = cyc;
			break;
		}
	}
	
	GetNumListItems(panel, pc->pcstep, &nl);	// Need to know how many to add or remove
	
	int steps = uipc->cyc_steps[cyc];
	if(steps == nl)								// It's all good.
		return;
	
	// The easy one, since -1 deletes everything from steps on.
	if(steps < nl) {
		DeleteListItem(panel, pc->pcstep, steps, -1);
	}
	
	// The harder one, need to add some instructions
	if(steps > nl) {
		int elements;
		char **c = generate_char_num_array(nl+1, steps, &elements);
		for(i=0; i<elements; i++) {
			InsertListItem(panel, pc->pcstep, -1, c[i], i+nl);
			free(c[i]);
		}
		free(c);
	}
	
	if(nl == 0) {
		SetCtrlIndex(panel, pc->pcstep, 0);
		SetCtrlAttribute(panel, pc->pcstep, ATTR_DFLT_INDEX, 0);
	}
}

void change_cycle_step(int num) {
	// Feed this an instruction number and it will update uipc and
	// update the phase cycle accordingly.
	
	int to, from, nl;
	int panel = pc->inst[num];
	
	GetNumListItems(panel, pc->pcstep, &nl);
	if(nl == 0)
		return;		// Weird error.
	
	// The old value is stored as the default value so we know where it came from
	GetCtrlVal(panel, pc->pcstep, &to);
	GetCtrlAttribute(panel, pc->pcstep, ATTR_DFLT_INDEX, &from);
	
	if(from < 0)
		from = 0;
	
	// Now we need to make sure that the uipc->c_instrs array is up to date.
	int cyc = -1, i;
	for(i = 0; i<uipc->ncins; i++) {
		if(uipc->cyc_ins[i] == num) 
			cyc = uipc->ins_cycs[i];
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
	if(uipc->max_cinstrs[num] > to && uipc->c_instrs[num][to] != NULL)
		set_instr(num, uipc->c_instrs[num][to]);
	
	// Now it's all done, so we just set the default index to the new index.
	SetCtrlAttribute(panel, pc->pcstep, ATTR_DFLT_INDEX, to);
	
	/************TO DO***************
	TODO: I should also implement a "check box" for what phase cycles have already
	been set up. Do this later.
	********************************/
}

void change_cycle_num_steps(int cyc, int steps) {
	// Function for changing the number of steps in a phase cycle.
	int i, j, nl, cp;

	int elements;
	char **c = NULL;
	if(steps > uipc->cyc_steps[cyc])
		c = generate_char_num_array(1, steps, &elements);
	
	for(i=0; i<uipc->ncins; i++) {
		if(uipc->ins_cycs[i] != cyc)
			continue;
		
		cp = pc->inst[uipc->cyc_ins[i]];
		
		if(steps < uipc->cyc_steps[cyc]) {
			GetNumListItems(cp, pc->pcstep, &nl);
			if(nl >= steps)
				DeleteListItem(cp, pc->pcstep, steps, -1);
		} else {
			GetNumListItems(cp, pc->pcstep, &nl);
			for(j=nl; j<steps; j++)
				InsertListItem(cp, pc->pcstep, -1, c[j], j);
		}
		
		SetCtrlVal(cp, pc->pcsteps, steps);
	}
	
	if(c != NULL) {
		for(i = 0; i<elements; i++)
			free(c[i]);
		free(c);
	}
	
	uipc->cyc_steps[cyc] = steps;
}

void update_cyc_instr(int num, PINSTR *instr, int step) {
	// Updates the uipc variable for instruction number "num" at step "step"
	// with the instruction you feed it. Make sure that max_cinstrs is set
	// before you use this function. 
	int i;
   											  
	// Make this array the right size.
	if(uipc->c_instrs[num] == NULL)
		uipc->c_instrs[num] = malloc(sizeof(PINSTR*)*(step+1));
	else if (uipc->max_cinstrs[num] <= step) 
		uipc->c_instrs[num] = realloc(uipc->c_instrs[num], sizeof(PINSTR*)*(step+1));
	
	if(step >= uipc->max_cinstrs[num]) {					// Matches exclusively both of
		for(i = uipc->max_cinstrs[num]; i<=step; i++)   	// the above conditions.
			uipc->c_instrs[num][i] = NULL;
		
		uipc->max_cinstrs[num] = step+1;
	}
	
	// If it's never been allocated, allocate it.
	if(uipc->c_instrs[num][step] == NULL)
		uipc->c_instrs[num][step] = malloc(sizeof(PINSTR));
	
	// Finally just copy the instr into the uipc array.
	copy_pinstr(instr, uipc->c_instrs[num][step]);
	
}

void populate_cyc_points()
{	 
	// Function for updating the UI with the values from the uipc var
	int j, on, nl, cyc;
	
	// A char array of labels
	int elements;
	char **c = generate_char_num_array(1, uipc->nc, &elements); 

	int panel;
	for(int i = 0; i<uipc->ncins; i++) {
		panel = pc->inst[uipc->cyc_ins[i]];
		
		// First make the number of dimensions per control correct
		GetNumListItems(panel, pc->pclevel, &nl);
		if(nl < uipc->nc) {
			for(j=nl; j<uipc->nc; j++)
				InsertListItem(panel, pc->pclevel, -1, c[j], j);
		} else if (nl > uipc->nc) {
			DeleteListItem(panel, pc->pclevel, uipc->nc, -1);
		}
		
		// Now update the number of steps.
		cyc = uipc->ins_cycs[i];				// In case the cycle changed
		SetCtrlIndex(panel, pc->pclevel, cyc);
		SetCtrlVal(panel, pc->pcsteps, uipc->cyc_steps[cyc]);
	}
	
	for(j=0; j<elements; j++)
		free(c[j]);
	free(c);
	
	// Now update the number of cycles
	int nc;
	GetCtrlVal(pc->numcycles[1], pc->numcycles[0], &nc);
	if(uipc->nc <0) {	// Error check
		uipc->nc = 0;
	}
	
	if (nc != uipc->nc) {
		SetCtrlVal(pc->numcycles[1], pc->numcycles[0], uipc->nc);
		change_num_cycles();
	}
}

/****************** Instruction Manipulation *******************/ 
int move_instruction(int to, int from)
{
	// Moves an instruction somewhere else in the list and shifts everything around
	// accordingly.
	int diff = (int)fabs(to-from) + 1, inst_buffer[diff], inst_top[diff], cinst_buffer[diff], cinst_top[diff], i, start;
	
	if(to == from)
		return 0;
	
	if(to<from)
	{
		inst_buffer[0] = pc->inst[from];
		cinst_buffer[0] = pc->cinst[from];
		start = to;
		
		for (i = 1; i<diff; i++)
		{
			inst_buffer[i] = pc->inst[to+i-1];
			cinst_buffer[i] = pc->cinst[to+i-1];
		}
	}
	else
	{
		inst_buffer[diff-1] = pc->inst[from];
		cinst_buffer[diff-1] = pc->cinst[from];
		start = from;
														;
		for (i = 0; i<diff-1; i++)
		{
			inst_buffer[i] = pc->inst[from+i+1];
			cinst_buffer[i] = pc->cinst[from+i+1];
		}
	}
	
	// Check if it's in a loop before and after, and if there's a change, then you shoul
	int ninstructions = uipc->ni;
	int loop_locations[ninstructions][3]; // An array with all the loop locations and their corresponding end-points.
	int j = 0, ins, end_ins;
	for(i = 0; i<ninstructions; i++) {
		GetCtrlVal(pc->inst[i], pc->instr, &ins);
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
			SetCtrlVal(pc->inst[c_e], pc->instr_d, to);
		} else if (c_l > to) {
			SetCtrlVal(pc->inst[c_e], pc->instr_d, c_l+1);
		} else if (c_l > from) {
			SetCtrlVal(pc->inst[c_e], pc->instr_d, c_l-1);
		}

	}
	
	// Update the uipc var.
	int end = start+diff;
	// ND Instructions
	for(i = 0; i<uipc->ndins; i++) {
		ins = uipc->dim_ins[i];
		if(ins < start || ins > end)
			continue;
		
		if(ins == from)
			uipc->dim_ins[i] = to;
		else if (to > from)
			uipc->dim_ins[i]++;
		else if (from < to)
			uipc->dim_ins[i]--;
	}
	
	// Phase cycled instructions
	for(i = 0; i<uipc->ncins; i++) {
		ins = uipc->cyc_ins[i];
		if(ins < start || ins > end)
			continue;
		
		if(ins == from)
			uipc->cyc_ins[i] = to;
		else if (to > from)
			uipc->cyc_ins[i]--;
		else if (to < from)
			uipc->cyc_ins[i]++;
	}

	// Now the actual moving and updating of things.
	for (i = 0; i<diff; i++)
	{
		GetPanelAttribute(pc->inst[start+i], ATTR_TOP, &inst_top[i]);
		GetPanelAttribute(pc->cinst[start+i], ATTR_TOP, &cinst_top[i]);
	}
	
	for(i = 0; i<diff; i++)
	{ 
		pc->inst[start+i] = inst_buffer[i];
		pc->cinst[start+i] = cinst_buffer[i];
		SetPanelAttribute(pc->inst[start+i], ATTR_TOP, inst_top[i]);
		SetPanelAttribute(pc->cinst[start+i], ATTR_TOP, cinst_top[i]);
		SetCtrlVal(pc->inst[start+i], pc->ins_num, start+i);
		SetCtrlVal(pc->cinst[start+i], pc->cins_num, start+i);
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
			//	update_instr_data_nd(pc->cinst[c_l], 2); // This will update the times.
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
	
	if(uipc->max_cinstrs != NULL)
	{
		if(uipc->c_instrs[num] != NULL) {
			for(int i = 0; i<uipc->max_cinstrs[num]; i++) {
				if(uipc->c_instrs[num][i] != NULL) {
					free(uipc->c_instrs[num][i]);
				}
			}
			free(uipc->c_instrs[num]);
			uipc->c_instrs[num] = NULL;
		}
		uipc->max_cinstrs[num] = 0;
	}
	
	// We also want to clear stored ND information.
	if(get_nd_state(num))
		update_nd_state(num, 0);
	
	// Clear the expression controls
	char *def_val;
	int def_len;

	// Reset to default values.
	GetCtrlAttribute(pc->cinst[num], pc->cexpr_data, ATTR_DFLT_VALUE_LENGTH, &def_len);
	def_val = malloc(def_len+1);
	
	GetCtrlAttribute(pc->cinst[num], pc->cexpr_data, ATTR_DFLT_VALUE, def_val);
	SetCtrlVal(pc->cinst[num], pc->cexpr_data, def_val);
	free(def_val);
	
	GetCtrlAttribute(pc->cinst[num], pc->cexpr_delay, ATTR_DFLT_VALUE_LENGTH, &def_len);
	def_val = malloc(def_len+1);
	
	GetCtrlAttribute(pc->cinst[num], pc->cexpr_delay, ATTR_DFLT_VALUE, def_val);
	SetCtrlVal(pc->cinst[num], pc->cexpr_delay, def_val);
	free(def_val);
	
	// Now reset the colors and boldness.
	SetCtrlAttribute(pc->cinst[num], pc->cexpr_delay, ATTR_TEXT_BGCOLOR, VAL_WHITE);
	SetCtrlAttribute(pc->cinst[num], pc->cexpr_data, ATTR_TEXT_BGCOLOR, VAL_WHITE);
	
	SetCtrlAttribute(pc->cinst[num], pc->cexpr_delay, ATTR_TEXT_COLOR, VAL_BLACK);
	SetCtrlAttribute(pc->cinst[num], pc->cexpr_data, ATTR_TEXT_COLOR, VAL_BLACK);
	
	SetCtrlAttribute(pc->cinst[num], pc->cexpr_delay, ATTR_TEXT_BOLD, 0);
	SetCtrlAttribute(pc->cinst[num], pc->cexpr_data, ATTR_TEXT_BOLD, 0);
	
	// Clear out the ring controls
	int nl;
	GetNumListItems(pc->inst[num], pc->pcstep, &nl);
	if(nl)							   
		DeleteListItem(pc->inst[num], pc->pcstep, 0, -1);
	
	GetNumListItems(pc->inst[num], pc->pclevel, &nl);
	if(nl)							   
		DeleteListItem(pc->inst[num], pc->pclevel, 0, -1);
	
	GetNumListItems(pc->cinst[num], pc->dim, &nl);
	if(nl)							   
		DeleteListItem(pc->cinst[num], pc->dim, 0, -1);
	
	
}

void change_number_of_instructions() {
	// Gets "num" from pc->ninst and changes the number of instructions. If num < uipc->ni, t
	// the instructions at the end are hidden. if uipc->max_ni > num > uipc > uipc->ni, the 
	// old instructions are made visible again. If num > uipc->max_ni, a new instruction is 
	// created at the end and max_ni is incremented.
	
	int num, i;
	GetCtrlVal(pc->ninst[1], pc->ninst[0], &num);
	
	if(num == uipc->ni)
		return;			// No change needed
	
	if(num<uipc->ni) {						// In this case, we just hide the panels
		for(i = num; i<uipc->max_ni; i++) {
			HidePanel(pc->inst[i]);
			HidePanel(pc->cinst[i]);
		}
		uipc->ni = num;
		return;
	}
	
	if(num>uipc->max_ni) {		// The only really important part is to make new panels if there aren't any left		
		int top, left, height, ctop, cleft, cheight; 	// Getting the GUI values if we need them. 
		GetPanelAttribute(pc->inst[uipc->max_ni-1], ATTR_TOP, &top);	     	// Need these vals for both the
		GetPanelAttribute(pc->cinst[uipc->max_ni-1], ATTR_TOP, &ctop); 		 	// ND instrs and the pulse ones
		GetPanelAttribute(pc->inst[uipc->max_ni-1], ATTR_LEFT, &left);		 
		GetPanelAttribute(pc->cinst[uipc->max_ni-1], ATTR_LEFT, &cleft);
		GetPanelAttribute(pc->inst[uipc->max_ni-1], ATTR_HEIGHT, &height);
		GetPanelAttribute(pc->cinst[uipc->max_ni-1], ATTR_HEIGHT, &cheight);

		pc->inst = realloc(pc->inst, sizeof(int)*num);
		pc->cinst = realloc(pc->cinst, sizeof(int)*num);
		for(i=uipc->max_ni; i<num; i++) {
			pc->inst[i] = LoadPanel(pc->PProgCPan, pc->uifname, pc->pulse_inst);	// Make a new instruction
			SetPanelPos(pc->inst[i], top+=height+5, left);		// Place it and increment "top"
			SetCtrlAttribute(pc->inst[i], pc->xbutton, ATTR_DISABLE_PANEL_THEME, 1);
			
			pc->cinst[i] = LoadPanel(pc->PPConfigCPan, pc->uifname, pc->md_inst);	// Make a new ND instr
			SetPanelPos(pc->cinst[i], ctop+=cheight+5, cleft);
		
			// Update the instruction numbers
			SetCtrlVal(pc->inst[i], pc->ins_num, i);
			SetCtrlVal(pc->cinst[i], pc->cins_num, i);
		}

		if(uipc->c_instrs != NULL) {
			uipc->c_instrs = realloc(uipc->c_instrs, sizeof(PINSTR**)*num);
			uipc->max_cinstrs = realloc(uipc->max_cinstrs, sizeof(int)*num);
			for(i=uipc->max_ni; i<num; i++) {
				uipc->c_instrs[i] = NULL;
				uipc->max_cinstrs[i] = 0;
			}
		}

		uipc->max_ni = num;		// We've increased the max number of instructions, so increment it.
	}
	
	int ndon;
	GetCtrlVal(pc->ndon[1], pc->ndon[0], &ndon);
	
	for(i=uipc->ni; i<num; i++) {	// Now we can show the remaining panels.
		DisplayPanel(pc->inst[i]);
		DisplayPanel(pc->cinst[i]);
		SetPanelAttribute(pc->cinst[i], ATTR_DIMMED, !ndon);
	}
	
	uipc->ni = num;	// And update the uipc value.
}

void delete_instruction(int num) {
	// Deletes an instruction by clearing the value, moving it to the end and
	// reducing the number of instructions available.
	
	clear_instruction(num);

	if(uipc->ni == 1)
		return;			// Can't have 0 instructions
	
	move_instruction(uipc->max_ni-1, num);
	SetCtrlVal(pc->ninst[1], pc->ninst[0], uipc->ni-1);
	change_number_of_instructions();
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
		case LONG_DELAY:
			return 1;
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
	add_constant(c, "nd", C_INT, &uipc->nd);				// Number of dimensions
	add_constant(c, "nc", C_INT, &uipc->nc);				// Number of ccles.
		
	// Now the steps per dimension and the steps per cycle
	char *current_var = malloc(5);
	int i;
	
	for(i=0;i<uipc->nc; i++) {
		sprintf(current_var, "mcs%d", i);				// Maximum cycle step for cycle i
		add_constant(c, current_var, C_INT, &uipc->cyc_steps[i]);
	}
		
	for(i=0;i<uipc->nd; i++){
		sprintf(current_var, "mds%d", i);				// Max dim step for dimension i
		add_constant(c, current_var, C_INT, &uipc->dim_steps[i]);
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
	// cstep must be a position in acqusition space of size uipc->nd+uipc->nc
	char *current_var = malloc(5);
	int i;
	
	// change_constant redirects to add_constant if the constant is not found.
	for(i=0;i<uipc->nc; i++) {
			sprintf(current_var, "ccs%d", i);					// Current cycle step for cycle i
			change_constant(c, current_var, C_INT, &cstep[i]);
		}
	
	for(i=0;i<uipc->nd; i++) {
		sprintf(current_var, "cds%d", i);					// Current dim step for dimension i
		change_constant(c, current_var, C_INT, &cstep[i+uipc->nc]);
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
		}
	}
	
	if(p->nFuncs)
		p->func_locs = malloc(sizeof(int)*p->nFuncs);			// Allocate the array of function indexes

	// Allocate the arrays of pointers.
	p->v_ins_locs = malloc(sizeof(int**)*p->nVaried); 		// Array of pointers to arrays of ints.
	p->funcs = malloc(sizeof(pfunc*)*p->nFuncs);			// Array of pointers to functions
	p->instrs = malloc(sizeof(PINSTR*)*p->nUniqueInstrs); 	// Array of pointers to instructions
	
	// Allocate the instruction array and function arrays
	for(int i = 0; i < p->nUniqueInstrs; i++)
		p->instrs[i] = malloc(sizeof(PINSTR));
	for(int i = 0; i < p->nVaried; i++)
		p->v_ins_locs[i] = malloc(sizeof(int)*p->max_n_steps);
	for(int i = 0; i < p->nFuncs; i++) 
		p->funcs[i]	= malloc(sizeof(pfunc));
}

void free_pprog(PPROGRAM *p) {
	// The main thing to do here is to free all the arrays and the arrays of arrays.
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
	
	for(int i = 0; i<p->nVaried; i++) {
		free(p->v_ins_locs[i]);
		free(p->data_exprs[i]);
		free(p->delay_exprs[i]);
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


//////////////////////////////////////////////////////////////
// 															//
//					General Utilities						//
// 															//
//////////////////////////////////////////////////////////////

/*********************** Linear Indexing ***********************/ 

int get_maxsteps(int *maxsteps) {
	// Retrieves the maxsteps array as well as max_n_steps. Return value 
	// is max_n_steps. maxsteps has size uipc->nc+uipc->nd;
	int i, max_n_steps = 1;
	for(i=0; i<uipc->nc; i++) {
		max_n_steps *= uipc->cyc_steps[i];
		maxsteps[i] = uipc->cyc_steps[i];
	}
	
	for(i = 0; i<uipc->nd; i++) {
		max_n_steps *= uipc->dim_steps[i];
		maxsteps[i+uipc->nc] = uipc->dim_steps[i];
	}
	
	return max_n_steps;
}

int get_lindex(int *cstep, int *maxsteps, int size)
{
	// Give this a position in space maxsteps, which is an array of size "size"
	// It returns the linear index of cstep
	// We'll assume you can figure out whether or not this is the end of the array
	
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
	GetCtrlVal(pc->inst[instr], pc->instr, &ins);
	
	if(ins != LOOP) {
		return -1;
	}
	
	// Search for the END_LOOP, check if it matches. Simple.
	int e_i_d, ni;
	GetCtrlVal(pc->ninst[1], pc->ninst[0], &ni);
	for(int i = instr+1; i<ni; i++)
	{
		GetCtrlVal(pc->inst[i], pc->instr, &ins);
		if(ins == END_LOOP)
		{
			GetCtrlVal(pc->inst[i], pc->instr_d, &e_i_d);
			if(e_i_d == instr)
			{
				return i;
			}
		}
	}
	
	return -1;
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

/*************************** Sorting ***************************/ 

void sort_linked(int **array, int num_arrays, int size) {
	// Feed this an array of arrays and it sorts by the first array.
	// Uses insertion sort, which is efficient for small data sets.
	// Sorts so that the first element is the smallest.
	
	int *buff = malloc(sizeof(int)*num_arrays); 	// Buffer array for the integers we're moving around.
	int i, j, k;
	
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
	// *c needs to be freed. The return value is the number of items in
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


//////////////////////////////////////////////////////////////
// 															//
//						Vestigial							//
// 															//
//////////////////////////////////////////////////////////////



int get_vdim(int panel, int varyctrl, int dimctrl) {
	// Get the dimension along which this instruction varies.
	// If panel is an MDInstr, varyctrl should be pc->vary and dimctrl should be pc->dim
	// If panel is a PulseInstP, varyctrl should be pc->pcon and dimctrl should be pc->pclevel
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
	// If panel is an MDInstr, varyctrl should be pc->vary and nsteps should be pc->nsteps
	// If panel is a PulseInstP, varyctrl should be pc->pcon and dimctrl should be pc->pcsteps
	int steps, von;

	GetCtrlVal(panel, varyctrl, &von); 	// Determine if it's even on.
	if(!von)
		return 0;						// Dimension = 0 means it's not on.
	
	GetCtrlVal(panel, nsteps, &steps);	// Everything's good
	return steps;
}


