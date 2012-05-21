/********************************************************************************
*                              Public Interface 								*
********************************************************************************/
#ifndef PULSE_PROGRAM_LIB_H
#define PULSE_PROGRAM_LIB_H

// Some defines that are useful for read_status
#define PB_STOPPED 1		// Bit 0 = Stopped
#define PB_RESET 2			// Bit 1 = Reset
#define PB_RUNNING 4		// Bit 2 = Running
#define PB_WAITING 8		// Bit 3 = Waiting

#define MC_TMODE_ID 0	// ID First
#define MC_TMODE_TF 1	// Transients first
#define MC_TMODE_PC	2	// Phase cycles first

// Some UI parameters.
#define MCUI_DEL_PREC 5		// Precision for delay controls.

/************************* Function Declarations *************************/
extern PPROGRAM *LoadPulseProgram(char *fname, int safe, int *ev);
extern int SavePulseProgram(PPROGRAM *p, char *fname, int safe);

extern PPROGRAM *load_pprogram(FILE *f, int *ev);
extern int save_pprogram(PPROGRAM *p, FILE *f);

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
extern void set_scan(int num, int state);
extern void set_scan_panel(int panel, int state);

extern void change_instr_delay(int panel);
extern void change_instr_data(int panel);

extern void change_instruction(int num);
extern void change_instruction_panel(int panel);

extern void change_fr_instr(int num);
extern void change_fr_instr_pan(int panel);

extern void change_trigger_ttl(void);
extern void change_trigger_ttl_safe(void);

extern void set_ttl_trigger(int panel, int ttl, int on);

extern int ttls_in_use(void);
extern int ttls_in_use_safe(void);

extern void swap_ttl(int to, int from);
extern void swap_ttl_safe(int to, int from);

// Get ND and Phase Cycling Instruction Parameters
extern int get_nd_state(int num);
extern int get_pc_state(int num); 

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

// Expanded Phase Cycle Functions  
extern void set_phase_cycle_expanded(int num, int state);
extern void set_phase_cycle_expanded_safe(int num, int state);

extern void move_expanded_instruction(int num, int to, int from);
extern void move_expanded_instruction_safe(int num, int to, int from);

extern void delete_expanded_instruction(int num, int step);
extern void delete_expanded_instruction_safe(int num, int step);

// Instruction Manipulation
extern int move_instruction(int to, int from);
extern int move_instruction_safe(int to, int from);

extern int move_fr_inst(int to, int from);
extern int move_fr_inst_safe(int to, int from);

extern void clear_instruction(int num);
extern void clear_instruction_safe(int num);

extern void clear_fr_instr(int num);

extern void clear_fr_instr(int num);
extern void clear_fr_instr_safe(int num);

extern void change_number_of_instructions(void);
extern void change_number_of_instructions_safe(void);

extern void change_fr_num_instrs(int num);
extern void change_fr_num_instrs_safe(int num);

extern void delete_instruction(int num);
extern void delete_instruction_safe(int num);

extern void delete_fr_instr(int num);
extern void delete_fr_instr_safe(int num);

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

// Math Evalution
extern int get_update_error(int err_code, char *err_message);

/********** Struct Manipluation **********/
extern void create_pprogram(PPROGRAM *p);
extern void free_pprog(PPROGRAM *p);

extern PINSTR null_pinstr(void);
extern int pinstr_cmp(PINSTR *pi1, PINSTR *pi2);
extern PINSTR *copy_pinstr(PINSTR *instr_in, PINSTR *instr_out);

extern PINSTR *generate_instructions(PPROGRAM *p, int cind, int *n_inst);

extern int get_transient(PPROGRAM *p, int cind);
extern int get_transient_from_step(PPROGRAM *p, int *step);

extern int get_cyc_step(PPROGRAM *p, int cind, int *cyc_step);
extern int get_dim_step(PPROGRAM *p, int cind, int *dim_step);
extern int get_var_ind(PPROGRAM *p, int cind);

extern int convert_lindex(PPROGRAM *p, int index, int old_mode, int new_mode);

extern int pprogramcpy(PPROGRAM *p_f, PPROGRAM *p_t);
extern int insert_instruction(PPROGRAM *p, PINSTR *instr, int num); 

/********** General Utilities ************/
// Linear indexing
extern int get_maxsteps(int *maxsteps);

extern int *get_steps_array(int tmode, int *maxsteps, int nc, int nd, int nt);
extern int get_steps_array_size(int tmode, int nc, int nd);

// Loop manipulation
extern int find_end_loop(int instr);
extern int find_end_loop_instrs(int instr, PINSTR *instrs, int n_instrs);
extern int in_loop(int instr, int big);

// Control manipulation
extern void change_visibility_mode(int panel, int *ctrls, int num, int mode);
extern void change_control_mode(int panel, int *ctrls, int num, int mode);

// PINSTR
extern int constant_array_pinstr(PINSTR **array, int size);

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

#endif
