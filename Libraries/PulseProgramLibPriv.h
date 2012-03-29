/********************************************************************************
*                              Private Interface 								*
********************************************************************************/
#ifndef PULSE_PROG_LIB_PRIV_H
#define PULSE_PROG_LIB_PRIV_H

/******************************** Includes *******************************/
#include <PulseProgramTypes.h>
#include <FileSaveTypes.h>
#include <MathParserLib.h>

#define PP_V_ID 1 		// Varies in an indirect dimension linearly.
#define PP_V_PC 2		// Varies in a phase cycle
#define PP_V_BOTH 3		// Varies according to both
#define PP_V_ID_EXPR 4	// The indirect dimension variation is according to an expression
#define PP_V_ID_ARB 8	// The indirect dimension variation is according to an arbitrary table

#define PF_DATA 1		// Takes a data argument
#define PF_DELAY 2		// Takes a delay argument

#define MC_DEF_MODE 0	// File is in define mode
#define MC_DAT_MODE 1	// File is in data mode.


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
#define MCPP_USE_PB "use_pb"
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

#define MCPP_PROPSNUM 20
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
// File I/O
extern fsave generate_header(PPROGRAM *p, int *ev);
extern fsave generate_instr_array(PPROGRAM *p);
extern fsave generate_ao(PPROGRAM *p, int *ev);
extern fsave generate_nd(PPROGRAM *p, int *ev);
extern fsave generate_skip_fs(PPROGRAM *p, int *ev);

extern PINSTR *read_pinstr_from_char(char *array, int n_inst, int *ev);

// UI functions
extern int ui_cleanup(int verbose);
extern int ui_cleanup_safe(int verbose);

// Set Instruction Parameters
extern int set_instr(int num, PINSTR *instr);
extern void set_instr_panel(int panel, PINSTR *instr);
extern void change_units(int panel);
extern void move_ttl(int num, int to, int from);
extern void move_ttl_panel(int panel, int to, int from);

extern void set_flags(int num, int flags);
extern void set_flags_panel(int panel, int flags);
extern void set_flags_range(int panel, int flags, int start, int end);

// Get ND and Phase Cycling Instruction parameters
extern void get_updated_instr(PINSTR *instr, int num, int *cstep, int *cind, int *dind, int mode);
extern void get_updated_instr_safe(PINSTR *instr, int num, int *cstep, int *cind, int *dind, int mode); 

// Set ND and Phase Cycling Instruction parameters
extern void populate_dim_points(void);
extern void populate_dim_points_safe(void);

// Set Phase Cycling Paramters
extern void update_cyc_instr(int num, PINSTR *instr, int step);
extern void update_cyc_instr_safe(int num, PINSTR *instr, int step);
extern void populate_cyc_points(void);
extern void populate_cyc_points_safe(void);

// Expanded Phase Cycle Functions
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

// User-Defined Function Interfaces
extern pfunc *get_pfunc(int func_index);
extern int get_reserved_flags(int func_index);
extern int takes_instr_data(int func_index);
extern int instr_data_min(int func_index);

// Math Evalution
extern constants *setup_constants(void);
extern constants *setup_constants_safe(void);
extern void update_constants(constants *c, int *cstep);

// Utilities
extern void get_pinstr_array(PINSTR **ins_ar, PPROGRAM *p, int *cstep);

extern int *get_steps_array(int tmode, int *maxsteps, int nc, int nd, int nt);

extern int get_steps_array_size(int tmode, int nc, int nd);



#endif
