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

//////////////////////////////////////////////////////
//                                                  //
//              Pulse Program Functions             //
//                                                  //
//////////////////////////////////////////////////////

PPROGRAM *get_current_program() { // Pre-uipc
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

PPROGRAM *get_current_program()  // Older version
{
 	//Generates a program from the loaded program.
	PPROGRAM *program;
	
	int i, j, n_inst, f[24], stop=0, *TTLctrls = malloc(24*sizeof(int));
 	double time, time_units, totalTime = 0.0, Short = 1*ns;
	
	for (i = 0; i<24; i++)
		TTLctrls[i] = Load_TTLs(i);
	
	if(n_inst_disp <1)
		return NULL;
	
	GetCtrlVal(inst[n_inst_disp-1], PulseInstP_Instructions, &i);
	
	n_inst = n_inst_disp;
	

	program = malloc(sizeof(PPROGRAM));
	
	//1D Program Values
	program->flags = malloc(n_inst*sizeof(int));
	program->instr = malloc(n_inst*sizeof(int));
	program->instr_data = malloc(n_inst*sizeof(int));
	program->trigger_scan = malloc(n_inst*sizeof(int));
	program->instruction_time = malloc(n_inst*sizeof(double));
	program->time_units = malloc(n_inst*sizeof(double));

	//2D Program Values
	fix_number_of_dimensions(); 
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NDimensionalOn, &i);
	if(i)
		GetCtrlVal(Pulse_Prog_Config, PPConfig_NumDimensions, &program->nDimensions);
	else
		program->nDimensions = 1;

	int nd = program->nDimensions;
	
	if(--nd >= 1)
	{
		program->nSteps = malloc(sizeof(int)*nd);
		program->step = malloc(sizeof(int)*nd);
		
		int on, color, nv = get_num_varying_instr (); 		
		
		for(i = 0; i<nd; i++)
		{
			GetValueFromIndex(Pulse_Prog_Config, PPConfig_DimensionPoint, i, &program->step[i]);
			GetValueFromIndex(Pulse_Prog_Config, PPConfig_DimensionPoints, i, &program->nSteps[i]);
		}
		
		program->nVaried = nv;
		
		program->dimension = malloc(sizeof(int)*nv);
		program->v_instr_num = malloc(sizeof(int)*nv);
		program->v_instr_type = malloc(sizeof(int)*nv);
		
		program->init_id = malloc(sizeof(int)*nv);
		program->inc_id = malloc(sizeof(int)*nv);
		program->final_id = malloc(sizeof(int)*nv);
		
		program->init = malloc(sizeof(double)*nv);
		program->initunits = malloc(sizeof(double)*nv);
		program->inc = malloc(sizeof(double)*nv);
		program->incunits = malloc(sizeof(double)*nv);
		program->final = malloc(sizeof(double)*nv);
		program->finalunits = malloc(sizeof(double)*nv);
		
		
		j = 0;
		for(i = 0; i<n_inst_disp; i++)
		{
			GetCtrlVal(cinst[i], MDInstr_VaryInstr, &on);
			if(!on)
				continue;
			
			GetCtrlAttribute(cinst[i], MDInstr_VaryInstr, ATTR_ON_COLOR, &color);
			
			GetCtrlVal(cinst[i], MDInstr_InstrNum, &program->v_instr_num[j]);
			GetCtrlVal(cinst[i], MDInstr_Dimension, &program->dimension[j]);
			
				
			if(color == VAL_RED) {
				GetCtrlVal(cinst[i], MDInstr_InitDelay, &program->init[j]);
				GetCtrlVal(cinst[i], MDInstr_InitDelayUnits, &program->initunits[j]);
				GetCtrlVal(cinst[i], MDInstr_Increment, &program->inc[j]);
				GetCtrlVal(cinst[i], MDInstr_IncrementUnits, &program->incunits[j]);
				GetCtrlVal(cinst[i], MDInstr_FinalDelay, &program->final[j]);
				GetCtrlVal(cinst[i], MDInstr_FinalDelayUnits, &program->finalunits[j]);
				
				program->v_instr_type[j++] = 0;
				
			} else if (color == VAL_BLUE) {
				// If you want to increment the instruction data rather than the time.
				GetCtrlVal(cinst[i], MDInstr_IntInstrData, &program->init_id[j]);
				GetCtrlVal(cinst[i], MDInstr_IncInstrData, &program->inc_id[j]);
				GetCtrlVal(cinst[i], MDInstr_FInstrData, &program->final_id[j]);
				
				program->initunits[j] = -1.0;
				program->incunits[j] = -1.0;
				program->finalunits[j] = -1.0;
				
				program->v_instr_type[j++] = 1;
			}
			
		}
	}
	
	//Phase cycling data
	
	GetCtrlVal(Pulse_Prog_Config, PPConfig_PhaseCycle, &program->phasecycle);
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NumCycles, &program->numcycles);
	GetCtrlVal(Pulse_Prog_Config, PPConfig_PhaseCycleInstr, &program->phasecycleinstr);
	GetCtrlVal(Pulse_Prog_Config, PPConfig_PhaseCycleFreq, &program->cyclefreq);
	
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NTransients, &program->ntransients);

	program->transient = 1;
	program->scan = 0;
	program->n_inst = n_inst;
	program->trigger_ttl = ttl_trigger;
	
	
	for(i = 0; i < n_inst_disp; i++)
	{
		for(j = 0; j<24; j++)
			GetCtrlVal(inst[i], TTLctrls[j], &f[j]);
		
		program->flags[i] = 0;
		for(j = 0; j<24; j++)
			program->flags[i] += f[j]*(int)pow(2.0, j);
		
		GetCtrlVal(inst[i], PulseInstP_InstDelay, &time);
		GetCtrlVal(inst[i], PulseInstP_TimeUnits, &time_units);
		GetCtrlVal(inst[i], PulseInstP_Instructions, &program->instr[i]);
		GetCtrlVal(inst[i], PulseInstP_Instr_Data, &program->instr_data[i]);
		GetCtrlVal(inst[i], PulseInstP_Scan, &program->trigger_scan[i]);
		
		time *= time_units;
		
		program->instruction_time[i] = time;
		program->time_units[i] = time_units;
		
		if(program->instr[i] == LONG_DELAY)
			totalTime += time*program->instr_data[i];
		else
			totalTime += time;
		
		if(program->trigger_scan[i])
			program->scan=1;
	}
	
	
	program->total_time = totalTime;
	
	GetCtrlVal(Pulse_Prog_Config, PPConfig_SampleRate, &program->samplingrate);
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NPoints, &program->nPoints);
	GetCtrlVal(Pulse_Prog_Config, PPConfig_DelayTime, &program->delaytime);
	
	return program;
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

PPROGRAM *LoadProgram_To_Program (char *loadfile)
{
	/*
	PPROGRAM *p = malloc(sizeof(PPROGRAM));
	
	int i = 0, j=0, number_of_inst;
	int trigger_scan, flags, instr, instr_data, ntransients, control[3];
	char *string_buffer = malloc(500), *string_match = malloc(500);
	float time, time_units, delaytime;
	FILE *fileid;

	if(file_exists(loadfile) < 1)
		return NULL;
	
	fileid = fopen(loadfile, "r+");
	if(fileid == NULL)
		return NULL;
	
	//Determine if the file is actually a pulse program.
	fgets(string_buffer, 500, fileid);
	if (!sscanf(string_buffer, "%s", string_match) || strstr(string_match, "#ValidPulseProgram#") == NULL)
	{	
		MessagePopup("Error", "Not a valid pulse program!");
		fclose(fileid);
		return NULL;
	}
	
	//Get number of instructions, number of dimensions, number of varying programs.
	
	while(!feof(fileid))
	{
		fgets(string_buffer, 500, fileid);
		if (sscanf(string_buffer, "%s %d", string_match, &p->n_inst) == 2 && strstr(string_match, "NInstructions=") != NULL)
			break;
	}
	
	if(feof(fileid))
	{
		fclose(fileid);
		return NULL;
	}
	
	rewind(fileid);
	
	while(!feof(fileid))
	{
		fgets(string_buffer, 500, fileid);
		if (sscanf(string_buffer, "%s %d", string_match, &p->nDimensions) == 2 && strstr(string_match, "Dimensions=") != NULL)
			break;
	}
	
	if(feof(fileid))
	{
		fclose(fileid);
		return NULL;
	}
	
	rewind(fileid);
	
	if(p->n_inst < 1 || p->nDimensions < 1)
	{
		fclose(fileid);
		return NULL;
	}
	
	//Allocate memory for the 1D instructions.
	int si = sizeof(int)*p->n_inst, sd = sizeof(double)*p->n_inst;
	
	p->flags = malloc(si);
	p->instr = malloc(si);
	p->instr_data = malloc(si);
	p->trigger_scan = malloc(si);
	
	p->instruction_time = malloc(sd);
	p->time_units = malloc(sd);
	
	p->scan = 0;
	
	if(p->nDimensions > 1)
	{
		while(!feof(fileid))
		{
			fgets(string_buffer, 500, fileid);
			if (sscanf(string_buffer, "%s %d", string_match, &p->nVaried) == 2 && strstr(string_match, "nVaried=") != NULL)
				break;
		}
	
		if(feof(fileid))
		{
			fclose(fileid);
			return NULL;
		}
	
		rewind(fileid);
		
		//Allocate memory for the 2D instructions.
		si = sizeof(int)*(p->nDimensions - 1);
		int sid = sizeof(int)*p->nVaried;
		sd = sizeof(double)*p->nVaried;
		
		p->v_instr_num = malloc(sizeof(int)*p->nVaried);
		p->v_instr_type = malloc(sizeof(int)*p->nVaried);
		p->dimension = malloc(sizeof(int)*p->nVaried);
		
		p->init = malloc(sd);
		p->initunits = malloc(sd);
		p->inc = malloc(sd);
		p->incunits = malloc(sd);
		p->final = malloc(sd);
		p->finalunits = malloc(sd);
		
		p->init_id = malloc(sid);
		p->inc_id = malloc(sid);
		p->final_id = malloc(sid);
		
		p->nSteps = malloc(si);
		p->step = malloc(si);
	}

	while(!feof(fileid))
	{
		fgets(string_buffer, 500, fileid);
		if(program_breakout(string_buffer, p) < 0)
		{
			fclose(fileid);
			return NULL;
		}
	}
	
	fclose(fileid);
	
	return p;
	*/
	
	return NULL;
}

int LoadProgram_To_Controls (PPROGRAM *p)
{
	
	int i, j, flags[24];
	/*//Start with a new program.
	NewProgram(NULL, NULL, EVENT_COMMIT, NULL, NULL, NULL);
	
	//Basic 1D Stuff
	SetCtrlVal(Pulse_Prog_Tab, PulseProg_InstNum, p->n_inst);
	change_number_of_instructions();
	SetCtrlVal(Pulse_Prog_Config, PPConfig_NTransients, p->ntransients);
	SetCtrlVal(Pulse_Prog_Config, PPConfig_NPoints, p->nPoints);
	SetCtrlVal(Pulse_Prog_Config, PPConfig_SampleRate, p->samplingrate);
	change_np_or_sr(0);
	
	
	//Phase Cycling
	SetCtrlVal(Pulse_Prog_Config, PPConfig_PhaseCycle, p->phasecycle);
	SetCtrlVal(Pulse_Prog_Config, PPConfig_NumCycles, p->numcycles);
	SetCtrlVal(Pulse_Prog_Config, PPConfig_PhaseCycleInstr, p->phasecycleinstr);
	SetCtrlVal(Pulse_Prog_Config, PPConfig_PhaseCycleFreq, p->cyclefreq);
	PhaseCycleCallback(Pulse_Prog_Config, PPConfig_PhaseCycle, EVENT_COMMIT, NULL, NULL, NULL);
	
	//Instructions
	for(i = 0; i<p->n_inst; i++)
	{
		for(j = 0; j<24; j++)
			SetCtrlVal(inst[i], Load_TTLs(j), (p->flags[i] & (int)pow(2, j)));
		
		SetCtrlVal(inst[i], PulseInstP_Instructions, p->instr[i]);
		InstrCallback(inst[i], PulseInstP_Instructions, EVENT_COMMIT, NULL, NULL, NULL);
		
		SetCtrlVal(inst[i], PulseInstP_Instr_Data, p->instr_data[i]);
		
		SetCtrlVal(inst[i], PulseInstP_TimeUnits, p->time_units[i]);
		ChangeTUnits(inst[i], PulseInstP_TimeUnits, EVENT_COMMIT, NULL, NULL, NULL);
		
		SetCtrlVal(inst[i], PulseInstP_InstDelay, p->instruction_time[i]/p->time_units[i]);
		ChangeInstDelay(inst[i], PulseInstP_InstDelay, EVENT_COMMIT, NULL, NULL, NULL);
		
		SetCtrlVal(inst[i], PulseInstP_Scan, p->trigger_scan[i]);
		
		if(ttl_trigger != p->trigger_ttl)
			move_ttl(inst[i], ttl_trigger, p->trigger_ttl);
			
		SetCtrlVal(inst[i], Load_TTLs(ttl_trigger), p->trigger_scan[i]);
	}
	
	//Setup multi-dimensional stuff
	if(p->nDimensions > 1)
	{
		char c[3];
		int pan;
		SetCtrlVal(Pulse_Prog_Config, PPConfig_NDimensionalOn, 1);
		ToggleND(Pulse_Prog_Config, PPConfig_NDimensionalOn, EVENT_COMMIT, NULL, NULL, NULL);

		SetCtrlVal(Pulse_Prog_Config, PPConfig_NumDimensions, p->nDimensions);
		NumDimensionCallback(Pulse_Prog_Config, PPConfig_NumDimensions, EVENT_COMMIT, NULL, NULL, NULL);
		
		populate_dim_points();
		
		for(i = 1; i<p->nDimensions; i++)
		{
			sprintf(c, "%d", i);
			ReplaceListItem(Pulse_Prog_Config, PPConfig_DimensionPoint, i-1, c, p->step[i-1]);
			ReplaceListItem(Pulse_Prog_Config, PPConfig_DimensionPoints, i-1, c, p->nSteps[i-1]);
		}
		
		for(i = 0; i<p->nVaried; i++)
		{
			pan = cinst[p->v_instr_num[i]];
			
			SetCtrlVal(pan, MDInstr_VaryInstr, 1);

			SetCtrlVal(pan, MDInstr_Dimension, p->dimension[i]);
			SetCtrlVal(pan, MDInstr_NumSteps, p->nSteps[p->dimension[i] - 1]);

			/*
			if(!p->v_instr_type[i]) {
				SetCtrlVal(pan, MDInstr_InitDelay, p->init[i]/p->initunits[i]);   
				SetCtrlVal(pan, MDInstr_InitDelayUnits, p->initunits[i]);
			
				SetCtrlVal(pan, MDInstr_Increment, p->inc[i]/p->incunits[i]);
				SetCtrlVal(pan, MDInstr_IncrementUnits, p->incunits[i]);
			
				SetCtrlVal(pan, MDInstr_FinalDelay, p->final[i]/p->finalunits[i]);
				SetCtrlVal(pan, MDInstr_FinalDelayUnits, p->finalunits[i]);
				
				SetCtrlAttribute(pan, MDInstr_VaryInstr, ATTR_ON_COLOR, VAL_RED);
				
				ChangeITUnits(pan, MDInstr_InitDelayUnits, EVENT_COMMIT, NULL, NULL, NULL);
				ChangeFDelay(pan, MDInstr_FinalDelay, EVENT_COMMIT, NULL, NULL, NULL);
			} else {
				SetCtrlVal(pan, MDInstr_IntInstrData, p->init_id[i]);
				SetCtrlVal(pan, MDInstr_IncInstrData, p->inc_id[i]);
				SetCtrlVal(pan, MDInstr_FInstrData, p->final_id[i]);
				
				update_instr_data_nd(pan, 2);
				
				SetCtrlAttribute(pan, MDInstr_VaryInstr, ATTR_ON_COLOR, VAL_BLUE);
			} 
			
			update_nd_state(pan, MDInstr_VaryInstr, -1);   
			
			ChangeDimension(pan, MDInstr_Dimension, EVENT_COMMIT, NULL, NULL, NULL);
			
		}
	}  */
	return 0;

}

int LoadProgram_IO (char *loadfile)
{
	//Loads a program from *loadfile into the pulse program panel.
	
	PPROGRAM *p = malloc(sizeof(PPROGRAM));
	
	p = LoadProgram_To_Program(loadfile);
	
	if(p == NULL)
		return 0;
	
	LoadProgram_To_Controls(p);
	
	return 0;
}


int SaveProgram_PPROGRAM(PPROGRAM *program, char *savefile)
{
	//Saves a PPROGRAM to a file.
	int i, j;
	FILE *fileid;
	
	fileid = fopen(savefile, "w+");
	
	fprintf(fileid, "#ValidPulseProgram#\n");
	fprintf(fileid, "NInstructions= %d\n", n_inst_disp);


	double delay_time = program->delaytime;
	fprintf(fileid, "NTransients= %d\n", program->ntransients);
	fprintf(fileid, "DelayTime= %lf\n", delay_time);
	fprintf(fileid, "TriggerTTL= %d\n\n", program->trigger_ttl);
	fprintf(fileid, "NPoints= %d\n", program->nPoints);
	fprintf(fileid, "SamplingRate= %lf\n\n", program->samplingrate);
	fprintf(fileid, "PhaseCycle= %d\n", program->phasecycle);
	fprintf(fileid, "NumCycles= %d\n", program->numcycles);
	fprintf(fileid, "CycleInstr= %d\n", program->phasecycleinstr);
	fprintf(fileid, "CycleFreq= %lf\n\n", program->cyclefreq);
	fprintf(fileid, "Dimensions= %d \n", program->nDimensions);
	if(program->nDimensions > 1)
	{
		fprintf(fileid, "[Point]\n");
		for(i = 0; i < program->nDimensions-1; i++)
			fprintf(fileid, "IndirectDim %d - %d of %d\n", i+1, program->step[i], program->nSteps[i]);
	}
	
	fprintf(fileid, "\n[Instructions]\n");
	for(i=0; i<program->n_inst; i++)
		fprintf(fileid, "Instruction %d %d %d %d %d %lf %lf\n", i, program->trigger_scan[i], program->flags[i], program->instr[i], program->instr_data[i], program->instruction_time[i]/program->time_units[i], program->time_units[i]);
	fprintf(fileid, "[EndInstructions]\n\n");
	
	if(program->nDimensions > 1)
	{
		fprintf(fileid, "\nnVaried= %d\n\n", program->nVaried);
		for(i = 0; i< program->nVaried; i++)
			if(!program->v_instr_type)
				fprintf(fileid, "VaryInstr %d %d %lf %lf %lf %lf %lf %lf %d\n", i, program->v_instr_num[i], program->init[i], program->initunits[i], program->inc[i], program->incunits[i], program->final[i], program->finalunits[i], program->dimension[i]); 
			else
				fprintf(fileid, "VaryInstr %d %d %d %lf %d %lf %d %lf %d\n", i, program->v_instr_num[i], program->init_id[i], program->initunits[i], program->inc_id[i], program->incunits[i], program->final_id[i], program->finalunits[i], program->dimension[i]);
	}
	
	fclose(fileid);
	return 0;
}	

int SaveProgram_IO(char *savefile) {
	//Saves the currently loaded program to file *savefile
	
	PPROGRAM *p = malloc(sizeof(PPROGRAM));
	p = get_current_program();
	SaveProgram_PPROGRAM(p, savefile);
	
	return 0;
}

//Auxiliary Pulse Programming Functions
int clear_instruction(int num)
{
	int i, nl;
	for(i = 0; i<24; i++)
		SetCtrlVal(inst[num], Load_TTLs(i), 0);
	SetCtrlVal(inst[num], PulseInstP_Instructions, 0);
	SetCtrlVal(inst[num], PulseInstP_Instr_Data, 0);
	SetCtrlVal(inst[num], PulseInstP_Scan, 0);
	SetCtrlIndex(inst[num], PulseInstP_TimeUnits, 2);
	SetCtrlVal(inst[num], PulseInstP_InstDelay, 1.0);
	Change_Scan(inst[num], PulseInstP_Scan, EVENT_COMMIT, NULL, NULL, NULL);
	ChangeTUnits(inst[num], PulseInstP_TimeUnits, EVENT_COMMIT, NULL, NULL, NULL);
	InstrCallback(inst[num], PulseInstP_Instructions, EVENT_COMMIT, NULL, NULL, NULL);
	InstrDataCallback(inst[num], PulseInstP_Instr_Data, EVENT_COMMIT, NULL, NULL, NULL);
	
	GetNumListItems(cinst[num], MDInstr_Dimension, &nl);
	if(nl > 0)
		DeleteListItem(cinst[num], MDInstr_Dimension, 0, -1);
	SetCtrlVal(cinst[num], MDInstr_VaryInstr, 0);
	update_nd_state(cinst[num], MDInstr_VaryInstr, -1);  
	
	return 0;
	
}
	
	
int change_number_of_instructions()
{
	int number, i, t, l;

	/*GetCtrlVal(Pulse_Prog_Tab, PulseProg_InstNum, &number);
	GetPanelAttribute(inst[0], ATTR_LEFT, &l);

	if (number > n_inst_disp)
	{
		if (number > ninstructions)
		{
		for (i=ninstructions; i<number; i++)
		{
			GetPanelAttribute(inst[i-1], ATTR_TOP, &t);
			inst[i] = DuplicatePanel(Pulse_Prog_Panel, inst[0], "", t+30, l);
			SetCtrlVal(inst[i], PulseInstP_InstNum, i);
			DisplayPanel(inst[i]);
			
			GetPanelAttribute(cinst[0], ATTR_LEFT, &l);
			GetPanelAttribute(cinst[i-1], ATTR_TOP, &t);
			cinst[i] = DuplicatePanel(PPConfigSubPanel, cinst[0], "", t+31, l);
			SetCtrlVal(cinst[i], MDInstr_InstrNum, i);
			DisplayPanel(cinst[i]);
			
			clear_instruction(i); 
		}
		ninstructions = number;
		}
		for (i = n_inst_disp-1; i<number; i++)
		{
			SetPanelAttribute(inst[i], ATTR_VISIBLE, 1);
			SetPanelAttribute(cinst[i], ATTR_VISIBLE, 1);
		}
		n_inst_disp = number;
	} else if (number < n_inst_disp){
		for (i = number; i<ninstructions; i++)
		{
			SetPanelAttribute(inst[i], ATTR_VISIBLE, 0);
			SetPanelAttribute(cinst[i], ATTR_VISIBLE, 0);
		}
		n_inst_disp = number;
	}
	*/
	return 0;
}

int program_breakout(char *line, PPROGRAM *p)
{
	//Give this function a label, and if it matches one in the set, it returns a pointer to a place in memory.
	//int *rv is set to determine if a successful match was made, and if so, what kind.
	//If rv is set to a negative number, there was an error.
	//rv = 0 => No match
	//rv = 1 => Successful match on indirect dimension point.
	//rv = 2 => Successful match on instruction.
	//rv = 3 => Successful match on varying instruction.
	//rv = 4->15 => Successful match on i-4th component.
	//rv = -1 => Bad input.
	//rv = -2 => Integer not found.
	//rv = -3 => Double not found.
	//rv = -4 => Indirect dimension point not found.
	//rv = -5 => Instruction not found.
	//rv = -6 => Varying instruction not found.
	
	
	/*
	if(p == NULL || line == NULL)
		return -1;
	
	char *string_match = malloc(500);
	if(sscanf(line, "%s", string_match) < 1)
		return 0;
	
	char *labels[15] = {"NInstructions=", "NTransients=", "DelayTime=", "TriggerTTL=", "NPoints=", "SamplingRate=", "PhaseCycle=", "NumCycles=", "CycleInstr=", "CycleFreq=", "Dimensions=", "nVaried=", "IndirectDim", "Instruction" , "VaryInstr"};
	int i;
	
	for(i = 0; i<15; i++)
	{
		if(strstr(string_match, labels[i]) != NULL && string_match[0] != '[')
			break;
		if (i == 14)
			i += 2;
	}
	
	if(i > 15)
		return 0;
	
	int takesint[9] = {0, 1, 3, 4, 6, 7, 8, 10, 11}, takesdouble[3] = {2, 5, 9};
	int ival = 0;
	double dval = 0.0;
	
	if(isin(i, takesint, 9) >= 0)
	{
		if(sscanf(line, "%*s %d", &ival) < 1)
			return -2;
	} else if(isin(i, takesdouble, 3) >= 0) {
		if(sscanf(line, "%*s %lf", &dval) < 1)
			return -3;
	} else {
		if(i == 12)
		{
			int dim, step, nsteps;
			if(sscanf(line, "%*s %d %*s %d %*s %d", &dim, &step, &nsteps) < 3)
				return -4;
			p->step[dim-1] = step;
			p->nSteps[dim-1] = nsteps;
			
			return 1;
		} else if(i == 13) {
			int ln, instruction, instr_data, flags, trigger_scan;
			double instr_time, time_units;
			if(sscanf(line, "%*s %d %d %d %d %d %lf %lf", &ln, &trigger_scan, &flags, &instruction, &instr_data, &instr_time, &time_units) < 7)
				return -5;
			
			p->trigger_scan[ln] = trigger_scan;
			p->flags[ln] = flags;
			p->instr[ln] = instruction;
			p->instr_data[ln] = instr_data;
			p->instruction_time[ln] = instr_time*time_units;
			p->time_units[ln] = time_units;
			
			if(trigger_scan)
				p->scan = 2;
			
			return 3;
		} else if (i == 14) {
			int ln, dim, vi;
			double init, initu, inc, incu, final, finalu;
			if(sscanf(line, "%*s %d %d %lf %lf %lf %lf %lf %lf %d", &ln, &vi, &init, &initu, &inc, &incu, &final, &finalu, &dim) < 9)
				return -6;
			
			p->v_instr_num[ln] = vi;
			p->dimension[ln] = dim;   
			
			if(initu <= 0.0) {
				// The units are set to -1 if this is a type 1 2D instruction
				p->init_id[ln] = init;
				p->inc_id[ln] = inc;
				p->final_id[ln] = final;
				
				p->initunits[ln] = -1.0;
				p->incunits[ln] = -1.0;
				p->initunits[ln] = -1.0;
				
				p->v_instr_type[ln] = 1;
			} else {
				p->init[ln] = init*initu;
				p->initunits[ln] = initu;
				p->inc[ln] = inc*incu;
				p->incunits[ln] = incu;
				p->final[ln] = final*finalu;
				p->finalunits[ln] = finalu;
				
				p->v_instr_type[ln] = 0;
			}
	
			
			return 3;
		}
	}
	
	if(i == 0)
		p->n_inst = ival;
	else if (i == 1)
		p->ntransients = ival;
	else if (i == 2)
		p->delaytime = dval;
	else if (i == 3)
		p->trigger_ttl = ival;
	else if (i == 4)
		p->nPoints = ival;
	else if (i == 5)
		p->samplingrate = dval;
	else if (i == 6)
		p->phasecycle = ival;
	else if (i == 7)
		p->numcycles = ival;
	else if (i == 8)
		p->phasecycleinstr = ival;
	else if (i == 9)
		p->cyclefreq = dval;
	else if (i == 10)
		p->nDimensions = ival;
	else if (i == 11)
		p->nVaried = ival;
	
	return 4+i;
	
	*/
	
	return 0;
}

int get_nd_state(int panel, int control) {
	// If you don't know the state of the ND instruction, this function gets it for you.
	// State 0 = Off
	// State 1 = On/Red
	// State 2 = Off/Red
	
	int val, state;
	GetCtrlVal(panel, control, &val);

	if(!val) {
		state = 0;
	} else { 
		int color;
		GetCtrlAttribute(panel, control, ATTR_ON_COLOR, &color);  
		if (color == VAL_RED) {
			state = 1;
		} else if (color == VAL_BLUE) {
			state = 2;
		} else {
			state = 0;
		}
	}

	return state;
} // Copied


//////////////////////////////////////////////////////
//                                                  //
//              UI Functions			            //
//                                                  //
//////////////////////////////////////////////////////

int clear_instruction(int num)
{
	int i, nl;
	for(i = 0; i<24; i++)
		SetCtrlVal(inst[num], Load_TTLs(i), 0);
	SetCtrlVal(inst[num], PulseInstP_Instructions, 0);
	SetCtrlVal(inst[num], PulseInstP_Instr_Data, 0);
	SetCtrlVal(inst[num], PulseInstP_Scan, 0);
	SetCtrlIndex(inst[num], PulseInstP_TimeUnits, 2);
	SetCtrlVal(inst[num], PulseInstP_InstDelay, 1.0);
	Change_Scan(inst[num], PulseInstP_Scan, EVENT_COMMIT, NULL, NULL, NULL);
	ChangeTUnits(inst[num], PulseInstP_TimeUnits, EVENT_COMMIT, NULL, NULL, NULL);
	InstrCallback(inst[num], PulseInstP_Instructions, EVENT_COMMIT, NULL, NULL, NULL);
	InstrDataCallback(inst[num], PulseInstP_Instr_Data, EVENT_COMMIT, NULL, NULL, NULL);
	
	GetNumListItems(cinst[num], MDInstr_Dimension, &nl);
	if(nl > 0)
		DeleteListItem(cinst[num], MDInstr_Dimension, 0, -1);
	SetCtrlVal(cinst[num], MDInstr_VaryInstr, 0);
	update_nd_state(cinst[num], MDInstr_VaryInstr, -1);  
	
	return 0;
	
}

int move_ttl(int panel, int to, int from)
{
	int bufferval, val, i;
	
	GetCtrlVal(panel, Load_TTLs(from), &val);
	
	if(to == from)
		return 0;
	if(to > from)
	{
		for(i = from; i < to; i++)
		{
			GetCtrlVal(panel, Load_TTLs(i+1), &bufferval);
			SetCtrlVal(panel, Load_TTLs(i), bufferval);
		}
		SetCtrlVal(panel, Load_TTLs(i), val);
	} else {
		for(i = from; i > to; i--)
		{
			GetCtrlVal(panel, Load_TTLs(i-1), &bufferval);
			SetCtrlVal(panel, Load_TTLs(i), bufferval);
		}
		SetCtrlVal(panel, Load_TTLs(i), val);
	}
	
	return 0;
}

int move_instruction(int to, int from)
{
	int diff = (int)fabs(to-from) + 1, inst_buffer[diff], inst_top[diff], cinst_buffer[diff], cinst_top[diff], i, start;
	
	if(to == from)
		return 0;
	
	int s_s, s_e;
	
	if(to<from)
	{
		inst_buffer[0] = inst[from];
		cinst_buffer[0] = cinst[from];
		start = to;
		
		for (i = 1; i<diff; i++)
		{
			inst_buffer[i] = inst[to+i-1];
			cinst_buffer[i] = cinst[to+i-1];
		}
	}
	else
	{
		inst_buffer[diff-1] = inst[from];
		cinst_buffer[diff-1] = cinst[from];
		start = from;

		for (i = 0; i<diff-1; i++)
		{
			inst_buffer[i] = inst[from+i+1];
			cinst_buffer[i] = cinst[from+i+1];
		}
	}
	
	// Check if it's in a loop before and after, and if there's a change, then you shoul
	int loop_locations[ninstructions][2]; // An array with all the loop locations and their corresponding end-points.
	int j = 0, ins, end_ins;
	for(i = 0; i<ninstructions; i++) {
		GetCtrlVal(inst[i], PulseInstP_Instructions, &ins);
		if(ins == LOOP) {
			end_ins = find_end_loop(i);
			if(end_ins >= 0) {
				loop_locations[j][0] = i; // Which instruction is the loop.
				loop_locations[j++][1] = end_ins; // Which instruction is the end of the loop.
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
			SetCtrlVal(inst[c_e], PulseInstP_Instr_Data, to);
		} else if (c_l > to) {
			SetCtrlVal(inst[c_e], PulseInstP_Instr_Data, c_l+1);
		} else if (c_l > from) {
			SetCtrlVal(inst[c_e], PulseInstP_Instr_Data, c_l-1);
		}

	}
		
		
	
	// Now the actual moving and updating of things.
	for (i = 0; i<diff; i++)
	{
		GetPanelAttribute(inst[start+i], ATTR_TOP, &inst_top[i]);
		GetPanelAttribute(cinst[start+i], ATTR_TOP, &cinst_top[i]);
	}
	
	for(i = 0; i<diff; i++)
	{ 
		inst[start+i] = inst_buffer[i];
		cinst[start+i] = cinst_buffer[i];
		SetPanelAttribute(inst[start+i], ATTR_TOP, inst_top[i]);
		SetPanelAttribute(cinst[start+i], ATTR_TOP, cinst_top[i]);
		SetCtrlVal(inst[start+i], PulseInstP_InstNum, start+i);
		SetCtrlVal(cinst[start+i], MDInstr_InstrNum, start+i);
	}

	// Now that it's moved, we want to recalculate if necessary.
	for(i = 0; i<j; i++) {
		// Convenience variables
		c_l = loop_locations[i][0];
		c_e = loop_locations[i][1];
		
		// If we came from inside a loop or moved to outside of a loop, adjust accordingly.
		if((c_l < to && c_e > to) || (c_l < from && c_e > from)) {
			int state = get_nd_state(cinst[c_l], MDInstr_VaryInstr);
			if(state == 2) {
				update_instr_data_nd(cinst[c_l], 2); // This will update the times.
			}
		}
	}
	
	return 0; 
}

int change_number_of_instructions()
{
	int number, i, t, l;

	GetCtrlVal(Pulse_Prog_Tab, PulseProg_InstNum, &number);
	GetPanelAttribute(inst[0], ATTR_LEFT, &l);

	if (number > n_inst_disp)
	{
		if (number > ninstructions)
		{
			for (i=ninstructions; i<number; i++)
			{
				GetPanelAttribute(inst[i-1], ATTR_TOP, &t);
				inst[i] = DuplicatePanel(Pulse_Prog_Panel, inst[0], "", t+30, l);
				SetCtrlVal(inst[i], PulseInstP_InstNum, i);
				DisplayPanel(inst[i]);
			
				GetPanelAttribute(cinst[0], ATTR_LEFT, &l);
				GetPanelAttribute(cinst[i-1], ATTR_TOP, &t);
				cinst[i] = DuplicatePanel(PPConfigSubPanel, cinst[0], "", t+31, l);
				SetCtrlVal(cinst[i], MDInstr_InstrNum, i);
				DisplayPanel(cinst[i]);
			
				clear_instruction(i); 
			}
		ninstructions = number;
		}
		for (i = n_inst_disp-1; i<number; i++)
		{
			SetPanelAttribute(inst[i], ATTR_VISIBLE, 1);
			SetPanelAttribute(cinst[i], ATTR_VISIBLE, 1);
		}
		n_inst_disp = number;
	} else if (number < n_inst_disp){
		for (i = number; i<ninstructions; i++)
		{
			SetPanelAttribute(inst[i], ATTR_VISIBLE, 0);
			SetPanelAttribute(cinst[i], ATTR_VISIBLE, 0);
		}
		n_inst_disp = number;
	}
	return 0;
}



int update_nd_state(int panel, int control, int state)
{
	// Changes the state of a given ND control.
	// State 0 is off.
	// State 1 is on/red -> Time incrementing mode
	// State 2 is on/blue -> Instruction data incrementing mode
	// State == -1 -> Figure out what state we're in for us.
	
	int instr;
	GetCtrlVal(panel, MDInstr_InstrNum, &instr);
	
	if(state < 0) {
		state = get_nd_state(panel, control);
	}
	
	if(state == 1 || state == 2)  {
		// Un-dim the number of steps and dimension selector.
		SetCtrlAttribute(panel, MDInstr_NumSteps, ATTR_DIMMED, 0);
		SetCtrlAttribute(panel, MDInstr_Dimension, ATTR_DIMMED, 0);
		
		// In either on state, we need to dim the main controls
		change_dimension(panel);
		ChangeNumSteps(panel, MDInstr_NumSteps, EVENT_COMMIT, NULL, NULL, NULL);
	}
	
 	if(state == 0) {
		// In the 0 state, we're off, so that should be dimmed time increment stuff

		// First dim the controls for the time increment
		SetCtrlAttribute(panel, MDInstr_Dimension, ATTR_DIMMED, 1); 
		SetCtrlAttribute(panel, MDInstr_FinalDelay, ATTR_DIMMED, 1);
		SetCtrlAttribute(panel, MDInstr_FinalDelayUnits, ATTR_DIMMED, 1);
		SetCtrlAttribute(panel, MDInstr_Increment, ATTR_DIMMED, 1);
		SetCtrlAttribute(panel, MDInstr_IncrementUnits, ATTR_DIMMED, 1);
		SetCtrlAttribute(panel, MDInstr_InitDelay, ATTR_DIMMED, 1);
		SetCtrlAttribute(panel, MDInstr_InitDelayUnits, ATTR_DIMMED, 1);
		SetCtrlAttribute(panel, MDInstr_NumSteps, ATTR_DIMMED, 1);
	
		// Un-dim the main instructions.
		SetCtrlAttribute(inst[instr], PulseInstP_Instr_Data, ATTR_DIMMED, 0);
		SetCtrlAttribute(inst[instr], PulseInstP_InstDelay, ATTR_DIMMED, 0);
		SetCtrlAttribute(inst[instr], PulseInstP_TimeUnits, ATTR_DIMMED, 0);
		SetCtrlAttribute(inst[instr], PulseInstP_InstDelay, ATTR_MIN_VALUE, 0.000001); // Set to a non-zero value and let it choose appropriately.
		ChangeTUnits(inst[instr], PulseInstP_InstDelay, EVENT_COMMIT, NULL, NULL, NULL); // Let it figure out the right minimum time.

		// Then hide the instruction increment stuff
		SetCtrlAttribute(panel, MDInstr_IntInstrData, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_IncInstrData, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_FInstrData, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_InitTimeData, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_IncTimeData, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_FTimeData, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_InitTimeDataUnits, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_IncTimeDataUnits, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_FTimeDataUnits, ATTR_VISIBLE, 0);
	
		// Now show the dimmed controls.
		SetCtrlAttribute(panel, MDInstr_FinalDelay, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_FinalDelayUnits, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_Increment, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_IncrementUnits, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_InitDelay, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_InitDelayUnits, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_Instr_Data, ATTR_VISIBLE, 1);  

		SetCtrlAttribute(panel, control, ATTR_ON_COLOR, VAL_RED); // Change color to red.
	} else if(state == 1) {
		// In the 1 state, we want un-dimmed time increment stuff.
	
		// First un-dim the controls for the time increment
		SetCtrlAttribute(panel, MDInstr_FinalDelay, ATTR_DIMMED, 0);
		SetCtrlAttribute(panel, MDInstr_FinalDelayUnits, ATTR_DIMMED, 0);
		SetCtrlAttribute(panel, MDInstr_Increment, ATTR_DIMMED, 0);
		SetCtrlAttribute(panel, MDInstr_IncrementUnits, ATTR_DIMMED, 0);
		SetCtrlAttribute(panel, MDInstr_InitDelay, ATTR_DIMMED, 0);
		SetCtrlAttribute(panel, MDInstr_InitDelayUnits, ATTR_DIMMED, 0);

	
		// Then hide the instruction increment stuff
		SetCtrlAttribute(panel, MDInstr_IntInstrData, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_IncInstrData, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_FInstrData, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_InitTimeData, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_IncTimeData, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_FTimeData, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_InitTimeDataUnits, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_IncTimeDataUnits, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_FTimeDataUnits, ATTR_VISIBLE, 0);
	
		// Now show the un-dimmed controls.
		SetCtrlAttribute(panel, MDInstr_FinalDelay, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_FinalDelayUnits, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_Increment, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_IncrementUnits, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_InitDelay, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_InitDelayUnits, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_Instr_Data, ATTR_VISIBLE, 1);
		
		// Dim some of the main panel stuff, undim other parts
		SetCtrlAttribute(inst[instr], PulseInstP_Instr_Data, ATTR_DIMMED, 0);
		SetCtrlAttribute(inst[instr], PulseInstP_InstDelay, ATTR_DIMMED, 1);
		SetCtrlAttribute(inst[instr], PulseInstP_TimeUnits, ATTR_DIMMED, 1);
		
		// Allow instructions to be 0.
		SetCtrlAttribute(inst[instr], PulseInstP_InstDelay, ATTR_MIN_VALUE, 0.0); // Allow instructions to be 0.
		SetCtrlAttribute(panel, MDInstr_FinalDelay, ATTR_MIN_VALUE, 0.0); // Temp
		SetCtrlAttribute(panel, MDInstr_InitDelay, ATTR_MIN_VALUE, 0.0); // Temp

	
		SetCtrlAttribute(panel, control, ATTR_ON_COLOR, VAL_RED); // Change color to red.
	} else if(state == 2) {
		// In the 2 state, we want to hide the time increment stuff and show the instruction increment stuff
	
		// First hide the time increment controls.
		SetCtrlAttribute(panel, MDInstr_FinalDelay, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_FinalDelayUnits, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_Increment, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_IncrementUnits, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_InitDelay, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_InitDelayUnits, ATTR_VISIBLE, 0);
		SetCtrlAttribute(panel, MDInstr_Instr_Data, ATTR_VISIBLE, 0);
	
		//Then show the instruction increment stuff
		SetCtrlAttribute(panel, MDInstr_IntInstrData, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_IncInstrData, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_FInstrData, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_InitTimeData, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_IncTimeData, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_FTimeData, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_InitTimeDataUnits, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_IncTimeDataUnits, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, MDInstr_FTimeDataUnits, ATTR_VISIBLE, 1);
		
		// Dim some of the main panel stuff, undim other parts
		SetCtrlAttribute(inst[instr], PulseInstP_Instr_Data, ATTR_DIMMED, 1);
		SetCtrlAttribute(inst[instr], PulseInstP_InstDelay, ATTR_DIMMED, 0);
		SetCtrlAttribute(inst[instr], PulseInstP_TimeUnits, ATTR_DIMMED, 0);
		
		
		// If it hasn't been turned on yet, it's all 0s, so initialize it appropriately.
		int v;
		GetCtrlVal(panel, MDInstr_IncInstrData, &v);
		if(v == 0) {
			GetCtrlVal(panel, MDInstr_Instr_Data, &v);
			
			SetCtrlVal(panel, MDInstr_IntInstrData, v);
			SetCtrlVal(panel, MDInstr_IncInstrData, 1);
			
			// Set the base units to whatever they are on the main panel.
			double units;
			GetCtrlVal(inst[instr], PulseInstP_TimeUnits, &units);
			SetCtrlVal(panel, MDInstr_InitTimeDataUnits, units);
			SetCtrlVal(panel, MDInstr_IncTimeDataUnits, units); 
			SetCtrlVal(panel, MDInstr_FTimeDataUnits, units); 
			
			update_instr_data_nd(panel, 2);
		}
			
		SetCtrlAttribute(panel, control, ATTR_ON_COLOR, VAL_BLUE); // Change the color
	}
	
	
	return 0;
	
}

int change_dimension (int panel)
{
	int dimmed, nd, nl, i;
	GetCtrlAttribute(panel, MDInstr_Dimension, ATTR_DIMMED, &dimmed);
	
	if(dimmed)
		return 0;
	
	GetNumListItems(panel, MDInstr_Dimension, &nl);
	
	GetCtrlVal(Pulse_Prog_Config, PPConfig_NumDimensions, &nd); 
	
	if(nl >= nd)
	{
		GetCtrlIndex(panel, MDInstr_Dimension, &i);
		
		if(i >= nd)
			SetCtrlIndex(panel, MDInstr_Dimension, nd-1);
		
		DeleteListItem(panel, MDInstr_Dimension, nd-1, -1);
	} else if (nl < nd) {
		for(i = nl+1; i<nd; i++)
		{
			char *lab = malloc(3);
			sprintf(lab, "%d", i);
			InsertListItem(panel, MDInstr_Dimension, -1, lab, i);
		}
	}
	
	
	
	ChangeDimension(panel, MDInstr_Dimension, EVENT_COMMIT, NULL, NULL, NULL);

	populate_dim_points();

	return 0;
	
}

int save_session(char *filename) { // Primary session saving function
	// Generates a pair of files, an xml file named filename.xml and a program named
	// filename.tdms. If you pass NULL, session_fname is used.
	
	char *fname = (filename == NULL)?session_fname:filename;
	int i;
	
	// This bit is a workaround, because nc_open seems to leave files open for some
	// reason. Until I figure out how to unlock the files, this mechanism for using
	// a buffered additional file should work.
	if(FileExists(fname, 0)) { // Check if there's some issue deleting the old one
		// We won't go through so much trouble if this isn't the default
		if(filename != NULL)
			return -1;
	
		// Generate a new filename to use, since there's some issue.
		int i = 1, l = strlen(session_fname)-2;
		char *new_fname = malloc(l+7);
		char *old_fname = malloc(l);
		strncpy(old_fname, session_fname, l);
		old_fname[l-1] = '\0';

		int dv = DeleteFile(session_fname);
		
		while(i < 10000) {
			sprintf(new_fname, "%s%04d.nc", old_fname, i++);
			if(!FileExists(new_fname, 0))
				break;
			else if(!DeleteFile(new_fname))
				break;
		}

		if(i < 10000 && dv) 
			fname = new_fname;
		else if (dv)  
			return -1;
		else 
			free(new_fname);
		
		free(old_fname);
	}
	
	// Now we know we have a working filename, so we can do netCDF stuff.
	int rv, ncid;
	if(rv = nc_create(fname, NC_NETCDF4, &ncid))
		goto error;		// Error opening file.
	
	return rv;
	
	// Get some information we'll need.
	int d_len, len;
	char *base_fname = NULL, *f_path = NULL;
	
	/***************** Strings *****************/
	// Base file name
	GetCtrlValStringLength(mc.basefname[1], mc.basefname[0], &len);
	base_fname = malloc(len+1);
	GetCtrlVal(mc.basefname[1], mc.basefname[0], base_fname);
	
	// Path where files are stored
	GetCtrlValStringLength(mc.path[1], mc.path[0], &len);
	f_path = malloc(len+1);
	GetCtrlVal(mc.path[1], mc.path[0], f_path);
	
	// Define some attributes
	if(rv = nc_put_att_string(ncid, NC_GLOBAL, "base_fname", 1, &base_fname))
		goto error;
	
	if(rv = nc_put_att_string(ncid, NC_GLOBAL, "path", 1, &f_path))
		goto error;

	// Free the strings.
	free(base_fname);
	free(f_path);
	base_fname = NULL;
	f_path = NULL;

	/***************** Numerics *****************/
	
	// Save the Broken TTLs as an int.
	if(rv = nc_put_att_int(ncid, NC_GLOBAL, "broken_ttls", NC_INT, 1, &uipc.broken_ttls))
		goto error;
	
	// The last thing we need to do is save the program to the file, in the group "Program"
	// First, get the program.
	PPROGRAM *p = get_current_program();
	if(p == NULL)
		return -3;	 // Error with the program.
	
	// Create a group for the file to be in, called "Program"
	if(rv = nc_def_grp(ncid, "Program", &gid)) 
			goto error;

	if(rv = nc_save_program(ncid, gid, p))
		goto error;
	
	error:
	// Close the file
	if(ncid >= 0)
		nc_close(ncid);
	
	// Free whatever needs freeing.
	if(base_fname == NULL)
		free(base_fname);
	
	if(f_path == NULL)
		free(f_path);
	
	if(p != NULL)
		free_pprog(p);
	
	if(filename == NULL && (strcmp(fname, session_fname) != 0))
		free(fname);
	
	return rv;
}

int load_session(char *filename) { // Primary session loading function
 	// Function for loading everything from a netCDF file. Pass NULL to filename
	// to get the default session saving location. This will load from the 
	// primary session saving location only. If it is missing, everything is
	// left at defaults
	
	return 0;
	
	int rv;
	int ncid = -1, pfid;
	
	char *fname = (filename == NULL)?session_fname:filename;
	char *base_fname = NULL, *f_path = NULL;
	PPROGRAM *p = NULL;
	
	// Choose the most recent file saved.
	if(filename == NULL) {
		int i, l = strlen(session_fname)-2;
		char *new_fname = malloc(l+7);
		char *old_fname = malloc(l);
		strncpy(old_fname, session_fname, l);
		old_fname[l-1] = '\0';
		
		for(i = 1; i < 10000; i++) {
			sprintf(new_fname, "%s%04d.nc", old_fname, i);
			if(!FileExists(new_fname, 0))
				break;
		}
		
		if(i < 10000 && i > 1) {
			sprintf(new_fname, "%s%04d.nc", old_fname, i-1);
			fname = new_fname;
		} else 
			free(new_fname);
		
		free(old_fname);
	}
	
	
	// Open the netCDF file.
	if(rv = nc_open(fname, NC_NOWRITE, &ncid))
		goto error;
	
	nc_close(ncid);
	return rv;
	
	// First make some variables that we'll need.
	// Additionally, we're just going to give a pass to any attributes or variables that are
	// missing, in case this came from an earlier version with fewer fields.
	
	// File saving strings
	if((rv = nc_get_att_string(ncid, NC_GLOBAL, "base_fname", &base_fname)) && (rv != NC_ENOTATT))
		goto error;
	
	if((rv = nc_get_att_string(ncid, NC_GLOBAL, "path", &f_path)) && (rv != NC_ENOTATT))
		goto error;
	
	// Broken TTLs
	if((rv = nc_get_att_int(ncid, NC_GLOBAL, "broken_ttls", &uipc.broken_ttls)) && (rv != NC_ENOTATT))
		goto error;
	else if (rv)
		uipc.broken_ttls = 0;
	
	// Set the relevant fields.
	if(f_path != NULL)
		SetCtrlVal(mc.path[1], mc.path[0], f_path);
	if(base_fname != NULL)
		SetCtrlVal(mc.basefname[1], mc.basefname[0], base_fname);
	
	setup_broken_ttls();
	
	// We're done with that stuff now, so let's get the pulse program.
	int e_v, gid;
	
	if(rv = nc_inq_grp_ncid(ncid, "Program", &gid))
		goto error;
	
	p = nc_load_program(ncid, gid, &e_v);
	
	if(p == NULL) {
		rv = e_v;
		goto error;
	}
	
	// Then we can just load the program to the controls.
	set_current_program(p);
	
	// Jump here if there's an error.
	error:
	if(ncid >= 0)
		nc_close(ncid);
	
	if ((rv == NC_ENOTATT) || (rv == NC_ENOTVAR))
		rv = 0;
		
	if(p != NULL)
		free_pprog(p);
	
	if(filename == NULL && (strcmp(fname, session_fname) != 0))
		free(fname);
	
	return rv;
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
	/*
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
	*/
	return 0;
}

int nc_put_pfuncs(int ncid, int gid, pfunc **funcs, int *func_locs, int nFuncs, int tFuncs, nc_type NC_PINSTR) {
	// Writes the array of pfuncs to ncid in the group gid. If gid < 0, gid = ncid;
	// This assumes that you give it the file open and in data mode. The function 
	// does definitions, but will return the file in data mode.
	/*
	
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
	*/	
	return 0;

}

int nc_save_program(int ncid, int gid, PPROGRAM *p) {
	// Does all the heavy lifting in saving a program to a file. Pass it an open
	// nc file. ncid and gid can be the same if you are saving to the base group.
	
	int rv;
	int *v_ins_dids = NULL, *var_ids = NULL;
	char *del_exprs = NULL, *dat_exprs = NULL;

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
		
		// This one's a bit weird,  because we're going to have to change the string arrays into
		// a single char array, delimited by newlines.
		int del_e_len, dat_e_len;
		del_exprs = generate_nc_string(p->delay_exprs, p->nVaried, &del_e_len);
		dat_exprs = generate_nc_string(p->data_exprs, p->nVaried, &dat_e_len);
		
		if(rv = nc_put_att(gid, v_ins_id, "delay_exprs", NC_CHAR, del_e_len, del_exprs))
			goto error;
		if(rv = nc_put_att(gid, v_ins_id, "data_exprs", NC_CHAR, dat_e_len, dat_exprs))
			goto error;
		
		// Now if it's necessary, the skip_locs array
		if(p->skip) {
			if(rv = nc_def_var(gid, "skip_locs", NC_INT, 1, &v_ins_dids[1], &skip_locs_id))
				goto error;
			
			size_t exprlen = strlen(p->skip_expr)+1;
			if(rv = nc_put_att(gid, skip_locs_id, "skip_expr", NC_CHAR, exprlen, p->skip_expr))
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
	if(var_ids != NULL)
		free(var_ids);
	
	if(v_ins_dids != NULL)
		free(v_ins_dids);
	
	if(del_exprs != NULL)
		free(del_exprs);
	
	if(dat_exprs != NULL)
		free(dat_exprs);
	
	return rv;
	
}


PPROGRAM *nc_load_program(int ncid, int gid, int *err_val) {
	err_val[0] = 0;
	int retval = 0, pos = 0; 		// For error checking
	int ndele = 0, ndate = 0;
	char **data = NULL, **delay = NULL;
	char *delay_exprs = NULL, *data_exprs = NULL;

	// Let's make a PProgram
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
		
		/*// Create an array of strings to get the delay and data expressions.
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
		
		*/
		
		// We need to get an array of strings out of the newline-delimited strings
		int del_e_len, dat_e_len;
		
		if(retval = nc_inq_attlen(gid, v_ins_id, "delay_exprs", &del_e_len))
			goto error;
		
		if(retval = nc_inq_attlen(gid, v_ins_id, "data_exprs", &dat_e_len))
			goto error;
		
		delay_exprs = malloc(del_e_len);
		data_exprs = malloc(dat_e_len);
		
		if(retval = nc_get_att_text(gid, v_ins_id, "delay_exprs", delay_exprs))
			goto error;
			
		if(retval = nc_get_att_text(gid, v_ins_id, "data_exprs", data_exprs))
			goto error;
		
		// Best guess for how many we'll find.
		ndele = p->nVaried;
		ndate = p->nVaried;
		
		// Get the arrays.
		delay = get_nc_strings(delay_exprs, &ndele);
		data = get_nc_strings(data_exprs, &ndate);
		
		if(ndele != p->nVaried || ndate != p->nVaried) {
			retval = -250;
			goto error;
		}
		
		// Free the old p->data and p->delay arrays and replace them with what we just got
		/*
		if(p->data_exprs != NULL) {
			for(i = 0; i < p->nVaried; i++)
				free(p->data_exprs[i]);
			free(p->data_exprs); 
		} */
		
		p->data_exprs = data;
		
		/*
		if(p->delay_exprs != NULL) {
			for(i = 0; i < p->nVaried; i++)
				free(p->delay_exprs[i]);
			free(p->delay_exprs);
		}*/
		
		p->delay_exprs = delay;

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
	
	
	// Move the TTL trigger to where it is now, if there's a difference.
	if(p->trigger_ttl != uipc.trigger_ttl) {
		for(i = 0; i < p->nUniqueInstrs; i++)
			p->instrs[i]->flags = move_bit(p->instrs[i]->flags, uipc.trigger_ttl, p->trigger_ttl);
		p->trigger_ttl = uipc.trigger_ttl;
	}
	
	// Check if the program utilizes broken TTLs, and if so, offer a switch.
	int reserved = 0;
	for(i = 0; i < p->nUniqueInstrs; i++)
		reserved = reserved|p->instrs[i]->flags;
	int overlap = reserved & uipc.broken_ttls;

	// If there's any overlap, offer to swap things around.
	if(overlap) {
		char *resp = malloc(3);
		char *message = malloc(200);
		for(i = 0; i < 24; i++) {
			if(overlap & 1<<i) {
				sprintf(message, "There is a conflict in TTL %d. If you would like to swap with another TTL line, enter the line (0-23). Enter -1 or cancel if you would like to keep it where it is.", i);
				if(PromptPopup("TTL Conflict", message, resp, 2) != VAL_USER_CANCEL) {
					int fc;
					if(sscanf(resp, "%d", &fc) > 0 && fc >= 0 && fc < 24)
						swap_ttl(i, fc);
				}
			}
		}
		free(message);
		free(resp);
	}
	
	return p;
		
	error:
	if(delay_exprs != NULL)
		free(delay_exprs);
	
	if(data_exprs != NULL) 
		free(data_exprs);

	if(pos == 1)
		free(p);

	if(pos >= 2)
		free_pprog(p);
	
	err_val[0] = retval;

	return NULL;
}
