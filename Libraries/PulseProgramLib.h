/******************************** Includes *******************************/
// Some defines that are useful for read_status
#define PB_STOPPED 1		// Bit 0 = Stopped
#define PB_RESET 2			// Bit 1 = Reset
#define PB_RUNNING 4		// Bit 2 = Running
#define PB_WAITING 8		// Bit 3 = Waiting

#define PP_V_ID 1 		// Varies in an indirect dimension linearly.
#define PP_V_PC 2		// Varies in a phase cycle
#define PP_V_BOTH 3		// Varies according to both
#define PP_V_ID_EXPR 4	// The indirect dimension variation is according to an expression
#define PP_V_ID_ARB 8	// The indirect dimension variation is according to an arbitrary table

#define PF_DATA 1		// Takes a data argument
#define PF_DELAY 2		// Takes a delay argument

#define MC_DEF_MODE 0	// File is in define mode
#define MC_DAT_MODE 1	// File is in data mode.

#define MC_TMODE_ID 0	// ID First
#define MC_TMODE_TF 1	// Transients first
#define MC_TMODE_PC	2	// Phase cycles first

// File type
#define MCTD_TDMS 0		// TDMS
#define MCTD_TDM 1		// TDM

// Some UI parameters.
#define MCUI_DEL_PREC 5		// Precision for delay controls.

// Pulse Program Saving Macros:
// Main Groups
#define MCPP_PROGHEADER "[PulseProgram]"

#define MCPP_GROUPSNUM 5

// Header names
#define MCPP_PROPHEADER "[Properties]"
#define MCPP_VERSION "Version"
#define MCPP_NP "np"
#define MCPP_SR "sr"
#define MCPP_NT "nt"
#define MCPP_TRIGTTL "trigger_ttl"
#define MCPP_TMODE "tmode"
#define MCPP_SCAN "scan"
#define MCPP_VARIED "varied"
#define MCPP_NINST "n_inst"
#define MCPP_TOTALTIME "total_time"
#define MCPP_NUINSTRS "nUniqueInstrs"
#define MCPP_NDIMS "nDims"
#define MCPP_NCYCS "nCycles"
#define MCPP_NVARIED "nVaried"
#define MCPP_MAXNSTEPS "max_n_steps"
#define MCPP_REALNSTEPS "real_n_steps"
#define MCPP_SKIP "skip"
#define MCPP_NAOUT "nAout"
#define MCPP_NAOVAR "n_ao_var"

#define MCPP_PROPSNUM 19
#define MCPP_PROPORD 0

// Instructions
#define MCPP_INSTHEADER "[Instructions]"
#define MCPP_INSTORD 1
#define MCPP_INSTNUMFIELDS 6

#define MCPP_INST_FLAGS "flags"		// Field names
#define MCPP_INST_INSTR "instr"
#define MCPP_INST_INSTRDATA "instr_data"
#define MCPP_INST_TRIGGERSCAN "trigger_scan"
#define MCPP_INST_INSTRTIME "instr_time"
#define MCPP_INST_TIMEUNITS "time_units"

#define MCPP_INST_NAMES {MCPP_INST_FLAGS, MCPP_INST_INSTR, MCPP_INST_INSTRDATA, MCPP_INST_TRIGGERSCAN, MCPP_INST_INSTRTIME, MCPP_INST_TIMEUNITS}
#define MCPP_INST_TYPES {FS_INT, FS_INT, FS_INT, FS_UCHAR, FS_DOUBLE, FS_UCHAR}

// AOut names
#define MCPP_AOHEADER "[AnalogOutput]"
#define MCPP_AONUM 5
#define MCPP_AOVARIED "ao_varied"
#define MCPP_AODIM	"ao_dim"
#define MCPP_AOVALS "ao_vals"
#define MCPP_AOCHANS "ao_chans"
#define MCPP_AOEXPRS "ao_exprs"

#define MCPP_AOORD 2

// Multidimensional
#define MCPP_NDHEADER "[ND/PC]"
#define MCPP_NDNUM 8
#define MCPP_MAXSTEPS "maxsteps"
#define MCPP_STEPS "steps"
#define MCPP_VINS "v_ins"
#define MCPP_VINSDIM "v_ins_dim"
#define MCPP_VINSMODE "v_ins_mode"
#define MCPP_VINSLOCS "v_ins_locs"
#define MCPP_DELAYEXPRS "delay_exprs"
#define MCPP_DATAEXPRS "data_exprs"

#define MCPP_NDORD 3

// Skip
#define MCPP_SKIPHEADER "[Skip]"
#define MCPP_SKIPNUM 2
#define MCPP_SKIPEXPR "skip_expr"
#define MCPP_SKIPLOCS "skip_locs"

#define MCPP_SKIPORD 4

/************************* Function Declarations *************************/
extern PPROGRAM *LoadPulseProgram(char *fname, int safe, int *ev);
extern PPROGRAM *load_pprogram(FILE *f, int *ev);

extern PINSTR *read_pinstr_from_char(char *array, int n_inst, int *ev);

extern int SavePulseProgram(PPROGRAM *p, char *fname, int safe);
extern int save_pprogram(PPROGRAM *p, FILE *f);

extern fsave generate_header(PPROGRAM *p, int *ev);
extern fsave generate_instr_array(PPROGRAM *p);
extern fsave generate_ao(PPROGRAM *p, int *ev);
extern fsave generate_nd(PPROGRAM *p, int *ev);
extern fsave generate_skip_fs(PPROGRAM *p, int *ev);


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

extern int *get_steps_array(int tmode, int *maxsteps, int nc, int nd, int nt);
extern int get_steps_array_size(int tmode, int nc, int nd);

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
extern void swap_ttl_safe(int to, int from);
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
extern void resize_expanded_safe(int num, int steps);
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
extern void move_aout(int to, int from);
extern void move_aout_safe(int to, int from);
extern void clear_aout(int num);

extern void change_ao_device(int num);
extern void change_ao_device_safe(int num);
extern void change_ao_chan(int num);
extern void change_ao_chan_safe(int num);
extern void change_ao_chan_uipc(int num);
extern void change_ao_chan_uipc_safe(int num);
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
extern char *get_ao_full_chan_name(int dev, int chan);
extern char *get_ao_full_chan_name_safe(int dev, int chan);
extern void get_ao_dev_chan(char *name, int *dev, int *chan);
extern void get_ao_dev_chan_safe(char *name, int *dev, int *chan);

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

extern PINSTR *generate_instructions(PPROGRAM *p, int cind, int *n_inst);
extern int get_transient(PPROGRAM *p, int cind);
extern int get_transient_from_step(PPROGRAM *p, int *step);
extern int get_cyc_step(PPROGRAM *p, int cind, int *cyc_step);
extern int get_dim_step(PPROGRAM *p, int cind, int *dim_step);
extern int get_var_ind(PPROGRAM *p, int cind);

extern int pprogramcpy(PPROGRAM *p_f, PPROGRAM *p_t);
extern int insert_instruction(PPROGRAM *p, PINSTR *instr, int num); 

/********** General Utilities ************/
// Linear indexing
extern int get_maxsteps(int *maxsteps);

// Loop manipulation
extern int find_end_loop(int instr);
extern int find_end_loop_instrs(int instr, PINSTR *instrs, int n_instrs);
extern int in_loop(int instr, int big);

// Control manipulation
extern void change_visibility_mode(int panel, int *ctrls, int num, int mode);
extern void change_control_mode(int panel, int *ctrls, int num, int mode);

// PINSTR
extern int constant_array_pinstr(PINSTR **array, int size);

// Math Evalution
extern int get_update_error(int err_code, char *err_message);
extern constants *setup_constants(void);
extern constants *setup_constants_safe(void);
extern void update_constants(constants *c, int *cstep);

/************** Safe Pulseblaster Functions *************/
extern int pb_initialize(int verbose);
extern int pb_init_safe(int verbose);
extern int pb_start_programming_safe(int verbose, int device);
extern int pb_stop_programming_safe(int verbose);
extern int pb_inst_pbonly_safe(int verbose, unsigned int flags, int inst, int inst_data, double length);
extern int pb_close_safe(int verbose);
extern int pb_read_status_or_error(int verbose);
extern int pb_read_status_safe(int verbose);
extern int pb_start_safe(int verbose);
extern int pb_stop_safe(int verbose);
