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
	
	
	// Error values:
	// -1 => Error opening file
	
	int retval = 0, errval;			// For error checking in the netCDF functions
	
	int ncid, gid;
	
	// Create the netCDF file if it doesn't already exist.
	if(group == NULL) {
		if(retval = nc_create(filename, NC_CLOBBER, &ncid))
			return -1;	// Error opening file
		
		gid = ncid;
	} else {
		if(retval = nc_open(filename, NC_WRITE, &ncid))
			return -1;	// Error opening file
		
		nc_def_grp(ncid, group, &gid);	// Create the requested group in the file. 
	}
	
	// It may be necessary to call nc_redef here, but I'll try without.
	int ud_funcs = 0;		// Whether or not we need to store user-defined funcs
	if(p->nFuncs)
		ud_funcs = 1;
	
	// Add in the basic attributes -> Error checking seems like it's going to be
	// tedious and probably not all that fruitful at the moment, so skip it for now.
	nc_put_att_int(gid, NC_GLOBAL, "np", NC_INT, 1, &p->np); 						// Number of points
	nc_put_att_double(gid, NC_GLOBAL, "sr", NC_DOUBLE, 1, &p->sr); 				// Sampling rate

	nc_put_att_int(gid, NC_GLOBAL, "trig_ttl", NC_INT, 1, &p->trigger_ttl); 			// Trigger TTL
	nc_put_att_int(gid, NC_GLOBAL, "scan", NC_INT, 1, &p->scan); 					// Scan bool
	
	nc_put_att_int(gid, NC_GLOBAL, "ud_funcs", NC_INT, 1, &ud_funcs);				// User-defined funcs bool
	nc_put_att_int(gid, NC_GLOBAL, "varied", NC_INT, 1, &p->varied);				// Varied bool
	nc_put_att_int(gid, NC_GLOBAL, "skip", NC_INT, 1, &p->skip);					// Skip bool
	
	nc_put_att_double(gid, NC_GLOBAL, "total_time", NC_DOUBLE, 1, &p->total_time);	// Total time
	
	// Now write the array of instructions.
	// The first thing to do is to define the PINSTR type.
	nc_type NC_PINSTR;
	nc_create_pinstr(gid, &NC_PINSTR);
	
	// Now we want to start creating our array variables
	// Start with the instrs variable					  

	int instrs_id, instr_did, i;													
	nc_def_dim(gid, "Instruction", p->nUniqueInstrs, &instr_did);			// Define the dimension for the instructions array
	nc_def_var(gid, "instrs", NC_PINSTR, 1, &instr_did, &instrs_id);		// Define the instructions array variable
	
	nc_def_var_endian(gid, instrs_id, NC_ENDIAN_LITTLE);	// Just in case, make sure it's little endian
	
	nc_put_att_int(gid, instrs_id, "nUniqueInstrs", NC_INT, 1, &p->nUniqueInstrs);
	nc_put_att_int(gid, instrs_id, "n_inst", NC_INT, 1, &p->n_inst);

	
	// Now if it's a varied experiment, set up the arrays of varied stuff.
	int v_ins_id, skip_locs_id;
	int *v_ins_dids;
	if(p->varied) {
		int nVaried = p->nVaried;
		int *maxsteps = p->maxsteps;

		// The dimensions we care about
		v_ins_dids = malloc(sizeof(int)*2);
		nc_def_dim(gid, "VariedInstr", nVaried, &v_ins_dids[0]);
		nc_def_dim(gid, "LIndexSteps", p->max_n_steps, &v_ins_dids[1]);
		
		// First the v_ins_locs array
		nc_def_var(gid, "v_ins_locs", NC_INT, 2, v_ins_dids, &v_ins_id);
		
		// Now we'll put a bunch of metadata on v_ins_locs
		// First the scalars
		nc_put_att_int(gid, v_ins_id, "nDims", NC_INT, 1, &p->nDims);
		nc_put_att_int(gid, v_ins_id, "nCycles", NC_INT, 1, &p->nCycles);
		nc_put_att_int(gid, v_ins_id, "nVaried", NC_INT, 1, &p->nVaried);
		nc_put_att_int(gid, v_ins_id, "max_n_steps", NC_INT, 1, &p->max_n_steps);
		nc_put_att_int(gid, v_ins_id, "real_n_steps", NC_INT, 1, &p->real_n_steps);
		
		// Then the vectors
		nc_put_att_int(gid, v_ins_id, "v_ins_num", NC_INT, nVaried, p->v_ins);
		nc_put_att_int(gid, v_ins_id, "v_ins_dim", NC_INT, nVaried, p->v_ins_dim);
		nc_put_att_int(gid, v_ins_id, "v_ins_mode", NC_INT, nVaried, p->v_ins_mode);
		nc_put_att_int(gid, v_ins_id, "maxsteps", NC_INT, p->nDims+p->nCycles, p->maxsteps);
		
		// Now if it's necessary, the skip_locs array
		if(p->skip) {
			nc_def_var(gid, "skip_locs", NC_INT, 1, &v_ins_dids[1], &skip_locs_id);
			
			size_t exprlen = strlen(p->skip_expr)+1;
			nc_put_att_text(gid, skip_locs_id, "skip_expr", exprlen, p->skip_expr);
		}
		
		free(v_ins_dids);
	}
	
	nc_enddef(ncid);	//	Leave definition mode, enter data mode.

	// Now the arrays are all defined and such, we need to populate the variables.
	// Starting with the instructions -> The first thing we need to do is build a char array
	// that contains all the data in the appropriate format.
	char **all_instrs = malloc(sizeof(char*)*p->nUniqueInstrs);
	for(i=0; i<p->nUniqueInstrs; i++) {
		all_instrs[i] = malloc(PINSTR_SIZE);			// Allocate space for the array (don't forget to free it!)
		make_nc_pinstr(p->instrs[i], all_instrs[i]);	// Build the memory block.
	}
	
	// Put the variable array now.
	nc_put_var(gid, instrs_id, all_instrs);
	
	// Free up memory.
	for(i=0; i<p->nUniqueInstrs; i++)
		free(all_instrs[i]);
	free(all_instrs);
	
	
	// Finally, we need to write the varied arrays (v_ins_locs and skip_locs)
	if(p->varied) {
		// These are going to be way easier -> start with v_ins_locs
		nc_put_var_int(gid, v_ins_id, &p->v_ins_locs[0][0]);		// We can just write the array (I hope)
		
		if(p->skip) {
			nc_put_var_int(gid, skip_locs_id, p->skip_locs);		// Same with this one
		}
	}
	
	
	if(ud_funcs)
		nc_put_pfuncs(ncid, gid, p->funcs, p->func_locs, p->nFuncs, p->tFuncs, NC_PINSTR); 	// Write the functions if need be.
	
	nc_close(ncid);		// All the values have been written, so we can close up shop.
	return 0;
}

int LoadPulseProgram(char *filename, PPROGRAM *p, char *group) {
	// Loads a netCDF file to a PPROGRAM. p is dynamically allocated
	// as part of the function, so make sure to free it when you are done with it.
	// Passing NULL to group looks for a program in the root group, otherwise
	// the group "group" in the root directory is used.
	// p is freed on error.
	
	// Returns 0 if successful, positive integers for errors.
	
	
	int retval = 0, pos = 0; 		// For error checking
	int ncid, gid;					// Root value, group id
	
	if((retval = nc_open(filename, NC_NOWRITE, &ncid)))
		goto error;						// On errors go to the cleanup function.
	
	if(group == NULL) 
		gid = ncid;
	else {
		if(retval = nc_inq_grp_ncid(ncid, group, &gid))
			goto error;
	}
	
	// Get the PINSTR type.
	int NC_PINSTR;
	if(retval = nc_inq_typeid(gid, "pinstr", &NC_PINSTR))
		goto error;
	
	// Let's make a PPRogram
	p = malloc(sizeof(PPROGRAM));
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
	
	// Now we need to get the varids of the variables and gather the metadata we need to allocate the rest of p
	int instr_id, v_ins_id, skip_locs_id, func_locs_id, *func_ids;		// Varyids

	// First the instructions
	if(retval = nc_inq_varid(gid, "instrs", &instr_id))
		goto error;
	
	if(retval = nc_get_att_int(gid, instr_id, "nUniqueInstrs", &p->nUniqueInstrs))
		goto error;
	
	if(retval = nc_get_att_int(gid, instr_id, "n_inst", &p->n_inst))
		goto error;
	
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
	char **ins_char_array = malloc(sizeof(char*)*p->nUniqueInstrs);
	for(i=0; i<p->nUniqueInstrs; i++)
		ins_char_array[i] = malloc(PINSTR_SIZE);
	pos++;
	
	if(retval = nc_get_var(gid, instr_id, ins_char_array))		// Read them out as a raw character array
		goto error;
	
	for(i=0; i<p->nUniqueInstrs; i++)
		get_nc_pinstr(p->instrs[i], ins_char_array[i]);			// Convert the instrs to an instr array
	
	for(i = 0; i<p->nUniqueInstrs; i++)							// We're done with the raw char array, so free it
		free(ins_char_array[i]);
	free(ins_char_array);
	pos++;	// Don't want to free malloced memory
	
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
	
	return 0;
	
	error:
	// We need to free memory before we exit if the function didn't complete correctly.
	// "pos" encodes where we were for this purpose.
	
	if(pos == 1)
		free(p);
	else if(pos == 3) {
		for(i = 0; i<p->nUniqueInstrs; i++)
			free(ins_char_array[i]);
		free(ins_char_array);
	}
	
	if(pos >= 2)
		free_pprog(p);
	
	return retval;
}


int nc_create_pinstr(int ncid, nc_type *typeid) {
	// Convenience function for creating the pinstr type in an open netcdf file.
	// typeid is a return value.
	
	int retval = 0;

	// Create the compound type
	if(retval = nc_def_compound(ncid, sizeof(PINSTR), "pinstr", typeid))
		return retval;

	// Add these one by one. offset needs to be incremented after each type is implemented.
	// Since there doesn't seem to be a postfix version of +=, each step increments offset
	// by the size of the last one.
	size_t offset = 0;
	nc_type type = *typeid;
	if(retval = nc_insert_compound(ncid, type, "flags", offset, NC_INT))
		return retval;
	
	if(retval = nc_insert_compound(ncid, type, "instr", offset+=sizeof(int), NC_INT))
		return retval;
	
	if(retval = nc_insert_compound(ncid, type, "instr_data", offset+=sizeof(int), NC_INT))
		return retval;
	
	if(retval = nc_insert_compound(ncid, type, "t_scan", offset+=sizeof(int), NC_CHAR))		// This is boolean, no reason to make it an int
		return retval;
	
	if(retval = nc_insert_compound(ncid, type, "t_units", ++offset, NC_CHAR))				// This can only have values 0-3, so it can also be an 8-bit integer
		return retval;																		// NC_BYTE has size 1 byte.
	
	if(retval = nc_insert_compound(ncid, type, "instr_time", ++offset, NC_DOUBLE))
		return retval;
	
	return 0;
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
		for(j = 0; j<funcs[i]->n_instr; j++) 
			get_nc_pinstr(funcs[i]->instrs[j], fins_ar[j]);	// Convert the array to a proper PINSTR
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
			
			make_nc_pinstr(funcs[i]->instrs[j], func_instrs[j]);		// Put a PINSTR there
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

void make_nc_pinstr(PINSTR *pinstr, char *nc_pinstr) {
	// Because netCDF user-defined types are built byte-by-byte, we need a function to
	// convert our struct into a char array that we can write.
	
	// Define the union for converting everything to chars.
	union picon {
		char ci[sizeof(int)];		// For NC_INT types
		char cd[sizeof(double)];	// For NC_DOUBLE types
		
		double d;
		int i;
	} p;
	
	// Now actually build a char array byte by byte.
	// Flags first
	int i, si = sizeof(int);
	p.i = pinstr->flags;
	int size=0;
	for(i = 0; i<si; i++) 
		nc_pinstr[size++] = p.ci[i];
	
	// Then instr
	p.i = pinstr->instr;
	for(i=0; i<si; i++) 
		nc_pinstr[size++] = p.ci[i];
	
	// Instr_data
	p.i = pinstr->instr_data;
	for(i=0; i<si; i++)
		nc_pinstr[size++] = p.ci[i];
	
	// Scan trigger and time units
	nc_pinstr[size++] = (unsigned char)pinstr->trigger_scan;
	nc_pinstr[size++] = (unsigned char)pinstr->time_units;
	
	// Finally instruction time
	p.d = pinstr->instr_time;
	for(i=0; i<sizeof(double); i++)
		nc_pinstr[size++] = p.cd[i];
}

void get_nc_pinstr(PINSTR *ins, char *nc_pinstr) {
	// Given a char array built as "make_nc_pinstr" is built, convert this back to a pinstr
	// this is the reverse operation of "make_nc_pinstr"
	
	// Define the union for converting everything from chars.
	union picon {
		char ci[sizeof(int)];		// For NC_INT types
		char cd[sizeof(double)];	// For NC_DOUBLE types
		
		double d;
		int i;
	} p;
	
	// Read it out byte-by-byte
	// Flags first
	int i, si = sizeof(int), size = 0;
	for(i = 0; i<si; i++) 
		p.ci[i] = nc_pinstr[size++];
	ins->flags = p.i;
	
	// Then instr
	for(i=0; i<si; i++) 
		p.ci[i] = nc_pinstr[size++];
	ins->instr = p.i;
	
	// Instr_data
	for(i=0; i<si; i++)
		p.ci[i] = nc_pinstr[size++];
	ins->instr_data = p.i;
	
	// Scan trigger and time units
	ins->trigger_scan = (int)nc_pinstr[size++];
	ins->time_units = (int)nc_pinstr[size++];
	
	// Finally instruction time
	for(i=0; i<sizeof(double); i++)
		p.cd[i] = nc_pinstr[size++];
	ins->instr_time = p.d;
}


//////////////////////////////////////////////////////////////
// 															//
//				UI Interaction Functions					//
// 															//
//////////////////////////////////////////////////////////////
/******************** General UI Functions *********************/
PPROGRAM *get_current_program() { // This function gets the current program from the UI controls and dumps it into a PPROGRAM
	PPROGRAM *p = malloc(sizeof(PPROGRAM));	// Dynamically allocate a PPROGRAM first

	ui_cleanup(); // We need to clean up the interface so that we can trust it.

	int nVaried = 0; 	// Start the counter

	// Retrieve what values we can from the UI elements
	GetCtrlVal(pc->ninst[1], pc->ninst[0], &p->n_inst);			// Number of instructions
	GetCtrlVal(pc->numcycles[1], pc->numcycles[0], &p->nCycles); 		// Number of phase cycles
	GetCtrlVal(pc->ndims[1], pc->ndims[0], &p->nDims);			// Number of dimensions
	GetCtrlVal(pc->skip[1], pc->skip[0], &p->skip);
	GetNumListItems(pc->inst[0], pc->instr, &p->tFuncs);		// Number of items in the instruction rings

	p->tFuncs -= 9;	// Subtract all the builtin functions, less one, since we're using this to initialize an array
	
	int nDims = p->nDims, nCycles = p->nCycles, nInst = p->n_inst;
	
	// Now we have to go through and get more information from the configuration
	int i, j;
	p->nUniqueInstrs = nInst; 									// If there's no variation, this is true. 
	
	int *dim_steps, *cycle_steps, *v_ins, *v_ins_mode, *v_ins_dim;		// Need some arrays to read out the multidimensional program
	
	// Allocate the memory for the arrays
	dim_steps = malloc(sizeof(int)*nDims);		// An array containing the number of steps per dimension
	cycle_steps = malloc(sizeof(int)*nCycles);  // An array containing the number of steps per cycle
	v_ins = malloc(0);
	v_ins_mode = malloc(0);
	v_ins_dim = malloc(0);
	
	if(nCycles || nDims) {	// We need to get nUniqueInstrs first if one of these is true.
		int dim, cycle, mode, dflags;

		for(i = 0; i<nInst; i++) {
			// See which are on
			dim = get_vdim(pc->cinst[i], pc->vary, pc->dim);
			cycle = get_vdim(pc->inst[i], pc->pcon, pc->pclevel);
			mode = 0; 	// Set the mode to be 0.
			dflags = 0;	// Set the dimension flags to be 0.
			
			if(dim--) {				// Need to postfix decrement here because it's a 0-based index.
				mode = PP_V_ID;		// It's definitely in PP_V_ID mode so far
				dflags = pow(2, (dim-nCycles));	// Flag the dimension
				if(!dim_steps[dim]) 						// Check if we've already assigned this in the array
					dim_steps[dim] = get_vsteps(pc->cinst[i], pc->vary, pc->nsteps);
				
				if(!cycle)
					p->nUniqueInstrs+=dim_steps[dim]-1;	// We're going to have this many more instructions, less the original one	
				
			}
			
			if(cycle--) {			// Need to postfix decrement here because, again, it's a 0-based index
				mode += PP_V_PC;	// PP_V_BOTH = PP_V_ID+PP_V_PC, so this indicates if it's just cycled or both
				dflags += pow(2, cycle);	// Increment the flag appropriately.
				if(!cycle_steps[cycle]) 	// Check if we've already assigned this in the array
					cycle_steps[cycle] = get_vsteps(pc->inst[i], pc->pcon, pc->pcsteps);
				
				if(!++dim) 
					p->nUniqueInstrs+=cycle_steps[cycle]-1;	// Again, this many more instructions, less the original one
			}
			
			if(dim && ++cycle) 
				p->nUniqueInstrs+=(cycle_steps[dim]*dim_steps[dim-1])-1;	// It's a multiplier => Some may be skipped, but just save them all
			
			if(dim || cycle) {
				v_ins = realloc(v_ins, ++nVaried*sizeof(int));				// Add another varied instruction
				v_ins_mode = realloc(v_ins_mode, nVaried*sizeof(int));		// And another mode slot
				v_ins_dim = realloc(v_ins_dim, nVaried*sizeof(int));		// Add another dimension slot
				v_ins[nVaried-1] = i;										// Actually populate these arrays
				v_ins_mode[nVaried-1] = mode;
				v_ins_dim[nVaried-1] = dflags;
			}
		}
	}
	
	if(p->nVaried)
		p->varied = 1;
	else
		p->varied = 0;	
	
	// Get maxsteps and max_n_steps from here.
	int *maxsteps = malloc(sizeof(int)*(nDims+nCycles)), max_n_steps = 1;
	for(i=0;i<nCycles;i++) {	// Total number of transients in a full cycle (number of transients acquired is different)
		max_n_steps*=cycle_steps[i];
		maxsteps[i]=cycle_steps[i];
	}
	
	for(i=0;i<nDims;i++) {
		maxsteps[i+nCycles]=dim_steps[i];
		max_n_steps *= dim_steps[i];
	}
	
	p->max_n_steps = max_n_steps;
	
	// Now we need to check which functions need to be included.
	int *ffound = malloc(sizeof(int)*p->tFuncs), nf = 0, f;
	int scan = 0, pcon;
	
	for(i = 0; i<nInst; i++) {
		if(nf < p->tFuncs) {
			GetCtrlVal(pc->inst[i], pc->pcon, &pcon);
			if(pcon) {										// If this is phase cycled, there could be functions hidden
				int pcs;									// in the phase cycling tables, so we need to check them.
				GetCtrlVal(pc->inst[i], pc->pcsteps, &pcs);
				for(j=1; j<=pcs; j++) {
		//			GetTableCellVal(pc->inst[i], pc->pctable, MakePoint(2, j), &f);	
					f-=9;
					if(f >= 0 && !ffound[f]) {
						ffound[f] = 1;
						nf++;
					}
				}
			} else {
				GetCtrlVal(pc->inst[i], pc->instr, &f);
				f-=9;	// Subtract off the number of atomic functions
				if(f >= 0 && !ffound[f]) {
					ffound[f] = 1;
					nf++;
				}
			}
		}
		
		if(!scan)
			GetCtrlVal(pc->inst[i], pc->scan, &scan);	// Check if this program involves a scan.
	}
	
	p->nFuncs = nf;			// We only want to include the functions that are actually being used
	
	// Now we have everything we need (and a bit more) to allocate everything for a pprogram. 
	create_pprogram(p);

	// Grab some basic values from the UI
	GetCtrlVal(pc->np[1], pc->np[0], &p->np);						// Number of points
	GetCtrlVal(pc->sr[1], pc->sr[0], &p->sr);						// Sampling rate
	GetCtrlVal(pc->timeest[1], pc->timeest[0], &p->total_time);  	 // Total time
	GetCtrlVal(pc->trig_ttl[1], pc->trig_ttl[0], &p->trigger_ttl);	// Trigger TTL

	// Now populate the control with some stuff you've already gathered
	p->scan = scan;			// Whether or not there's a scan
	
	for(i=0;i<nVaried;i++) { 	// Need to loop because array assignments are illegal and I need to free v_ins and v_ins_mode;
		p->v_ins[i] = v_ins[i];
		p->v_ins_mode[i] = v_ins_mode[i];
		p->v_ins_dim[i] = v_ins_dim[i];
	}
	
	// Free these
	free(v_ins);
	free(v_ins_mode);
	free(v_ins_dim);
	
	// Get all the instructions as they are in the current view.
	for(i = 0; i<nInst; i++) 
		get_instr(p->instrs[i], i);
										   
	// Populate maxsteps
	for(i=0;i<(nDims+nCycles);i++)
		p->maxsteps[i] = maxsteps[i];
	
	if(p->varied) {					// Only do this next part if there's variation
		int nextinstr = nInst-1;	// Counter for total number of instructions generated - starts at the last instruction
		
		int *places = malloc(sizeof(int)*(nDims+nCycles));	// Used for indexing.
		for(i=0; i<(nDims+nCycles); i++) {
			places[i] = 1;
			for(j=1; j<=i; j++)
				places[i]*=maxsteps[j-1];
		}
		
		for(i=0; i<nVaried;i++) {
			// First we're going to initialize the instructions we got, in case the user was viewing something other than
			// the 0th index in all dimensions.
			int dstep = -1, pcstep = -1, dlev = -1, clev = -1;

			// We want to initialize these to the 1st in each index, but leave it negative if it doesn't vary that way
			// While we're doing this, we're also going to figure out what dimension this varies along, for later.
			if(p->v_ins_mode[i] & PP_V_PC) {	// Matches PC and BOTH  	
				pcstep = 1;						// 1-based index	
				for(j=0; j<nCycles;j++) {
					if(p->v_ins_dim[i] & (int)pow(2, j)) {
						clev = j;
						break;
					}
				}
			} 
			if(p->v_ins_mode[i] & PP_V_ID)  {// Matches ID and BOTH
				dstep = 0;					// 0-based index
				for(j=nCycles; j<(nCycles+nDims);j++) {
					if(p->v_ins_dim[i] & (int)pow(2, j)) {
						dlev = j;
						break;
					}
				}
			}

			update_instr(pc, p->instrs[p->v_ins[i]], p->v_ins[i], dstep, pcstep);
		
			// Now we are going to generate all the unique instructions stemming from this instruction
			// and as we do that, we'll generate the v_ins_locs locator index.
			int k, l;	// Counters
			
			if(p->v_ins_mode[i] == PP_V_PC) {	// One of two linear conditions
				for(j = 1; j<=cycle_steps[clev]; j++) {
					copy_pinstr(p->instrs[p->v_ins[i]], p->instrs[++nextinstr]);	// Prefix increment
					update_instr(pc, p->instrs[nextinstr], p->v_ins[i], -1, j);
					
					// Populate the v_ins_locs that should point to this instruction.
					for(k=(j*places[clev]); k<max_n_steps; k+=places[clev]*cycle_steps[clev]) {
						for(l=0; l<places[clev]; l++)
							p->v_ins_locs[i][k+l] = nextinstr;
					}
				}
			} else if(p->v_ins_mode[i] == PP_V_ID) {
				for(j = 0; j<dim_steps[dlev]; j++) {
					copy_pinstr(p->instrs[p->v_ins[i]], p->instrs[++nextinstr]);
					update_instr(pc, p->instrs[nextinstr], p->v_ins[i], j, -1);
					
					// Populate the v_ins_locs that should point to this instruction.
					for(k=(j*places[dlev]); k<max_n_steps; k+=places[dlev]*dim_steps[dlev-nCycles]) {
						for(l=0; l<places[dlev]; l++)
							p->v_ins_locs[i][k+l] = nextinstr;
					}
					
				}
			} else if(p->v_ins_mode[i] == PP_V_BOTH) {
				int placestep = places[dlev]/places[clev];				//  dlev is always greater than clev
				int r; 	// One more counter
					for(int m=0; m<dim_steps[dlev]; m++) {
						for(j=1; j<=cycle_steps[clev]; j++) {
						copy_pinstr(p->instrs[p->v_ins[i]], p->instrs[++nextinstr]);
						update_instr(pc, p->instrs[nextinstr], p->v_ins[i], m, j);
						
						for(k = m*places[dlev]; k<max_n_steps; k+=dim_steps[dlev-nCycles]*places[dlev]) {
							for(l=j*places[clev]; l<places[dlev]; l+=cycle_steps[clev]*places[clev]) {
								for(r=0; r<places[clev]; r++)
									p->v_ins_locs[i][k+l+r] = nextinstr;
							}
						}
					}
				}
			}
		}
		free(places);
	}
	
	if(p->skip) 	{					// If we've imposed a skip condition, we should evaluate it
		// Get the skip expression
		int expr_len;
		GetCtrlValStringLength(pc->skiptxt[1], pc->skiptxt[0], &expr_len);
		p->skip_expr = realloc(p->skip_expr, expr_len+1);
		GetCtrlVal(pc->skiptxt[1], pc->skiptxt[0], p->skip_expr);
		
		
		// Now actually evaluate the skip conditions
		int real_n_steps = max_n_steps;								// Assumption is we don't skip anything
		double s;
		int *cstep = malloc(sizeof(int)*(nDims+nCycles));

		constants *c = setup_constants();
		
		for(j=0; j<max_n_steps; j++) {
			get_cstep(j, cstep, maxsteps, nDims+nCycles); 			// Get the next step
		
			update_constants(c, cstep);
			
			// Now we've got all the constants and ui_cleanup doesn't allow skip conditions that give errors,
			// so now we can just store the value for this step.
			int err_val = 0;
			s = parse_math(p->skip_expr, c, &err_val, 0);			// Evaluate the expression
			if(s) {
				p->skip_locs[j] = 1;
				real_n_steps--;										// Since we skipped one, there's 1 fewer step
			
			}
			else
				p->skip_locs[j] = 0;
		
		}
		
		free_constants(c);
		free(cstep);
	}
	
	free(maxsteps);
	free(cycle_steps);
	free(dim_steps);
	
	
	// Finally store the functions that you've defined in the program.
	int tFuncs = p->tFuncs, nFuncs = p->nFuncs;
	j = 0;
	for(i=0; i<tFuncs; i++) {
		if(tFuncs > nFuncs)
			p->func_locs = realloc(p->func_locs, sizeof(int)*tFuncs);
		
		if(ffound[i]) {
			p->funcs[j] = get_pfunc(i+9);
			p->func_locs[i] = j++;
		} else 
			p->func_locs[i] = -1;
	}
	free(ffound);
	
	return p;
}

void ui_cleanup() {
	// Function for cleaning up the input so it's consistent - i.e. turning off ND if there are no varied instrs.
	int i, dim, cyc, nl, on;
	int ndims=0, npc=0, dimf=0, cycf=0, ndins=0, ncins=0;
	int *dim_steps = malloc(sizeof(int)*uipc->nd), *cyc_steps = malloc(sizeof(int)*uipc->nc);
	int *dim_ins = malloc(sizeof(int)*uipc->ndins), *ins_dims = malloc(sizeof(int)*uipc->ndins);
	int *cyc_ins = malloc(sizeof(int)*uipc->ncins), *ins_cycs = malloc(sizeof(int)*uipc->ncins);
	
	// Update the uipc function from the controls and reduce the nominal dimensionality of the experiment
	// if there are cycles/dimensions with only 1 step or which appear in no instructions. 
	for(i = 0; i<uipc->ni; i++) {
		// ND Bit first
		GetNumListItems(pc->cinst[i], pc->dim, &nl);
		on = get_nd_state(i);
		if(nl > 0 && on) {
			GetCtrlVal(pc->cinst[i], pc->pclevel, &dim);
			if(uipc->dim_steps[dim]) {
				dim_ins[ndins] = i;
				ins_dims[ndins++] = dim;
				if((dimf - (dimf=(dimf|(int)pow(2,dim)))) < 0) 	 
					GetCtrlVal(pc->cinst[i], pc->nsteps, &dim_steps[ndims++]);
			}
		} else if(on) 
			update_nd_state(i, 0);
		
		// Now phase cycling
		GetNumListItems(pc->inst[i], pc->pclevel, &nl);
		on = get_pc_state(i);
		if(nl > 0 && on) {
			GetCtrlVal(pc->inst[i], pc->pclevel, &cyc);
			if(uipc->cyc_steps[cyc] > 1) {
				cyc_ins[ncins] = i;
				ins_cycs[ncins++] = cyc;
				if((cycf - (cycf=(cycf|(int)pow(2,cyc)))) < 0) // Adds the flag and returns negative if that was necessary
					GetCtrlVal(pc->inst[i], pc->pcsteps, &cyc_steps[npc++]);
			} else
				update_pc_state(i, 0);
		} else if(on)
			update_pc_state(i, 0);
	}
	
	// Now if we deleted dimensions or cycles, the nominal dimension of everything higher than the deleted
	// dimensions will be reduced. We need to update the uipc variable according.
	for(int j=0; j<uipc->nd; j++) {
		if(!(dimf & (int)pow(2, j))) {
			for(i=0; i<ndins; i++) {
				if(ins_dims[i] > j)	// The == condition should never be met, so no worries there
					ins_dims[i]--;	
			}
		}
	}
	
	for(int j=0; j<uipc->nc; j++) {	// Same thing, but for cycles, obviously.
		if(!(cycf & (int)pow(2, j))) {
			for(i=0; i<ncins; i++) {
				if(ins_cycs[i] > j)	
					ins_cycs[i]--;	
			}
		}
	}
	
	// Now we need to update the uipc and controls appropriately.
	uipc->nd = uipc->max_nd = ndims;
	uipc->nc = uipc->max_nc = npc;
	uipc->ndins = ndins;
	uipc->ncins = ncins;
	
	// Tighten up the memory allocation of these arrays
	dim_steps = realloc(dim_steps, sizeof(int)*ndims);
	dim_ins = realloc(dim_ins, sizeof(int)*ndins);
	ins_dims = realloc(ins_dims, sizeof(int)*ndins);
	cyc_steps = realloc(cyc_steps, sizeof(int)*npc);
	cyc_ins = realloc(cyc_ins, sizeof(int)*ncins);
	ins_cycs = realloc(ins_cycs, sizeof(int)*ncins);
 
	// Free the old arrays - if you don't do this there will be memory leaks when you reassign the pointer
	free(uipc->dim_steps);
	free(uipc->dim_ins);
	free(uipc->ins_dims);
	free(uipc->cyc_steps);
	free(uipc->cyc_ins);
	free(uipc->ins_cycs);
	
	// Reassign the pointers to the new arrays.
	uipc->dim_steps = dim_steps;
	uipc->dim_ins = dim_ins;
	uipc->ins_dims = ins_dims;
	uipc->cyc_steps = cyc_steps;
	uipc->cyc_ins = cyc_ins;
	uipc->ins_cycs = ins_cycs;

	// Update the UI.
	populate_dim_points();
	populate_cyc_points();
	SetCtrlVal(pc->ndims[1], pc->ndims[0], uipc->nd);
	SetCtrlVal(pc->ndon[1], pc->ndon[0], uipc->nd);
	SetCtrlVal(pc->numcycles[1], pc->numcycles[0], uipc->nc);
} // Needs update

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
		.instr_time = 100.0,
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
	
	// Set up the delay time
	SetCtrlVal(panel, pc->delay, instr->instr_time);
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
/**************** Get ND Instruction Parameters ****************/  
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
	int j, on, nl, dim;
	
	// A char array of labels
	int elements;
	char **c = generate_char_num_array(1, uipc->nd, &elements);
	
	int panel;
	for(int i = 0; i<uipc->ndins; i++) {
		panel = pc->cinst[uipc->dim_ins[i]];
		on = get_nd_state(i);
		if(on) {	// This condition should never be met, but it's not so bad to have it
			// First make the number of dimensions per control correct
			GetNumListItems(panel, pc->dim, &nl);
			if(nl < uipc->nd) {
				for(j=nl; j<uipc->nd; j++)
					InsertListItem(panel, pc->dim, -1, c[j], j);
			} else if (nl > uipc->nd) {
				DeleteListItem(panel, pc->dim, uipc->nd, -1);
			}
			
			// Now update the number of steps.
			GetCtrlIndex(panel, pc->dim, &dim);
			SetCtrlVal(panel, pc->nsteps, uipc->dim_steps[dim]);
		}
	}
	
	for(j=0; j<elements; j++)
		free(c[j]);
	free(c);
} //  Needs update

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
				uipc->err_del = err_del;	  	// Save the error
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
				uipc->err_dat = err_dat;	  	// Save the error
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
		
		if(err_del) {
			del_bgcolor = VAL_RED;
			del_textcolor = VAL_OFFWHITE;
		} else 
			del_bgcolor = VAL_GREEN;
	} 
	
	if(eval_data) {
		free(expr_data);
		
		dat_bold = 1;
		
		if(err_dat) {
			dat_bgcolor = VAL_RED;
			dat_textcolor = VAL_OFFWHITE;
		} else
			dat_bgcolor = VAL_GREEN;
	} 
	
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
	
	expr = malloc(len+1);
	GetCtrlVal(pan, ctrl, expr);
	
	// Get max_n_steps and maxsteps;
	int *maxsteps = malloc(sizeof(int)*size);
	uipc->max_n_steps = 0;
	for(i=0; i<uipc->nc; i++) {
		uipc->max_n_steps+=uipc->cyc_steps[i];
		maxsteps[i] = uipc->cyc_steps[i];
	}
	
	for(i=0; i<uipc->nd; i++) { 
		uipc->max_n_steps+=uipc->dim_steps[i];
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
	int *cstep = malloc(sizeof(int)*size);
	for(i=0; i<max_n_steps; i++) {
		get_cstep(i, cstep, maxsteps, size); 	// Convert from linear index to cstep.
		
		update_constants(c, cstep);				// Update the dynamic constants (variables)
		
		double val = parse_math(expr, c, &err, 0);
		
		if(val)
			uipc->skip_locs[i] = 1;
		else
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

/**************** Get Phase Cycling Parameters *****************/ 
int get_pc_state(int num) {
	// Gets the state of phase cycling. At the moment there are only two states
	// but I'll probably add more, so it's easier this way.
	
	int val;
	GetCtrlVal(pc->inst[num], pc->pcon, &val);
	
	return val;
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
			if(--uipc->ncins == 0) {
				free(uipc->cyc_ins);
				free(uipc->ins_cycs);
				
				uipc->cyc_ins = NULL;
				uipc->ins_cycs = NULL;
			} else {
				uipc->cyc_ins = realloc(uipc->cyc_ins, sizeof(int)*uipc->ncins);
				uipc->ins_cycs = realloc(uipc->cyc_ins, sizeof(int)*uipc->ncins);
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
	
	int steps = uipc->cyc_steps[cyc];
	
	if(uipc->c_instrs[num] == NULL) {
		uipc->c_instrs[num] = malloc(sizeof(PINSTR*)*(from+1));
	} else if (uipc->max_cinstrs[num] <= from) {
		uipc->c_instrs[num] = realloc(uipc->c_instrs[num], sizeof(PINSTR*)*(from+1));
	}
	
	if(from >= uipc->max_cinstrs[num]) {
		for(i=uipc->max_cinstrs[num]; i<=from; i++) 
			uipc->c_instrs[num][i] = NULL;
		
		uipc->max_cinstrs[num] = from+1;
	}
	
	if(uipc->c_instrs[num][from] == NULL)
		uipc->c_instrs[num][from] = malloc(sizeof(PINSTR));
	
	get_instr(uipc->c_instrs[num][from], num); 		// Save the old instruction
	
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

	if(steps == uipc->cyc_steps[cyc])
		return;			//No change

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

void populate_cyc_points()
{	 
	// Function for updating the UI with the values from the uipc var
	int j, on, nl, cyc;
	
	// A char array of labels
	int elements;
	char **c = generate_char_num_array(1, uipc->nc, &elements); 
	int panel;
	for(int i = 0; i<uipc->ncins; i++) {
		panel = pc->cinst[uipc->cyc_ins[i]];
		on = get_pc_state(i);
		if(on) {	// This condition should never be met, but it's not so bad to have it
			// First make the number of dimensions per control correct
			GetNumListItems(panel, pc->pclevel, &nl);
			if(nl < uipc->nc) {
				for(j=nl; j<uipc->nc; j++)
					InsertListItem(panel, pc->pclevel, -1, c[j], j);
			} else if (nl > uipc->nc) {
				DeleteListItem(panel, pc->pclevel, uipc->nc, -1);
			}
			
			// Now update the number of steps.
			GetCtrlIndex(panel, pc->pclevel, &cyc);
			SetCtrlVal(panel, pc->pcsteps, uipc->cyc_steps[cyc]);
		}
	}
	
	for(j=0; j<elements; j++)
		free(c[j]);
	free(c);
} // Needs update

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
//				PProgram/PINSTR Manipulation				//
// 															//
//////////////////////////////////////////////////////////////

void create_pprogram(PPROGRAM *p) { // Initially allocates the PPROGRAM space
	// Give us a pre-malloc'ed PPROGRAM for this function
	// It needs the following fields already assigned:
	// 	p->n_inst		p->max_n_steps
	//	p->varied		p->nFuncs
	//	p->skip			p->nDims
	//	p->nVaried	   	p->nCycles
	
	if(p->varied) {
		p->maxsteps = malloc(sizeof(int)*(p->nDims+p->nCycles)); 				// Allocate the maximum position point
		p->v_ins = malloc(sizeof(int)*p->nVaried); 			// Allocate the array of varied instruction locations.
		p->v_ins_dim = malloc(sizeof(int)*p->nVaried);			// Allocate the array of varied dimension per instr
		p->v_ins_mode = malloc(sizeof(int)*p->nVaried);		// Allocate the array of variation modes
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
	for(int i = 0; i<p->nUniqueInstrs; i++)
		p->instrs[i] = malloc(sizeof(PINSTR));
	for(int i = 0; i<p->nVaried; i++)
		p->v_ins_locs[i] = malloc(sizeof(int)*p->max_n_steps);
	for(int i = 0; i<p->nFuncs; i++) 
		p->funcs[i]	= malloc(sizeof(pfunc));
}

void free_pprog(PPROGRAM *p) {
	// The main thing to do here is to free all the arrays and the arrays of arrays.
	if(p->varied) {
		free(p->maxsteps);
		free(p->v_ins);
		free(p->v_ins_dim);
		free(p->v_ins_mode);
		free(p->func_locs);
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
	
	for(int i = 0; i<p->nVaried; i++)
		free(p->v_ins_locs[i]);

	// Free the arrays of pointers.
	if(p->varied)
		free(p->v_ins_locs);
	if(p->nFuncs)
		free(p->funcs);
	free(p->instrs);
	
	free(p);
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
	if(size == 0)
		return -2; 				// Size cannot be 0.
	
	int i, place = 1;
	for(i = 0; i<--size; i++)
		place*=maxsteps[i-1];		// We need to start with the last place.
	
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
		place /= maxsteps[i];		// Update the place
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
void update_instr(ppcontrols *pc, PINSTR *instr, int num, int dstep, int pcstep) {
	// Updates instr, the instruction at num to the value it has at the specified
	// indices dstep (number of steps along the relevant dimension) and clindex 
	// (1-based linear index in the phase cycling table).
	// Setting either to -1 is a skip condition

	// Fix phase cycling.	
	if(dstep >= 0) {
		// This comes after, because the ID variations are less versatile, and we'll let the tie go to them
		// Once we implement an increment-based phase cycling method, this will likely change
		if(instr->instr <= 8) { 	// An atomic function
//			GetTableCellVal(pc->cinst[num], pc->tlist, MakePoint(1, dstep), &instr->instr_time);
//			GetCtrlVal(pc->cinst[num], pc->tlistu, &instr->time_units);	
		}
		
//		if(takes_instr_data(instr->instr))
//			GetTableCellVal(pc->cinst[num], pc->idlist, MakePoint(1, dstep), &instr->instr_data);
	}
}


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

int insert_instruction(PPROGRAM *p, PINSTR *instr, int num)
{
	/*
	PINSTR *instr_buffer = malloc(sizeof(PINSTR));
	int i;
	
	p->n_inst++;
	
	p->flags = realloc(p->flags, sizeof(int)*p->n_inst);
	p->instr = realloc(p->instr, sizeof(int)*p->n_inst);
	p->instr_data = realloc(p->instr_data, sizeof(int)*p->n_inst);
	p->trigger_scan = realloc(p->trigger_scan, sizeof(int)*p->n_inst);
	p->instruction_time= realloc(p->instruction_time, sizeof(double)*p->n_inst);
	p->time_units = realloc(p->time_units, sizeof(double)*p->n_inst);
	
	for(i = p->n_inst-1; i>num; i--)
	{
		instr_buffer->flags = p->flags[i-1];
		instr_buffer->instr = p->instr[i-1];
		instr_buffer->instr_data = p->instr_data[i-1];
		instr_buffer->trigger_scan = p->trigger_scan[i-1];
		instr_buffer->instruction_time = p->instruction_time[i-1];
		instr_buffer->time_units = p->time_units[i-1];
		
		p->flags[i] = instr_buffer->flags;
		p->instr[i] = instr_buffer->instr;
		p->instr_data[i] = instr_buffer->instr_data;
		p->trigger_scan[i] = instr_buffer->trigger_scan;
		p->instruction_time[i] = instr_buffer->instruction_time;
		p->time_units[i] = instr_buffer->time_units;
		
	}
	
	p->flags[num] = instr->flags;
	p->instr[num] = instr->instr;
	p->instr_data[num] = instr->instr_data;
	p->trigger_scan[num] = instr->trigger_scan;
	p->instruction_time[i] = instr->instruction_time;
	p->time_units[i] = instr->time_units;
	p->total_time += instr->instruction_time;
	*/
	return 0;

}

int pprogramcpy(PPROGRAM *p_f, PPROGRAM *p_t)
{
	/*
	if(p_f == NULL)
		return -1;
	
	p_t->nPoints = p_f->nPoints;
	p_t->transient = p_f->transient;
	p_t->scan = p_f->scan;
	p_t->n_inst = p_f->n_inst;
	p_t->ntransients = p_f->ntransients;
	p_t->trigger_ttl = p_f->trigger_ttl;
	p_t->delaytime = p_f->delaytime;
	p_t->total_time = p_f->total_time;
	p_t->samplingrate = p_f->samplingrate;
	
	p_t->nDimensions = p_f->nDimensions;
	
	p_t->phasecycle = p_f->phasecycle;
	p_t->cyclefreq = p_f->cyclefreq;
	p_t->numcycles = p_f->numcycles;
	p_t->phasecycleinstr = p_f->phasecycleinstr;

	int j = sizeof(p_f->filename);
	
	if(p_f->filename != NULL)
	{
		int len = strlen(p_f->filename);
		p_t->filename = malloc(len);
		strcpy(p_t->filename, p_f->filename);
	} else
		p_t->filename == NULL;
	
	
	int i, n = p_f->n_inst;
	
	p_t->flags = malloc(sizeof(int)*n);
	p_t->instr = malloc(sizeof(int)*n);
	p_t->instr_data = malloc(sizeof(int)*n);
	p_t->trigger_scan = malloc(sizeof(int)*n);
	p_t->instruction_time = malloc(sizeof(double)*n);
	p_t->time_units = malloc(sizeof(double)*n);
	
	for(i = 0; i<n; i++)
	{
		p_t->flags[i] = p_f->flags[i];
		p_t->instr[i] = p_f->instr[i];
		p_t->instr_data[i] = p_f->instr_data[i];
		p_t->trigger_scan[i] = p_f->trigger_scan[i];
		p_t->instruction_time[i] = p_f->instruction_time[i];
		p_t->time_units[i] = p_f->time_units[i];
	}
	
	if(p_f->nDimensions > 1)
	{
		p_t->nVaried = p_f->nVaried;     
			
		n = sizeof(int)*p_f->nVaried;
		p_t->dimension = malloc(n);
		p_t->v_instr_num = malloc(n);
		p_t->v_instr_type = malloc(n);
		
		n = sizeof(double)*p_f->nVaried;
		p_t->init = malloc(n);
		p_t->initunits = malloc(n);
		p_t->inc = malloc(n);
		p_t->incunits = malloc(n);
		p_t->final = malloc(n);
		p_t->finalunits = malloc(n);
		
		n = sizeof(int)*p_f->nVaried;
		p_t->init_id = malloc(n);
		p_t->inc_id = malloc(n);
		p_t->final_id = malloc(n);
		
		n = sizeof(int)*(p_f->nDimensions - 1);
		p_t->nSteps = malloc(n);
		p_t->step = malloc(n);
		
		for(i = 0; i<p_t->nVaried; i++)
		{
			p_t->init_id[i] = p_f->init_id[i];
			p_t->inc_id[i] = p_f->inc_id[i];
			p_t->final_id[i] = p_f->final_id[i];
			
			
			p_t->init[i] = p_f->init[i];
			p_t->initunits[i] = p_f->initunits[i];
			p_t->inc[i] = p_f->inc[i];
			p_t->incunits[i] = p_f->incunits[i];
			p_t->final[i] = p_f->final[i];
			p_t->finalunits[i] = p_f->finalunits[i];
			p_t->v_instr_num[i] = p_f->v_instr_num[i];
			p_t->v_instr_type[i] = p_f->v_instr_type[i];
			p_t->dimension[i] = p_f->dimension[i];
		}
		
		for(i = 0; i<p_t->nDimensions-1; i++)
		{
			p_t->step[i] = p_f->step[i];
			p_t->nSteps[i] = p_f->nSteps[i];
		}
	}
	*/
	return 0;

}


