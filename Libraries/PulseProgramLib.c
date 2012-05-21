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
#include <utility.h>
#include <ansi_c.h>
#include <FileSave.h>
#include <MathParserLib.h>				// For parsing math
#include <UIControls.h>					// For manipulating the UI controls

#include <MCUserDefinedFunctions.h>
#include <PulseProgramLib.h>
#include <PulseProgramLibPriv.h>
#include <SaveSessionLib.h>

#include <Version.h>
#include <General.h>
#include <ErrorLib.h>
#include <ErrorDefs.h>

#include <PPConversion.h>
#include <MC10.h>


#include <spinapi.h>

//////////////////////////////////////////////////////////////
// 															//
//				File Read/Write Operations					//
// 															//
//////////////////////////////////////////////////////////////

PPROGRAM *LoadPulseProgram(char *fname, int safe, int *ev) {
	int rv = 0, locked = 0;
	FILE *f = NULL;
	PPROGRAM *p = NULL;
	
	if(safe) { 
		CmtGetLock(lock_tdm);
		locked = 1;
	}
	
	if(fname == NULL || !file_exists(fname)) { rv = MCPP_ERR_NOFILE; goto error; }
	
	f = fopen(fname, "rb");
	if(f == NULL) { rv = MCPP_ERR_FILEREAD; goto error; }
	
	p = load_pprogram(f, &rv);
	if(rv != 0) { goto error; }
	
	
	error:
	
	if(f != NULL) { fclose(f); }

	if(safe && locked) {
		CmtReleaseLock(lock_tdm);
		locked = 0;
	}			   
	
	if(rv != 0) {
		if(p != NULL) { free_pprog(p); }
		p = NULL;
	}
	
	*ev = rv;
	return p;
}


PPROGRAM *load_pprogram(FILE *f, int *ev) {
	// Load a program from a .pp file (or any file with that format)
	
	if(f == NULL || feof(f)) { *ev = MCPP_ERR_NOFILE; return NULL; }
	
	PPROGRAM *p = NULL;
	fsave *fs = NULL, *bfs = NULL;
	char *buff = NULL, **names = NULL;
	int rv = 0, i, nsize = 0;
	int *plocs = NULL, *glocs = NULL;
	PINSTR *ilist = NULL;
	size_t count, si = sizeof(unsigned int), si8 = sizeof(unsigned char), sd = sizeof(double);
	unsigned int fs_size = 0, bfs_size = 0;
	
	flocs fl = read_flocs_from_file(f, &rv, -1);
	if(rv != 0) { goto error; }
	
	if(fl.num == 0) {
		rv = MCPP_ERR_NOFLOCS;
		goto error;
	}
	
	// Find the pulse program group and read it into a char array.
	for(i = 0; i < fl.num; i++) {
		if(strcmp(fl.name[i], MCPP_PROGHEADER) == 0) {
			fseek(f, fl.pos[i], SEEK_SET); // Move the header to the relevant position
			
			buff = malloc(fl.size[i]);
			
			count = fread(buff, 1, fl.size[i], f);
			if(count != fl.size[i]) {
				rv = MCPP_ERR_FILEREAD;
				goto error;
			}
			
			break;
		}
	}
	
	if(buff == NULL) { 
		rv = MCPP_ERR_FILE_NOPROG;
		goto error;
	}
	
	// Convert the char array into an fsave array.
	fs = read_all_fsaves_from_char(buff, &rv, &fs_size, fl.size[i]);
	if(rv != 0) { goto error; }
	if(buff != NULL) { free(buff); }
	buff = NULL;
	
	
	// Index the locations of all the groups.
	char *groups[MCPP_GROUPSNUM];
	
	groups[MCPP_PROPORD] = MCPP_PROPHEADER;
	groups[MCPP_INSTORD] = MCPP_INSTHEADER;
	groups[MCPP_AOORD] = MCPP_AOHEADER;
	groups[MCPP_NDORD] = MCPP_NDHEADER;
	groups[MCPP_SKIPORD] = MCPP_SKIPHEADER;
	
	// Read out the properties.
	nsize = fs_size;
	names = malloc(sizeof(char *)*nsize);
	for(i = 0; i < fs_size; i++) { names[i] = NULL; }

	// First copy the names out into a string array for quick indexing.
	for(i = 0; i < fs_size; i++) {
		names[i] = malloc(fs[i].ns);
		memcpy(names[i], fs[i].name, fs[i].ns);
		if(names[i][fs[i].ns-1] != '\0') {
			rv = MCPP_ERR_MALFORMED_FNAME;
			goto error;
		}
	}
	
	// Get the index, then we can throw away the names.
	glocs = strings_in_array(names, groups, fs_size, MCPP_GROUPSNUM);
	
	names = free_string_array(names, fs_size);
	nsize = 0;
	
	int pl = glocs[MCPP_PROPORD];
	if(pl < 0) {
		rv = MCPP_ERR_FILE_NOPROPS;
		goto error;
	}
	
	if(glocs[MCPP_INSTORD] < 0) {
		rv = MCPP_ERR_NOINSTRS;
		goto error;
	}
	
	buff = malloc(fs[pl].size);
	memcpy(buff, fs[pl].val.c, fs[pl].size);
	
	bfs = read_all_fsaves_from_char(buff, &rv, &bfs_size, fs[pl].size);
	if(rv != 0) { goto error; }
	
	// Read out the properties.
	nsize = bfs_size;
	names = malloc(sizeof(char *)*nsize);
	for(i = 0; i < bfs_size; i++) { names[i] = NULL; }
	
	// First copy the names out into a string array for quick indexing.
	for(i = 0; i < bfs_size; i++) {
		names[i] = malloc(bfs[i].ns);
		memcpy(names[i], bfs[i].name, bfs[i].ns);
		if(names[i][bfs[i].ns-1] != '\0') {
			rv = MCPP_ERR_MALFORMED_FNAME;
			goto error;
		}
	}
	
	// We need to make the PPROGRAM now.
	p = calloc(1, sizeof(PPROGRAM));
	
	// Initializations for legacy reasons:
	p->use_pb = 1;
	p->valid = 1;
	
	char *props[MCPP_PROPSNUM] = {MCPP_VERSION, MCPP_NP, MCPP_SR, MCPP_NT, MCPP_TRIGTTL, MCPP_TMODE, 
								 MCPP_SCAN, MCPP_USE_PB, MCPP_VARIED, MCPP_NINST, MCPP_TOTALTIME, MCPP_NUINSTRS, 
								 MCPP_NDIMS, MCPP_NCYCS, MCPP_NVARIED, MCPP_MAXNSTEPS, MCPP_REALNSTEPS, 
								 MCPP_SKIP, MCPP_NAOUT, MCPP_NAOVAR};
	void *pfields[MCPP_PROPSNUM] = {NULL, &(p->np), &(p->sr), &(p->nt), &(p->trigger_ttl), &(p->tmode),
									&(p->scan), &(p->use_pb), &(p->varied), &(p->n_inst), &(p->total_time),
									&(p->nUniqueInstrs), &(p->nDims), &(p->nCycles), &(p->nVaried),
									&(p->max_n_steps), &(p->real_n_steps), &(p->skip), &(p->nAout), 
									&(p->n_ao_var)};
	
	plocs = strings_in_array(names, props, bfs_size, MCPP_PROPSNUM);
	
	if(plocs == NULL) {
		rv = MCPP_ERR_PROG_PROPS_LABELS;
		goto error;
	}
	
	// These are all scalars, we'll read the values directly into the PPROGRAM;
	// TODO: Add error checking for required items.
	int loc;
	for(i = 0; i < MCPP_PROPSNUM; i++) {
		loc = plocs[i];
		if(loc < 0) { continue; }
	
		if(pfields[loc] == NULL) { continue; }
		
		switch(bfs[loc].type) {
			case FS_INT:
				*(int *)pfields[i] = *(bfs[loc].val.i);
				break;
			case FS_UCHAR: // Anything stored as UCHAR in the file is actually stored as int in PPROGRAM.
				*(int *)pfields[i] = *(unsigned char *)(bfs[loc].val.c);
				break;
			case FS_DOUBLE:
				*(double *)pfields[i] = *(bfs[loc].val.d);
				break;
			default:
				continue;
		}
	}
	
	free(plocs);
	plocs = NULL;
	
	free(buff);
	buff = NULL;
	
	names = free_string_array(names, nsize);
	nsize = 0;
	
	bfs = free_fsave_array(bfs, bfs_size);
	bfs_size = 0;
	// We can now create the PPROGRAM before reading out the instrs array and such.
	create_pprogram(p);
	
	// Get the instructions array
	pl = glocs[MCPP_INSTORD];
	
	ilist = read_pinstr_from_char(fs[pl].val.c, p->nUniqueInstrs, &rv);
	if(rv != 0) { goto error; }

	for(i = 0; i < p->nUniqueInstrs; i++) { 
		memcpy(p->instrs[i], &(ilist[i]), sizeof(PINSTR));
	}
	
	free(ilist);
	ilist = NULL;
	
	// Read out the things related to ND (if necessary)
	// Must be done before the AOut and Skip stuff
	pl = glocs[MCPP_NDORD];
	if(p->varied) {
		if(pl < 0) { rv = MCPP_ERR_NOPROG; goto error; }
		
		buff = malloc(fs[pl].size);
		memcpy(buff, fs[pl].val.c, fs[pl].size);
		
		bfs = read_all_fsaves_from_char(buff, &rv, &bfs_size, fs[pl].size);
	
		free(buff);
		buff = NULL;
		
		nsize = bfs_size;
		names = malloc(sizeof(char *)*nsize);
		for(i = 0; i < bfs_size; i++) { names[i] = NULL; }

		for(i = 0; i < bfs_size; i++) {
			names[i] = malloc(bfs[i].ns);
			memcpy(names[i], bfs[i].name, bfs[i].ns);
			if(names[i][bfs[i].ns-1] != '\0') {
				rv = MCPP_ERR_MALFORMED_FNAME;
				goto error;
			}
		}
		
		char *ndnames[MCPP_NDNUM] = { MCPP_MAXSTEPS, MCPP_STEPS, MCPP_VINS, MCPP_VINSDIM, 
									  MCPP_VINSMODE, MCPP_VINSLOCS, MCPP_DELAYEXPRS, 
									  MCPP_DATAEXPRS };
		plocs = strings_in_array(names, ndnames, nsize, MCPP_NDNUM);
		
		if(plocs == NULL) { rv = MCPP_ERR_FIELDSMISSING; goto error; }    	
		
		int i = 0;
		int msl = plocs[i++], sl = plocs[i++], vinsl = plocs[i++], vidiml = plocs[i++], 
			vimodel = plocs[i++], vill = plocs[i++], dexl = plocs[i++], 
			daxl = plocs[i++];
		
		if(msl < 0) { rv = MCPP_ERR_FIELDSMISSING; goto error; }
		memcpy(p->maxsteps, bfs[msl].val.i, bfs[msl].size);	// Print the maxsteps array
		
		if(sl < 0) { 
			int *steps = get_steps_array(p->tmode, p->maxsteps, p->nCycles, p->nDims, p->nt);
			if(steps != NULL) {
				memcpy(p->steps, steps, (p->steps_size)*sizeof(unsigned int));
			} else {
				p->steps = NULL;
			}
		} else {
			memcpy(p->steps, bfs[sl].val.ui, bfs[sl].size);
		}
		
		if(p->nVaried > 0) { // We only need to do this if it's varied along a dimension or cycle
			if(vinsl < 0 || vidiml < 0 || vimodel < 0 || vill < 0) { rv = MCPP_ERR_FIELDSMISSING; goto error; }
			
			memcpy(p->v_ins, bfs[vinsl].val.i, bfs[vinsl].size);
			
			int j = 0;
			for(i = 0; i < p->nVaried; i++) {
				p->v_ins_dim[i] = (int)bfs[vidiml].val.uc[i];
				p->v_ins_mode[i] = (int)bfs[vimodel].val.uc[i];
				
				memcpy(p->v_ins_locs[i], &(bfs[vill].val.i[j]), p->max_n_steps*si);
				j+= p->max_n_steps;
			}
			
			int nvi = p->nVaried;
			int ns = nvi;
			if(dexl >= 0) {
				p->delay_exprs = get_nc_strings(bfs[dexl].val.c, &ns);
				
				if(ns != nvi) {
					rv = MCPP_ERR_INVALID_NC_STRING;
					goto error;
				}
			}
			
			if(daxl >= 0) {
				p->data_exprs = get_nc_strings(bfs[daxl].val.c, &ns);
				
				if(ns != nvi) {
					rv = MCPP_ERR_INVALID_NC_STRING;
					goto error;
				}
			}
		}
		
		free(plocs);
		plocs = NULL;
	}
	
	// Read out things related to analog outputs (if necessary)
	pl = glocs[MCPP_AOORD];
	if(p->nAout > 0) {
		if(pl < 0) { rv = MCPP_ERR_NOAOUT; goto error; }
		
		buff = malloc(fs[pl].size);
		memcpy(buff, fs[pl].val.c, fs[pl].size);
		
		bfs = read_all_fsaves_from_char(buff, &rv, &bfs_size, fs[pl].size);
	
		free(buff);
		buff = NULL;
		
		nsize = bfs_size;
		names = malloc(sizeof(char *)*nsize);
		for(i = 0; i < bfs_size; i++) { names[i] = NULL; }

		for(i = 0; i < bfs_size; i++) {
			names[i] = malloc(bfs[i].ns);
			memcpy(names[i], bfs[i].name, bfs[i].ns);
			if(names[i][bfs[i].ns-1] != '\0') {
				rv = MCPP_ERR_MALFORMED_FNAME;
				goto error;
			}
		}

		
		char *aonames[MCPP_AONUM] = {MCPP_AOVARIED, MCPP_AODIM, MCPP_AOVALS, MCPP_AOCHANS, MCPP_AOEXPRS};
		plocs = strings_in_array(names, aonames, nsize, MCPP_AONUM);
		
		if(plocs == NULL) { rv = MCPP_ERR_FIELDSMISSING; goto error; }
		
		int ao_varl = plocs[0], ao_diml = plocs[1], ao_valsl = plocs[2];
		int ao_chansl = plocs[3], ao_exprsl = plocs[4];
		
		if(ao_varl < 0) { rv = MCPP_ERR_FIELDSMISSING; goto error; }
		if(ao_valsl < 0) { rv = MCPP_ERR_FIELDSMISSING; goto error; }
		
		int avsize, point = 0;
		for(i = 0; i < p->nAout; i++) {
			p->ao_varied[i] = (int)(bfs[ao_varl].val.uc[i]);
			p->ao_dim[i] = (int)(bfs[ao_diml].val.uc[i]);
			
			// Dynamically allocate the ao_vals array, then copy it over and implement index
			// This delinearizes the array as we go along.
			if(p->ao_varied[i]) {
				avsize = p->maxsteps[p->ao_dim[i] + p->nCycles];
			} else {
				avsize = 1;
			}
			
			p->ao_vals[i] = malloc(avsize *= sd);
			
			memcpy(p->ao_vals[i], &(bfs[ao_valsl].val.d[point]), avsize);
			point += avsize;
		}
		
		// Get the nc_strings from the remaining two.
		int na = p->nAout;
		int ns = na;
		
		p->ao_chans = get_nc_strings(bfs[ao_chansl].val.c, &na);
		if(na != ns) {
			rv = MCPP_ERR_INVALID_NC_STRING;
			goto error;
		}
		
		p->ao_exprs = get_nc_strings(bfs[ao_exprsl].val.c, &na);
		if(na != ns) {
			rv = MCPP_ERR_INVALID_NC_STRING;
			goto error;
		}
		
		free(plocs);
		plocs = NULL;
		
		names = free_string_array(names, nsize);
		nsize = 0;
		
		bfs = free_fsave_array(bfs, bfs_size);
		bfs_size = 0;
	}
	
	
	// Now the skips
	pl = glocs[MCPP_SKIPORD];
	if(p->skip) {
		if(pl < 0) {
			rv = MCPP_ERR_FIELDSMISSING;
			goto error;
		}
		
		buff = malloc(fs[pl].size);
		memcpy(buff, fs[pl].val.c, fs[pl].size);
		
		bfs = read_all_fsaves_from_char(buff, &rv, &bfs_size, fs[pl].size);
	
		free(buff);
		buff = NULL;
		
		nsize = bfs_size;
		names = malloc(sizeof(char *)*nsize);
		for(i = 0; i < bfs_size; i++) { names[i] = NULL; }

		for(i = 0; i < bfs_size; i++) {
			names[i] = malloc(bfs[i].ns);
			memcpy(names[i], bfs[i].name, bfs[i].ns);
			if(names[i][bfs[i].ns-1] != '\0') {
				rv = MCPP_ERR_MALFORMED_FNAME;
				goto error;
			}
		}
		
		char *skipnames[MCPP_SKIPNUM] = {MCPP_SKIPEXPR, MCPP_SKIPLOCS};
		plocs = strings_in_array(names, skipnames, nsize, MCPP_SKIPNUM);
		
		int skexl = plocs[0], sll = plocs[1];
		
		if(bfs[skexl].size <= 0) {
			p->skip = 0;
			free(p->skip_expr);
			p->skip_expr = NULL;
		} else {
			p->skip_expr = realloc(p->skip_expr, bfs[skexl].size);
			memcpy(p->skip_expr, bfs[skexl].val.c, bfs[skexl].size);
			
			memcpy(p->skip_locs, bfs[sll].val.i, bfs[sll].size);
		}
		
		free(plocs);
		plocs = NULL;
	}
	
	error:
	free_flocs(&fl);
	fs = free_fsave_array(fs, fs_size);
	bfs = free_fsave_array(bfs, bfs_size);
	
	if(glocs != NULL) { free(glocs); }
	if(plocs != NULL) { free(plocs); }
	if(names != NULL) { free_string_array(names, nsize); }
	if(buff != NULL) { free(buff); }
	if(ilist != NULL) { free(ilist); }
	
	if(rv != 0) {
		free_pprog(p);
		p = NULL;
	}
	
	*ev = rv;
	return p;
}

PINSTR *read_pinstr_from_char(char *array, int n_inst, int *ev) {
	// Pass this the contents of the FS_CUSTOM struct that contains an array
	// of instructions, this will read that out into an array of PINSTRs.
	
	if(array == NULL) { *ev = MCPP_ERR_NOSTRING; return NULL; }
	if(n_inst <= 0) { *ev = MCPP_ERR_NOINSTRS; return NULL; }
	
	PINSTR *in = NULL;
	char **c = NULL;
	int rv = 0, i;
	int *locs = NULL;
	void *val = NULL;
	unsigned int n_entries, c_size = 0, ns;
	size_t si = sizeof(unsigned int), si8 = sizeof(unsigned char), sd = sizeof(double);
	
	char *pos = array;
	memcpy(&n_entries, pos, si);
	pos += si;
	
	if(n_entries == 0) { rv = MCF_ERR_CUST_NOENTRIES; goto error; } // TODO: Fix
	
	in = malloc(sizeof(PINSTR)*n_inst);
	c = calloc(n_entries, sizeof(char *));
	c_size = n_entries;
	
	// Read out the field names (gives order)
	for(i = 0; i < n_entries; i++) { 
		memcpy(&ns, pos, si);
		pos += si;
		
		if(ns == 0) { continue; }
		
		c[i] = malloc(ns);
		memcpy(c[i], pos, ns);
		pos += ns + si8;		// Skip the type.
	}
	
	void *ifields[MCPP_INSTNUMFIELDS];
	unsigned int inst_sizes[MCPP_INSTNUMFIELDS], types[MCPP_INSTNUMFIELDS] = MCPP_INST_TYPES;
	for(i = 0; i < MCPP_INSTNUMFIELDS; i++) { 	// Field sizes
		inst_sizes[i] = get_fs_type_size(types[i]);
	}
	
	char *names[MCPP_INSTNUMFIELDS] = MCPP_INST_NAMES;
	
	locs = strings_in_array(names, c, MCPP_INSTNUMFIELDS, n_entries);	// Find the locations as stored
	
	// Read them out into the various instr arrays.
	int j, l = 0;
	val = malloc(sd);	// Big enough to contain the largest item
	for(i = 0; i < n_inst; i++) {
		ifields[0] = &(in[i].flags);
		ifields[1] = &(in[i].instr);
		ifields[2] = &(in[i].instr_data);
		ifields[3] = &(in[i].trigger_scan);
		ifields[4] = &(in[i].instr_time);
		ifields[5] = &(in[i].time_units);
		
		for(j = 0; j < n_entries; j++) {
			l = locs[j];
			if(l < 0) { continue; }	// Skip if it's not found
			
			// Read out the value.
			memset(val, 0, sd);	// Zero it out first - > may not be necessary
			memcpy(val, pos, inst_sizes[l]); 
			pos += inst_sizes[l];
			
			switch(types[l]) {
				case FS_INT:
				case FS_UCHAR:
					*(int *)ifields[l]  = *(int *)val;
					break;
				case FS_UINT:
					*(unsigned int*)ifields[l] = *(unsigned int *)val;
					break;
				case FS_DOUBLE:
					*(double*)ifields[l] = *(double *)val;
					break;
			}
		}
	}
	
	error:
	if(c != NULL && c_size > 0) { free_string_array(c, c_size); }
	if(locs != NULL) { free(locs); }
	if(val != NULL) { free(val); }
	
	if(rv != 0) {
		free(in);
		in = NULL;
	}
	
	*ev = rv;
	return in;
}

int SavePulseProgram(PPROGRAM *p, char *filename, int safe) {
	// Save a program to file

	// Try to open the file
	int rv = 0, locked = 0;
	FILE *f = NULL;
	char *tname = NULL;
	char *new_fname = NULL;
	char *fname = NULL, *ext = NULL;
	
	if(filename == NULL) { return MCPP_ERR_NOFILE; }
	
	if(p == NULL) { 
		p = safe?get_current_program_safe():get_current_program();
		if(p == NULL) {
			rv = MCPP_ERR_NOPROG;
			goto error;
		}
	}
	
	// Make sure it's got the right extension.
	ext = get_extension(filename);
	if(ext == NULL || strlen(ext) < 3 || strcmp(ext+1, PPROG_EXTENSION)) {
		// Append the extension.
		int nl = strlen(filename);
		if(ext != NULL) { nl -= strlen(ext); }
		
		fname = calloc(nl+strlen(PPROG_EXTENSION)+1, 1);
		strncpy(fname, filename, nl);
		strcat(fname, ".");
		strcat(fname, PPROG_EXTENSION);
	} else {
		fname = malloc(strlen(filename)+1);
		strcpy(fname, filename);
	}
	
	// Start by creating a temporary filename.
	tname = temp_file(PPROG_EXTENSION);
	
	if(tname == NULL) { 
		rv MCPP_ERR_TEMP_FILE_NAME;
		goto error;
	}
	
	// Open a temporary file for writing
	f = fopen(tname, "wb+");
	if(f == NULL) { 
		rv = MCPP_ERR_TEMP_FILE;
		goto error; 
	}
	
	if(safe) {
		CmtGetLock(lock_tdm);
		locked = 1;
	}
	
	rv = save_pprogram(p, f);
	
	fclose(f);
	f = NULL;
	
	int err = 0;
	if(file_exists(fname)) {
		err = remove(fname);

		if(err) {
			int len = strlen(fname)+1;
			new_fname = malloc(len+8);
			int i;
			for(i = len; i > 0; i--) {
				if(fname[i] == '.') { break ; }
			}
			
			fname[i] = '\0';
			
			for(int j = 1; j < 100; i++) { 
				sprintf(new_fname, "%s(%d).pp", fname, j);
				if(!file_exists(new_fname)) { break; }
			}
			
			if(file_exists(new_fname)) { 
				int rv = -MCPP_ERR_FILE_MOVING;
				goto error; 
			}
		}
	}
	
	if(new_fname == NULL) { 
		err = rename(tname, fname);
	} else { 
		err = rename(tname, new_fname);
		free(new_fname);
		new_fname = NULL;
	}
	
	if(safe && locked) {
		CmtReleaseLock(lock_tdm);	
	}
	
	error:
	if(f != NULL) { fclose(f); }
	if(fname != NULL) { free(fname); }
	if(tname != NULL)  {
		if(file_exists(tname)) { remove(tname); }
		free(tname);
	}
	if(new_fname != NULL) { 
		free(new_fname);	
	}
	
	return rv;
}
 
int save_pprogram(PPROGRAM *p, FILE *f) {
	// Save the pulse program to a file "fname"

	// Vars
	int rv = 0;
	char *buff = NULL;
	fsave header = null_fs(), instrs = null_fs();
	fsave ao = null_fs(), nd = null_fs(), skip = null_fs();
	
	if(f == NULL) { 
		rv = MCPP_ERR_NOFILE;
		goto error;
	}
	
	if(p == NULL) { 
		rv = MCPP_ERR_NOPROG;
		goto error;
	}
	
	int s = 0;
	size_t written;
	size_t sd = sizeof(double), si = sizeof(int), si8 = sizeof(unsigned char);
	
	// First the main "Pulse Program" header
	char *hstr = MCPP_PROGHEADER;
	int len = strlen(hstr)+1;
	
	// Write the title
	fwrite(&len, si, 1, f); 
	fwrite(hstr, si8, len, f);
	
	// Get the position for later.
	fpos_t pos_marked;
	unsigned int pos_init, pos_done;
	unsigned char type = FS_CONTAINER;
	
	fwrite(&type, sizeof(unsigned char), 1, f);
	
	fgetpos(f, &pos_marked);
	fwrite(&len, si, 1, f); // Write as a placeholder for size
	
	pos_init = ftell(f);

	// Header first
	header = generate_header(p, &rv);
	if(rv != 0) { goto error; }
	
	if(rv = fwrite_fs(f, &header)) { goto error; }
	
	header = free_fsave(&header);
	
	// Now that we have the header written, we want to start writing 
	// some of the arrays. We'll start with the instruction array, as that's
	// the most important thing.
	instrs = generate_instr_array(p);
	if(instrs.type == FS_NULL) {
		rv = MCPP_ERR_NOINSTRS;
		goto error;
	}
	
	if(rv = fwrite_fs(f, &instrs)) { goto error; }
	
	instrs = free_fsave(&instrs);
	
	// Next write the array of things relating to analog outputs (if necessary)
	if(p->nAout > 0) {
		ao = generate_ao(p, &rv);
		if(rv != 0) {
			rv = MCPP_ERR_NOAOUT;
			goto error;
		}
		
		if(rv = fwrite_fs(f, &ao)) { goto error; }
		
		ao = free_fsave(&ao);
	}
	
	// Now the multidimensional stuff.
	if(p->varied) {
		nd = generate_nd(p, &rv);
		if(rv != 0) {
			rv = MCPP_ERR_NOND;	
			goto error;
		}
		
		if(rv = fwrite_fs(f, &nd)) { goto error; }
		
		nd = free_fsave(&nd);
	}
	
	// Now the skip array
	if(p->skip) {
		skip = generate_skip_fs(p, &rv);
		if(rv != 0) { 
			rv = MCPP_ERR_NOSKIP;
			goto error;
		}
		
		if(rv = fwrite_fs(f, &skip)) { goto error; }
		
		skip = free_fsave(&skip);
	}
	
	// Get the final position
	pos_done = ftell(f);

	// Write the difference back at the beginning
	unsigned int size = pos_done - pos_init;
	fpos_t pos_end;
	
	fgetpos(f, &pos_end);
	fsetpos(f, &pos_marked);
	
	fwrite(&size, sizeof(unsigned int), 1, f);	// This will break for programs bigger than
												// 4GB. Be aware of this extremely common scenario.

	error:
	if(buff != NULL) { free(buff); }
	
	if(header.type != FS_NULL) { free_fsave(&header); }
	if(instrs.type != FS_NULL) { free_fsave(&instrs); }
	if(ao.type != FS_NULL) { free_fsave(&ao); }
	if(nd.type != FS_NULL) { free_fsave(&nd); }
	if(skip.type != FS_NULL) { free_fsave(&skip); }
	
	return rv;	
}

fsave generate_header(PPROGRAM *p, int *ev) {
	// Generates the header as an unsigned char array
	// Must be freed when you're done with it.
	
	int rv = 0;
	int h_f = MCPP_PROPSNUM, cf = 0;
	fsave *fs = NULL, out = null_fs();
	char *header = NULL;

	if(p == NULL) { 
		rv = MCPP_ERR_NOPROG;
		goto error; 
	}
	
	double version = PPROG_VERSION;

	// Create an fsave array
	fs = calloc(h_f, sizeof(fsave));
	
	// Version (Double)
	fs[cf] = make_fs(MCPP_VERSION);
	if(rv = put_fs(&fs[cf], &version, FS_DOUBLE, 1)) { goto error; }
	
	// Num Points (Int)
	fs[++cf] = make_fs(MCPP_NP);
	if(rv = put_fs(&fs[cf], &(p->np), FS_INT, 1)) { goto error; }
	
	// Sampling Rate (Double)
	fs[++cf] = make_fs(MCPP_SR);
	if(rv = put_fs(&fs[cf], &(p->sr), FS_DOUBLE, 1)) { goto error; }
	
	// Num Transients (Int)
	fs[++cf] = make_fs(MCPP_NT);
	if(rv = put_fs(&fs[cf], &(p->nt), FS_INT, 1)) { goto error; }
	
	// Trigger TTL  (UChar)
	fs[++cf] = make_fs(MCPP_TRIGTTL);
	if(rv = put_fs(&fs[cf], &(p->trigger_ttl), FS_UCHAR, 1)) { goto error; }
	
	// Transient mode (UChar)
	fs[++cf] = make_fs(MCPP_TMODE);
	if(rv = put_fs(&fs[cf], &(p->tmode), FS_UCHAR, 1)) { goto error; }
	
	// Scan (UChar)
	fs[++cf] = make_fs(MCPP_SCAN);
	if(rv = put_fs(&fs[cf], &(p->scan), FS_UCHAR, 1)) { goto error; }
	
	// Use PB (UChar)
	fs[++cf] = make_fs(MCPP_USE_PB);
	if(rv = put_fs(&fs[cf], &(p->use_pb), FS_UCHAR, 1)) { goto error; }
	
	// Varied (UChar)
	fs[++cf] = make_fs(MCPP_VARIED);
	if(rv = put_fs(&fs[cf], &(p->varied), FS_UCHAR, 1)) { goto error; }
	
	// Num Instrs (Int)
	fs[++cf] = make_fs(MCPP_NINST);
	if(rv = put_fs(&fs[cf], &(p->n_inst), FS_INT, 1)) { goto error; }
	
	// NUniqueInstrs (Int)
	fs[++cf] = make_fs(MCPP_NUINSTRS);
	if(rv = put_fs(&fs[cf], &(p->nUniqueInstrs), FS_INT, 1)) { goto error; }

	// Total time (Double)
	fs[++cf] = make_fs(MCPP_TOTALTIME);
	if(rv = put_fs(&fs[cf], &(p->total_time), FS_DOUBLE, 1)) { goto error; }

	// Num dims (Int)
	fs[++cf] = make_fs(MCPP_NDIMS);
	if(rv = put_fs(&fs[cf], &(p->nDims), FS_INT, 1)) { goto error; }
	
	// Num cycles (Int)
	fs[++cf] = make_fs(MCPP_NCYCS);
	if(rv = put_fs(&fs[cf], &(p->nCycles), FS_INT, 1)) { goto error; }

	// Num Varied (Int)
	fs[++cf] = make_fs(MCPP_NVARIED);
	if(rv = put_fs(&fs[cf], &(p->nVaried), FS_INT, 1)) { goto error; }
	
	// Max num steps (Int)
	fs[++cf] = make_fs(MCPP_MAXNSTEPS);
	if(rv = put_fs(&fs[cf], &(p->max_n_steps), FS_INT, 1)) { goto error; }
	
	// Real num steps (Int)
	fs[++cf] = make_fs(MCPP_REALNSTEPS);
	if(rv = put_fs(&fs[cf], &(p->real_n_steps), FS_INT, 1)) { goto error; }
	
	// Skip (UChar)
	fs[++cf] = make_fs(MCPP_SKIP);
	if(rv = put_fs(&fs[cf], &(p->skip), FS_UCHAR, 1)) { goto error; }
	
	// nAout (Int)
	fs[++cf] = make_fs(MCPP_NAOUT);
	if(rv = put_fs(&fs[cf], &(p->nAout), FS_INT, 1)) { goto error; }
	
	// Num AO Varying (Int)
	fs[++cf] = make_fs(MCPP_NAOVAR);
	if(rv = put_fs(&fs[cf], &(p->n_ao_var), FS_INT, 1)) { goto error; }
	
	// Generate the container
	out = make_fs(MCPP_PROPHEADER);
	if(rv = put_fs_container(&out, fs, h_f)) { goto error; }

	error:
	if(fs != NULL) { free_fsave_array(fs, h_f); }
	if(header != NULL) { free(header); }
	*ev = rv;

	return out;
}

fsave generate_instr_array(PPROGRAM *p) {
	// Generates a sequential array of instructions.
	if(p == NULL || p->instrs == NULL) { return null_fs(); }
	char *c = NULL;
	
	
	int n_instr = p->nUniqueInstrs;
	
	// Define the struct.
	unsigned int types[MCPP_INSTNUMFIELDS] = MCPP_INST_TYPES;
	char *names[MCPP_INSTNUMFIELDS] = MCPP_INST_NAMES;
	size_t size = 0, si = sizeof(unsigned int), si8 = sizeof(unsigned char), sd = sizeof(double);
	
	
	// Go through and create the char array.
	int i;
	for(i = 0; i < MCPP_INSTNUMFIELDS; i++) {
		size += get_fs_type_size(types[i]); 
	}
	
	c = malloc(size*n_instr);
	char *pos = c;
	
	
	// Must be harmonized with MCPP_INST_NAMES!
	// Current order is:
	// flags, instr, instr_data, trigger_scan, instr_time, time_units
	unsigned char ucbuff;
	for(i = 0; i < n_instr; i++) {
		memcpy(pos, &(p->instrs[i]->flags), si);
		pos += si;
		
		memcpy(pos, &(p->instrs[i]->instr), si);
		pos += si;
		
		memcpy(pos, &(p->instrs[i]->instr_data), si);
		pos += si;
		
		ucbuff = (unsigned char)(p->instrs[i]->trigger_scan);
		memcpy(pos, &ucbuff, si8);
		pos += si8;

		memcpy(pos, &(p->instrs[i]->instr_time), sd);
		pos += sd;
		
		ucbuff = (unsigned char)(p->instrs[i]->time_units);
		memcpy(pos, &ucbuff, si8);
		pos += si8;
	}
	
	fsave fs = make_fs(MCPP_INSTHEADER);
	put_fs_custom(&fs, c, MCPP_INSTNUMFIELDS, types, names, n_instr);

	error:
	if(c != NULL) { free(c); }
	
	return fs;
}

fsave generate_ao(PPROGRAM *p, int *ev) {
	// Generates the information relating to analog outputs
	fsave out = null_fs();
	char *val = NULL, *ao_chans = NULL, *ao_exprs = NULL;
	double *ao_vals = NULL;
	int i, rv = 0;
	fsave *fs = NULL;
	unsigned char *ucbuff = NULL;
	size_t sd = sizeof(double), si = sizeof(unsigned int), si8 = sizeof(unsigned char);
	
	
	if(p == NULL) { 
		rv = -200;
		goto error; 
	}

	// Generate field array
	int nf = MCPP_AONUM;
	fs = malloc(sizeof(fsave)*nf);
	int cf = 0;
	
	// AOut Varied (UChar)
	ucbuff = malloc(si8*p->nAout);
	for(i = 0; i < p->nAout; i++) {
		ucbuff[i] = (unsigned char)(p->ao_varied[i]);	
	}
	
	fs[cf] = make_fs(MCPP_AOVARIED);
	if(rv = put_fs(&fs[cf], ucbuff, FS_UCHAR, p->nAout)) { goto error; }
	
	// AOut Dimension (UChar)
	for(i = 0; i < p->nAout; i++) {
		ucbuff[i] = (unsigned char)(p->ao_dim[i]);
	}
	
	fs[++cf] = make_fs(MCPP_AODIM);
	if(rv = put_fs(&fs[cf], ucbuff, FS_UCHAR, p->nAout)) { goto error; }
	
	free(ucbuff);
	ucbuff = NULL;
	
	// AO Vals (Double, linearized array)
	size_t s_vals = p->nAout-p->n_ao_var; 
	ao_vals = malloc(sd*((s_vals == 0)?1:s_vals));

	if(p->n_ao_var > 0) {
		size_t c_size = 0;
		for(i = 0; i < p->nAout; i++) {
			if(p->ao_varied[i]) {
				c_size = p->maxsteps[p->nCycles+p->ao_dim[i]];
			} else {
				c_size = 1;
			}
		
			if(c_size > 1) {
				s_vals += c_size;
				ao_vals = realloc(ao_vals, s_vals*sd);
			}
			
			memcpy(&ao_vals[s_vals - c_size], p->ao_vals[i], sd*c_size); // Copy into the linearized array
		}
	} else {
		for(i = 0; i < p->nAout; i++) {
			ao_vals[i] = p->ao_vals[i][0];
		}
	}
	
	fs[++cf] = make_fs(MCPP_AOVALS);
	if(rv = put_fs(&fs[cf], ao_vals, FS_DOUBLE, s_vals)) { goto error; }
	
	// AOChans and AOExprs - Stored as newline-delimited strings (Char)
	int s_chans = 0, s_exprs = 0;
	ao_chans = generate_nc_string(p->ao_chans, p->nAout, &s_chans);
	ao_exprs = generate_nc_string(p->ao_exprs, p->nAout, &s_exprs);
	
	fs[++cf] = make_fs(MCPP_AOCHANS);
	if(rv = put_fs(&fs[cf], ao_chans, FS_CHAR, s_chans)) { goto error; }
	
	fs[++cf] = make_fs(MCPP_AOEXPRS);
	if(rv = put_fs(&fs[cf], ao_exprs, FS_CHAR, s_exprs)) { goto error; }

	// Finally generate the container
	out = make_fs(MCPP_AOHEADER);
	if(rv = put_fs_container(&out, fs, nf)) { goto error; }
	
	error:
	if(ucbuff != NULL) { free(ucbuff); }
	if(fs != NULL) { free_fsave_array(fs, nf); }
	if(ao_chans != NULL) { free(ao_chans); }
	if(ao_exprs != NULL) { free(ao_exprs); }
	if(ao_vals != NULL) { free(ao_vals); }

	*ev = rv;
	
	if(rv != 0) {
		if(out.val.c != NULL) { free_fsave(&out); }
		
		out = null_fs();
	}
	
	return out;
}

fsave generate_nd(PPROGRAM *p, int *ev) {
	// Save the ND-related instructions
	int rv = 0;
	int *v_ins_locs = NULL;
	fsave out = null_fs(), *fs = NULL;
	char *val = NULL, *delay_exprs = NULL, *data_exprs = NULL;
	unsigned char *ucbuff = NULL;
	size_t sd = sizeof(double), si = sizeof(unsigned int), si8 = sizeof(unsigned char);
	
	int nf = MCPP_NDNUM, cf = 0;
	fs = malloc(sizeof(fsave)*nf);

	// Maxsteps (Int array, size p->nVaried)
	fs[cf] = make_fs(MCPP_MAXSTEPS);
	if(rv = put_fs(&fs[cf], p->maxsteps, FS_INT, p->nDims + p->nCycles)) { goto error; }
	
	fs[++cf] = make_fs(MCPP_STEPS);
	if(rv = put_fs(&fs[cf], p->steps, FS_UINT, p->steps_size)) { goto error; } 
	
	if(p->nVaried > 0) { 
		// Varied instructions (Int array, size: p->nVaried)
		fs[++cf] = make_fs(MCPP_VINS);
		if(rv = put_fs(&fs[cf], p->v_ins, FS_INT, p->nVaried)) { goto error; }
	
		// Varied instruction dimensions (UChar array, Size: p->nVaried)
		ucbuff = malloc(p->nVaried*si8);
		for(int i = 0; i < p->nVaried; i++) {
			ucbuff[i] = (unsigned char)p->v_ins_dim[i];		
		}
	
		fs[++cf] = make_fs(MCPP_VINSDIM);
		if(rv = put_fs(&fs[cf], ucbuff, FS_UCHAR, p->nVaried)) { goto error; }
	
		// Varied instruction mode (UChar array, Size: p->nVaried)
		for(int i = 0; i < p->nVaried; i++) { 
			ucbuff[i] = (unsigned char)p->v_ins_mode[i];	
		}
	
		fs[++cf] = make_fs(MCPP_VINSMODE);
		if(rv = put_fs(&fs[cf], ucbuff, FS_UCHAR, p->nVaried)) { goto error; }
	
		free(ucbuff);
		ucbuff = NULL;
	
		// Now store the linearized v_ins_locs array.
		v_ins_locs = malloc(si*p->nVaried*p->max_n_steps);
		int k = 0;
		for(int i = 0; i < p->nVaried; i++) {
			memcpy(&(v_ins_locs[k]), p->v_ins_locs[i], p->max_n_steps*si);
			k += p->max_n_steps;
		}
		
		fs[++cf] = make_fs(MCPP_VINSLOCS);
		if(rv = put_fs(&fs[cf], v_ins_locs, FS_INT, p->max_n_steps*p->nVaried)) { goto error; }
		
		// Delay expressions - newline-delimited string array
		unsigned int s_delay = 0, s_data = 0;
		delay_exprs = generate_nc_string(p->delay_exprs, p->nVaried, &s_delay);
		fs[++cf] = make_fs(MCPP_DELAYEXPRS);
		if(rv = put_fs(&fs[cf], delay_exprs, FS_CHAR, s_delay)) { goto error; }
	
		// Data expressions - newline-delimited string array
		data_exprs = generate_nc_string(p->data_exprs, p->nVaried, &s_data);
		fs[++cf] = make_fs(MCPP_DATAEXPRS);
		if(rv = put_fs(&fs[cf], data_exprs, FS_CHAR, s_data)) { goto error; } 
	}
	
	cf++;	// Convert from index to number actually written.
	
	// Generate the container
	out = make_fs(MCPP_NDHEADER);
	if(rv = put_fs_container(&out, fs, cf)) { goto error; }

	error:
	if(v_ins_locs != NULL) { free(v_ins_locs); }
	if(ucbuff != NULL) { free(ucbuff); }
	if(val != NULL) { free(val); }
	if(delay_exprs != NULL) { free(delay_exprs); }
	if(data_exprs != NULL) { free(data_exprs); }
	if(fs != NULL) { free_fsave_array(fs, cf); }
	
	*ev = rv;
	
	if(rv != 0) {
		out = free_fsave(&out);
	}
	
	return out;
}

fsave generate_skip_fs(PPROGRAM *p, int *ev) {
	// Save the ND-related instructions
	int rv = 0;
	fsave out = null_fs(), *fs = NULL;
	char *val = NULL;
	unsigned char *ucbuff = NULL;
	
	int nf = MCPP_SKIPNUM, cf = 0;
	fs = malloc(sizeof(fsave)*nf);

	// First the skip expression	(Char Array - single string)
	fs[cf] = make_fs(MCPP_SKIPEXPR);
	if(rv = put_fs(&fs[cf], p->skip_expr, FS_CHAR, strlen(p->skip_expr)+1)) { goto error; }
	
	// Then the skip_locs linearized array	(Linearized UChar array)
	ucbuff = malloc(sizeof(unsigned char)*p->max_n_steps);
	for(int i = 0; i < p->max_n_steps; i++) {
		ucbuff[i] = (unsigned char)p->skip_locs[i];	
	}
	
	fs[++cf] = make_fs(MCPP_SKIPLOCS);
	if(rv = put_fs(&fs[cf], ucbuff, FS_UCHAR, p->max_n_steps)) { goto error; }
	
	free(ucbuff);
	ucbuff = NULL;
	
	// Generate the container
	out = make_fs(MCPP_SKIPHEADER);
	if(rv = put_fs_container(&out, fs, nf)) { goto error; }

	error:
	if(ucbuff != NULL) { free(ucbuff); }
	if(val != NULL) { free(val); }
	if(fs != NULL) { free_fsave_array(fs, nf); }
	
	*ev = rv;
	
	if(rv != 0) {
		out = free_fsave(&out);
	}
	
	return out;
	
}


//////////////////////////////////////////////////////////////
// 															//
//				PulseBlaster Interactions					//
// 															//
//////////////////////////////////////////////////////////////

int load_pb_info(int verbose) {
	// Counts the number of pulseblasters in the system and populates the ring control.
	int nd = pb_count_boards();	// Count them bitches up.

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
		pb_initialize(verbose);
		
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
	
	int rv = pb_start_programming(PULSE_PROGRAM);
	if(rv < 0)  
		return rv;
	
	int *fid = malloc(sizeof(int)*n_inst);
	for(i = 0; i < n_inst; i++)
		fid[i] = -1571573;		// Distinct number.
	
	int in, instr_data; 
	
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
		
		if(in == LONG_DELAY) { instr_data--; }
		
		fid[i] = pb_inst_pbonly(ilist[i].flags, in, instr_data, ilist[i].instr_time);
		if(fid[i] < 0) {
			rv = -3;
			break;
		}
	}
	char *err = NULL;
	
	if(rv == -3) { err = pb_get_error(); }
	
	pb_stop_programming();
	
	// Error processing.
	if(rv == -3) {
		if(verbose) {
			char *err_str = malloc(strlen(err)+strlen("Error in Instruction 0000: \n\n")+10);
			sprintf(err_str, "Error in Instruction %d:\n\n %s", i, err);
			MessagePopup("Error Programming Board", err_str);
			free(err_str);
		}
		
		return fid[i];
	}

	return 0;
}

int program_pulses_safe(PINSTR *ilist, int n_inst, int verbose) {  
	CmtGetLock(lock_pb);
	int rv = program_pulses(ilist, n_inst, verbose);
	CmtReleaseLock(lock_pb);
	
	return rv;
}

int update_status(int verbose) {
	// Read the status from the board and update the status controls.
	// Does this in a thread-safe manner. Returns the status value.
	
	int rv = 0;
	
	rv = pb_read_status_or_error(verbose);
	
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

int update_status_safe(int verbose) {
	CmtGetLock(lock_pb);
	int rv = update_status(verbose);
	CmtReleaseLock(lock_pb);
	
	return rv;
}

//////////////////////////////////////////////////////////////
// 															//
//				UI Interaction Functions					//
// 															//
//////////////////////////////////////////////////////////////
/******************** General UI Functions *********************/
PPROGRAM *get_current_program() { // This function gets the current program from the UI controls and dumps it into a PPROGRAM
	PPROGRAM *p = calloc(1, sizeof(PPROGRAM));	// Dynamically allocate a PPROGRAM first

	ui_cleanup(0); // Clean up the interface and uipc variable so that we can trust it.

	// Grab what statics we can from the uipc variable
	p->n_inst = uipc.ni;								// Number of instructions
	p->nCycles = uipc.nc;								// Number of cycles
	p->nDims = uipc.nd;									// Number of indirect dimensions
	p->nAout = uipc.anum;
	p->use_pb = uipc.uses_pb;
	p->valid = 1;
	
	int i;
	int *steps = NULL;
	
	p->n_ao_var = 0;
	for(i = 0; i < uipc.anum; i++) {
		if(uipc.ac_varied[i]) { p->n_ao_var++; }	
	}
	
	if(p->nDims || p->nCycles) {				// If either of these is true, it's a varied experiment
		p->varied = 1;
		p->nVaried = (p->nDims?uipc.ndins:0) + uipc.ncins;	// Number of instructions that are varied.
	} else {
		p->varied = 0;				// It's still varied if you have analog outputs
		p->nVaried = 0;
	}
	
	GetCtrlVal(pc.skip[1], pc.skip[0], &p->skip);			// Whether or not skip is on
	GetCtrlVal(pc.nt[1], pc.nt[0], &p->nt);
	GetCtrlVal(pc.tfirst[1], pc.tfirst[0], &p->tmode);		// Transient acquisition mode
	
	GetNumListItems(pc.inst[0], pc.instr, &p->tFuncs);// Number of items in the instruction rings
	p->tFuncs -= 9; // Subtract off atomic functions	// Eventually we'll get this from all_funcs

	// Convenience variables
	int nFuncs = 0, tFuncs = p->tFuncs, nDims=p->nDims, nCycles=p->nCycles;
	int nInst = p->n_inst, nVaried = p->nVaried, j;

	// Now we just need max_n_steps , p->nUniqueInstrs and nFuncs before we can create the pprogram.
	int *maxsteps = malloc(sizeof(int)*(nDims+nCycles));
	int max_n_steps = p->max_n_steps = get_maxsteps(maxsteps);
	
	p->nUniqueInstrs = nInst;			// True for a static experiment
	
	// For a varied experiment, we need to update nUniqueInstrs. We'll get some other info as we do that. 
	int *v_ins, *v_ins_dim, *v_ins_mode;
	if(p->nVaried) {
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
				p->nUniqueInstrs += (uipc.cyc_steps[uipc.ins_cycs[cinc++]]*uipc.dim_steps[uipc.ins_dims[dinc++]])-1;
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
	if(p->skip) {									
		p->real_n_steps = uipc.real_n_steps;
	} else {
		p->real_n_steps = p->max_n_steps;
	}
	
	// Whether or not there's a scan.
	p->scan = !p->use_pb;
	for(i = 0; i<uipc.ni; i++) {
		if(p->scan) { break; }  
		GetCtrlVal(pc.inst[i], pc.scan, &p->scan);
	}
	
	GetCtrlVal(pc.np[1], pc.np[0], &p->np);
	GetCtrlVal(pc.sr[1], pc.sr[0], &p->sr);
	GetCtrlVal(pc.trig_ttl[1], pc.trig_ttl[0], &p->trigger_ttl);
	p->total_time = uipc.total_time;
	
	// Now allocate the arrays.
	create_pprogram(p);
	
	// Now we can assign what arrays we've got already.
	if(p->varied) {
		memcpy(p->maxsteps, maxsteps, sizeof(int)*(p->nDims+p->nCycles));
	
		steps = get_steps_array(p->tmode, p->maxsteps, p->nCycles, p->nDims, p->nt);
		if(steps != NULL) {
			memcpy(p->steps, steps, sizeof(int)*p->steps_size);
			free(steps);
			steps = NULL;
		}
	}
	
	// Add in the analog outputs
	if(p->nAout) {
		int size, len;
		memcpy(p->ao_varied, uipc.ac_varied, sizeof(int)*uipc.anum);
		memcpy(p->ao_dim, uipc.ac_dim, sizeof(int)*uipc.anum);
		
		for(i = 0; i < uipc.anum; i++) {
			// First the values
			size = uipc.ac_varied[i]?uipc.dim_steps[uipc.ac_dim[i]]:1;
			size *= sizeof(double);
			p->ao_vals[i] = malloc_or_realloc(p->ao_vals[i], size);
			memcpy(p->ao_vals[i], uipc.ao_vals[i], size);
			
			// Save the channel -> This is NULL for disabled chans.
			p->ao_chans[i] = get_ao_full_chan_name(uipc.ao_devs[i], uipc.ao_chans[i]);
			
			// Save the expression if necessary
			if(uipc.ac_varied[i] == 2) {
				GetCtrlValStringLength(pc.ainst[i], pc.aincexpr, &len);
				p->ao_exprs[i] = malloc_or_realloc(p->ao_exprs[i], len+1);
				GetCtrlVal(pc.ainst[i], pc.aincexpr, p->ao_exprs[i]);
			}
		}
	}
	
	// Grab the instructions from the UI.
	for(i=0; i<nInst; i++)
		get_instr(p->instrs[i], i);
	
	if(p->nVaried) {
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
		for(i=0; i < nVaried; i++) {
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

PPROGRAM *get_current_program_safe() {
	CmtGetLock(lock_uipc);
	PPROGRAM *p = get_current_program();
	CmtReleaseLock(lock_uipc);

	return p;
}

int *get_steps_array(int tmode, int *maxsteps, int nc, int nd, int nt) {
	int *steps = NULL;
	int size = get_steps_array_size(tmode, nc, nd);
	
	if(size <= 0) { goto error; }
	
	steps = malloc(sizeof(int)*size);
	
	switch(tmode) {
		case MC_TMODE_ID:
			steps[nd] = nt;
			memcpy(steps, maxsteps + nc, sizeof(int)*nd);
			break;
		case MC_TMODE_TF:
			steps[0] = nt;
			memcpy(steps+1, maxsteps+nc, sizeof(int)*nd);
			break;
		case MC_TMODE_PC:
			steps[nc+nd] = nt/nc;
			memcpy(steps, maxsteps, sizeof(int)*(nd+nc));
			break;
	}
	
	error:
	
	return steps;
}

int get_steps_array_size(int tmode, int nc, int nd) {
	// Determine the size of the steps array
	
	if(tmode == MC_TMODE_PC && nc <= 0) {
		tmode = MC_TMODE_ID;
	}
	
	switch(tmode) {
		case MC_TMODE_ID:
		case MC_TMODE_TF:
			return nd + 1;
		case MC_TMODE_PC:
			return nd+nc+1;
		default:
			return MCPP_ERR_INVALIDTMODE;
	}	
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
	
	for(i=0; i < p->n_inst; i++) {
		set_instr(i, p->instrs[i]);
	}
	
	// Create the analog outputs
	SetCtrlVal(pc.anum[1], pc.anum[0], p->nAout);
	change_num_aouts();

	// Now update the varied instructions
	if(p->nDims) {
		change_num_dims(p->nDims+1);
		
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
	
	// Whether or not to use the PulseBlaster
	uipc.uses_pb = p->use_pb;
	SetCtrlVal(pc.uses_pb[1], pc.uses_pb[0], p->use_pb);
	
	if(p->tmode >= 0) {
		SetCtrlVal(pc.tfirst[1], pc.tfirst[0], p->tmode);
	}
	
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
				if(uipc.c_instrs[num] == NULL || uipc.max_cinstrs[num] < p->maxsteps[cyc]) {
					uipc.c_instrs[num] = malloc_or_realloc(uipc.c_instrs[num], sizeof(PINSTR*)*p->maxsteps[cyc]);
					
					for(j = uipc.max_cinstrs[num]; j < p->maxsteps[cyc]; j++) // Replace with memset?
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
					
					if(del) {
						for(j = 0; j < steps; j++)
							uipc.nd_delays[ind][j] = nd_delays[j];
					}
			
					if(dat) {
						for(j = 0; j < steps; j++)
							uipc.nd_data[ind][j] = nd_data[j];
					}
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
	
	// Now the analog outs if they exist.
	if(p->nAout) {
		int size, len, dim, steps, dev = -1, chan = -1;
		int *ac_varied = malloc(sizeof(int)*p->nAout);
		
		memcpy(ac_varied, p->ao_varied, sizeof(int)*p->nAout);
		memcpy(uipc.ac_dim, p->ao_dim, sizeof(int)*p->nAout);
		
		for(i = 0; i < p->nAout; i++) {
			// Generate the values ourselves from the start and finish.
			SetCtrlVal(pc.ainst[i], pc.ainitval, p->ao_vals[i][0]);
			set_ao_nd_state(i, ac_varied[i]);
			
			if(uipc.ac_varied[i]) {
				dim = p->ao_dim[i];
				steps = p->maxsteps[dim+p->nCycles];
				
				SetCtrlVal(pc.ainst[i], pc.afinval, p->ao_vals[i][steps-1]);
				
				if(uipc.ac_varied[i] == 1) {
					update_ao_increment(i, MC_INC);	
				} else if(uipc.ac_varied[i] == 2) {
					// Need to get the expression first
					SetCtrlVal(pc.ainst[i], pc.aincexpr, p->ao_exprs[i]);
					
					//update_ao_increment_from_exprs(i);
				}
			} else {
				uipc.ao_vals[i][0] = p->ao_vals[i][0];	
			}
			
			// Now we want to get the channel.
			get_ao_dev_chan(p->ao_chans[i], &dev, &chan);
			if(dev < 0) {
				SetCtrlVal(pc.ainst[i], pc.aodev, uipc.default_adev);
			} else {
				SetCtrlVal(pc.ainst[i], pc.aodev, dev);
			}
			
			SetCtrlVal(pc.ainst[i], pc.aochan, chan);
			change_ao_chan_uipc(i);
		}
		
		free(ac_varied);
		
		for(i = 0; i < uipc.anum; i++) {
			populate_ao_chan(i);	
		}
	}


	if(cstep != NULL)
		free(cstep);
}

void set_current_program_safe(PPROGRAM *p) {
	CmtGetLock(lock_uipc);
	set_current_program(p);
	CmtReleaseLock(lock_uipc);
}


void load_prog_popup() {
	// Function that generates a popup and allows the user to load a program from file.
	char *path = malloc(MAX_FILENAME_LEN+MAX_PATHNAME_LEN);
	char *dfs = malloc(strlen(PPROG_EXTENSION)+3);
	sprintf(dfs, "*.%s", PPROG_EXTENSION);
	
	int rv = FileSelectPopup((uipc.ppath != NULL)?uipc.ppath:"Programs", dfs, PPROG_ALLEXTS, "Load Program From File", VAL_LOAD_BUTTON, 0, 0, 1, 1, path);

	free(dfs);
	
	if(rv == VAL_NO_FILE_SELECTED) {
		free(path);
		return;
	} else {
		add_prog_path_to_recent_safe(path);
	}

	int err_val = 0;
	PPROGRAM *p = NULL;
	
	char *ext = get_extension(path);
	if(ext != NULL && strcmp(ext, ".tdm") == 0) {
		p = LoadPulseProgramDDC(path, 1, &err_val);	
	} else {
		p = LoadPulseProgram(path, 1, &err_val);	
	}
	
	if(ext != NULL) { free(ext); }
	
	if(err_val != 0)
		display_ddc_error(err_val);

	if(p != NULL) {
		set_current_program_safe(p);
		free_pprog(p);
	}

	free(path);
}

void save_prog_popup() {
	char *path = malloc(MAX_FILENAME_LEN+MAX_PATHNAME_LEN);
	char *dfs = malloc(strlen(PPROG_EXTENSION)+3);
	sprintf(dfs, "*.%s", PPROG_EXTENSION);

	int rv = FileSelectPopup((uipc.ppath != NULL)?uipc.ppath:"Programs", dfs, PPROG_ALLEXTS, "Save Program To File", VAL_SAVE_BUTTON, 0, 0, 1, 1, path);

	free(dfs);
	
	if(rv == VAL_NO_FILE_SELECTED) {
		free(path);
		return;
	} else
		add_prog_path_to_recent_safe(path);

	PPROGRAM *p = get_current_program_safe();

	int err_val = SavePulseProgram(p, path, 1);

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
		uipc.ppath = malloc_or_realloc(uipc.ppath, strlen(path)+1);
		strcpy(uipc.ppath, path);
	}
	
	free(path);
}

void add_prog_path_to_recent_safe(char *opath) {
	CmtGetLock(lock_uipc);
	add_prog_path_to_recent(opath);
	CmtReleaseLock(lock_uipc);
}

void clear_program() {
	// Deletes all instructions for a new program.
	
	// Clear all the instructions
	for(int i = 0; i < uipc.max_ni; i++) { clear_instruction(i); }
	for(int i = 0; i < uipc.max_anum; i++) { clear_aout(i); }
	for(int i = 0; i < uipc.fr_max_ni; i++) { clear_fr_instr(i); }
	
	// Update the number of instructions and analog outputs
	SetCtrlVal(pc.ninst[1], pc.ninst[0], 1);
	change_number_of_instructions();
	
	SetCtrlVal(pc.anum[1], pc.anum[0], 0);
	change_num_aouts();
	
	SetCtrlVal(pc.FRPan, pc.fninst, 1);
	change_fr_num_instrs(1);
	
	// Update the ND information
	SetCtrlVal(pc.numcycles[1], pc.numcycles[0], 0);
	change_num_cycles();
	
	// Free up the uipc stuff
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
	
	// Turn off the skips if necessary.
	SetCtrlVal(pc.skip[1], pc.skip[0], 0);
	
	SetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_TEXT_BGCOLOR, VAL_WHITE);
	SetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_TEXT_COLOR, VAL_BLACK);
	SetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_TEXT_BOLD, 0);
	SetCtrlAttribute(pc.skip[1], pc.skip[0], ATTR_CTRL_MODE, VAL_INDICATOR);
	
	int def_len;
	GetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_DFLT_VALUE_LENGTH, &def_len);
	char *def_val = malloc(def_len+1);
	
	GetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_DFLT_VALUE, def_val);
	SetCtrlVal(pc.skiptxt[1], pc.skiptxt[0], def_val);
	free(def_val);
}

void clear_program_safe() {
	CmtGetLock(lock_uipc);
	clear_program();
	CmtReleaseLock(lock_uipc);
}

void get_pinstr_array(PINSTR **ins_ar, PPROGRAM *p, int *cstep) {
	// Give this the current step and the PPROGRAM and it returns the array of instructions
	// that you will be using at that step.
	int i;
	
	// Copy over all the initial instructions to start with.
	for(i = 0; i < p->n_inst; i++) {
		copy_pinstr(p->instrs[i], ins_ar[i]);
	}
	
	if(p->varied) {
		return;			// If there's no variation, we're done.
	}
	
	int ins_ind, lindex = get_lindex(cstep, p->maxsteps, p->nDims+p->nCycles);
	
	// Now we just need to copy the instructions from p->intrs. The locations for each
	// stage in acquisition space is stored in p->v_ins_locs.
	for(i = 0; i < p->nVaried; i++) {
		copy_pinstr(ins_ar[p->v_ins[i]], p->instrs[p->v_ins_locs[i][lindex]]);
	}
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
	int data_on, delay_on, ao_on = 0, ao_on_disabled = 0;
	double time;
	
	// First go through and find any problems within the multidimensional acquisition, if applicable.
	if(uipc.nd) {
		if(uipc.ac_varied != NULL) {
			for(i = 0; i < uipc.anum; i++) {
				if(uipc.ac_varied[i]) {
					ao_on = 1;
					
					// We may want to notify if it's only in a disabled channel.
					if(uipc.ao_devs[i] < 0 || uipc.ao_chans[i] < 0) {
						ao_on_disabled = 2;		// Always 2, for bit encoding reasons.
					} else {
						ao_on_disabled = 0;
						break;
					}
				}
			}
		}
		
		if(!ao_on && uipc.ndins == 0) {		// No instructions actually vary.
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
			if(uipc.nd_delays == NULL || uipc.nd_delays[i] == NULL || constant_array_double(uipc.nd_delays[i], steps)) {
				delay_on = 0;
			} else {
				delay_on = 1;
			}
			
			// Check if data variation is on
			if(uipc.nd_data == NULL || uipc.nd_data[i] == NULL || constant_array_int(uipc.nd_data[i], steps)) {
				data_on = 0;
			} else {
				data_on = 1;
			}
			
			// If neither is actually on, turn variation off.
			if(!data_on && !delay_on) {
				change = 1;
				update_nd_state(ind, 0);	
				i--;					// Need to decrement i because we're removing an instruction.
			}
		}
		
		// Iterate through the analog outputs now and turn them off if they don't really.
		for(i = 0; i < uipc.anum; i++) {
			if(!uipc.ac_varied[i]) { continue; }
			
			if(uipc.ac_dim[i] < 0 || constant_array_double(uipc.ao_vals[i], uipc.dim_steps[uipc.ac_dim[i]])) {
				set_ao_nd_state(i, 0);
			}
		}
		
		// Now check that there aren't more dimensions than are actually being varied.
		for(i = 0; i<uipc.nd; i++) {
			// First check ND instructions
			for(j=0; j<uipc.ndins; j++) {
				// Break on first example of this dimension
				if(uipc.ins_dims[j] == i) { break; }
			}
			
			if(j < uipc.ndins) { continue; }
			
			// Now check analog outputs
			for(j = 0; j < uipc.anum; j++) {
				// Skip anything not varying
				if(!uipc.ac_varied[j]) {
					continue;
				}
				
				if(uipc.ac_dim[j] == i) { break; }
			}
			
			if(j < uipc.anum) { continue; }
			
			// If we're at this point, we need to remove this dimension from the experiment
			change = 1;
			
			remove_array_item(uipc.dim_steps, i, uipc.nd); // Remove the steps.
			for(j=0; j<uipc.ndins; j++) {
				if(uipc.ins_dims[j] > i) { uipc.ins_dims[j]--; }
			}
			uipc.nd--;				// Update number of dimensions
			i--;					// We removed one, so drop it back one.
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
					if(uipc.c_instrs[ind][j] == NULL) {
						update_cyc_instr(ind, uipc.c_instrs[ind][j-1], j); 
					}
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
			remove_array_item(uipc.cyc_steps, i, uipc.nc);	// Remove the steps from the array
			for(j=0; j<uipc.ncins; j++) {
				if(uipc.ins_cycs[j] > i)
					uipc.ins_cycs[j]--;
			}
			uipc.nc--;			// Update number of cycles
			i--;				// We removed one, so decrement the counter.
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

	// Finally, if we're in verbose mode, give us a popup. - Not implemented
	if(verbose) {
		return change|ao_on_disabled;
	}
	
	// Return this anyway. This allows for two different non-verbose modes, based on what you
	// do with the return value. You can either ignore it or cancel the operation even without
	// the user's input. We can implement a user-defined preference to decide on a case-by-case
	// basis what to do in the various situations that ui_cleanup() is called.
	return change|ao_on_disabled;
} 

int ui_cleanup_safe(int verbose) {
	CmtGetLock(lock_uipc);
	int rv = ui_cleanup(verbose);
	CmtReleaseLock(lock_uipc);
	
	return rv;
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
	double min = 0; 	// Minimum is 0ns
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
PINSTR null_pinstr() {
	PINSTR ins = {
		.flags = 0,
		.instr = 0,
		.instr_data = 0,
		.trigger_scan = 0,
		.instr_time = MCPP_DEFAULT_DELAY,
		.time_units = MCPP_DEFAULT_UNITS
	};
	
	return ins;
}

int set_fr_instr(int num, PINSTR instr) {
	// Pass this the program controls, instr number and it sets the controls appropriately.
	int ni;
	GetCtrlVal(pc.FRPan, pc.fninst, &ni);
	
	if(num > ni) {
		return MCUI_ERR_INVALID_INST;	
	}
	
	int panel = pc.finst[num];
	set_fr_instr_panel(panel, instr);
	
	return 0;
}

void set_fr_instr_panel(int panel, PINSTR instr) {
	int i;
	
	set_fr_flags_panel(panel, instr.flags);
	
	SetCtrlIndex(panel, pc.fr_inst, instr.instr);
	double delay = instr.instr_time/(instr.time_units?pow(1000, instr.time_units):1);
	
	SetCtrlVal(panel, pc.fr_delay, delay);
	SetCtrlIndex(panel, pc.delayu, instr.time_units);
	
	if(takes_instr_data(instr.instr)) {						 
		SetCtrlAttribute(panel, pc.fr_inst_d, ATTR_DIMMED, 0);
		SetCtrlVal(panel, pc.fr_inst_d, instr.instr_data);
	} else {
		SetCtrlAttribute(panel, pc.fr_inst_d, ATTR_DIMMED, 0);
		SetCtrlVal(panel, pc.fr_inst_d, 0);
	}
}

int set_instr(int num, PINSTR *instr) {
	// Pass this the program controls, instruction number and the instr and it sets the controls appropriately
	
	int ni;
	GetCtrlVal(pc.ninst[1], pc.ninst[0], &ni);			// For error checking
	if(num >= ni)											// Can't set an instruction if it's not there
		return MCUI_ERR_INVALID_INST;						// Error -> Instruction invalid
	
	int panel = pc.inst[num];					
	set_instr_panel(panel, instr);
	change_instruction(num);
	return 1;												// Success
}

void set_instr_panel(int panel, PINSTR *instr) {
	int i;
	
	PINSTR ins;
	
	if(instr == NULL) {
		ins = null_pinstr();
		instr = &ins;
	}
	
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

void change_fr_instr_delay(int panel) {
	double val;
	int units;
	
	GetCtrlVal(panel,  pc.fr_delay, &val);
	GetCtrlVal(panel, pc.fr_delay_u, &units);
	
	SetCtrlAttribute(panel, pc.fr_delay, ATTR_PRECISION, get_precision(val, MCUI_DEL_PREC));
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
		update_nd_increment_safe(num, MC_FINAL);
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

void change_fr_instr(int num) {
	int panel = pc.finst[num];
	
}

void change_fr_instr_pan(int panel) {
	int ind;
	GetCtrlVal(panel, pc.fr_inst, &ind);
	
	// The up the minimum
	int min = instr_data_min(ind);
	SetCtrlAttribute(panel, pc.fr_inst_d, ATTR_MIN_VALUE, min);
	
	if(!takes_instr_data(ind)) {
		SetCtrlVal(panel, pc.fr_inst_d, 0);
	}
	
	SetCtrlAttribute(panel, pc.fr_inst_d, ATTR_DIMMED, !takes_instr_data(ind));
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

void swap_ttl_safe(int to, int from) {
	CmtGetLock(lock_uipc);
	swap_ttl(to, from);
	CmtReleaseLock(lock_uipc);
}

void move_ttl(int num, int to, int from) {
	// Move a TTL from "from" to "to", and shift all the others in response.
	// This is a "safe" function, and will skip broken ttls. If the initial or
	// final value is marked broken, the operation will fail and return -1.
	int panel = pc.inst[num];
	
	move_ttl_panel(panel, to, from);
}

void move_ttl_safe(int num, int to, int from) {
	CmtGetLock(lock_uipc);
	move_ttl(num, to, from);
	CmtReleaseLock(lock_uipc);
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
	
	set_flags_range(panel, pc.TTLs, flags, to, from);
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

void move_ttl_panel_safe(int panel, int to, int from) {
	CmtGetLock(lock_uipc);
	move_ttl_panel(panel, to, from);
	CmtReleaseLock(lock_uipc);
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
		if(uipc.c_instrs != NULL && uipc.c_instrs[i] != NULL && uipc.max_cinstrs[i] > 0) {
			for(int j = 1; j < uipc.max_cinstrs[i]; j++)
				uipc.c_instrs[i][j]->flags = move_bit_skip(uipc.c_instrs[i][j]->flags, skip, nt, ot);
		}

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
	
	uipc.trigger_ttl = nt;
}

void change_trigger_ttl_safe() {
	CmtGetLock(lock_uipc);
	change_trigger_ttl();
	CmtReleaseLock(lock_uipc);
}

void set_ttl_trigger(int panel, int ttl, int on) {
	// Pass -1 to ttl to set it to whatever the current trigger is.
	if(ttl < 0 || ttl >= 24)
		ttl = uipc.trigger_ttl;		// Exactly one call - does not need to be locked by itself.
	
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

void set_fr_flags(int num, int flags) {
	int panel = pc.inst[num];
	set_fr_flags_panel(panel, flags);
}

void set_fr_flags_panel(int panel, int flags) {
	set_flags_range(panel, pc.fr_TTLs, flags, 0, 23);	
}

void set_flags_panel(int panel, int flags) {
	// Sets the flags of panel to be the appropriate thing.
	
	set_flags_range(panel, pc.TTLs, flags, 0, 23);
}

void set_flags_range(int panel, int TTLs[], int flags, int start, int end) {
	// Sets all the flags in a given range.
	if(start > end) {
		int buff = end;
		end = start;
		start = buff;
	}
	
	for(int i = start; i <= end; i++)
		SetCtrlVal(panel, TTLs[i], (flags&(1<<i))?1:0);
		
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

int ttls_in_use_safe() {
	CmtGetLock(lock_uipc);
	int rv = ttls_in_use();
	CmtReleaseLock(lock_uipc);
	
	return rv;
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

void get_updated_instr_safe(PINSTR *instr, int num, int *cstep, int *cind, int *dind, int mode) {
	CmtGetLock(lock_uipc);
	get_updated_instr(instr, num, cstep, cind, dind, mode);
	CmtReleaseLock(lock_uipc);
}

/**************** Set ND Instruction Parameters ****************/  
void set_ndon(int ndon) {
	// Sets whether it's a multidimensional experiment
	int dimmed, val = ndon, nd, i;

	if(val) {
		GetCtrlVal(pc.ndims[1], pc.ndims[0], &nd);
		dimmed = 0;
		change_num_dims(nd);
	} else {
		dimmed = 1;
		uipc.nd = 0;
	}
	
	// Set the controls appropriately.
	SetCtrlVal(pc.ndon[1], pc.ndon[0], val);
	SetCtrlVal(pc.andon[1], pc.andon[0], val);
	SetPanelAttribute(pc.PPConfigCPan, ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pc.andims[1], pc.andims[0], ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pc.ndims[1], pc.ndims[0], ATTR_DIMMED, dimmed);
	
	for(i = 0; i<uipc.max_ni; i++) {
		SetPanelAttribute(pc.cinst[i], ATTR_DIMMED, dimmed);
		set_instr_nd_mode(i, ndon?get_nd_state(i):0);
	}
	
	if(!uipc.nc) {
		SetCtrlAttribute(pc.skip[1], pc.skip[0], ATTR_DIMMED,dimmed);
		SetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_DIMMED, dimmed);
	}
	
	for(i = 0; i < uipc.anum; i++) {
		set_aout_nd_dimmed(i, dimmed);
	}
}

void set_ndon_safe(int ndon) {
	CmtGetLock(lock_uipc);
	set_ndon(ndon);
	CmtReleaseLock(lock_uipc);
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
		}
	} else {
		if(ind < 0) {				// only need to update if we're not already there.
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

void update_nd_state_safe(int num, int state) {
	CmtGetLock(lock_uipc);
	update_nd_state(num, state);
	CmtReleaseLock(lock_uipc);
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

void change_num_dims(int nd) { 	// Updates the number of dimensions in the experiment
	int i, j;

	SetCtrlVal(pc.andims[1], pc.andims[0], nd);
	SetCtrlVal(pc.ndims[1], pc.ndims[0], nd);

	if(--nd == uipc.nd)
		return;
	
	// Update the UI elements 
	int elements = 0; 
	char **c = (nd > uipc.nd)?generate_char_num_array(1, nd, &elements):NULL;
	
	if(uipc.ndins) {	
		// We are going to be updating uipc as we go, so we want a local copy so that as we
		// iterate through it we don't end up skipping instructions and such
		int ndins = uipc.ndins;
		int *ins_dims = malloc(sizeof(int)*ndins), *dim_ins = malloc(sizeof(int)*ndins);
		
		memcpy(ins_dims, uipc.ins_dims, sizeof(int)*ndins);
		memcpy(dim_ins, uipc.dim_ins, sizeof(int)*ndins);
		
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
	}
	
	if(uipc.ac_varied != NULL) { 
		for(i = 0; i < uipc.max_anum; i++) {
			if(!uipc.ac_varied)
				continue;
			
			if(uipc.ac_dim[i] >= nd) {
				set_ao_nd_state(i, 0);	// Turn it off if it's not used.
				continue;
			}
			
			if(nd < uipc.nd) {
				DeleteListItem(pc.ainst[i], pc.adim, nd, -1); // Delete the rest of them.
			} else {
				for(j=uipc.nd;j<nd;j++) { InsertListItem(pc.ainst[i], pc.adim, -1, c[j], j); }
			}
		}
	}
	
	if(c != NULL) { free_string_array(c, elements); } // Free c if necessary.
	
	// Update the uipc variable
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
}

void change_num_dims_safe(int nd) {
	CmtGetLock(lock_uipc);
	change_num_dims(nd);
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
	
	int cdim;
	for(i = 0; i < uipc.anum; i++) {
		if(uipc.ac_varied[i]) {
			if(dim == uipc.ac_dim[i])
				change_ao_steps_instr(i, steps);
		}
	}
	
	// Now update the uipc variable and we're done.
	uipc.dim_steps[dim] = steps;
}

void change_num_dim_steps_safe(int dim, int steps) {
	CmtGetLock(lock_uipc);
	change_num_dim_steps(dim, steps);
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

void change_nd_steps_instr_safe(int num, int steps) {
	CmtGetLock(lock_uipc);
	change_nd_steps_instr(num, steps);
	CmtReleaseLock(lock_uipc);
}

void change_ao_steps_instr(int num, int steps) {
	int state = get_ao_nd_state(num);
	int panel = pc.ainst[num];
	
	// Update controls
	if(state) {
		SetCtrlVal(panel, pc.asteps, steps);
	
		uipc.ao_vals[num] = realloc(uipc.ao_vals[num], sizeof(double)*steps);
		
		if(state == 1) {
			update_ao_increment(num, MC_INC);
		} else if(state == 2) {
			// update_ao_increment_from_exprs(num);	
		}
	}
}

void change_ao_steps_instr_safe(int num, int steps) {
	CmtGetLock(lock_uipc);
	change_ao_steps_instr(num, steps);
	CmtReleaseLock(lock_uipc);
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
	
	ind = int_in_array(uipc.dim_ins, num, uipc.ndins);
	
	if(ind < 0) 
		return;				// Something's wrong, return.
	
	// Update ins_dims and change the number of steps for the control.
	uipc.ins_dims[ind] = dim;

	change_nd_steps_instr(num, uipc.dim_steps[dim]);
}

void change_dimension_safe(int num) {
	CmtGetLock(lock_uipc);
	change_dimension(num);
	CmtReleaseLock(lock_uipc);
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
		change_num_dims(uipc.nd+1);
	}
}

void populate_dim_points_safe() {
	CmtGetLock(lock_uipc);
	populate_dim_points();
	CmtReleaseLock(lock_uipc);
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
	
	if(ind >= 0) {
		if(uipc.nd_delays[ind] == NULL) 
			uipc.nd_delays[ind] = malloc(sizeof(double)*steps);
		else
			uipc.nd_delays[ind] = realloc(uipc.nd_delays[ind], sizeof(double)*steps);
	
		for(i=0; i<steps; i++) 
			uipc.nd_delays[ind][i] = init+inc*i;
	}
	
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
		if(ind >= 0) {
			if(uipc.nd_delays[ind] == NULL) 
				uipc.nd_data[ind] = malloc(sizeof(int)*steps);
			else
				uipc.nd_data[ind] = realloc(uipc.nd_data[ind], sizeof(int)*steps);
	
			for(int i=0; i<steps; i++) 
				uipc.nd_data[ind][i] = initd+incd*i;
		}
		
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

void update_nd_increment_safe(int num, int mode) {
	CmtGetLock(lock_uipc);
	update_nd_increment(num, mode);
	CmtReleaseLock(lock_uipc);
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
	
	// Build the basic cstep
	cstep = calloc(uipc.nd+uipc.nc, sizeof(int));
	
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
		
		if(uipc.err_del = err_del) {			// Not a typo, I'm actually setting the
			del_bgcolor = VAL_RED;			    // value of the uipc error field here.
			del_textcolor = VAL_OFFWHITE;
		} else 
			del_bgcolor = VAL_GREEN;
	} 
	
	if(eval_data) {
		free(expr_data);
		
		dat_bold = 1;
		
		SetCtrlVal(pc.cinst[num], pc.dat_init, uipc.nd_data[ind][0]);
		SetCtrlVal(pc.cinst[num], pc.dat_fin, uipc.nd_data[ind][steps-1]);
		
		if(uipc.err_dat = err_dat) {			// Again, not a typo.
			dat_bgcolor = VAL_RED;
			dat_textcolor = VAL_OFFWHITE;
		} else 
			dat_bgcolor = VAL_GREEN;
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

void update_nd_from_exprs_safe(int num) {
	CmtGetLock(lock_uipc);
	update_nd_from_exprs(num);
	CmtReleaseLock(lock_uipc);
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
	
	// May need to change this.
	ui_cleanup(0);					// Clean up before evaluation.
	
	expr = malloc(len+1);
	GetCtrlVal(pan, ctrl, expr);
	
	// Get max_n_steps and maxsteps;
	int *maxsteps = malloc(sizeof(int)*size);
	
	uipc.max_n_steps = 1;
	for(i=0; i<uipc.nc; i++) {
		uipc.max_n_steps*=uipc.cyc_steps[i];
		maxsteps[i] = uipc.cyc_steps[i];
	}
	
	for(i=0; i<uipc.nd; i++) { 
		uipc.max_n_steps*=uipc.dim_steps[i];
		maxsteps[i+uipc.nc] = uipc.dim_steps[i];
	}

	// Allocate space for skip_locs. One for each step. We'll use an
	// unsigned char array since bools aren't a thing in C
	int max_n_steps = uipc.max_n_steps;
	if(max_n_steps == 0) {
		free(maxsteps);
		free(expr);
		return;
	}
	
	uipc.skip_locs = malloc_or_realloc(uipc.skip_locs, sizeof(unsigned char)*max_n_steps);
																						  
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

void update_skip_condition_safe() {
	CmtGetLock(lock_uipc);
	update_skip_condition();
	CmtReleaseLock(lock_uipc);
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
			int elements = 0;
			char **c = NULL;
			
			if(uipc.nc > nl) {
				c = generate_char_num_array(1, uipc.nc, &elements);
			
				for(i = nl; i<uipc.nc; i++) 
					InsertListItem(panel, pc.pclevel, -1, c[i], i); 	// Insert the cycles
				
				c = free_string_array(c, elements);	// Null detection inherent.
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
				
				c = free_string_array(c, elements);  
			}
			
			// Now we need to update the uipc var.
			uipc.ncins++;
			uipc.cyc_ins = malloc_or_realloc(uipc.cyc_ins, uipc.ncins*sizeof(int));
			uipc.ins_cycs = malloc_or_realloc(uipc.ins_cycs, uipc.ncins*sizeof(int));
			
			uipc.ins_cycs[uipc.ncins-1] = cyc;
			uipc.cyc_ins[uipc.ncins-1] = num;
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
	
	GetCtrlAttribute(pc.nt[1], pc.nt[0], ATTR_INCR_VALUE, &oiv); // There may be something wrong here.
	SetCtrlAttribute(pc.nt[1], pc.nt[0], ATTR_INCR_VALUE, niv);
	
	int nt;
	GetCtrlVal(pc.nt[1], pc.nt[0], &nt);
	SetCtrlVal(pc.nt[1], pc.nt[0], (int)((nt/oiv)*niv));
	
	change_visibility_mode(panel, all_ctrls, all_num, all);
	SetCtrlVal(panel, pc.pcon, state);
}

void update_pc_state_safe(int num, int state) {
	CmtGetLock(lock_uipc);
	update_pc_state(num, state);
	CmtReleaseLock(lock_uipc);
}

void change_num_cycles() {
	// Function called if the cycle number control is changed.
	int nc, i, j;
	GetCtrlVal(pc.numcycles[1], pc.numcycles[0], &nc);
	
	if(nc == uipc.nc)
		return;
	
	// Update the UI elements
	if(uipc.ncins) {														   	   
		int elements = 0;
		char **c = (nc>uipc.nc)?generate_char_num_array(1, nc, &elements):NULL;

		// We are going to be updating uipc as we go, so we want a local copy so that as we
		// iterate through it we don't end up skipping instructions and such
		int ncins = uipc.ncins;
		int *ins_cycs = malloc(sizeof(int)*ncins), *cyc_ins = malloc(sizeof(int)*ncins);
		
		memcpy(ins_cycs, uipc.ins_cycs, sizeof(int)*ncins);
		memcpy(cyc_ins, uipc.cyc_ins, sizeof(int)*ncins);
	
		for(j = 0; j<ncins; j++) {
			if(ins_cycs[j] >= nc) {
				update_pc_state(cyc_ins[j], 0);
				continue;
			}
		
			if(nc < uipc.nc) {
				DeleteListItem(pc.inst[cyc_ins[j]], pc.pclevel, nc, -1);	// Delete the rest of them	
			} else {
				for(i=uipc.nc; i<nc; i++)  { InsertListItem(pc.inst[cyc_ins[j]], pc.pclevel, -1, c[i], i); }
			}
		}
	
		free(cyc_ins);
		free(ins_cycs);
	
		if(c != NULL) { c = free_string_array(c, elements); }
	}
	
	// Only if this has never been allocated - change_num_instructions will
	// take care of reallocation otherwise.
	if(uipc.c_instrs == NULL) {
		uipc.c_instrs = calloc(uipc.max_ni, sizeof(PINSTR**));
		uipc.max_cinstrs = calloc(uipc.max_ni, sizeof(int));
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
	
	int dimmed = (uipc.nc || uipc.nd)?0:1;
	
	SetCtrlAttribute(pc.skip[1], pc.skip[0], ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pc.skiptxt[1], pc.skiptxt[0], ATTR_DIMMED, dimmed);
}

void change_num_cycles_safe() {
	CmtGetLock(lock_uipc);
	change_num_cycles();
	CmtReleaseLock(lock_uipc);
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
	for(i = 0; i<uipc.ncins; i++) {
		if(uipc.cyc_ins[i] == num) {			// Find the place in the cyc_ins array
			uipc.ins_cycs[i] = cyc;
			break;
		}
	}
	
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

void change_cycle_safe(int num) {
	CmtGetLock(lock_uipc);
	change_cycle(num);
	CmtReleaseLock(lock_uipc);
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

void change_cycle_step_safe(int num) {
	CmtGetLock(lock_uipc);
	change_cycle_step(num);
	CmtReleaseLock(lock_uipc);
}

void change_cycle_num_steps(int cyc, int steps) {
	// Function for changing the number of steps in a phase cycle.
	int i, j, nl, cp;

	if(steps == uipc.cyc_steps[cyc])
		return;	// No change.
	
	int oldsteps = uipc.cyc_steps[cyc];
	uipc.cyc_steps[cyc] = steps;
	
	int elements = 0;
	char **c = (steps > oldsteps)?generate_char_num_array(1, steps, &elements):NULL;
	
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
		}
		
		SetCtrlVal(cp, pc.pcsteps, steps);
	}
	
	c = free_string_array(c, elements);
	
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

void change_cycle_num_steps_safe(int cyc, int steps) {
	CmtGetLock(lock_uipc);
	change_cycle_num_steps(cyc, steps);
	CmtReleaseLock(lock_uipc);
}

void update_cyc_instr(int num, PINSTR *instr, int step) {
	// Updates the uipc variable for instruction number "num" at step "step"
	// with the instruction you feed it. Make sure that max_cinstrs is set
	// before you use this function. 

	// Make this array the right size.
	if(uipc.c_instrs[num] == NULL) {
		uipc.c_instrs[num] = calloc((step+1), sizeof(PINSTR*));
	} else if (uipc.max_cinstrs[num] <= step) {
		uipc.c_instrs[num] = realloc(uipc.c_instrs[num], sizeof(PINSTR*)*(step+1));
	}
	
	if(step >= uipc.max_cinstrs[num]) {					// Matches exclusively both of the above conditions
		memset(uipc.c_instrs[num]+step, 0, sizeof(PINSTR*)*((step-uipc.max_cinstrs[num])+1));
		uipc.max_cinstrs[num] = step+1;
	}
	
	// If it's never been allocated, allocate it.
	if(uipc.c_instrs[num][step] == NULL) { 
		uipc.c_instrs[num][step] = malloc(sizeof(PINSTR));
	}
	
	// Finally just copy the instr into the uipc array.
	copy_pinstr(instr, uipc.c_instrs[num][step]);
}

void update_cyc_instr_safe(int num, PINSTR *instr, int step) {
	CmtGetLock(lock_uipc);
	update_cyc_instr(num, instr, step);
	CmtReleaseLock(lock_uipc);
}

void populate_cyc_points()
{	 
	// Function for updating the UI with the values from the uipc var
	int j, on, nl, cyc;
	
	// A char array of labels
	int elements = 0;
	char **c = generate_char_num_array(1, uipc.nc, &elements); 

	int panel;
	for(int i = 0; i<uipc.ncins; i++) {
		panel = pc.inst[uipc.cyc_ins[i]];
		
		// First make the number of dimensions per control correct
		GetNumListItems(panel, pc.pclevel, &nl);
		if(nl < uipc.nc) {
			for(j=nl; j<uipc.nc; j++) { InsertListItem(panel, pc.pclevel, -1, c[j], j); }
		} else if (nl > uipc.nc) {
			DeleteListItem(panel, pc.pclevel, uipc.nc, -1);
		}
		
		// Now update the number of steps.
		cyc = uipc.ins_cycs[i];				// In case the cycle changed
		SetCtrlIndex(panel, pc.pclevel, cyc);
		SetCtrlVal(panel, pc.pcsteps, uipc.cyc_steps[cyc]);
	}
	
	c = free_string_array(c, elements);
	
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

void populate_cyc_points_safe() {
	CmtGetLock(lock_uipc);
	populate_cyc_points();
	CmtReleaseLock(lock_uipc);
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
		uipc.cyc_pans = malloc_or_realloc(uipc.cyc_pans, sizeof(int*)*(num+1));
		
		int diff = (num-uipc.n_cyc_pans)+1;
		if(diff > 0) { memset(&uipc.cyc_pans[uipc.n_cyc_pans], 0, diff*sizeof(int*)); }
		
		uipc.cyc_pans[num] = malloc(sizeof(int)*uipc.cyc_steps[cyc]);
		uipc.n_cyc_pans = num+1;
		
		// Set up the new subpanels
		uipc.cyc_pans[num][0] = pc.inst[num];
		for(i = 1; i < uipc.cyc_steps[cyc]; i++) {
			uipc.cyc_pans[num][i] = setup_expanded_instr(num, i);
		}
			
		// Do the actual expansion.
		SetPanelAttribute(panel, ATTR_FRAME_STYLE, VAL_OUTLINED_FRAME);
		SetPanelAttribute(panel, ATTR_FRAME_THICKNESS, 1);
		
		resize_expanded(num, steps);
		
		// Change the callback function for the step-changer
		int info = num*1024;
		InstallCtrlCallback(panel, pc.pcstep, ChangePhaseCycleStepExpanded, (void*)info);

	} else {
		int ind = int_in_array(uipc.cyc_ins, num, uipc.ncins);
		if(ind < 0)  { return; }
		
		SetPanelAttribute(panel, ATTR_FRAME_STYLE, VAL_HIDDEN_FRAME);
		SetPanelAttribute(panel, ATTR_FRAME_THICKNESS, 0);

		resize_expanded(num, 1);
		save_expanded_instructions(ind);
		
		for(i = uipc.cyc_steps[cyc]-1; i > 0; i--) {
			DiscardPanel(uipc.cyc_pans[num][i]);
		}
		
		free(uipc.cyc_pans[num]); 
		uipc.cyc_pans[num] = NULL;
		
		// Change back the callback function for the step-changer
		InstallCtrlCallback(panel, pc.pcstep, ChangePhaseCycleStep, NULL);
	}
	
	SetCtrlAttribute(panel, pc.expandpc, ATTR_VISIBLE, !state);
	SetCtrlAttribute(panel, pc.collapsepc, ATTR_VISIBLE, state);
}

void set_phase_cycle_expanded_safe(int num, int state) {
	CmtGetLock(lock_uipc);
	set_phase_cycle_expanded(num, state);
	CmtReleaseLock(lock_uipc);
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
	DiscardCtrl(subp, pc.pclevel);
	DiscardCtrl(subp, pc.pcon);
	DiscardCtrl(subp, pc.pcsteps);
	DiscardCtrl(subp, pc.collapsepc);
	DiscardCtrl(subp, pc.expandpc);
	SetCtrlAttribute(subp, pc.ins_num, ATTR_VISIBLE, 0);
	SetCtrlVal(subp, pc.ins_num, num);
	
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
	int elements = 0, cyc;
	GetCtrlIndex(pc.inst[num], pc.pclevel, &cyc);
	char **c = generate_char_num_array(1, uipc.cyc_steps[cyc], &elements);
	for(int i = 0; i < uipc.cyc_steps[cyc]; i++) 
		InsertListItem(subp, pc.pcstep, -1, c[i], i);
	
	SetCtrlAttribute(subp, pc.pcstep, ATTR_DIMMED, 0);
	SetCtrlIndex(subp, pc.pcstep, step);
	
	c = free_string_array(c, elements);
	
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

int setup_expanded_instr_safe(int num, int step) {
	CmtGetLock(lock_uipc);
	int rv = setup_expanded_instr(num, step);
	CmtReleaseLock(lock_uipc);
	
	return rv;
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

void resize_expanded_safe(int num, int steps) {
	CmtGetLock(lock_uipc);
	resize_expanded(num, steps);
	CmtReleaseLock(lock_uipc);
}

void save_all_expanded_instructions() {
	// Saves all currently expanded instructions to the uipc variable.
	int i, expanded;
	
	for(i = 0; i < uipc.ncins; i++) {
		GetCtrlAttribute(pc.inst[uipc.cyc_ins[i]], pc.collapsepc, ATTR_VISIBLE, &expanded);
		if(expanded) {	// Save only if we need to.
			save_expanded_instructions(i);
		}
	}
}

void save_all_expanded_instructions_safe() {
	CmtGetLock(lock_uipc);
	save_all_expanded_instructions();
	CmtReleaseLock(lock_uipc);
}

void save_expanded_instructions(int ind) {
	// For convenience, this saves all the instructions at index "ind" in the uipc
	// phase cycling arrays (cyc_ins, ins_cycs, etc)
	
	// Start at the end and move backwards, so you don't have to realloc every time.
	for(int i = uipc.cyc_steps[uipc.ins_cycs[ind]]-1; i >= 0; i--) {
		save_expanded_instruction(uipc.cyc_ins[ind], i);
	}
}

void save_expanded_instructions_safe(int ind) {
	CmtGetLock(lock_uipc);
	save_expanded_instructions(ind);
	CmtReleaseLock(lock_uipc);
}

void save_expanded_instruction(int num, int step) {
	// Saves an expanded information to the uipc.c_instrs field.

	// Array allocation stuff
	if(uipc.c_instrs[num] == NULL) { uipc.max_cinstrs[num] = 0; }
	uipc.c_instrs[num] = malloc_or_realloc(uipc.c_instrs[num], sizeof(PINSTR*)*(step+1)); 
	
	if(uipc.max_cinstrs[num] < step+1){ 
		memset(uipc.c_instrs[num], 0, sizeof(PINSTR*)*((step+1)-uipc.max_cinstrs[num]));
		uipc.max_cinstrs[num] = step+1;
	}
	
	// Now we have a proper uipc variable, just add in the instruction.
	if(uipc.c_instrs[num][step] == NULL) { uipc.c_instrs[num][step] = malloc(sizeof(PINSTR)); }
	get_instr_panel(uipc.c_instrs[num][step], uipc.cyc_pans[num][step]);
}

void save_expanded_instruction_safe(int num, int step) {
	CmtGetLock(lock_uipc);
	save_expanded_instruction(num, step);
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

void move_expanded_instruction_safe(int num, int to, int from) {
	CmtGetLock(lock_uipc);
	move_expanded_instruction(num, to, from);
	CmtReleaseLock(lock_uipc);
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
			if(uipc.max_cinstrs[num] < uipc.cyc_steps[cyc]) {
				uipc.c_instrs[num] = realloc(uipc.c_instrs[num], sizeof(PINSTR*)*uipc.cyc_steps[cyc]);
				uipc.max_cinstrs[num] = uipc.cyc_steps[cyc];
			}
			
			in_buff = uipc.c_instrs[num][step];
			for(j =	step; j < uipc.cyc_steps[cyc]-1; i++) {
				uipc.c_instrs[num][j] = uipc.c_instrs[num][j+1];	
			}
			uipc.c_instrs[num][j] = in_buff;
		}
	}
	
	// Now we just change the number of instructions in the cycle.
	change_cycle_num_steps(cyc, uipc.cyc_steps[cyc]-1);
}

void delete_expanded_instruction_safe(int num, int step) {
	CmtGetLock(lock_uipc);
	delete_expanded_instruction(num, step);
	CmtReleaseLock(lock_uipc);
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
	
	// Expanded panels may need to be fiddled with.
	if(uipc.n_cyc_pans > 0) { 
		int max_ind = uipc.n_cyc_pans-1;	// First find the new highest panel.
		if((from <= max_ind && to > max_ind) /*1*/ || (from >= max_ind && to <= max_ind) /*2*/) { 
			if(from < to) { // Case 1
				if(uipc.cyc_pans[from] != NULL) {
					max_ind = to;
				} else {
					for(i = max_ind; i > from; i--) {
						if(uipc.cyc_pans[i] != NULL) { break; }	
					}
				
					max_ind = i-1; // It's going to move down one spot.
				}
			} else if(from > to) { // Case 2
				if(from > max_ind) {
					max_ind++; 		// It will just move up a spot.	
				} else {
					for(i = from-1; i > to; i--) {
						if(uipc.cyc_pans[i] != NULL) { break; }
					}
					max_ind = i+1;	// Whatever it is moves up a spot.
				}
			}
		}
	
		// Reallocate beforehand if necessary.
		if(max_ind+1 > uipc.n_cyc_pans) {
			// This will only happen if to > from and the new max_ind <= to.
			uipc.cyc_pans = realloc(uipc.cyc_pans, (max_ind+1)*sizeof(int*));
			memset(&uipc.cyc_pans[uipc.n_cyc_pans], 0, (max_ind+1)-uipc.n_cyc_pans);
		}


		start = (to > from)?from:to;
		end = (to > from)?to:from;
	
		int le = (end>max_ind)?max_ind:end;
		int *pan = uipc.cyc_pans[start];
		for(i = start; i < le; i++) {
			uipc.cyc_pans[i] = uipc.cyc_pans[i+1];	
		}

		if(end == max_ind) {
			uipc.cyc_pans[end] = pan;	
		}

		if(++max_ind < uipc.n_cyc_pans) {
			uipc.cyc_pans = realloc(uipc.cyc_pans, (max_ind)*sizeof(int*));
		}
	
		uipc.n_cyc_pans = max_ind;
	}
	
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

int move_instruction_safe(int to, int from) {
	CmtGetLock(lock_uipc);
	int rv = move_instruction(to, from);
	CmtReleaseLock(lock_uipc);
	
	return rv;
}


int move_fr_inst(int to, int from) {
	// Moves first-run instructions.
	//
	// 0 on no change
	// 1 on success.
	if(to > uipc.fr_ni || from > uipc.fr_ni || from < 0 || to < 0 || from == to) {
		return 0;
	}
	
	int *n_array = generate_mover_array(to, from, ((to>from)?to:from)+1);
	if(n_array == NULL) { return 0; }
	
	int pan_height;
	GetPanelAttribute(pc.finst[0], ATTR_HEIGHT, &pan_height);
	
	int i, j;
	
	int s, e;
	if(to > from) {
		s = from;
		e = to+1;
	} else {
		s = to;
		e = from+1;
	}
				  
	int *inst_buff = malloc(sizeof(int)*e);
	memcpy(inst_buff, pc.finst, sizeof(int)*e);
	
	for(i = s; i < e; i++) {
		j = n_array[i];
		SetPanelAttribute(pc.finst[i], ATTR_HEIGHT, MC_FR_INST_OFF+(pan_height+MC_FR_INST_SEP)*j);
		SetCtrlVal(pc.finst[i], pc.fr_inum, j);
		
		pc.finst[i] = inst_buff[j];
	}
	
	free(inst_buff);
	free(n_array); 
	
	return 1;
}

int move_fr_inst_safe(int to, int from) {
	CmtGetLock(lock_uipc);
	int rv = move_fr_inst(to, from);
	CmtReleaseLock(lock_uipc);
	
	return rv;
}

void clear_instruction(int num) {
	// Takes an instruction and restores it to defaults.
	
	set_instr(num, NULL);		// This sets everything to defaults
	
	// If this was a phase cycled instruction, this is the only time we want
	// to clear that, so we're going to free up the memory and such.
	if(get_pc_state(num)) { update_pc_state(num, 0); }

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
	
	SetCtrlVal(pc.inst[num], pc.pcsteps, 2);
	
	// We also want to clear stored ND information.
	if(get_nd_state(num)) { update_nd_state(num, 0); }
	
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
	if(nl) { DeleteListItem(pc.inst[num], pc.pcstep, 0, -1); }
	
	GetNumListItems(pc.inst[num], pc.pclevel, &nl);
	if(nl) { DeleteListItem(pc.inst[num], pc.pclevel, 0, -1); }
	
	GetNumListItems(pc.cinst[num], pc.dim, &nl);
	if(nl) { DeleteListItem(pc.cinst[num], pc.dim, 0, -1); }
}

void clear_instruction_safe(int num) { 
	CmtGetLock(lock_uipc);
	clear_instruction(num);
	CmtReleaseLock(lock_uipc);
}

void clear_fr_instr(int num) {
	 set_fr_instr(num, null_pinstr()); // Really quite simple now.
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
		uipc.ni = num;
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

		if(uipc.c_instrs != NULL) {
			uipc.c_instrs = realloc(uipc.c_instrs, sizeof(PINSTR**)*num);
			uipc.max_cinstrs = realloc(uipc.max_cinstrs, sizeof(int)*num);
			for(i=uipc.max_ni; i<num; i++) {
				uipc.c_instrs[i] = NULL;
				uipc.max_cinstrs[i] = 0;
			}
		}

		uipc.max_ni = num;		// We've increased the max number of instructions, so increment it.
		setup_broken_ttls();
	}
	
	int ndon;
	GetCtrlVal(pc.ndon[1], pc.ndon[0], &ndon);
	
	for(i=uipc.ni; i<num; i++) {	// Now we can show the remaining panels.
		DisplayPanel(pc.inst[i]);
		DisplayPanel(pc.cinst[i]);
		SetPanelAttribute(pc.cinst[i], ATTR_DIMMED, !ndon);
	}
	
	uipc.ni = num;	// And update the uipc value.
}

void change_number_of_instructions_safe() {
	CmtGetLock(lock_uipc);
	change_number_of_instructions();
	CmtReleaseLock(lock_uipc);
}

void change_fr_num_instrs(int num) {
	// Change the number of instructions in the "first run" instructions.
	int i;
	
	if(num < 1) {
		num = 1;	
	}
	
	if(num == uipc.fr_ni) {
		return;
	}
	
	if(num < uipc.fr_ni) {
		for(i = num; i < uipc.fr_max_ni; i++) {
			HidePanel(pc.finst[i]);
		}
		
		uipc.fr_ni = num;
		return;
	}

	if(num > uipc.fr_max_ni) {
		int left, height;
		GetPanelAttribute(pc.finst[0], ATTR_HEIGHT, &height);
		GetPanelAttribute(pc.finst[0], ATTR_LEFT, &left);
		
		pc.finst = realloc(pc.finst, sizeof(int)*num);
		double del;
		
		for(i = uipc.fr_max_ni; i < num; i++) {
			pc.finst[i] = LoadPanel(pc.FRCPan, MC_UI, pc.fr_inst);
			
			SetPanelPos(pc.finst[i], MC_FR_INST_OFF+(height+MC_FR_INST_SEP)*i, left);
			SetCtrlAttribute(pc.finst[i], ATTR_DISABLE_PANEL_THEME, 1);
			
			GetCtrlVal(pc.inst[i], pc.fr_delay, &del);
			SetCtrlAttribute(pc.finst[i], pc.fr_delay, ATTR_PRECISION, get_precision(del, MCUI_DEL_PREC));
			
			SetCtrlVal(pc.finst[i], pc.fr_inum, i);
			
			DisplayPanel(pc.finst[i]);
		}
		
		uipc.fr_max_ni = num;
		setup_broken_ttls();
	}
	
	for(i = uipc.fr_ni; i < num; i++) {
		DisplayPanel(pc.finst[i]);	
	}
	
	uipc.fr_ni = num;
}

void change_fr_num_instrs_safe(int num) {
	CmtGetLock(lock_uipc);
	change_fr_num_instrs(num);
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

void delete_instruction_safe(int num) {
	CmtGetLock(lock_uipc);
	delete_instruction(num);
	CmtReleaseLock(lock_uipc);
}

void delete_fr_instr(int num) {
	clear_fr_instr(num);
	
	if(uipc.fr_ni == 1) {
		return ;
	}
		
	move_fr_inst(uipc.fr_max_ni-1, num);
	
	SetCtrlVal(pc.FRPan, pc.fninst, uipc.fr_ni-1);
	change_fr_num_instrs(uipc.fr_ni-1);
}

void delete_fr_instr_safe(int num) {
	CmtGetLock(lock_uipc);
	delete_fr_instr(num);
	CmtReleaseLock(lock_uipc);
}

/****************** Analog Output Manipulation *******************/ 
void change_ao_device(int num) {
	// Change which device a given instruction uses.
	int pan = pc.ainst[num], ctrl = pc.aodev, nl, dev;
	
	GetNumListItems(pan, ctrl, &nl);
	if(nl <= 0)
		return;
	
	GetCtrlVal(pan, ctrl, &dev);
	
	uipc.ao_devs[num] = dev;
	populate_ao_chan(num);
}

void change_ao_device_safe(int num) {
	CmtGetLock(lock_uipc);
	change_ao_device(num);
	CmtReleaseLock(lock_uipc);
}

void change_ao_chan(int num) {
	change_ao_chan_uipc(num);
	
	//  Go through and re-populate the controls 
	for(int i = 0; i < uipc.max_anum; i++) {
		populate_ao_chan(i);
	}
}

void change_ao_chan_safe(int num) {
	CmtGetLock(lock_uipc);
	change_ao_chan(num);
	CmtReleaseLock(lock_uipc);
}

void change_ao_chan_uipc(int num) { 
	// Change the channel a given instruction uses.
	int pan = pc.ainst[num], ctrl = pc.aochan, dev = uipc.ao_devs[num];
	int nl, ind = -1;
	
	if(dev < 0)
		return;
	
	// If it's another available channel, remove it from the list.
	GetNumListItems(pan, ctrl, &nl);
	if(nl >= 2) {
		GetCtrlVal(pan, ctrl, &ind);
		if(ind == uipc.ao_chans[num])
			return; 	// No change.
		
		int spot = ind;
		if(ind >= 0) { 
			spot = int_in_array(uipc.ao_avail_chans[dev], ind, uipc.anum_avail_chans[dev]);
		}
	
		if(spot >= 0) {
			ind = uipc.ao_avail_chans[dev][spot];
			remove_array_item(uipc.ao_avail_chans[dev], spot, uipc.anum_avail_chans[dev]--);
			uipc.ao_avail_chans[dev] = realloc(uipc.ao_avail_chans[dev], sizeof(int)*uipc.anum_avail_chans[dev]);
		}
	}
	
	// If it was unavailable before, we need to add it back in.
	if(uipc.ao_chans[num] >= 0) {
		uipc.ao_avail_chans[dev] = add_item_to_sorted_array(uipc.ao_avail_chans[dev], uipc.ao_chans[num], uipc.anum_avail_chans[dev]++);
	}
	
	uipc.ao_chans[num] = ind;
}

void change_ao_chan_uipc_safe(int num) {
	CmtGetLock(lock_uipc);
	change_ao_chan_uipc(num);
	CmtReleaseLock(lock_uipc);
}

void populate_ao_dev(int num) {
	int pan = pc.ainst[num], ctrl = pc.aodev, dev, nl;
	
	GetNumListItems(pan, ctrl, &nl);
	if(nl > 0) { DeleteListItem(pan, ctrl, 0, -1); }
	
	InsertListItem(pan, ctrl, -1, "None", -1);
	
	for(int i = 0; i < uipc.anum_devs; i++) {
		InsertListItem(pan, ctrl, -1, uipc.adev_display[i], i);	
	}
	
	dev = uipc.ao_devs[num];
	
	SetCtrlVal(pan, ctrl, dev);
	SetCtrlAttribute(pan, ctrl, ATTR_DFLT_VALUE, uipc.default_adev);
}

void populate_ao_dev_safe(int num) {
	CmtGetLock(lock_uipc);
	populate_ao_dev(num);
	CmtReleaseLock(lock_uipc);
}

void populate_ao_chan(int num) {
	// Populate the selected channel ring control as appropriate.
	int dev, ind, pan = pc.ainst[num], ctrl = pc.aochan, cind, nl, i;
	
	dev = uipc.ao_devs[num];
	ind = uipc.ao_chans[num];
	
	// Clear the old list
	GetNumListItems(pan, ctrl, &nl);
	if(nl > 0) { DeleteListItem(pan, ctrl, 0, -1); }
	
	// Populate it again.
	InsertListItem(pan, ctrl, -1, "Disable", -1);
	if(dev < 0)
		return;
	
	if(uipc.anum_avail_chans[dev] > 0) {
		for(i = 0; i < uipc.anum_avail_chans[dev]; i++) {
			cind = uipc.ao_avail_chans[dev][i];
			if(ind >= 0 && ind < cind) {  // Insertion sort, essentially.
				InsertListItem(pan, ctrl, -1, uipc.ao_all_chans[dev][ind], ind);
				break;
			}
	
			InsertListItem(pan, ctrl, -1, uipc.ao_all_chans[dev][cind], cind);
		}
		
		if(ind >= 0 && i == uipc.anum_avail_chans[dev]) {
			InsertListItem(pan, ctrl, -1, uipc.ao_all_chans[dev][ind], ind);
		}
		
		for(i = i; i < uipc.anum_avail_chans[dev]; i++) {
			cind = uipc.ao_avail_chans[dev][i];
			InsertListItem(pan, ctrl, -1, uipc.ao_all_chans[dev][cind], cind);		
		}

	} else if(ind >= 0) {
		InsertListItem(pan, ctrl, -1, uipc.ao_all_chans[dev][ind], ind); 		
	}
	
	SetCtrlVal(pan, ctrl, ind);
}

void populate_ao_chan_safe(int num) {
	CmtGetLock(lock_uipc);
	populate_ao_chan(num);
	CmtReleaseLock(lock_uipc);
}

void change_num_aouts() {
	// Gets "num" from pc.anum and changes the number of analog output channels.
	// Same behavior as change_number_of_instructions.
	unsigned short int num, i;
	GetCtrlVal(pc.anum[1], pc.anum[0], &num);
	
	if(num == uipc.anum)
		return;		// We're done.
	
	int ndon;
	GetCtrlVal(pc.ndon[1], pc.ndon[0], &ndon);
												
	set_aout_dimmed(0, (num == 0), ndon);
	
	if(num < uipc.anum) {
		int dev, ind, s;
		
		//Hide the panels.
		for(i = num+((num==0)?1:0); i<uipc.max_anum; i++) {
			HidePanel(pc.ainst[i]);
			
			dev = uipc.ao_devs[i];
			ind = uipc.ao_chans[i];
			
			// Release the channel for use by something else.
			if(dev >= 0 && ind >= 0) {
				s = uipc.anum_avail_chans[dev];
				uipc.ao_avail_chans[dev] = realloc(uipc.ao_avail_chans[dev], s+1);
				uipc.ao_avail_chans[dev][s] = ind;
			}
		}
		
		// Refresh the views.
		for(i = 0; i < uipc.max_anum; i++) {
			populate_ao_chan(i);	
		}
		
		uipc.anum = num;
		return;
	}

	// Make some more panels if necessary.
	if(num > uipc.max_anum) {
		int top, left, height, mi = (uipc.max_anum > 0)?uipc.max_anum-1:0;
		GetPanelAttribute(pc.ainst[mi], ATTR_TOP, &top);
		GetPanelAttribute(pc.ainst[mi], ATTR_LEFT, &left);
		GetPanelAttribute(pc.ainst[mi], ATTR_HEIGHT, &height);
		
		pc.ainst = realloc(pc.ainst, sizeof(int)*num);
	
		// Make some entries in the uipc var for the new instructions.
		uipc.ac_varied = malloc_or_realloc(uipc.ac_varied, sizeof(int)*num);
		uipc.ac_dim = malloc_or_realloc(uipc.ac_dim, sizeof(int)*num);
		uipc.ao_devs = malloc_or_realloc(uipc.ao_devs, sizeof(int)*num);
		uipc.ao_chans = malloc_or_realloc(uipc.ao_chans, sizeof(int)*num);
		uipc.ao_vals = malloc_or_realloc(uipc.ao_vals, sizeof(double*)*num);
		uipc.ao_exprs = malloc_or_realloc(uipc.ao_exprs, sizeof(char*)*num);
	
		// Null or -1 initialize these. Keep in mind that memset is really for
		// byte arrays (unsigned short ints). 0 always null-initializes, but
		// -1 just happens to work for ints in this bit-ordering. If you tried to
		// do something like memset(int_array, 5, num_bytes), you'd have a weird
		// result. This is probably NOT good coding practice when your bit order
		// isn't constrained.
		int diff = num-uipc.max_anum;
		memset(uipc.ac_varied+uipc.max_anum, 0, sizeof(int)*diff);
		memset(uipc.ac_dim+uipc.max_anum, -1, sizeof(int)*diff);
		memset(uipc.ao_chans+uipc.max_anum, -1, sizeof(int)*diff);
		memset(uipc.ao_exprs+uipc.max_anum, 0, sizeof(char*)*diff);
		
		if(uipc.max_anum == 0) {
			uipc.ao_vals[0] = malloc(sizeof(double));
			GetCtrlAttribute(pc.ainst[0], pc.ainitval, ATTR_DFLT_VALUE, uipc.ao_vals[0]);
			
			uipc.ao_devs[0] = uipc.default_adev;
			
			populate_ao_dev(0);
			populate_ao_chan(0);
			
			uipc.max_anum++;
		}
		
		for(i=uipc.max_anum; i<num; i++) {
			pc.ainst[i] = LoadPanel(pc.AOutCPan, pc.uifname, pc.a_inst);
			SetPanelPos(pc.ainst[i], top+=height+5, left);
			
			uipc.ao_vals[i] = malloc(sizeof(double));
			GetCtrlAttribute(pc.ainst[i], pc.ainitval, ATTR_DFLT_VALUE, uipc.ao_vals[i]);
			
			uipc.ao_devs[i] = uipc.default_adev;
			
			populate_ao_dev(i);
			populate_ao_chan(i);

		}

		
		uipc.max_anum = num;
	}
	for(i = uipc.anum; i<num; i++) {
		DisplayPanel(pc.ainst[i]);
		
		set_aout_nd_dimmed(i, !ndon);
	}
	
	// Finally update uipc for later.
	uipc.anum = num;
}

void change_num_aouts_safe() { 
	CmtGetLock(lock_uipc);
	change_num_aouts();
	CmtReleaseLock(lock_uipc);
}

void delete_aout(int num) {
	clear_aout(num);		// Clear the values.
	
	SetCtrlVal(pc.anum[1], pc.anum[0], uipc.anum-1);
	
	// Move it to the end if necessary.
	if(uipc.anum > 1) {
		move_aout(uipc.max_anum-1, num);
	}
	
	change_num_aouts();
}

void delete_aout_safe(int num) {
	CmtGetLock(lock_uipc);
	delete_aout(num);
	CmtReleaseLock(lock_uipc);
}

void move_aout(int to, int from) {
	// Moves Analog Out channels around.   
	int diff = to-from;
	
	if(diff == 0)
		return;				// No change
	
	int start, end;
	if(diff > 0) {
		start = from;
		end = to;
	} else {
		start = to;
		end = from;
	}
	
	// Save the one we're moving to a buffer.
	int var, dim, dev, chan, instr, btop, top;
	double *vals;
	char *exprs;
	GetPanelAttribute(pc.ainst[start], ATTR_TOP, &btop);
	
	instr = pc.ainst[start];     
	
	var = uipc.ac_varied[start];
	dim = uipc.ac_dim[start];
	dev = uipc.ao_devs[start];
	chan = uipc.ao_chans[start];
	vals = uipc.ao_vals[start];
	exprs = uipc.ao_exprs[start];

	// Now move each one back one.
	for(int i = start; i < end; i++) {
		GetPanelAttribute(pc.ainst[i+1], ATTR_TOP, &top);	// Move it.
		SetPanelAttribute(pc.ainst[i+1], ATTR_TOP, btop);
		btop = top;
		
		pc.ainst[i] = pc.ainst[i+1];
		
		uipc.ac_varied[i] = uipc.ac_varied[i+1];
		uipc.ac_dim[i] = uipc.ac_dim[i+1];
		uipc.ao_devs[i] = uipc.ao_devs[i+1];
		uipc.ao_chans[i] = uipc.ao_chans[i+1];
		uipc.ao_vals[i] = uipc.ao_vals[i+1];
		uipc.ao_exprs[i] = uipc.ao_exprs[i+1];
	}
	
	// Now move from buffer to the last stop.
	SetPanelAttribute(instr, ATTR_TOP, top);
	pc.ainst[end] = instr;
	
	uipc.ac_varied[end] = var;
	uipc.ac_dim[end] = dim;
	uipc.ao_devs[end] = dev;
	uipc.ao_chans[end] = chan;
	uipc.ao_vals[end] = vals;
	uipc.ao_exprs[end] = exprs;
}

void move_aout_safe(int to, int from) {
	CmtGetLock(lock_uipc);
	move_aout(to, from);
	CmtReleaseLock(lock_uipc);
}

void clear_aout(int num) {
	int	val, nl; 
	
	// Set values to 0.
	SetCtrlVal(pc.ainst[num], pc.ainitval, 0.0);
	SetCtrlVal(pc.ainst[num], pc.aincval, 0.0);
	SetCtrlVal(pc.ainst[num], pc.afinval, 0.0);
	
	set_ao_nd_state(num, 0);
	
	// Clear the expression control
	SetCtrlVal(pc.ainst[num], pc.aincexpr, "");
	SetCtrlAttribute(pc.ainst[num], pc.aincexpr, ATTR_VISIBLE, 0);
	SetCtrlAttribute(pc.ainst[num], pc.aincexpr, ATTR_TEXT_BGCOLOR, VAL_WHITE);
	
	// Set channel to default value.
	GetNumListItems(pc.ainst[num], pc.aochan, &nl);
	if(nl) {
		SetCtrlIndex(pc.ainst[num], pc.aochan, 0);
		change_ao_chan_uipc(num);
	}
	
	// Set device to default value.
	GetNumListItems(pc.ainst[num], pc.aodev, &nl);
	if(nl > 0) {
		GetCtrlAttribute(pc.ainst[num], pc.aodev, ATTR_DFLT_VALUE, &val);
		SetCtrlVal(pc.ainst[num], pc.aodev, val);
		change_ao_device(num);
	}
	
	// Variation stuff.
	GetNumListItems(pc.ainst[num], pc.adim, &nl);
	if(nl) { DeleteListItem(pc.ainst[num], pc.adim, 0, -1); }
	
	GetCtrlAttribute(pc.ainst[num], pc.asteps, ATTR_DFLT_VALUE, &val);
	SetCtrlVal(pc.ainst[num], pc.asteps, val);
	
	// uipc variable
	uipc.ac_dim[num] = -1;
	uipc.ao_vals[num] = realloc(uipc.ao_vals[num], sizeof(double));
	GetCtrlAttribute(pc.ainst[num], pc.ainitval, ATTR_DFLT_VALUE, uipc.ao_vals[num]);
}

void set_aout_dimmed(int num, int dimmed, int nd) {
	// Dim an entire analog output
	// nd tells you whether or not to undim the multidimensional bit.
	int pan = pc.ainst[num];
	
	set_aout_nd_dimmed(num, dimmed || !nd);
	
	SetCtrlAttribute(pan, pc.ainitval, ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pan, pc.aodev, ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pan, pc.aochan, ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pan, pc.axbutton, ATTR_DIMMED, dimmed);
}

void set_aout_dimmed_safe(int num, int dimmed, int nd) {
	CmtGetLock(lock_uipc);
	set_aout_dimmed(num, dimmed, nd);
	CmtReleaseLock(lock_uipc);
}

void set_aout_nd_dimmed(int num, int dimmed) {
	// Dim the ND controls for the analog output.
	int pan = pc.ainst[num];
	int state = get_ao_nd_state(num);
	
	SetCtrlAttribute(pan, pc.aindon, ATTR_DIMMED, dimmed);
	
	if(state) {
		SetCtrlAttribute(pan, pc.asteps, ATTR_DIMMED, dimmed);
		SetCtrlAttribute(pan, pc.aincexpr, ATTR_DIMMED, dimmed);
		SetCtrlAttribute(pan, pc.aincval, ATTR_DIMMED, dimmed);
		SetCtrlAttribute(pan, pc.afinval, ATTR_DIMMED, dimmed);
		SetCtrlAttribute(pan, pc.adim, ATTR_DIMMED, dimmed);
	}
	
	if(uipc.ac_varied != NULL) {
		uipc.ac_varied[num] = (state && !dimmed); // Is this actually necessary?
	}
}

void set_aout_nd_dimmed_safe(int num, int dimmed) {
	CmtGetLock(lock_uipc);
	set_aout_nd_dimmed(num, dimmed);
	CmtReleaseLock(lock_uipc);
}

void change_ao_val(int num) {
	// Function called to change the analog output value on channel "num"
	// Only call this for non-varying channels.
	int pan = pc.ainst[num];
	double val;
	
	GetCtrlVal(pan, pc.ainitval, &val);
	
	uipc.ao_vals[num][0] = val;		
}

void change_ao_val_safe(int num) {
	CmtGetLock(lock_uipc);
	change_ao_val(num);
	CmtReleaseLock(lock_uipc);
}

void set_ao_nd_state(int num, int state) {
	// For allowing variation in an analog dimension.
	int pan = pc.ainst[num], dimmed = 1, expr_visible = 0;
	
	SetCtrlVal(pan, pc.aindon, state);
	SetCtrlAttribute(pan, pc.aindon, ATTR_ON_COLOR, (state == 2)?VAL_BLUE:VAL_RED);
	
	switch(state) {
		case 2:
			expr_visible = 1;
		case 1:
			dimmed = 0;
	}
	
	// Update the controls if appropriate
	int nl, dim = -1, dims, steps;
	if(state) {
		GetNumListItems(pan, pc.adim, &nl);

		if(nl < uipc.nd) {
			for(int i = nl; i < uipc.nd; i++) {
				int elements;
				char **c = generate_char_num_array(1, uipc.nd, &elements);
				
				for(i = nl; i < uipc.nd; i++) {
					InsertListItem(pan, pc.adim, -1, c[i], i);
				}
				
				c = free_string_array(c, elements);
			}
		} else if(nl > uipc.nd) {
			DeleteListItem(pan, pc.adim, uipc.nd, -1);	
		}
		
		GetCtrlVal(pan, pc.adim, &dim);
		SetCtrlVal(pan, pc.asteps, uipc.dim_steps[dim]);
		
		uipc.ao_vals[num] = realloc(uipc.ao_vals[num], sizeof(double)*uipc.dim_steps[dim]);
		
		dimmed = 0;
		
		if(state == 2) {
			expr_visible = 1;
		} else {
			state = 1;
			update_ao_increment(num, MC_INC);
		}
	}
	
	if(uipc.ac_varied != NULL) { 
		uipc.ac_varied[num] = state;
		if(uipc.ac_dim != NULL) {
			uipc.ac_dim[num] = dim;
		}
	}
	
	SetCtrlAttribute(pan, pc.asteps, ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pan, pc.aincexpr, ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pan, pc.aincval, ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pan, pc.afinval, ATTR_DIMMED, dimmed);
	SetCtrlAttribute(pan, pc.adim, ATTR_DIMMED, dimmed);
	
	SetCtrlAttribute(pan, pc.aincexpr, ATTR_VISIBLE, expr_visible);
	SetCtrlAttribute(pan, pc.aincval, ATTR_VISIBLE, !expr_visible);

}

void set_ao_nd_state_safe(int num, int state) {
	CmtGetLock(lock_uipc);
	set_ao_nd_state(num, state);
	CmtReleaseLock(lock_uipc);
}

int get_ao_nd_state(int num) {
	int pan = pc.ainst[num], on, col, state;
	
	GetCtrlVal(pan, pc.aindon, &on);
	GetCtrlAttribute(pan, pc.aindon, ATTR_ON_COLOR, &col);
	state = (on && col == VAL_BLUE)?2:on;
	
	return state;
}

char *get_ao_full_chan_name(int dev, int chan) {
	// Gets the full name of the channel, including the device.
	// This will eventually be used to distinguish between channel
	// naming mechanisms when more than DAQs are supported.
	//
	// This returns a malloced string, which needs to be freed
	
	char *s = NULL;
	
	// Failure conditions
	if(dev < 0 || uipc.adev_true == NULL || uipc.adev_true[dev] == NULL) {
		return s;
	}
	
	char *dname = uipc.adev_true[dev], *cname;
	
	// If no channel is provided but a dev is provided, it'll be of the form "Dev1/"
	if(chan < 0 || uipc.ao_all_chans == NULL || uipc.ao_all_chans[dev] == NULL ||
	   uipc.ao_all_chans[dev][chan] == NULL) {
		cname = "";
	} else {
		cname = uipc.ao_all_chans[dev][chan];
	}
	int dlen = strlen(dname), clen = strlen(cname);
	
	// We'll assume that neither has a separator.
	s = malloc(dlen+clen+2);

	if(dname[dlen-1] == '/') { dlen--; }	// Don't copy a trailing slash
	if(cname[0] == '/') { cname++; } 		// Move the pointer up one.
	
	// Concatenate the two strings with a separator.
	strncpy(s, dname, dlen);
	s[dlen] = '/';
	strcpy(s+dlen+1, cname); // cname is null-terminated.
	
	return s;
}

char *get_ao_full_chan_name_safe(int dev, int chan) {
	CmtGetLock(lock_uipc);
	char *rv = get_ao_full_chan_name(dev, chan);
	CmtReleaseLock(lock_uipc);
	
	return rv;
}

void get_ao_dev_chan(char *name, int *dev_out, int *chan_out) {
	// Given the full name as would be returned from get_ao_full_chan_name,
	// this will give you the indices for the device and channel.
	//
	// Passing NULL to dev_out or chan_out will suppress that output.
	// Values are set to -1 if not found.
	//
	// TODO: This function may need to be updated and the
	// prototype changed when non-DAQ boards are supported.
	
	int dev = -1, chan = -1;
	
	// No input or no boards, then it's definitely not in here.
	if(name == NULL || uipc.anum_devs < 1) { goto error; }
	
	// First find the device
	char *p = malloc(strlen(name)+1);
	strcpy(p, name);
	
	p = strtok(p, "/");
	if(p != NULL) { dev = string_in_array(uipc.adev_true, p, uipc.anum_devs); }
	
	if(dev < 0) { goto error; }
	
	// Now that we have the device, search for the channel.
	p = strchr(name, '/');
	if(p != NULL) { chan = string_in_array(uipc.ao_all_chans[dev], ++p, uipc.anum_all_chans[dev]); }
	
	error:
	
	if(dev_out != NULL) { *dev_out = dev; }
	if(chan_out != NULL) { *chan_out = chan; }
}

void get_ao_dev_chan_safe(char *name, int *dev, int *chan) {
	CmtGetLock(lock_uipc);
	get_ao_dev_chan(name, dev, chan);
	CmtReleaseLock(lock_uipc);
}

void update_ao_increment(int num, int mode) {
	// Modes:	
	// MC_INIT = Change initial, leave the other two
	// MC_INC = Change increment, leave the other two
	// MC_FINAL = Change final
	
	double init, inc, fin;
	double min, max;
	int steps;				// Number of steps
	int panel = pc.ainst[num];
	
	// Get the values that are there now.
	GetCtrlVal(panel, pc.ainitval, &init);
	GetCtrlVal(panel, pc.aincval, &inc);
	GetCtrlVal(panel, pc.afinval, &fin);
	
	// Get the number of steps
	GetCtrlVal(panel, pc.asteps, &steps);
	
	// Minimum and maximum values should be stored in the init.
	GetCtrlAttribute(panel, pc.ainitval, ATTR_MIN_VALUE, &min);
	GetCtrlAttribute(panel, pc.ainitval, ATTR_MAX_VALUE, &max);
	
	// Perform the calculations.
	steps--;
	switch(mode) {
		case MC_INIT:
			init = fin-(inc*steps); // Simple enough
		
			// Check the min/max.
			if(init < min) {
				init = min;
				fin = min+steps*inc;
			} else if(init > max) {
				init = max;
				fin = max+steps*inc;
			}
			
			// If our changes pushed fin to min/max, change inc
			if(fin > max) {
				fin = max;
			} else if(fin < min) {
				fin = min;
			}
			
			break;
		case MC_FINAL:
			fin = init+(inc*steps);
			
			// Enforce min/max values.
			if(fin < min) {
				fin = min;
				init = fin-(inc*steps);
			} else if(fin > max) {
				fin = max;
				init = fin-(inc*steps);
			}
			
			if(init < min) {
				init = min;
			} else if(init > max) {
				init = max;
			}
			break;
	}
	
	inc = (fin-init)/steps;	// This must always be true anyway.
	
	// Update the uipc variable
	for(int i = 0; i <= steps; i++) {
		uipc.ao_vals[num][i] = init + inc*i;
	}
	
	SetCtrlVal(panel, pc.ainitval, init);
	SetCtrlVal(panel, pc.aincval, inc);
	SetCtrlVal(panel, pc.afinval, fin);
}

void update_ao_increment_safe(int num, int mode) {
	CmtGetLock(lock_uipc);
	update_ao_increment(num, mode);
	CmtReleaseLock(lock_uipc);
}
	

void change_ao_dim(int num) {
	int dim, pan = pc.ainst[num];
	
	GetCtrlVal(pan, pc.adim, &dim);
	
	// Hopefully dim isn't malformed!
	SetCtrlVal(pan, pc.asteps, uipc.dim_steps[dim]);
	
	uipc.ac_dim[num] = dim;
	
	int state = uipc.ac_varied[num];
	
	if(state == 2) {
		update_ao_increment(num, MC_INC);
	} else {
		//update_ao_increment_from_exprs(num);	
	}
}

void change_ao_dim_safe(int num) {
	CmtGetLock(lock_uipc);
	change_ao_dim(num);
	CmtReleaseLock(lock_uipc);
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

constants *setup_constants_safe() {
	CmtGetLock(lock_uipc);
	constants *c = setup_constants();
	CmtReleaseLock(lock_uipc);
	
	return c;
}

void update_constants(constants *c, int *cstep) { // Updates the constants for a given position in acq. space
	// cstep must be a position in acqusition space of size uipc.nd+uipc.nc+1
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
	//	p->varied		p->nFuncs       p->nAout
	//	p->skip			p->nDims
	//	p->nVaried	   	p->nCycles

	if(p->varied) {
		p->maxsteps = malloc(sizeof(int)*(p->nDims+p->nCycles)); 				// Allocate the maximum position point
		
		p->steps_size = get_steps_array_size(p->tmode, p->nCycles, p->nDims);
		
		if(p->steps_size > 0) {
			p->steps = malloc(p->steps_size*sizeof(unsigned int));
		} else {
			p->steps = NULL;	
		}
	} else {
		p->maxsteps = NULL;
		p->steps = NULL;
	}
	
	if(p->nVaried) {
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
		p->v_ins = NULL;
		p->v_ins_dim = NULL;
		p->v_ins_mode = NULL;
		
		p->delay_exprs = NULL;
		p->data_exprs = NULL;
	}
	
	if(p->nAout) {
		p->ao_varied = calloc(p->nAout, sizeof(int));
		p->ao_vals = calloc(p->nAout, sizeof(double*));   
		p->ao_chans = calloc(p->nAout, sizeof(char*));
		p->ao_exprs = calloc(p->nAout, sizeof(char*));
		
		p->ao_dim = malloc((p->nAout)*sizeof(int));		
		memset(p->ao_dim, -1, sizeof(int)*p->nAout);
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
	
	if(p->steps != NULL) { free(p->steps); }
	
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
	
	if(p->nAout) {
		free(p->ao_varied);
		free(p->ao_dim);
		
		free_doubles_array(p->ao_vals, p->nAout);
		free_string_array(p->ao_chans, p->nAout);
		free_string_array(p->ao_exprs, p->nAout);
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

PINSTR *generate_instructions(PPROGRAM *p, int cind, int *n_inst) {
	// Generates the list of instructions that you want for the given cstep.
	// Return value is a list of instructions. Outputs the new number of instructions
	// to n_inst, which need not be set to the right initial value. The return value
	// is dynamically allocated, and must be freed.

	*n_inst = -1;
	if(p == NULL) { return NULL; }
	
	int lindex = get_var_ind(p, cind);
	if(lindex < 0) { return NULL; }
	
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

int get_transient(PPROGRAM *p, int cind) {
	if(p == NULL) { return MCPP_ERR_NOPROG; }
	
	int ct = cind;
	
	if(p->varied) {
		int *cs = malloc(p->steps_size*sizeof(int));
	
		get_cstep(cind, cs, p->steps, p->steps_size);
		
		ct = get_transient_from_step(p, cs);
		
		free(cs);
	}
	
	return ct;
}

int get_transient_from_step(PPROGRAM *p, int *step) {
	// Get the current transient from a step variable.
	// This is the kind of variable you'd get from get_cstep
	// on the p->steps array.
	
	if(p == NULL) { return MCPP_ERR_NOPROG; }
	if(step == NULL) { return MCPP_ERR_NOARRAY; }
	
	int ct;

	if(!p->varied || p->tmode == MC_TMODE_TF) {
		ct = step[0];
	} else {	// This is also the starting point for MC_TMODE_PC
		ct = step[p->steps_size-1];
	}
	
	if(p->tmode == MC_TMODE_PC) {
		for(int i = 0; i < p->nCycles; i++) {
			ct *= step[i];
		}
	}
	
	return ct;
}

int get_dim_step(PPROGRAM *p, int cind, int *dim_step) {
	// Get dimension-only step information from the PPROGRAM
	
	if(p == NULL) { return MCPP_ERR_NOPROG; }
	
	int *cs = malloc(p->steps_size*sizeof(int));
	int *ds = dim_step;
	
	get_cstep(cind, cs, p->steps, p->steps_size);

	if(p->tmode == MC_TMODE_ID) {
		memcpy(ds, cs, p->nDims * sizeof(int));
	} else if (p->tmode == MC_TMODE_TF) {
		memcpy(ds, cs + 1, p->nDims * sizeof(int));
	} else if (p->tmode == MC_TMODE_PC) {
		memcpy(ds, cs + p->nCycles, p->nDims * sizeof(int));
	} else {
		free(cs);
		return MCPP_ERR_INVALIDTMODE;
	}
	
	free(cs);
	return 0;
}

int get_cyc_step(PPROGRAM *p, int cind, int *cyc_step) {
	// Get the the cycle-only step information from the PPROGRAM.
	
	if(p == NULL) { return MCPP_ERR_NOPROG; }
	if(p->nCycles <= 0) { return 1; }
	
	if(cyc_step == NULL) { return MCPP_ERR_NOARRAY; }
	
	int *cs = malloc(p->steps_size*sizeof(int));
	int *ds = cyc_step;
	
	get_cstep(cind, cs, p->steps, p->steps_size);
	int ct;
	
	if(p->tmode == MC_TMODE_ID) {
		ct = cs[p->steps_size - 1];
	} else if (p->tmode == MC_TMODE_TF) {
		ct = cs[0];
	} else if (p->tmode == MC_TMODE_PC) {
		cs[p->nCycles] = cs[p->steps_size-1];
		int *as = malloc((p->nCycles+1)*sizeof(int));
		memcpy(as, p->steps, (p->nCycles)*sizeof(int));
		
		as[p->steps_size] = p->nt;
		ct = get_lindex(cs, as, p->nCycles+1);
		free(as);
		
		memcpy(ds, cs, (p->nCycles+1)*sizeof(int));
	} else {
		free(cs);
		return MCPP_ERR_INVALIDTMODE;
	}
	
	free(cs);
	return 0;	
}

int get_var_ind(PPROGRAM *p, int cind) {
	// Get the linear index from the cstep index (excludes transients)
	
	int rv = -1;
	int *cs = malloc((p->nCycles+p->nDims)*sizeof(int));
	
	if(p->varied) {
		if(p->nCycles) {
			if(rv = get_cyc_step(p, cind, cs)) { goto error; }	
		}
		if(p->nDims) {
			if(rv = get_dim_step(p, cind, cs + p->nCycles)) { goto error; } 
		}
	
		rv = get_lindex(cs, p->maxsteps, p->nDims+p->nCycles);
	} else {
		rv = cind;	
	}
	
	error:
	
	free(cs);
	return rv;
}
									
int convert_lindex(PPROGRAM *p, int index, int old_mode, int new_mode) {
	// Convert the linear index as stored in the PPROGRAM to a
	// linear index from a new mode.
	if(p == NULL) { return MCPP_ERR_NOPROG; }

	int mode = p->tmode;
	int rv = 0;
	int *maxstep = NULL, *step = NULL;
	
	// Need this part for later indexing.
	int tdiv = 1, nt = p->nt;
	if(new_mode != old_mode && new_mode == MC_TMODE_PC || old_mode == MC_TMODE_PC) {
		if(p->nCycles) {
			for(int i = 0; i < p->nCycles; i++) {
				tdiv *= p->maxsteps[i]; 		
			}
			
			nt /= tdiv;
			if(nt < 1) {
				return MCPP_ERR_INVALIDTMODE;
			} else if(nt == 1 && new_mode == MC_TMODE_PC) {
				new_mode = MC_TMODE_TF;
			}
		} else {
			new_mode = MC_TMODE_ID;
		}
	}
	
	// Check if conversion needs to be done. No need
	// to convert the first index or if the modes are the
	// same. Additionally, all modes operate the same way
	// when there are no indirect dimensions.
	//
	// MC_TMODE_PC is equivalent to MC_TMODE_ID when there
	// are no phase cycles.
	if(index == 0 || old_mode == new_mode || p->nDims < 1) {
		return index;
	}
	
	p->tmode = old_mode;
	
	int step_size = p->nDims + ((new_mode == MC_TMODE_PC)?p->nCycles:1);
	maxstep = malloc(sizeof(unsigned int)*step_size);
	step = malloc(sizeof(unsigned int)*step_size);
	
	int *dim_step = NULL;
	int t = get_transient(p, index);
	if(t < 0) {
		rv = t;
		goto error;
	}
	
	switch(new_mode) {
		case MC_TMODE_TF:
			step[0] = t;
			dim_step = step+1;
			
			maxstep[0] = nt;
			memcpy(maxstep+1, p->maxsteps+p->nCycles, sizeof(unsigned int)*p->nDims);
			break;
		case MC_TMODE_ID:
			step[p->nDims] = t;
			dim_step = step;
			
			maxstep[p->nDims] = nt;
			memcpy(maxstep, p->maxsteps+p->nCycles, sizeof(unsigned int)*p->nDims);
			break;
		case MC_TMODE_PC:
			step[p->nCycles+p->nDims] = t;
			dim_step = step+p->nCycles;
			if(rv = get_cyc_step(p, index, step)) { goto error; }
			
			maxstep[p->nCycles+p->nDims] = nt;
			memcpy(maxstep, p->maxsteps, sizeof(unsigned int)*(p->nDims+p->nCycles));
			
			break;
		default:
			rv = MCPP_ERR_INVALIDTMODE;
			goto error;
	}
	
	if(rv = get_dim_step(p, index, dim_step)) { goto error; }
	
	rv = get_lindex(step, maxstep, step_size);
	
	error:
	if(step != NULL) { free(step); }
	if(maxstep != NULL) { free(maxstep); }
	
	p->tmode = mode;
	
	return rv;
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

int get_maxsteps_safe(int *maxsteps) {
	CmtGetLock(lock_uipc);
	int rv = get_maxsteps(maxsteps);
	CmtReleaseLock(lock_uipc);
	
	return rv;
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

int constant_array_pinstr(PINSTR **array, int size) {
	// Give this an array of PINSTRs and the size of the array
	// and it checks to see if the array is the same everywhere.
	
	for(int i=1; i<size; i++) {
		if(!pinstr_cmp(array[i], array[0]))	// If it's not the same as the first one
			return 0;						// it's not the same everywhere.
	}

	return 1;
}

//////////////////////////////////////////////////////////////
// 															//
//					Thread Safe Functions					//
// 															//
//////////////////////////////////////////////////////////////

/******************** Safe PulseBlaster Functions *********************/ 
int pb_initialize(int verbose) {
	int rv = pb_init();

	if(rv < 0 && verbose)
		MessagePopup("Error Initializing PulseBlaster", pb_get_error());
	
	pb_set_clock(100.0);	
	
	if(rv >= 0)
		SetInitialized(1);

	return rv;
}

int pb_init_safe (int verbose)
{
	CmtGetLock(lock_pb);
	int rv = pb_initialize(verbose);
	CmtReleaseLock(lock_pb);

	return rv;
}

int pb_start_programming_safe(int verbose, int device)
{
	CmtGetLock(lock_pb);
	int rv = pb_start_programming(device);
	CmtReleaseLock(lock_pb);
	
	if(rv != 0 && verbose)
		MessagePopup("Error Starting PulseBlaster Programming", pb_get_error());

	return rv;
}

int pb_stop_programming_safe(int verbose)
{
	int rv = pb_stop_programming();
	CmtReleaseLock(lock_pb);
	
	if(rv <0 && verbose)
		MessagePopup("Error", pb_get_error());

	return rv;
}

int pb_inst_pbonly_safe (int verbose, unsigned int flags, int inst, int inst_data, double length)
{
	CmtGetLock(lock_pb);
	int rv = pb_inst_pbonly(flags, inst, inst_data, length);
	CmtReleaseLock(lock_pb);
		
	if(rv <0 && verbose)
		MessagePopup("Error Programming PulseBlaster", pb_get_error());

	return rv;
}
	

int pb_close_safe (int verbose)
{
	CmtGetLock(lock_pb);
	int rv = pb_close();
	CmtReleaseLock(lock_pb);
	if(rv < 0 && verbose)
		MessagePopup("Error Closing PulseBlaster", pb_get_error());
	
	SetInitialized(0);
		
	return rv;
}

int pb_read_status_or_error(int verbose) {
	int rv = 0;
	if(!GetInitialized()) {
		rv = pb_initialize(verbose);
		if(rv < 0)
			goto error;
	}

	rv = pb_read_status();
	
	error:
	if((rv <= 0 || rv > 32) && verbose)
	{
		int i, length = (int)((log10(abs(rv))/log10(2))+1);
		char *status = malloc(length+1);
		
		for(i = 0; i<length; i++)
			if(rv & (int)pow(2, i))
				status[length-i-1] = '1';
			else
				status[length-i-1] = '0';
		status[length] = '\0';
		
		
		char *err = pb_get_error();		// No need to free, not malloced.
		char *err_str = malloc(strlen(err)+strlen(status)+strlen(", Status: \n")+2);
		
		sprintf(err_str, "%s, Status: %s\n", err, status);
		MessagePopup("Read Pulseblaster Status Error", err_str);
		
		free(err_str);
	}
	
	return rv;
}

int pb_read_status_safe(int verbose)
{
	CmtGetLock(lock_pb);
	int rv = pb_read_status_or_error(verbose);
	CmtReleaseLock(lock_pb);

	return rv;
}

int pb_start_safe(int verbose)
{
	int rv = 0;

	if(!GetInitialized()) {
		rv = pb_init_safe(verbose);
		if(rv < 0)  { return rv; }
	}
	
	CmtGetLock(lock_pb);
	rv = pb_start();
	CmtReleaseLock(lock_pb);

	if(rv < 0 && verbose)
		MessagePopup("PulseBlaster Start Error", pb_get_error());

	return rv;
}

int pb_stop_safe(int verbose)
{
	
	CmtGetLock(lock_pb);
	int rv = pb_stop();
	CmtReleaseLock(lock_pb);
	
	if(rv < 0 && verbose)
		MessagePopup("PulseBlaster Stop Error", pb_get_error());
	
	return rv;
}
