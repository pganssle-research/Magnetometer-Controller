												  	//Start with the basic 1-D Stuff


typedef struct DEFAULTS
{
	int NPoints;
	int InputChannel;
	int TriggerChannel;
	int TriggerTTL;
	int SavedSession;
	int WritetoFile;
	int overwritefile;
	int lastprog;
	int cont_mode;
	int phasecycle;
	int phasecycleinstr;
	int numcycles;
	
	char *pathname;
	char *filename;
	char *loadprogram;
	char *lastprogrambuffer;
	char *defaultsloc;
		
	double SamplingRate;
} DEFAULTS; 

int LoadProgram_To_Controls (PPROGRAM *p)
{
	
	int control[3], nl, i, j, *ival;
	double *dval;
	void *val;
	PPROGRAM *pval;
	

	
	program_breakout(0, control, p);
	
	nl = control[0];
	int pan;
	
	for(i = 1; i<=nl; i++)
	{
		if(p->nDimensions < 2 && (i == 13 || i == 15))
				continue;
		val = program_breakout(i, control, p);
		
		if(val == NULL)
			return -1;
		
		if(i == 4 || i == 12)
			continue;
		
		if(p->nDimensions > 1)
		{
			SetCtrlVal(Pulse_Prog_Config, PPConfig_NDimensionalOn, 1);
			ToggleND(Pulse_Prog_Config, PPConfig_NDimensionalOn, EVENT_COMMIT, NULL, NULL, NULL);
			SetCtrlVal(Pulse_Prog_Config, PPConfig_NumDimensions, p->nDimensions);
			NumDimensionCallback(Pulse_Prog_Config, PPConfig_NumDimensions, EVENT_COMMIT, NULL, NULL, NULL);
		}
		
		if(control[2] == 1)
		{
			ival = val;
			SetCtrlVal(control[0], control[1], ival[0]);
		} else if(control[2] == 2) {
			dval = val;
			SetCtrlVal(control[0], control[1], dval[0]);
		} else if(control[2] == 3) {
			pval = val;
			int k = pval->nPoints;
		
			//TTLs
			for(j = 0; j<24; j++)
				SetCtrlVal(inst[k], Load_TTLs(j), pval->flags[j]);
		
			//Instructions
			SetCtrlVal(inst[k], PulseInstP_Instructions, pval->instr[0]);
			InstrCallback(inst[k], PulseInstP_Instructions, EVENT_COMMIT, NULL, 0, 0);
			SetCtrlVal(inst[k], PulseInstP_Instr_Data, pval->instr_data[0]);
		
			SetCtrlVal(inst[k], PulseInstP_TimeUnits, pval->time_units[0]);
			ChangeTUnits(inst[k], PulseInstP_TimeUnits, EVENT_COMMIT, NULL, 0, 0);
			SetCtrlVal(inst[k], PulseInstP_InstDelay, pval->instruction_time[0]/pval->time_units[0]);
			ChangeInstDelay(inst[k], PulseInstP_InstDelay, EVENT_COMMIT, NULL, NULL, NULL);
		
			SetCtrlVal(inst[k], PulseInstP_Scan, pval->trigger_scan[0]);
		
			if(ttl_trigger != p->trigger_ttl)
				move_ttl(inst[k], ttl_trigger, p->trigger_ttl);
			
			SetCtrlVal(inst[k], Load_TTLs(ttl_trigger), pval->trigger_scan[0]); 
		} else if (control[2] == 4) {
			continue;
			int pan;
			ival = val;
			pan = control[0];
			char c[10];
			int nl1, nl2;
			sprintf(c, "%d", control[1]+1);
			GetNumListItems(pan, PPConfig_DimensionPoint, &nl1);
			GetNumListItems(pan, PPConfig_DimensionPoints, &nl2);
			
			if(nl1 < p->nDimensions || nl2 < p->nDimensions)
				populate_dim_points();
			
			ReplaceListItem(pan, PPConfig_DimensionPoint, control[1], c, p->step[control[1]]);
			ReplaceListItem(pan, PPConfig_DimensionPoints, control[1], c, p->nSteps[control[1]]);
		} else if (control[2] == 5) {
			pval = val;
			
			pan = cinst[pval->v_instr_num[0]];
			
			SetCtrlVal(pan, MDInstr_VaryInstr, 1);
			SetCtrlVal(pan, MDInstr_InitDelay, pval->init[0]);
			SetCtrlVal(pan, MDInstr_InitDelayUnits, pval->initunits[0]);
			SetCtrlVal(pan, MDInstr_Increment, pval->inc[0]);
			SetCtrlVal(pan, MDInstr_IncrementUnits, pval->incunits[0]);
			SetCtrlVal(pan, MDInstr_FinalDelay, pval->final[0]);
			SetCtrlVal(pan, MDInstr_FinalDelayUnits, pval->finalunits[0]);
			
			ChangeInstrVary(pan, MDInstr_VaryInstr, EVENT_COMMIT, NULL, NULL, NULL);
			
			SetCtrlVal(pan, MDInstr_Dimension, pval->dimension[0]);
			SetCtrlVal(pan, MDInstr_NumSteps, p->nSteps[pval->dimension[0] - 1]);     
			
			ChangeNumSteps(pan, MDInstr_NumSteps, EVENT_COMMIT, NULL, NULL, NULL);
		}
	}
	

	
	change_number_of_instructions();
	PhaseCycleCallback(Pulse_Prog_Config, PPConfig_PhaseCycle, EVENT_COMMIT, NULL, NULL, NULL);
			
	
	return 0;
}

void *program_breakout(int ln, int *output, PPROGRAM *p)
{
	//Returns the value on line ln of the program, with lines defined as:
	//0 = Total number of lines 
	//1 = Number of instructions
	//2 = Number of transients
	//3 = Delay Time between transients
	//4 = Trigger TTL
	//5 = Number of points
	//6 = Sampling rate
	//7 = Phase Cycle
	//8 = NumCycles
	//9 = CycleInstr
	//10 = CycleFreq
	//11 = Number of Dimensions
	//12 = Number of Varied Instructions
	//13 = First Indirect Dimension Point
	//14 = First Instruction
	//15 = First varied instruction
	//16 -> 15 + NDim = Indirect dimension points.
	//16 + NDim -> 15+NDim+NOI = Remainder of instructions
	//16 + NDim+NOI -> 15+NDIM+NOI+NV = Remaining varied instructions
	
	//Output is a 3-valued array.
	//Index 0 returns the panel handle of the relevant control.
	//Index 1 returns the control id.
	//Index 2 returns the data type.
	//   ->-1 = ND
	//   ->1 = int
	//   ->2 = double
	//   ->3 = instruction
	//   ->4 = 2D Instruction Point
	//   ->5 = 2D Instruction
	//If Index 2 returns 3, a PPROGRAM is generated with 24 instructions, each representing
	//one of the TTLs. The program's ->flags parameter will be set 0 or 1 to indicate if the
	//TTL flag should be 0 or 1 at that index. All other values are stored in the 0th index.
	//nPoints returns the instruction # (0 index).
	
	if(p == NULL || p->n_inst < 1 || p->nDimensions < 1 || ln < 0)
		return NULL;
	
	int nd = p->nDimensions, nv = p->nVaried;
	
	if(nd > 1 && nv < 0)
		return NULL;

	if(nd-- == 1)
		nv = 0;
	
	int n = 15, number_of_lines = n+p->n_inst+nd+nv-1;
	if(number_of_lines < n)
		number_of_lines = n;
	int control[number_of_lines+1][3], i;
	
	if(ln > number_of_lines)
		return NULL;
	
	if(ln == 0)
	{
		output[0] = number_of_lines;
		output[1] = -1;
		output[2] = -1;
		return NULL;
	}
	
	for(i = 1; i<12; i++)
		control[i][0] = Pulse_Prog_Config;

	control[1][0] = Pulse_Prog_Tab;
	control[4][0] = Pulse_Prog_Tab;
	
	if(ninstructions < p->n_inst)
	{
		i = n_inst_disp;
		SetCtrlVal(Pulse_Prog_Tab, PulseProg_InstNum, p->n_inst);
		change_number_of_instructions();
		SetCtrlVal(Pulse_Prog_Tab, PulseProg_InstNum, i);
		change_number_of_instructions();
	}
	
	for(i = 1; i < nd; i++)
	{
		control[i+n][0] = Pulse_Prog_Config;
		control[i+n][1] = i;
		control[i+n][2] = 4;
	}
	
	for(i = 1; i < p->n_inst; i++)
	{
		control[i+n+nd][0] = inst[i];
		control[i+n+nd][1] = -1;
		control[i+n+nd][2] = 3;
	}
	
	for(i = 1; i < nv; i++)
	{
		if(p->v_instr_num[i] < 0 || p->v_instr_num[i] > p->n_inst)
			control[i+n+nd+p->n_inst-2][0] = -1;
		else
			control[i+n+nd+p->n_inst-2][0] = cinst[p->v_instr_num[i]];
		control[i+n+nd+p->n_inst-2][1] = -1;
		control[i+n+nd+p->n_inst-2][2] = 5;
	}
	
	control[13][0] = Pulse_Prog_Config;
	control[13][1] = 0;
	control[13][2] = 4;
	
	control[14][0] = inst[0];
	control[14][1] = -1;
	control[14][2] = 3;
	
	if(nd == 0)
		control[15][0] = -1;
	else if(p->v_instr_num[0] < 0 || p->v_instr_num[0] > p->n_inst)
		control[15][0] = -1;
	else
		control[15][0] = cinst[p->v_instr_num[0]];

	control[15][1] = -1;
	control[15][2] = 5;
	
	
		
	int takesint[9] = {1, 2, 4, 5, 7, 8, 9, 11, 12}, takesdouble[3] = {3, 6, 10};
	
	for(i = 0; i < 9; i++)
		control[takesint[i]][2] = 1;
	for(i = 0; i < 3; i++)
		control[takesdouble[i]][2] = 2;
	
	//Define the control ids.
	control[1][1] = PulseProg_InstNum;
	control[2][1] = PPConfig_NTransients;
	control[3][1] = PPConfig_DelayTime;
	control[4][1] = PulseProg_Trigger_TTL;
	control[5][1] = PPConfig_NPoints;
	control[6][1] = PPConfig_SampleRate;
	control[7][1] = PPConfig_PhaseCycle;
	control[8][1] = PPConfig_NumCycles;
	control[9][1] = PPConfig_PhaseCycleInstr;
	control[10][1] = PPConfig_PhaseCycleFreq;
	control[11][1] = PPConfig_NumDimensions;
	control[12][1] = -1;
	
	
	//Set outputs.
	output[0] = control[ln][0];
	output[1] = control[ln][1];
	output[2] = control[ln][2];
	 
	//Return the pointers
	if(control[ln][2] == 3)
	{
		PPROGRAM *m = malloc(sizeof(PPROGRAM));

		if(ln == 14)
			ln = 0;
		else
			ln -= n+nd;
		
		m->instr = malloc(sizeof(int));
		m->instr_data = malloc(sizeof(int));
		m->trigger_scan = malloc(sizeof(int));
		
		m->instruction_time = malloc(sizeof(double));
		m->time_units = malloc(sizeof(double));
		
		m->flags = malloc(sizeof(int)*24);
		
		for(i = 0; i<24; i++)
			m->flags[i] = ((int)pow(2, i) & p->flags[ln]);
		
		m->instr[0] = p->instr[ln];
		m->instr_data[0] = p->instr_data[ln];
		m->trigger_scan[0] = p->trigger_scan[ln];
		m->instruction_time[0] = p->instruction_time[ln];
		m->time_units[0] = p->time_units[ln];
		m->nPoints = ln;
		
		return m;
	}
	
	if(control[ln][2] == 4)
	{
		int *rv = malloc(2*sizeof(int));
		if(ln == 13)
			ln = 0;
		else
			ln -= n;
		
		rv[0] = p->step[ln];
		rv[1] = p->nSteps[ln];
		
		return rv;
	}
	
	if(control[ln][2] == 5)
	{
		PPROGRAM *m = malloc(sizeof(PPROGRAM));
		
		if(ln == n)
			ln = 0;
		else
			ln -= n+nd+p->n_inst-2;
		
		m->init = malloc(sizeof(double));
		m->initunits = malloc(sizeof(double));
		m->inc = malloc(sizeof(double));
		m->incunits = malloc(sizeof(double));
		m->final = malloc(sizeof(double));
		m->finalunits = malloc(sizeof(double));
		
		m->v_instr_num = malloc(sizeof(int));
		m->dimension = malloc(sizeof(int));
		
		m->init[0] = p->init[ln];
		m->initunits[0] = p->initunits[ln];
		m->inc[0] = p->inc[ln];
		m->incunits[0] = p->incunits[ln];
		m->final[0] = p->final[ln];
		m->finalunits[0] = p->finalunits[ln];
		m->v_instr_num[0] = p->v_instr_num[ln];
		m->dimension[0] = p->dimension[ln];
		
		return m;
	}
	
	if(ln == 1)
		return &p->n_inst;
	if(ln == 2)
		return &p->ntransients;
	if(ln == 3)
		return &p->delaytime;
	if(ln == 4)
		return &p->trigger_ttl;
	if(ln == 5)
		return &p->nPoints;
	if(ln == 6)
		return &p->samplingrate;
	if(ln == 7)
		return &p->phasecycle;
	if(ln == 8)
		return &p->numcycles;
	if(ln == 9)
		return &p->phasecycleinstr;
	if(ln == 10)
		return &p->cyclefreq;
	if(ln == 11)
		return &p->nDimensions;
	if(ln == 12)
		return &p->nVaried;
	
	
	return NULL;

}

PPROGRAM *LoadProgram_To_Program (char *loadfile)
{
	PPROGRAM *p = malloc(sizeof(PPROGRAM));
	
	int i = 0, j=0, number_of_inst;
	int trigger_scan, flags, instr, instr_data, ntransients, control[3];
	char *string_buffer = malloc(500), *string_match = malloc(500);
	float time, time_units, delaytime;
	FILE *fileid;
	fpos_t *position = malloc(sizeof(fpos_t));
	
	if(!file_exists(loadfile))
		return NULL;
	
	fileid = fopen(loadfile, "r+");
	
	//Determine if the file is actually a pulse program.
	fgets(string_buffer, 500, fileid);
	if (!sscanf(string_buffer, "%s", string_match) || strstr(string_match, "#ValidPulseProgram#") == NULL)
	{	
		printf("Not a valid pulse program!");
		return NULL;
	}
	
	fgetpos(fileid, position);
	
	p->n_inst = 1;
	p->nDimensions = 1;
	//Get the total number of objects in a program.
	program_breakout(0, control, p); 
	
	int n = control[0];
	char *labels[n];

	//Get the character labels in the program and write them to a 2-dimensional array.
	for(i = 1; i<=n; i++)
		labels[i-1] = get_program_label(i);
	
	//Find number of instructions. 
	while(!feof(fileid))
	{
		fgets(string_buffer, 500, fileid);
		if(sscanf(string_buffer, "%s %d", string_match, &p->n_inst) > 1 && (strstr(string_match, labels[0]) != NULL))
			break;
	}
	
	if(feof(fileid))
		return NULL;
	
	rewind(fileid);
	
	while(!feof(fileid))
	{
		fgets(string_buffer, 500, fileid);
		if(sscanf(string_buffer, "%s %d", string_match, &p->nDimensions) > 1 && (strstr(string_match, labels[10]) != NULL))
			break;
	}
	
	if(feof(fileid))
		return NULL;
	
	rewind(fileid);
	
	//If there are no instructions, it isn't a program.
	if(p->n_inst < 1 || p->nDimensions < 1)
	{
		fclose(fileid);
		return NULL;
	}
	
	int nl = control[0], *ival;
	double *dval;
	void *val;
	int si = p->n_inst*sizeof(int), sd = p->n_inst*sizeof(double);
	
	if(p->nDimensions > 1)
	{
		while(!feof(fileid))
		{
			fgets(string_buffer, 500, fileid);
			if(sscanf(string_buffer, "%s %d", string_match, &p->nVaried) > 1 && (strstr(string_match, labels[11]) != NULL))
				break;
		}
		if(p->nVaried < 1)
			return NULL;
		
		p->v_instr_num = malloc(sizeof(int)*p->nVaried);
		p->dimension = malloc(sizeof(int)*p->nVaried);
		
		p->init = malloc(sizeof(double)*p->nVaried);
		p->initunits = malloc(sizeof(double)*p->nVaried);
		p->inc = malloc(sizeof(double)*p->nVaried);		
		p->incunits = malloc(sizeof(double)*p->nVaried);		
		p->final = malloc(sizeof(double)*p->nVaried);		
		p->finalunits = malloc(sizeof(double)*p->nVaried);
		
		p->nSteps = malloc(sizeof(int)*p->nDimensions);
		p->step = malloc(sizeof(int)*p->nDimensions);
	}
			
	
	//Allocate memory for the program contents.
	p->flags = malloc(si);
	p->instr = malloc(si);
	p->instr_data = malloc(si);
	p->trigger_scan = malloc(si);
	
	p->instruction_time = malloc(sd);
	p->time_units = malloc(sd);
	
	p->scan = 0;

	fsetpos(fileid, position);
	int k = 0;
	while(!feof(fileid))
	{
		fgets(string_buffer, 500, fileid);
		if(sscanf(string_buffer, "%s", string_match) < 1 || feof(fileid))
			continue;
		for(j = 0; j < n; j++)
		{
			if(strstr(string_match, labels[j]) != NULL  && string_match[0] != '[')
				break;
			if(j == n-1)
				j += 2;
		}
		
		if(j > n)
			continue;
		
		val = program_breakout(j+1, control, p);
		
		if(val == NULL)
			return NULL;
		
		if(control[2] == 1)
		{
			ival = val;
			if(sscanf(string_buffer, "%s %d", string_match, ival) < 2)
				return NULL;
		} else if (control[2] == 2) {
			dval = val;
			if(sscanf(string_buffer, "%s %lf", string_match, dval) < 2)
				return NULL;
		} else if (control[2] == 3) {
			int flags, instr, instr_data, trigger_scan, ln;
			double time, time_units;
			
			if(sscanf(string_buffer, "%s %d %d %d %d %d %lf %lf", string_match, &ln, &trigger_scan, &flags, &instr, &instr_data, &time, &time_units) < 8)
				return NULL;
			
			p->trigger_scan[ln] = trigger_scan;
			p->flags[ln] = flags;
			p->instr[ln] = instr;
			p->instr_data[ln] = instr_data;
			p->instruction_time[ln] = time*time_units;
			p->time_units[ln] = time_units;
			
			if(p->trigger_scan)
				p->scan = 1;
		} else if (control[2] == 4) {
			ival = val;
			int point, points, dim;
			if(sscanf(string_buffer, "%*s %d %*s %d %*s %d", &dim, &point, &points) < 3)
				return NULL;
			
			p->step[dim-1] = point;
			p->nSteps[dim-1] = points;
		} else if (control[2] == 5) {
			int ln, dim;
			double init, initu, inc, incu, final, finalu;
			if(sscanf(string_buffer, "%*s %d %lf %lf %lf %lf %lf %lf %d", &ln, &init, &initu, &inc, &incu, &final, &finalu, &dim) < 8)
				return NULL;
			
			p->init[k] = init*initu;
			p->initunits[k] = initu;
			p->inc[k] = inc*incu;
			p->incunits[k] = incu;
			p->final[k] = final*finalu;
			p->finalunits[k] = finalu;
			p->v_instr_num[k] = ln;
			p->dimension[k] = dim;
			
			if(k > 0)
			{
				if(p->v_instr_num[k] < p->v_instr_num[k-1])
					swap_2d_instrs(p, k, k-1);
			}
			k++;
		}
	}
		
	fclose(fileid);

	return p;
}

char *get_program_label(int ln)
{
	//Returns the label associated with the program instruction that should fall on line ln
	//of a program.
	
	char *labels[16] = {"-1000", "NInstructions=", "NTransients=", "DelayTime=", "TriggerTTL=", "NPoints=", "SamplingRate=", "PhaseCycle=", "NumCycles=", "CycleInstr=", "CycleFreq=", "Dimensions=", "nVaried=", "IndirectDim", "Instruction" , "VaryInstr"};

	if(ln == 0)
	{
		int maxlen = 0, i;
		
		for(i = 1; i<16; i++)
		{
			if(maxlen < strlen(labels[i]))
				maxlen = strlen(labels[i]);
		}
		
		if(maxlen < 1000)
			sprintf(labels[0], "%d", maxlen);
		else
			return 0;
	}
	
	return labels[ln];
}

int swap_2d_instrs(PPROGRAM *p, int from, int to)
{
	//This program swaps two 2D instructions in a pulse program.
	//This is used to ensure that the 2D instructions will be in the right order.
	int v_i_b, d_b;
	double i_b, iu_b, in_b, inu_b, f_b, fu_b;
	
	v_i_b = p->v_instr_num[to]; 
	d_b = p->dimension[to];
	i_b = p->init[to];
	iu_b = p->initunits[to];
	in_b = p->inc[to];
	inu_b = p->incunits[to];
	f_b = p->final[to];
	fu_b = p->finalunits[to];
	
	p->v_instr_num[to] = p->v_instr_num[from];
	p->dimension[to] = p->v_instr_num[from];
	p->init[to] = p->init[from];
	p->initunits[to] = p->initunits[from]; 
	p->inc[to] = p->inc[from]; 
	p->incunits[to] = p->incunits[from]; 
	p->final[to] = p->final[from]; 
	p->finalunits[to] = p->finalunits[from];
	
	p->v_instr_num[from] = v_i_b;
	p->dimension[from] = d_b;
	p->init[from] = i_b;
	p->initunits[from] = iu_b;
	p->inc[from] = in_b;
	p->incunits[from] = inu_b;
	p->final[from] = f_b;
	p->finalunits[from] = fu_b;
	
	return 0;
}

int plot_fid(double *transient, double *average, int t, int np)
{
	int plot;
	
	if(t == 1)
	{
		SetCtrlAttribute(FID, FID_FIDGraph, ATTR_XAXIS_GAIN, 0.1);
		DeleteGraphPlot(FID, FID_FIDGraph, -1,VAL_IMMEDIATE_DRAW);
		ClearListCtrl(FID, FID_TransientNum);
		SetCtrlAttribute(FID, FID_FIDGraph, ATTR_DIMMED, 0);
	}
	
	char *c = malloc(10);
	sprintf(c, "%d", t);
	
	plot = PlotY (FID, FID_FIDGraph, transient, np, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_RED);  
	InsertListItem(FID, FID_TransientNum, -1, c, plot);
	
	if(t == 2)
	{
		plot = PlotY(FID, FID_FIDGraph, average, np, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_RED);
		InsertListItem(FID, FID_TransientNum, 0, "Average", plot);
	}
	
	if(t>2)
	{
		GetValueFromIndex(FID, FID_TransientNum, 0, &plot);
		SetPlotAttribute(FID, FID_FIDGraph, plot, ATTR_PLOT_YDATA, average);
	}
	
	if(cont_mode)
	{
		GetNumListItems(FID, FID_TransientNum, &plot);
		GetValueFromIndex(FID, FID_TransientNum, plot-1, &plot);
	}
	
	return plot;
}


int plot_fft(double *transient, double *average, int t, int np, double sr, float *rc, float *ic)
{
	//Calculates the Fourier Transform, then plots it.
	int i, coutsize = (int)((double)np/2 + 1), realplot, imagplot, plot;
	fftw_complex *output = fftw_malloc(sizeof(fftw_complex)*coutsize);
	double *real_fft = malloc(sizeof(double)*coutsize), *imag_fft = malloc(sizeof(double)*coutsize);
	
	double *signalarray = malloc(sizeof(double)*np);
	
	//Create the plan.
	fftw_plan p = fftw_plan_dft_r2c_1d(np, signalarray, output, FFTW_PRESERVE_INPUT);
	
	//Fill the input array with the transient data.
	for(i = 0; i<np; i++)
		signalarray[i] = transient[i];
	
	//Fourier transform the input data.
	fftw_execute(p);
	
	for(i = 0; i<coutsize; i++)
	{
		real_fft[i] = output[0][i];
		imag_fft[i] = output[1][i];
	}
	
	if(rc != NULL)
	{
		for(i = 0; i<coutsize; i++)
			rc[i] = (float)real_fft[i];
	}
	if(ic != NULL)
	{
		for(i = 0; i<coutsize; i++)
			ic[i] = (float)imag_fft[i];
	}
	
	
	//Plot the individual transient data
	if(t == 1)
	{
		SetCtrlAttribute(Spectrum, Spectrum_SpectrumGraph, ATTR_XAXIS_GAIN, sr/np);
		DeleteGraphPlot(Spectrum, Spectrum_SpectrumGraph, -1,VAL_IMMEDIATE_DRAW); 
		ClearListCtrl(Spectrum, Spectrum_TransientNum);
	}
		
	plot = PlotY(Spectrum, Spectrum_SpectrumGraph, real_fft, coutsize, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_YELLOW);
	realplot = PlotY(Spectrum, Spectrum_SpectrumGraph, real_fft, coutsize, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_YELLOW);
	imagplot = PlotY(Spectrum, Spectrum_SpectrumGraph, imag_fft, coutsize, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_YELLOW);
	
	//Make the individual plots accessible
	char *c = malloc(30);
	sprintf(c, "Transient %d", t);
	InsertListItem(Spectrum, Spectrum_TransientNum, -1, c, plot);
	InsertListItem(Spectrum, Spectrum_TransientNum, -1, "->Real Chan", realplot);
	InsertListItem(Spectrum, Spectrum_TransientNum, -1, "->Imag Chan", imagplot);
	free(c);
	
	if(t < 2)
		return plot;
	
	//If there is more than one transient, generate the FFT of the average.
	for(i = 0; i<np; i++)
		signalarray[i] = average[i];
	
	//Execute plan and destroy it, as we are done now.
	fftw_execute(p);
	fftw_destroy_plan(p);
	
	for(i = 0; i<coutsize; i++)
	{
		real_fft[i] = output[0][i];
		imag_fft[i] = output[1][i];
	}
	
	if(t == 2)
	{
		plot = PlotY(Spectrum, Spectrum_SpectrumGraph, real_fft, coutsize, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_YELLOW);
		realplot = PlotY(Spectrum, Spectrum_SpectrumGraph, real_fft, coutsize, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_TRANSPARENT);
		imagplot = PlotY(Spectrum, Spectrum_SpectrumGraph, imag_fft, coutsize, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_TRANSPARENT);
		
		InsertListItem(Spectrum, Spectrum_TransientNum, 0, "->Imag Chan", imagplot);
		InsertListItem(Spectrum, Spectrum_TransientNum, 0, "->Real Chan", realplot);
		InsertListItem(Spectrum, Spectrum_TransientNum, 0, "Average", plot);
	} else {
		GetValueFromIndex(Spectrum, Spectrum_TransientNum, 0, &plot);
		GetValueFromIndex(Spectrum, Spectrum_TransientNum, 1, &realplot);
		GetValueFromIndex(Spectrum, Spectrum_TransientNum, 2, &imagplot);
		
		SetPlotAttribute(Spectrum, Spectrum_SpectrumGraph, plot, ATTR_PLOT_YDATA, real_fft);
		SetPlotAttribute(Spectrum, Spectrum_SpectrumGraph, realplot, ATTR_PLOT_YDATA, real_fft);
		SetPlotAttribute(Spectrum, Spectrum_SpectrumGraph, imagplot, ATTR_PLOT_YDATA, imag_fft);
	}
	
	if(cont_mode)
	{
		GetNumListItems(Spectrum, Spectrum_TransientNum, &plot);
		GetValueFromIndex(Spectrum, Spectrum_TransientNum, plot-3, &plot);
	}
		
	return plot;
}

int get_data()
{
	//Gets the data, plots it and writes it to file if requested.
	//Mode 0 -> First sample taken.
	//Mode 1 -> Take samples and add them into DAQsignal
	//Mode 2 -> Last transient
	//Mode 3 -> One transient
	
	int i, j, overwrite, writetofile, np = running_prog->nPoints, mode, t = running_prog->transient, nt = running_prog->ntransients;
	int32 nsamplesread;
	double sr = running_prog->samplingrate;
	fpos_t *position = malloc(sizeof(fpos_t));
	char *pathname = malloc(500), *string_buffer = malloc(500), *string_match = malloc(500);
	char *filename = malloc(200), c;
	time_t seconds = time(NULL);

	GetCtrlVal(panelHandle, MainPanel_Path, pathname);
	GetCtrlVal(panelHandle, MainPanel_Filename, filename);
	GetCtrlVal(panelHandle, MainPanel_OverwriteFile, &overwrite);
	GetCtrlVal(panelHandle, MainPanel_WriteToFile, &writetofile);
	
	if(nt == 1)
		mode = 3;
	else if(t == nt)
		mode = 2;
	else if(t < nt)
	{
		if(t == 1)
			mode = 0;
		else
			mode = 1;
		running_prog->transient++;
	}
	else
		return -1;
	
	
	if(mode == 0 || mode == 3)
	{
		DAQsignal = malloc(sizeof(double)*np);
		for(i = 0; i<np; i++)
			DAQsignal[i] = 0.0;
	}
	
	float64 *samplearray = malloc(sizeof(float64)*np);

	//Read out task.
	DAQmxReadAnalogF64(acquireSignal, -1, 60.0, DAQmx_Val_GroupByChannel, samplearray, np,  &nsamplesread, NULL);

	for(i = 0; i<np; i++)
		DAQsignal[i] = samplearray[i];
	
	if(nsamplesread !=  np)
		return -2;
	
	FILE *fileout, *buffer;
	char *tdfilename = malloc(500), *fftfilename = malloc(500), *extension = ".txt", *bufferfile1 = malloc(L_tmpnam), *bufferfile2 = malloc(L_tmpnam);

	char *newfilename = malloc(200);
	sprintf(newfilename, "%s%04d", filename, 0);
	if(t == 1)
		running_prog->filename = malloc(200);    
	
	if((!overwrite || !writetofile) && t == 1)
	{
		char *checkfilename = malloc(500);

		for(i = 0; i<10000; i++)
		{
			sprintf(newfilename, "%s%04d", filename, i);

			sprintf(checkfilename, "%s%s\\", pathname, newfilename);
			if(!file_exists(checkfilename))
			break;
		}
		free(checkfilename);  
	}
	
	if(t == 1)
		strcpy(running_prog->filename, newfilename); 
	
	free(newfilename);
	filename = running_prog->filename;
	
	strcat(pathname, filename);
	strcat(pathname, "\\");
	if(!file_exists(pathname))
		MakeDir(pathname);
	sprintf(tdfilename, "%s%s%s%06d.txt", pathname, filename, "-FIDTransient", t);
	sprintf(fftfilename, "%s%s%s%06d.txt", pathname, filename, "-FFTTransient", t);
	strcat(pathname, filename);
	SetCtrlVal(FID, FID_CurrentPathname, pathname);
	strcat(pathname, extension);
	
	//FID and FFT Plotting
	int coutsize = (int)((float)np/2+1);
	double *rc = malloc(sizeof(double)*coutsize), *ic = malloc(sizeof(double)*coutsize);
	int fidplot, fftplot;
	
	if(t == 1)
		clear_plots();
	
	if(t>1)
	{
		fidplot = average_from_file(pathname, DAQsignal, np);
		if(fidplot < 0)
			return -(5-fidplot);
		
		if(t == 2)
			insert_transient(0);
	}
	
	insert_transient(t); 
	
	if(t> 1 && !cont_mode)
	{
				
		fft(DAQsignal, rc, ic, np);
		
		plot_fft_data(rc, ic, np, sr, 0);
		plot_fid_data(DAQsignal, np, 0, sr);
		switch_transient_index(0);
	} else {
		fft(samplearray,rc, ic, np);
		plot_fft_data(rc, ic, np, sr, t);
		plot_fid_data(samplearray, np, t, sr);
		switch_transient_index(t);
	}
	
	SetCtrlAttribute(FID, FID_TransientNum, ATTR_DIMMED, 0);
	SetCtrlAttribute(Spectrum, Spectrum_TransientNum, ATTR_DIMMED, 0);
	SetCtrlAttribute(Spectrum, Spectrum_Channel, ATTR_DIMMED, 0);
	int index;
	GetCtrlIndex(Spectrum, Spectrum_Channel, &index);
	if(index == 0)
		SetCtrlAttribute(Spectrum, Spectrum_ChangePhase, ATTR_DIMMED, 0);
	
	//Write the Data to an ASCII file
	tmpnam(bufferfile1);
	buffer = fopen(bufferfile1, "w+");
	fclose(buffer);

	if(t == 1)
		fileout = fopen(pathname, "w+");
	else
	{
		tmpnam(bufferfile2);
		fileout = fopen(bufferfile2, "w+");
	}
	
	//Make a header with experiment, date and time.
	fprintf(fileout, "Experiment: %s\n", filename);
	fprintf(fileout, "Completed: %s\n", ctime(&seconds));
	fprintf(fileout, "RawTime: %ld\n\n", (int)seconds);
	fprintf(fileout, "TransientsCompleted= %d of %d\n\n", t, nt);
	
	//Number of points, sampling rate
	fprintf(fileout, "NPoints= %d\nSamplingRate= %lf\nDimensions= %d\n\n", running_prog->nPoints, running_prog->samplingrate, running_prog->nDimensions);


	//Print the FID
	if(running_prog->nDimensions == 1)
	{
		fprintf(fileout, "[FID]\n");
		for(i = 0; i<np; i++)
			fprintf(fileout, "%lf\n", DAQsignal[i]);
		fprintf(fileout, "[ENDFID]\n\n");
	}
	
	fclose(fileout);
	if (t> 1) {
		remove(pathname);
		rename(bufferfile2, pathname);
	}

	//Print Individual Transient Data
	fileout = fopen(tdfilename, "w");
	fprintf(fileout, "Transient Data - Experiment: %s\n", filename);
	fprintf(fileout, "Completed: %s\n", ctime(&seconds));
	fprintf(fileout, "RawTime: %ld\n\n", seconds);
	fprintf(fileout, "Transient %d of %d\n\n", t, nt);
	
	//Print out the pulse program used.
	SaveProgram_PPROGRAM(running_prog, bufferfile1);      
	
	buffer = fopen(bufferfile1, "r+");
	
	while(!feof(buffer))
	{
		c = getc(buffer);
		if(feof(buffer))
			break;
		putc(c, fileout);
	}
	
	rewind(buffer); 

	fprintf(fileout, "\n\n[TransientData]\n");
	
	for(i = 0; i<np; i++)
		fprintf(fileout, "%lf\n", samplearray[i]);
	fprintf(fileout, "[EndTransientData]\n\n");
	fclose(fileout);
	
	
	//Print out the FFT for each transient
	fprintf(fileout, "FFT - Experiment: %s\n", filename);
	fprintf(fileout, "Completed: %s\n", ctime(&seconds));
	fprintf(fileout, "RawTime: %ld\n\n", seconds);
	fprintf(fileout, "Transient %d of %d\n\n", t, nt);
	
	while(!feof(buffer))
	{
		c = getc(buffer);
		if(feof(buffer))
			break;
		putc(c, fileout);
	}
	
	fprintf(fileout, "\n\n[RealChannel]\t[ImagChannel]\n");
	
	for(i = 0; i<coutsize; i++)
		fprintf(fileout, "%lf\t%lf\n", rc[i], ic[i]);
	fprintf(fileout, "[EndFFT]\n");
	fclose(fileout); 
	
	fclose(buffer);
	remove(bufferfile1);

	
	if(mode < 2)
	{
		DAQmxStopTask(acquireSignal);
		return 0;
	}
	
	return 1;
}

DEFAULTS *GetDefaults(int from_panel)
{
	DEFAULTS *d = malloc(sizeof(DEFAULTS));
	d->pathname = malloc(500), d->filename = malloc(200), d->loadprogram = malloc(500), d->lastprogrambuffer = malloc(500), d->defaultsloc = malloc(500);
	int control[3], i, to;
	void *val;
	
	defaults_breakout(1, from_panel, control, NULL);
	
	to = control[0]+1;
	
	for(i = 2; i<= to; i++)
	{
		val = defaults_breakout(i, from_panel, control, d);
		if(control[2] == 4)
			GetCtrlIndex(control[0], control[1], val);
		else
			GetCtrlVal(control[0], control[1], val);
	}
	
	return d;
}


DEFAULTS *LoadDefaults_IO(int to_panel, char *filename)
{
	int i, panel = Pulse_Prog_Config, ic_max_index, tc_max_index;
	char *string_buffer = malloc(500), *string_match = malloc(500);
	double sr; 
	DEFAULTS *d = malloc(sizeof(DEFAULTS));
	FILE *fileid;
	d->pathname = malloc(500), d->defaultsloc = malloc(500), d->filename = malloc(200), d->lastprogrambuffer = malloc(500), d->loadprogram = malloc(500);
	
	
	if(to_panel)
	{
		panel = DefaultsPanel;
		d->TriggerChannel = DefaultsP_Trigger_Channel;
		d->InputChannel = DefaultsP_AcquisitionChannel;
	} else {
		d->TriggerChannel = PPConfig_Trigger_Channel;
		d->InputChannel = PPConfig_AcquisitionChannel;
	}
	
	GetNumListItems(panel, d->InputChannel, &ic_max_index);
	GetNumListItems(panel, d->TriggerChannel, &tc_max_index);
	
	
	fileid = fopen(filename, "r+");
	
	//Check if it's a valid file.
	fgets(string_buffer, 500, fileid);
	if(!sscanf(string_buffer, "%s", string_match) || strstr(string_match, "#BoardDefaults#") == NULL)
	{
		MessagePopup("Error reading defaults", "Defaults file is invalid.");
		return NULL;
	}
	
	//Get NPoints - upon failure notify and set to 5000.
	fgets(string_buffer, 500, fileid);
	if(!sscanf(string_buffer, "%s %d", string_match, &d->NPoints) || strstr(string_match, "NPoints=") == NULL || d->NPoints < 0)
		d->NPoints = 5000;

	//Get sampling rate. Upon failure set to 10000.
	fgets(string_buffer, 500, fileid);
	if(!sscanf(string_buffer, "%s %lf", string_match, &sr) || strstr(string_match, "SamplingRate=") || sr < 0.0)
		d->SamplingRate = 10000;
	else
		d->SamplingRate = sr;
	
	//Get input channel index. Upon failure default to 0.
	fgets(string_buffer, 500, fileid);
	if(!sscanf(string_buffer, "%s %d", string_match, &d->InputChannel) || strstr(string_match, "InputChannel=") == NULL || d->InputChannel < 0 || d->InputChannel >= ic_max_index)
		d->InputChannel = 0;
	
	//Get trigger channel index. Upon failure default to 0.
	fgets(string_buffer, 500, fileid);
	if(!sscanf(string_buffer, "%s %d", string_match, &d->TriggerChannel) || strstr(string_match, "TriggerChannel=") == NULL || d->TriggerChannel < 0 || d->TriggerChannel >= tc_max_index)
		d->TriggerChannel = 0;
	
	//Get trigger TTL. Upon failure default to 23.
	fgets(string_buffer, 500, fileid);
	if(!sscanf(string_buffer, "%s %d", string_match, &d->TriggerTTL) || strstr(string_match, "TriggerTTL=") == NULL || d->TriggerTTL < 0 || d->TriggerTTL > 23)
		d->TriggerTTL = 23;
	
	//Get Pathname - upon failure, use whatever the default already is.
	fgets(string_buffer, 500, fileid);
	if(!sscanf(string_buffer, "%s %s", string_match, d->pathname) || strstr(string_match, "Path=") == NULL)
		GetCtrlVal(panelHandle, MainPanel_Path, d->pathname);

	//Get file name.
	fgets(string_buffer, 500, fileid);
	if(!sscanf(string_buffer, "%s %s", string_match, d->filename) || strstr(string_match, "Filename=") == NULL)
		GetCtrlVal(panelHandle, MainPanel_Filename, d->filename);
	
	//Determine whether the user wants to use the last program that was active before closing. If missing, default to 0.
	fgets(string_buffer, 500, fileid);
	if(!sscanf(string_buffer, "%s %d", string_match, &d->lastprog) || strstr(string_match, "LastProgram=") == NULL || d->lastprog > 1 || d->lastprog < 0)
		d->lastprog = 0;
	
	//Determine what the default program loaded should be. Upon failure, set to whatever's there already and set lastprog to 1.
	 fgets(string_buffer, 500, fileid);
	 if(!sscanf(string_buffer, "%s %s", string_match, d->loadprogram) || strstr(string_match, "LoadProgram=") == NULL)
	 {
		 GetCtrlVal(DefaultsPanel, DefaultsP_DefaultProgram, &d->loadprogram);
		 d->lastprog = 1;
	 }
	 
	 //Determine where the last program used should be. Upon failure, use what's there and set lastprog to 0.
	 fgets(string_buffer, 500, fileid);
	 if(!sscanf(string_buffer, "%s %s", string_match, d->lastprogrambuffer) || strstr(string_match, "LastProgramBuffer=") == NULL)
	 {
		 GetCtrlVal(DefaultsPanel, DefaultsP_LastProgramBuff, d->lastprogrambuffer);
		 d->lastprog = 0;
	 }
	 

	//Get WritetoFile. Do not notify upon failure. Defaults to 1 on failure.
	fgets(string_buffer, 500, fileid);
	if(!sscanf(string_buffer, "%s %d", string_match, &d->WritetoFile) || strstr(string_match, "WriteToFile=") == NULL || d->WritetoFile > 1 || d->WritetoFile < 0)
		d->WritetoFile = 1;
	
	//Get OverWriteFile. Do not notify upon failure. Defaults to 0 on failure.
	fgets(string_buffer, 500, fileid);
	if(!sscanf(string_buffer, "%s %d", string_match, &d->overwritefile) || strstr(string_match, "OverWriteFile=") == NULL || d->overwritefile > 1 || d->overwritefile < 0)
		d->overwritefile = 1;

	//Determine if the user just wants to keep the configuration between sessions. Defaults to 0.
	fgets(string_buffer, 500, fileid);
	if(!sscanf(string_buffer, "%s %d", string_match, &d->SavedSession) || strstr(string_match, "SavedSession=") == NULL || d->SavedSession > 1 || d->SavedSession < 0)
		d->SavedSession = 0;
	
	//Get Continuous Mode
	fgets(string_buffer, 500, fileid);
	if(sscanf(string_buffer, "%s %d", string_match, &d->cont_mode) != 2 || strstr(string_match, "RunContinuously=") == NULL || d->cont_mode > 1 || d->cont_mode < 0)
		d->cont_mode = 0;
	
	//Determine if they want to phase cycle.
	fgets(string_buffer, 500, fileid);
	if(sscanf(string_buffer, "%s %d", string_match, &d->phasecycle) != 2 || strstr(string_match, "PhaseCycle=") == NULL || d->phasecycle > 1 || d->phasecycle < 0)
		d->phasecycle = 0;
	
	//Phase Cycle Instr.
	fgets(string_buffer, 500, fileid);
	if(sscanf(string_buffer, "%s %d", string_match, &d->phasecycleinstr) != 2|| strstr(string_match, "PhaseCycleInstr=") == NULL || d->phasecycleinstr < 0)
		d->phasecycleinstr = 0;
	
	//Phase Cycle Instr.
	fgets(string_buffer, 500, fileid);
	if(sscanf(string_buffer, "%s %d", string_match, &d->numcycles) != 2|| strstr(string_match, "NumCycles=") == NULL  || d->numcycles < 1)
		d->numcycles = 1;
	
	
	d->defaultsloc = defaultsloc;
	
	fclose(fileid);
	
	//All strings are stored with spaces replaced with quesiton marks, because sscanf demarcates between strings at whitespace.
	//Thus, when loading it back up again, we need to replace the question marks with white spaces. 

	for(i = 0; i<strlen(d->pathname); i++)
		if(d->pathname[i] == '\?')
			d->pathname[i] = ' ';
	
	for(i = 0; i<strlen(d->filename); i++)
		if(d->filename[i] == '\?')
			d->filename[i] = ' ';

	for(i = 0; i<strlen(d->loadprogram); i++)
		if(d->loadprogram[i] == '\?')
			d->loadprogram[i] = ' ';

	for(i = 0; i<strlen(d->defaultsloc); i++)
		if(d->defaultsloc[i] == '\?')
			d->defaultsloc[i] = ' ';

	for(i = 0; i<strlen(d->lastprogrambuffer); i++)
		if(d->lastprogrambuffer[i] == '\?')
			d->lastprogrambuffer[i] = ' ';
		
	return d;
}

int LoadDefaultstoControls(int to_panel, DEFAULTS *d)
{
	int control[3], i, *ival, to, max_index;
	double *dval;
	char *cval;
	void *val;
	
	defaults_breakout(1, to_panel, control, NULL);
	
	to=control[0]+1;
	
	for(i = 2; i<to; i++)
	{
		val = defaults_breakout(i, to_panel, control, d);

		if(control[2] == 1)
		{
			ival = val;
			Pulse_Prog_Tab;
			SetCtrlVal(control[0], control[1], ival[0]);
		} else if(control[2] == 2) {
			dval = val;
			SetCtrlVal(control[0], control[1], dval[0]);
		} else if(control[2] == 3) {
			cval = val;
			SetCtrlVal(control[0], control[1], cval);
		} else if(control[2] == 4) {
			ival = val;
			GetNumListItems(control[0], control[1], &max_index);
			if(max_index < ival[0])
				ival[0] = max_index;
			if(max_index > 0)
				SetCtrlIndex(control[0], control[1], ival[0]);
		}
	}
	
	if(!to_panel)
	{
		change_np_or_sr();
		
		nPoints = d->NPoints;
		
		ttl_trigger = d->TriggerTTL; 
		
		if(!d->lastprog)
			LoadProgram_IO (d->loadprogram);
		else
			LoadProgram_IO (d->lastprogrambuffer);
									 		
		Change_Trigger(Pulse_Prog_Tab, PulseProg_Trigger_TTL, EVENT_COMMIT, NULL, NULL, NULL);
		PhaseCycleCallback(Pulse_Prog_Config, PPConfig_PhaseCycle, EVENT_COMMIT, NULL, NULL, NULL);
		ContinuousRunCallback(Pulse_Prog_Tab, PulseProg_ContinuousRun, EVENT_COMMIT, NULL, NULL, NULL);
	}
	
	return 0;
}
		
	

int LoadDefaults (int to_panel, char *filename)
{
	//This program loads the defaults from *filename
	//If to_panel is 1, then it loads the defaults only into the Defaults panel.
	//If to_panel is 0, then it loads the defautls only onto the main panel.
	
	//For convenience's sake, this file just combines LoadDefaults_IO and LoadDefaultstoControls.
	//If I were writing this program again from scratch, I would change LoadDefaultstoControls to be this function
	//And simply require a call to LoadDefaults_IO prior to calling LoadDefaults.
	
	DEFAULTS *d = malloc(sizeof(DEFAULTS));
	
	d = LoadDefaults_IO(to_panel, filename);
	
	LoadDefaultstoControls(to_panel, d);
	
	return 0;
}

int SaveDefaults (char *filename, DEFAULTS *d)
{
	//This function will save a defaults file to *filename.
	//It does this by simply changing the defaults pre-existing at *filename to whatever is currently in the Defaults panel.
	
	int bint = 0;
	char *c = "";
	double dbl = 0.0;

	if(!file_exists(filename))
		Generate_Defaults_File(filename);

	ChangeDefaults(2, &d->NPoints, &dbl, c, filename);
	ChangeDefaults(3, &bint, &d->SamplingRate, c, filename);
	ChangeDefaults(4, &d->InputChannel, &dbl, c, filename);
	ChangeDefaults(5, &d->TriggerChannel, &dbl, c, filename);
	ChangeDefaults(6, &d->TriggerTTL, &dbl, c, filename);
	ChangeDefaults(7, &bint, &dbl, d->pathname,filename);
	ChangeDefaults(8, &bint, &dbl, d->filename,filename);
	ChangeDefaults(9, &d->lastprog, &dbl, c,filename);
	ChangeDefaults(10, &bint, &dbl, d->loadprogram, filename);
	ChangeDefaults(11, &bint, &dbl, d->lastprogrambuffer, filename);
	ChangeDefaults(12, &d->WritetoFile, &dbl, c, filename);
	ChangeDefaults(13, &d->overwritefile, &dbl, c, filename);
	ChangeDefaults(14, &d->SavedSession, &dbl, c, filename);
	ChangeDefaults(15, &d->cont_mode, &dbl, c, filename);
	ChangeDefaults(16, &d->phasecycle, &dbl, c, filename);
	ChangeDefaults(17, &d->phasecycleinstr, &dbl, c, filename);
	ChangeDefaults(18, &d->numcycles, &dbl, c, filename);
	ChangeDefaults(19, &bint, &dbl, d->defaultsloc, filename);
	
	return 0;
}

Generate_Defaults_File(char *location)
{
	//Generates a defaults file in case one doesn't exist.
	
	FILE *defaults;
	defaults = fopen(location, "w+");
	
	fprintf(defaults, "#BoardDefaults#\n");
	fprintf(defaults, "NPoints= %d\n", 5000);
	fprintf(defaults, "SamplingRate= %lf\n", 10000.00);
	fprintf(defaults, "InputChannel= %d\n", 0);
	fprintf(defaults, "TriggerChannel= %d\n", 0);
	fprintf(defaults, "TriggerTTL= %d\n", 23);
	fprintf(defaults, "Path= \n");
	fprintf(defaults, "Filename= %s\n", "Experiments");
	fprintf(defaults, "LastProgram= %d\n", 1);
	fprintf(defaults, "LoadProgram= \n");
	fprintf(defaults, "LastProgramBuffer= %s\n", "LoadProgramBuffer.txt");
	fprintf(defaults, "WriteToFile= %d\n", 1);
	fprintf(defaults, "OverWriteFile= %d\n", 0);
	fprintf(defaults, "SavedSession= %d\n", 0);
	fprintf(defaults, "RunContinuously= %d\n", 0);
	fprintf(defaults, "PhaseCycle= %d\n", 0);
	fprintf(defaults, "PhaseCycleInstr= %d\n", 0);
	fprintf(defaults, "NumCycles= %d\n", 1);
	fprintf(defaults, "DefaultsLocation= %s\n", "Defaults.txt");
	
	fclose(defaults);
	
	return 0;
}

int ChangeDefaults(int ln, int *intvalue, double *dblvalue, char *charvalue, char *filename)
{

	/*
	This function takes an integer ln and changes the value associated with that line in the defaults file *filename.
	If filename does not exist or is not a valid defaults file, the current defaults file is copied over and the line
	is changed.
	
	This function currently takes three inputs because I did not know how to pass generic arguments to the function.
	At some point in the future, I will change the nature,
	
	Lines are numbered as such:
	01 = #BoardDefaults#
	02 = NPoints
	03 = Sampling Rate
	04 = Input Channel
	05 = Trigger Channel
	06 = Trigger TTL
	07 = Path
	08 = Filename
	09 = Use Last Program
	10 = Default Program
	11 = Last Program Buffer
	12 = Write to File
	13 = Over Write File
	14 = Save sessions
	15 = Run Continuously
	16 = Phase Cycle
	17 = Phase Cycle Instr
	18 = Num Cycles
	19 = Defaults Location
	*/
	
	int intpreviousvalue, tisize = 12, takesint[12] = {2, 4, 5, 6, 9, 12, 13, 14, 15, 16, 17, 18}, tcsize = 5, takeschar[5] = {7, 8, 10, 11, 19}, tdsize = 1, takesdouble[1] = {3}, oldstrlen, newstrlen;
	double doublepreviousvalue;
	char *charpreviousvalue = malloc(500), *testchar = malloc(500);
	
	if(ln <= 1 || ln > 19)
		return -1;
	
	int i;
	char *string_buffer = malloc(500), *string_match = malloc(500), *newstring = malloc(500), c;
	
	FILE *fileid, *newfile;
	fpos_t fpos;
	
	if(!file_exists(filename))
		Generate_Defaults_File(filename);
				
	
	fileid = fopen(filename, "r+");	
	
	for (i= 0; i<ln; i++)
		fgets(string_buffer, 500, fileid);
	
	if (isin(ln, takesint, tisize) >=0) {
		if(!sscanf(string_buffer, "%s %d", string_match, &intpreviousvalue))
			return -1;
		
		sprintf(newstring, "%s %d\n", string_match, *intvalue);
	} else if(isin(ln, takesdouble, tdsize) >=0) {
		 if(!sscanf(string_buffer, "%s %lf", string_match, &doublepreviousvalue))
			return -1;
		
		sprintf(newstring, "%s %lf\n", string_match, *dblvalue);
	} else if(isin(ln, takeschar, tcsize)>=0) {
		if(!sscanf(string_buffer, "%s %s", string_match, charpreviousvalue))
			return -1;
		
		for(i = 0; i<strlen(charvalue); i++)
		{
			if(isspace(charvalue[i]))
				charvalue[i] = '\?';

		}
		sprintf(newstring, "%s %s\n", string_match, charvalue);
	} else {
		sprintf(errorstring, "Line number %d is invalid.\n", ln);
		MessagePopup("Error changing defaults", errorstring);
		return -1;
	}
	
	rewind(fileid);
	fclose(fileid);
	
	oldstrlen = strlen(string_buffer);
	newstrlen = strlen(newstring);
	
	fileid = fopen(filename, "r+");
	
	newfile = fopen("DefaultsTemporary000000000001.txt", "w+");

	for(i = 0; i<ln-1; i++)
	{
		fgets(string_buffer, 500, fileid);
		fputs(string_buffer, newfile);
	}
	
	fgets(string_buffer, 500, fileid);
	fputs(newstring, newfile);
	
	for(i = ln; i<19; i++)
	{
		fgets(string_buffer, 500, fileid);
		fputs(string_buffer, newfile);
	}
	
	fclose(newfile);
	fclose(fileid);
	
	remove(filename);
	rename("DefaultsTemporary000000000001.txt", filename);
	
	return 0;
}

void *defaults_breakout(int ln, int defaults_panel, int *output, DEFAULTS *d)
{
	//Returns a pointer to the value in d associated with line ln.
	
	int controls[18][3], i;
	
	if(ln == 1)
	{
		output[0] = 18;
		output[1] = 1;
		output[2] = 0;
		return NULL;
	}
	
	if(ln <1 || ln > 19)
		return NULL;
	
	for(i = 0; i<18; i++)
	{
		controls[i][0] = DefaultsPanel;
		controls[i][2] = 1;
	}

	controls[1][2] = 2;
	controls[5][2] = 3;
	controls[6][2] = 3;
	controls[8][2] = 3;
	controls[9][2] = 3;
	controls[17][2] = 3;
	controls[2][2] = 4;
	controls[3][2] = 4;
	
	if(defaults_panel)
	{
		controls[0][1] = DefaultsP_NPoints;
		controls[1][1] = DefaultsP_SampleRate;
		controls[2][1] = DefaultsP_AcquisitionChannel;
		controls[3][1] = DefaultsP_Trigger_Channel;
		controls[4][1] = DefaultsP_Trigger_TTL;
		controls[5][1] = DefaultsP_Path;
		controls[6][1] = DefaultsP_Filename;
		controls[10][1] = DefaultsP_WriteToFile;
		controls[11][1] = DefaultsP_OverwriteFile;
		controls[13][1] = DefaultsP_ContinuousRun;
		controls[14][1] = DefaultsP_PhaseCycle;
		controls[15][1] = DefaultsP_PhaseCycleInstr;
		controls[16][1] = DefaultsP_NumCycles;
		
	} else {
		for(i = 2; i<=6; i++)
			controls[i][0] = panelHandle;
		
		controls[0][0] = Pulse_Prog_Config;
		controls[1][0] = Pulse_Prog_Config;
		controls[2][0] = Pulse_Prog_Config;
		controls[3][0] = Pulse_Prog_Config;
		controls[4][0] = Pulse_Prog_Tab;
		controls[10][0] = panelHandle;
		controls[11][0] = panelHandle;
		controls[13][0] = Pulse_Prog_Tab;
		controls[14][0] = Pulse_Prog_Config;
		controls[15][0] = Pulse_Prog_Config;
		controls[16][0] = Pulse_Prog_Config;
		
		controls[0][1] = PPConfig_NPoints;
		controls[1][1] = PPConfig_SampleRate;
		controls[2][1] = PPConfig_AcquisitionChannel;
		controls[3][1] = PPConfig_Trigger_Channel;
		controls[4][1] = PulseProg_Trigger_TTL;
		controls[5][1] = MainPanel_Path;
		controls[6][1] = MainPanel_Filename;
		controls[10][1] = MainPanel_WriteToFile;
		controls[11][1] = MainPanel_OverwriteFile;
		controls[13][1] = PulseProg_ContinuousRun;
		controls[14][1] = PPConfig_PhaseCycle;
		controls[15][1] = PPConfig_PhaseCycleInstr;
		controls[16][1] = PPConfig_NumCycles;
	}
	
	controls[7][1] = DefaultsP_UseLast;
	controls[8][1] = DefaultsP_DefaultProgram;
	controls[9][1] = DefaultsP_LastProgramBuff;
	controls[12][1] = DefaultsP_SaveLastConfiguration;
	controls[17][1] = DefaultsP_DefaultsLocation;
	
	output[0] = controls[ln - 2][0];
	output[1] = controls[ln - 2][1];
	output[2] = controls[ln - 2][2];

	if(ln == 2)
		return &d->NPoints;
	if(ln == 3)
		return &d->SamplingRate;
	if(ln == 4)
		return &d->InputChannel;
	if(ln == 5)
		return &d->TriggerChannel;
	if(ln == 6)
		return &d->TriggerTTL;
	if(ln == 7)
		return d->pathname;
	if(ln == 8)
		return d->filename;
	if(ln == 9)
		return &d->lastprog;
	if(ln == 10)
		return d->loadprogram;
	if(ln == 11)
		return d->lastprogrambuffer;
	if(ln == 12)
		return &d->WritetoFile;
	if(ln == 13)
		return &d->overwritefile;
	if(ln == 14)
		return &d->SavedSession;
	if(ln == 15)
		return &d->cont_mode;
	if(ln == 16)
		return &d->phasecycle;
	if(ln == 17)
		return &d->phasecycleinstr;
	if(ln == 18)
		return &d->numcycles;
	if(ln == 19)
		return d->defaultsloc;
	
	return NULL;
}


		case EVENT_COMMIT:
			int val, instr;
			GetCtrlVal(panel, control, &val);
			GetCtrlVal(panel, MDInstr_InstrNum, &instr);
			
			if(val)
			{
				SetCtrlAttribute(panel, MDInstr_Dimension, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, MDInstr_FinalDelay, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, MDInstr_FinalDelayUnits, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, MDInstr_Increment, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, MDInstr_IncrementUnits, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, MDInstr_InitDelay, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, MDInstr_InitDelayUnits, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, MDInstr_NumSteps, ATTR_DIMMED, 0);
				SetCtrlAttribute(inst[instr], PulseInstP_InstDelay, ATTR_DIMMED, 1);
				SetCtrlAttribute(inst[instr], PulseInstP_TimeUnits, ATTR_DIMMED, 1);
				SetCtrlAttribute(inst[instr], PulseInstP_InstDelay, ATTR_MIN_VALUE, 0.0); // Allow instructions to be 0.
				SetCtrlAttribute(panel, MDInstr_FinalDelay, ATTR_MIN_VALUE, 0.0); // Temp
				SetCtrlAttribute(panel, MDInstr_InitDelay, ATTR_MIN_VALUE, 0.0); // Temp
				change_dimension(panel);
				ChangeNumSteps(panel, MDInstr_NumSteps, EVENT_COMMIT, NULL, NULL, NULL);

			} else {
				SetCtrlAttribute(panel, MDInstr_Dimension, ATTR_DIMMED, 1); 
				SetCtrlAttribute(panel, MDInstr_FinalDelay, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, MDInstr_FinalDelayUnits, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, MDInstr_Increment, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, MDInstr_IncrementUnits, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, MDInstr_InitDelay, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, MDInstr_InitDelayUnits, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, MDInstr_NumSteps, ATTR_DIMMED, 1);
				SetCtrlAttribute(inst[instr], PulseInstP_InstDelay, ATTR_DIMMED, 0);
				SetCtrlAttribute(inst[instr], PulseInstP_TimeUnits, ATTR_DIMMED, 0);
				SetCtrlAttribute(inst[instr], PulseInstP_InstDelay, ATTR_MIN_VALUE, 0.000001); // Set to a non-zero value and let it choose appropriately.
				ChangeTUnits(inst[instr], PulseInstP_InstDelay, EVENT_COMMIT, NULL, NULL, NULL); // Let it figure out the right minimum time.
			}

			break;
