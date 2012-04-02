/********************************************************************************
*                              Private Interface 								*
********************************************************************************/
/********************************** Includes *********************************/
#ifndef DATA_LIB_PRIV_H
#define DATA_LIB_PRIV_H

#include <NIDAQmx.h>
#include <cviddc.h>

// Cache filename.
#define MCD_AVGCACHE_FNAME "AverageCache.mcd"
#define MCD_DATCACHE_FNAME "DataCache.mcd"

// Filename format
#define MCD_FNAME_DATE_FORMAT "%y%m%d"
#define MCD_FNAME_DATE_LEN 6

#define MCD_FNAME_NUM_LEN 4
#define MCD_FNAME_FORMAT "%s-%s-%04d"

#define MCD_FNAME_FIXED_LEN MCD_FNAME_DATE_LEN + MCD_FNAME_NUM_LEN + 2 // Doesn't include null-termination or extension. 

// Data Date Stamp
#define MCD_DATE_FORMAT "%y%m%d"
#define MCD_DATESTAMP_MAXSIZE 6

#define MCD_TIME_FORMAT "%H:%m:%S, %a, %b %d, %Y"
#define MCD_TIMESTAMP_MAXSIZE 27

// Data groups
#define MCD_MAINDATA "[DataGroup]"
#define MCD_AVGDATA "[AvgGroup]"

// Data header
#define MCD_DATAHEADER "[Data Header]"
#define MCD_DATANUM 10

#define MCD_FILENAME "filename"
#define MCD_EXPNAME "ExperimentName"
#define MCD_EXPNUM "ExperimentNum"
#define MCD_HASH "HashCode"  
#define MCD_NCHANS "NumChans"  

#define MCD_DATESTAMP "DateStamp"
#define MCD_TIMESTART "TimeStarted"
#define MCD_TIMEDONE "TimeDone"

#define MCD_CIND "CurrentIndex"
#define MCD_MAXSTEPS "MaxSteps"

// Display header
#define MCD_DISPHEADER "[Display Header]"
#define MCD_DISPNUM 10

#define MCD_POLYFITON "poly_on"
#define MCD_POLYFITORDER "poly_ord"
#define MCD_PHASE "phase"
#define MCD_FFTCHAN "fft_channel"
#define MCD_SGAINS "spec_gains"
#define MCD_SOFFS "spec_offsets"
#define MCD_FGAINS "fid_gains"
#define MCD_FOFFS "fid_offsets"
#define MCD_SCHANSON "spec_chans_on"
#define MCD_FCHANSON "fid_chans_on"

// TDM stuff
#define MCTD_MAINDATA "DataGroup"			// The main data group name.
#define MCTD_AVGDATA "AvgGroup"				// The group of data averages.

#define MCTD_NP "NPoints"					// Number of points (data)
#define MCTD_SR "SamplingRate"				// Sampling rate 
#define MCTD_NT "NTransients"				// Number of transients total
#define MCTD_NDIM "NDims"					// Number of dimensions
#define MCTD_NCYCS "NCycles"				// Number of cycles
#define MCTD_NCHANS "NChans"				// Number of channels
#define MCTD_TIMESTART "TimeStarted"		// Time started
#define MCTD_TIMEDONE "TimeCompleted"		// Time completed

#define MCTD_CSTEP "CurrentStep"			// Current step as a linear index
#define MCTD_CSTEPSTR "CurrentStepString"	// Current step as a string

/*************************** Function Declarations ***************************/
/************** Running Experiment Functions *************/  
extern int run_experiment(PPROGRAM *p);   

extern double *get_data(TaskHandle aTask, int np, int nc, int nt, double sr, int *error, int safe);
extern int prepare_next_step(PPROGRAM *p);
extern int prepare_next_step_safe(PPROGRAM *p);

/*********************** File I/O ************************/   
// Data Saving
extern int initialize_mcd(char *fname, char *basefname, unsigned int num, PPROGRAM *p, unsigned int nc, time_t start, __int64 hash);
extern int initialize_mcd_safe(char *fname, char *basefname, unsigned int num, PPROGRAM *p, unsigned int nc, time_t start, __int64 hash);
extern int update_avg_data_mcd(char *fname, double **avg_data, PPROGRAM *p, double *data, int cind, int nc, int64 hash);
extern int save_data_mcd(char *fname, PPROGRAM *p, double *data, int cind, int nc, time_t done);

// Data Loading


// General Utilities
extern char *make_cstep_str(PPROGRAM *p, int cind, int avg, int *ev);   

// Legacy - TDM
extern int initialize_tdm(void);
extern int initialize_tdm_safe(int CE_lock, int TDM_lock);
extern int save_data(double *data, double **avg);
extern int save_data_safe(double *data, double **avg);

extern double *load_data_tdm(int lindex, DDCFileHandle file, DDCChannelGroupHandle mcg, DDCChannelHandle *chs, PPROGRAM *p, int nch, int *rv);
extern double *load_data_tdm_safe(int lindex, DDCFileHandle file, DDCChannelGroupHandle mcg, DDCChannelHandle *chs, PPROGRAM *p, int nch, int *rv);

extern int get_ddc_channel_groups(DDCFileHandle file, char **names, int num, DDCChannelGroupHandle *cgs);
extern int get_ddc_channel_groups_safe(DDCFileHandle file, char **names, int num, DDCChannelGroupHandle *cgs);

/****************** Experiment Running *******************/
extern int setup_cexp(CEXP *cexp);
extern int add_hash_to_cexp(CEXP *cexp);
extern int get_ct(CEXP *ce);

/****************** Device Interaction *******************/
extern int setup_DAQ_task(void);
extern int setup_DAQ_task_safe(int DAQ_lock, int UIDC_lock, int CE_lock);

extern int setup_DAQ_aouts(void);
extern int setup_DAQ_aouts_safe(int DAQ_lock, int UIPC_lock, int CE_lock);
extern int update_DAQ_aouts(void);
extern int update_DAQ_aouts_safe(int DAQ_lock, int CE_lock);

extern int clear_DAQ_task(void);
extern int clrea_DAQ_task_safe(void);

#endif
