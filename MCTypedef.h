/***************Type Definitions********************/

#define MC_FID	0
#define MC_FFT	1
#define MC_XAXIS	0
#define	MC_YAXIS	1
#define	MC_XYAXIS	2
#define MC_RIGHT 0
#define MC_LEFT 1
#define MC_UP 2
#define MC_DOWN 3
#define MC_SNAP_X 0
#define MC_SNAP_Y 1
#define MC_SNAP_XY 2
#define RMS_BORDER_COLOR VAL_RED
#define CM_NO_CUR 0
#define CM_POSITION 1
#define CM_RMS 2
#define CM_AVG 3

/*
typedef struct PPROGRAM
{
	int nPoints;
	int transient;
	int scan;
	int n_inst;
	int ntransients;
	int *flags;
	int *instr;
	int *instr_data;
	int *trigger_scan;
	int trigger_ttl;
	int phasecycle;
	int phasecycleinstr;
	int numcycles;
	
	double *instruction_time;
	double *time_units;
	double delaytime;
	double total_time;
	double samplingrate;
	double cyclefreq;
	
	//2D Program
	int nDimensions;
	int nVaried;
	int firststep;
	int *v_instr_type;
	int *v_instr_num;
	int *dimension;
	int *nSteps;
	int *step;
	
	int *init_id;
	int *inc_id;
	int *final_id;
	
	double *init;
	double *initunits;
	double *inc;
	double *incunits;
	double *final;
	double *finalunits;
					   
	char *filename;
	
    
} PPROGRAM;

typedef struct PINSTR
{
	int flags;
	int instr;
	int instr_data;
	int trigger_scan;
	
	double instruction_time;
	double time_units;
} PINSTR;
*/
