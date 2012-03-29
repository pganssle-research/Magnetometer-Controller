// PPConversion library
#ifndef PP_CONVERSION_H
#define PP_CONVERSION_H

#include <PulseProgramTypes.h>

// Includes
#include <cvitdms.h>
#include <cviddc.h>

// Macro definitions.
// Properties of the Programs group
// File type
#define MCTD_TDMS 0		// TDMS
#define MCTD_TDM 1		// TDM

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

#define MCTD_NAOUT "nAouts"					// p.nAouts
#define MCTD_NAOVAR "n_aout_var"			// p.n_ao_var

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

// Analog output details
#define MCTD_AOVAR "ao_varied"				// p.ao_varied
#define MCTD_AOVALS "ao_vals"				// p.ao_vals
#define MCTD_AODIM	"ao_dim"				// p.ao_dim     

// Analog output properties - on MCTD_AOVAR
#define MCTD_AOCHANS "ao_chans"				// p.ao_chans
#define MCTD_AOEXPRS "ao_exprs"				// p.ao_exprs

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

// Function declarations
extern int SavePulseProgramDDC(char *filename, int safe, PPROGRAM *p);
extern PPROGRAM *LoadPulseProgramDDC(char *filename, int safe, int *err_val);

extern int save_programDDC(DDCChannelGroupHandle pcg, PPROGRAM *p);
extern int save_programDDC_safe(DDCChannelGroupHandle pcg, PPROGRAM *p);
PPROGRAM *load_programDDC(DDCChannelGroupHandle pcg, int *err_val);
PPROGRAM *load_programDDC_safe(DDCChannelGroupHandle pcg, int *err_val);

extern void display_ddc_error(int err_val);

#endif
