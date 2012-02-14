/******************************** Includes *******************************/
#include <cvitdms.h>
#include <cviddc.h>

#define PP_V_ID 1 		// Varies in an indirect dimension linearly.
#define PP_V_PC 2		// Varies in a phase cycle
#define PP_V_BOTH 3		// Varies according to both
#define PP_V_ID_EXPR 4	// The indirect dimension variation is according to an expression
#define PP_V_ID_ARB 8	// The indirect dimension variation is according to an arbitrary table

#define PF_DATA 1		// Takes a data argument
#define PF_DELAY 2		// Takes a delay argument

#define MC_DEF_MODE 0	// File is in define mode
#define MC_DAT_MODE 1	// File is in data mode.

// File type
#define MCTD_TDMS 0		// TDMS
#define MCTD_TDM 1		// TDM


// Properties of the Programs group
#define MCTD_PGNAME	"ProgramGroup"			// Name of the group

#define MCTD_PNP "nPoints"					// p.np
#define MCTD_PSR "SampleRate"				// p.sr
#define MCTD_PNT "nTransients"				// p.nt
#define MCTD_PTMODE "TransAcqMode"			// p.tfirst
#define MCTD_PSCAN "Scan"					// p.scan
#define MCTD_PTRIGTTL "TriggerTTL"			// p.trigger_ttl
#define MCTD_PTOTTIME "TotalTime"			// p.totaltime
#define MCTD_PNINSTRS "nInstrs"				// p.n_inst
#define MCTD_PNUINSTRS "nUniqueInstrs"		// p.nUniqueInstrs

#define MCTD_PVAR "Varied"					// p.varied
#define MCTD_PNVAR "nVaried"				// p.nVaried
#define MCTD_PNDIMS "nDims"					// p.nDims
#define MCTD_PNCYCS "nCycles"				// p.nCycles
#define MCTD_PSKIP "Skip"					// p.skip
#define MCTD_PMAX_N_STEPS "MaxNSteps"		// p.max_n_steps
#define MCTD_PREAL_N_STEPS "RealNSteps"		// p.real_n_steps

#define MCTD_NFUNCS "nFuncs"				// p.nFuncs
#define MCTD_TFUNCS "tFuncs"				// p.tFuncs

// Program Instruction Details
#define MCTD_PROGFLAG "ProgFlag"			// p.instrs[i].flags;
#define MCTD_PROGTIME "ProgTime"			// p.instrs[i].instr_delay
#define MCTD_PROGINSTR "ProgInstr"			// p.instrs[i].instr;
#define MCTD_PROGUS "ProgUS"				// p.instrs[i].units and p.instrs[i].scan
#define MCTD_PROGID "ProgID"				// p.instrs[i].instr_data;

// Varying program details
#define MCTD_PMAXSTEPS "MaxSteps"			// p.maxsteps
#define MCTD_PVINS "VarIns"					// p.v_ins
#define MCTD_PVINSDIM "VarInsDims"			// p.v_ins_dim
#define MCTD_PVINSMODE "VarInsMode"			// p.v_ins_mode
#define MCTD_PVINSLOCS "VarInsLocs"			// p.v_ins_locs
#define MCTD_PSKIPLOCS	"SkipLocs"			// p.skip_locs
#define MCTD_PSKIPEXPR "SkipExpr"			// p.skip_expr
#define MCTD_PDELEXPRS "DelayExprs"			// p.delay_exprs
#define MCTD_PDATEXPRS "DataExprs"			// p.data_exprs

#define MCTD_PFUNCLOCS "FuncLocs"			// p.func_locs

// Function details
#define MCTD_PF_NAME "FuncNames"			// pfunc.name
#define MCTD_PF_RF "FuncResFlags"			// pfunc.r_flags
#define MCTD_PF_NINSTR "FuncNInstrs"		// pfunc.n_instr
#define MCTD_PF_ARGMODE "FuncArgMode"		// pfunc.argmode

// Function instruction details
#define MCTD_PFINSTR "FuncInstr"			// pfunc.instrs[i].instr
#define MCTD_PFFLAG "FuncFlag"				// pfunc.instrs[i].flags
#define MCTD_PFID "FuncID"					// pfunc.instrs[i].instr_data;
#define MCTD_PFUS "FuncUS"					// pfunc.instrs[i].time_units and scan

// Some defines that are useful for read_status
#define PB_STOPPED 1		// Bit 0 = Stopped
#define PB_RESET 2			// Bit 1 = Reset
#define PB_RUNNING 4		// Bit 2 = Running
#define PB_WAITING 8		// Bit 3 = Waiting


// Some UI parameters.
#define MCUI_DEL_PREC 5		// Precision for delay controls.

/************************* Function Declarations *************************/

/************** File I/O ****************/
extern int SavePulseProgram(char *filename, PPROGRAM *p);
extern PPROGRAM *LoadPulseProgram(char *filename, int *err_val);

extern int get_name(char *pathname, char *name, char *ending);
extern int save_program(DDCChannelGroupHandle pcg, PPROGRAM *p);
PPROGRAM *load_program(DDCChannelGroupHandle pcg, int *err_val);
PPROGRAM *tdms_load_program(TDMSChannelGroupHandle pcg, int *err_val);

extern char *generate_nc_string(char **strings, int numstrings, int *len);
extern char **get_nc_strings(char *string, int *nss);

extern void display_ddc_error(int err_val);
extern void display_tdms_error(int err_val);

/******** Pulseblaster Functions ********/
extern int load_pb_info(int verbose);
extern int program_pulses(PINSTR *ilist, int n_inst, int verbose);
extern int program_pulses_safe(PINSTR *ilist, int n_inst, int verbose);
extern int update_status(int verbose);
extern int update_status_safe(int verbose);

/************ UI Interfaces *************/
extern PPROGRAM *get_current_program(void);
extern PPROGRAM *get_current_program_safe(void);
extern void set_current_program(PPROGRAM *p);
extern void set_current_program_safe(PPROGRAM *p);

extern void load_prog_popup(void);
extern void save_prog_popup(void);
extern void add_prog_path_to_recent(char *path);
extern void add_prog_path_to_recent_safe(char *path);

extern void get_pinstr_array(PINSTR **ins_ar, PPROGRAM *p, int *cstep);

extern int ui_cleanup(int verbose);
extern int ui_cleanup_safe(int verbose);
extern void change_instr_units(int panel);
extern void clear_program(void);
extern void clear_program_safe(void);

// Basic Program Setup
extern void change_np_or_sr(int np_sr_at);
extern void change_nt(void);

// Get Instruction Parameters
extern void get_instr(PINSTR *instr, int num);
extern void get_instr_panel(PINSTR *instr, int panel);
extern int get_flags(int num);
extern int get_flags_panel(int panel);
extern int get_flags_range(int panel, int start, int end);

// Set Instruction Parameters
extern int set_instr(int num, PINSTR *instr);
extern void set_instr_panel(int panel, PINSTR *instr);
extern void set_scan(int num, int state);
extern void set_scan_panel(int panel, int state);
extern void change_units(int panel);
extern void change_instr_delay(int panel);
extern void change_instr_data(int panel);
extern void change_instruction(int num);
extern void change_instruction_panel(int panel);
extern void swap_ttl(int to, int from);
extern void move_ttl(int num, int to, int from);
extern void move_ttl_panel(int panel, int to, int from);
extern void change_trigger_ttl(void);
extern void change_trigger_ttl_safe(void);
extern void set_ttl_trigger(int panel, int ttl, int on);
extern void set_flags(int num, int flags);
extern void set_flags_panel(int panel, int flags);
extern void set_flags_range(int panel, int flags, int start, int end);
extern int ttls_in_use(void);
extern int ttls_in_use_safe(void);

// Get ND and Phase Cycling Instruction Parameters
extern int get_nd_state(int num);
extern int get_pc_state(int num); 
extern void get_updated_instr(PINSTR *instr, int num, int *cstep, int *cind, int *dind, int mode);
extern void get_updated_instr_safe(PINSTR *instr, int num, int *cstep, int *cind, int *dind, int mode); 

// Set ND Instruction Parameters
extern void set_ndon(int ndon);
extern void set_ndon_safe(int ndon);
extern void update_nd_state(int num, int state);
extern void update_nd_state_safe(int num, int state);
extern void set_instr_nd_mode(int num, int nd);
extern void change_num_dims(int num);
extern void change_num_dims_safe(int num);
extern void change_num_dim_steps(int dim, int steps);
extern void change_num_dim_steps_safe(int dim, int steps);
extern void change_nd_steps_instr(int num, int steps);
extern void change_nd_steps_instr_safe(int num, int steps);
extern void change_dimension(int num);
extern void change_dimension_safe(int num);
extern void populate_dim_points(void);
extern void populate_dim_points_safe(void);
extern void update_nd_increment(int num, int mode);
extern void update_nd_increment_safe(int num, int mode);
extern void update_nd_from_exprs(int num);
extern void update_nd_from_exprs_safe(int num);
extern void update_skip_condition(void);
extern void update_skip_condition_safe(void);
extern char *get_tooltip(int skip);

// Set Phase Cycling Parameters
extern void update_pc_state(int num, int state);
extern void update_pc_state_safe(int num, int state);
extern void change_num_cycles(void);
extern void change_num_cycles_safe(void);
extern void change_cycle(int num);
extern void change_cycle_safe(int num);
extern void change_cycle_step(int num);
extern void change_cycle_step_safe(int num);
extern void change_cycle_num_steps(int cyc, int steps);
extern void change_cycle_num_steps_safe(int cyc, int steps);
extern void update_cyc_instr(int num, PINSTR *instr, int step);
extern void update_cyc_instr_safe(int num, PINSTR *instr, int step);
extern void populate_cyc_points(void);
extern void populate_cyc_points_safe(void);

// Expanded Phase Cycle Functions
extern void set_phase_cycle_expanded(int num, int state);
extern void set_phase_cycle_expanded_safe(int num, int state);
extern int setup_expanded_instr(int num, int step);
extern int setup_expanded_instr(int num, int step);
extern void resize_expanded(int num, int steps);
extern void save_all_expanded_instructions(void);
extern void save_all_expanded_instructions_safe(void);
extern void save_expanded_instructions(int num);
extern void save_expanded_instructions_safe(int num);
extern void save_expanded_instruction(int num, int step);
extern void save_expanded_instruction_safe(int num, int step);
extern void move_expanded_instruction(int num, int to, int from);
extern void move_expanded_instruction_safe(int num, int to, int from);
extern void delete_expanded_instruction(int num, int step);
extern void delete_expanded_instruction_safe(int num, int step);

// Instruction Manipulation
extern int move_instruction(int to, int from);
extern int move_instruction_safe(int to, int from);
extern void clear_instruction(int num);
extern void clear_instruction_safe(int num);
extern void change_number_of_instructions(void);
extern void change_number_of_instructions_safe(void);
extern void delete_instruction(int num);
extern void delete_instruction_safe(int num);

// Analog Output Manipulation
extern void change_num_aouts(void);
extern void change_num_aouts_safe(void);
extern void delete_aout(int num);
extern void delete_aout_safe(int num);
extern void clear_aout(int num);

extern void change_ao_device(int num);
extern void change_ao_device_safe(int num);
extern void change_ao_chan(int num);
extern void change_ao_chan_safe(int num);
extern void populate_ao_dev(int num);
extern void populate_ao_dev_safe(int num);
extern void populate_ao_chan(int num);
extern void populate_ao_chan_safe(int num);

extern void set_aout_dimmed(int num, int dimmed, int nd);
extern void set_aout_dimmed_safe(int num, int dimmed, int nd);
extern void set_aout_nd_dimmed(int num, int dimmed);
extern void set_aout_nd_dimmed_safe(int num, int dimmed);
extern void set_ao_nd_state(int num, int state);
extern void set_ao_nd_state_safe(int num, int state);
extern int get_ao_nd_state(int num);

extern void change_ao_val(int num);
extern void change_ao_val_safe(int num);
extern void update_ao_increment(int num, int mode);
extern void update_ao_increment_safe(int num, int mode);
extern void change_ao_steps_instr(int num, int steps);
extern void change_ao_steps_instr_safe(int num, int steps);
extern void change_ao_dim(int num);										 
extern void change_ao_dim_safe(int num);

// User-Defined Function Interfaces
extern pfunc *get_pfunc(int func_index);
extern int get_reserved_flags(int func_index);
extern int takes_instr_data(int func_index);
extern int instr_data_min(int func_index);

/********** Struct Manipluation **********/
extern void create_pprogram(PPROGRAM *p);
extern void free_pprog(PPROGRAM *p);

extern int pinstr_cmp(PINSTR *pi1, PINSTR *pi2);
extern PINSTR *copy_pinstr(PINSTR *instr_in, PINSTR *instr_out);

extern PINSTR *generate_instructions(PPROGRAM *p, int *cstep, int *n_inst);

extern int pprogramcpy(PPROGRAM *p_f, PPROGRAM *p_t);
extern int insert_instruction(PPROGRAM *p, PINSTR *instr, int num); 

/********** General Utilities ************/
// Linear indexing
extern int get_maxsteps(int *maxsteps);
extern int get_lindex(int *cstep, int *maxsteps, int size);
extern int get_cstep(int lindex, int *cstep, int *maxsteps, int size);

// Loop manipulation
extern int find_end_loop(int instr);
extern int find_end_loop_instrs(int instr, PINSTR *instrs, int n_instrs);
extern int in_loop(int instr, int big);

// Control manipulation
extern void change_visibility_mode(int panel, int *ctrls, int num, int mode);
extern void change_control_mode(int panel, int *ctrls, int num, int mode);

// Array manipulation
extern void remove_array_item(int *array, int index, int num_items);
extern void remove_array_item_void(void *array, int index, int num_items, int type);
extern int constant_array_double(double *array, int size);
extern int constant_array_int(int *array, int size);
extern int constant_array_pinstr(PINSTR **array, int size);
extern int get_index(int *array, int val, int size);
extern int realloc_if_needed(char *array, int len, int new_len, int inc);

// Sorting
extern void sort_linked(int **array, int num_arrays, int size);
extern int __cdecl qsort_comp_array(const void *a, const void *b);

// String manipulation
extern char **generate_char_num_array(int first, int last, int *elems);
extern char *generate_expression_error_message(char *err_message, int *pos, int size);

// Math
int calculate_units(double val);
int get_precision(double val, int total_num);
extern int get_bits(int in, int start, int end);
extern int get_bits_in_place(int in, int start, int end);
extern int move_bit_skip(int in, int skip, int to, int from);
extern int move_bit(int in, int to, int from);

// Math Evalution
extern int get_update_error(int err_code, char *err_message);
extern constants *setup_constants(void);
extern void update_constants(constants *c, int *cstep);

/*************** Vestigial ***************/
extern int get_vdim(int panel, int varyctrl, int dimctrl);
extern int get_vsteps(int panel, int varyctrl, int nsteps);



