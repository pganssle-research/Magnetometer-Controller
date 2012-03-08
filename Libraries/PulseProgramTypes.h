/************************* Structure Definitions *************************/
typedef struct PINSTR // This is a structure for containing individual instructions.
{
	int flags;				// TTL flags. If a TTL is on, then flags & (TTL number) == 2^(TTL number), else 0
	int instr;				// The instruction
	int instr_data;			// The instruction data
	int trigger_scan;		// Whether or not you trigger a scan
	
	double instr_time;		// The instruction time
	int time_units;			// Instruction units. The multiplier is (1000)^(time_units), so ns = 0, us = 1, etc
} PINSTR;

typedef struct pfunc 		// Pulse program function
{
	char *name; 			// The name of the function
	
	int r_flags; 			// Reserved flags
	int n_instr; 			// Number of instructions
	int argmode;			// Whether or not this takes an instr_data argument
	PINSTR **instrs; 		// Array of instructions.
} pfunc;

typedef struct PPROGRAM // This is a structure for containing information about pulse programs.
{
	// The basics
	int np; 				// np in direct dimension
	double sr; 				// sr in direct dimension
	int nt;					// Number of transients
	int trigger_ttl; 		// According to this program, which flag corresponds to the trigger TTL?
	
	int tmode;				// Three options: 	MC_TMODE_ID	-> ID first, then advance transients. (Data acquisition)
							//					MC_TMODE_TF -> All transients first, then ID
							//					MC_TMODE_PC -> Phase Cycles first, then IDs, then repeat as necessary
	
	
	int scan; 				// Does this program have a scan argument in it?
	int varied; 			// Is it varied?
	
	int n_inst;				// How many instructions are we looking at
	
	double total_time; 		// How long the whole thing will take.
															  
	PINSTR **instrs; 		// Array of instructions
	int nUniqueInstrs; 		// Number of unique instructions (size of *instrs)
	
	// Variation (indirect dimensions and phase cycling)
	int nDims;		 		// How many dimensions (1+#indirect dimensions)
	int nCycles; 			// Number of cycles
	int nVaried; 			// Number of varied instructions (phase cycles or dimensions)
	int max_n_steps;		// Maximum number of steps possible  This is maxsteps[0]*maxsteps[1]*...*maxsteps[end]
	int real_n_steps;		// Actual number of steps, taking into account the skip condition

	int	skip;				// A boolean indicating if a skip condition is implemented
	char *skip_expr;		// A string containing the expression used to generate the skip file
	
	char **delay_exprs;		// An array of strings representing the delay expressions
	char **data_exprs;		// Same, but with data. Both are "" if unused.
	
	
	// The following two indices are little-endian and of size numDims+1
	// They are of the form [{position in transient space} {position in indirect sampling space}]
	int steps_size;			// Size of steps
	int *maxsteps; 			// Maximum position in the sampling space
	int *steps;				// Linear indexing dimension sizes based on tmode.
							// MC_TMODE_ID: [{dim1, ..., dimn}, nt]
							// MC_TMODE_TF: [nt, {dim1, ..., dimn}]
							// MC_TMODE_PC: [{pc1, ..., pcn}, {dim1, ..., dimn}, nt/(prod(pc[:]))]
	
	int *v_ins; 			// Index of which instructions are varied.
	int *v_ins_dim;			// Dimension/cycle along which it's varied. Encoded bitwise, cycles first
	int *v_ins_mode;		// The mode in which they are varied. PP_V_ID = indirect, PP_V_PC = phase cycling, PP_V_BOTH = both.
	
	int **v_ins_locs; 		// A multi-dimensional array stored as a 2D array indicating, for each varied instruction,
					  		// which instruction in the instruction array its spot in instr_locs should be pointing at.
							// The locations are stored as the values. The array will be of size [nVaried][total_num_steps]
							// When c_step is, say, [a b c] and maxsteps is [d e f], the index you want to look at is
							// v_ins_locs[a+b*d+c*(d*e)]; Example: c_step = [1 3 1] maxstep = [4 5 8] index = [1+12+20] = [33]
							// This array has total size max_n_steps*nVaried
	
	int *skip_locs;			// A linear-indexed array of size max_n_steps saying whether to skip that point in sampling space				
	
	// Analog outputs
	int nAout;				// Number of analog output channels
	int n_ao_var;			// Number of varied channels
	int *ao_varied;			// Whether or not it's varied					Size = [nAout]
	int *ao_dim;			// Dimension along which it varies.				Size = [nAout]
	double **ao_vals;		// Analog output values.						Size = [nAout][(ao_varied)?dim_steps[ao_dim[i]]:1]
	char **ao_chans;		// Full name of the channel for each thing.		Size = [nAout]
	char **ao_exprs;		// The expression generating the thing			Size = [nAout], null if ao_varied[i] != 2;
	
	
	// Function stuff
	int nFuncs;				// Number of functions included
	int tFuncs;				// Number of functions available when the program was saved.
	
	int *func_locs;			// The index corresponding the function. This is necessary because instructions are stored as ints.
	pfunc **funcs;			// The functions themselves. 
	
} PPROGRAM;
