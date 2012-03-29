/********************************** Includes *********************************/
#define DATA_LIB_H

#ifndef int64
#define int64 __int64
#endif

#ifndef PULSE_PROGRAM_TYPES_H
#include <PulseProgramTypes.h>
#endif

#ifndef UICONTROLS_H
#include <UIControls.h>
#endif

// Filename format
#define MCD_FNAME_DATE_FORMAT "%y%m%d"
#define MCD_FNAME_DATE_LEN 6

#define MCD_FNAME_NUM_LEN 4
#define MCD_FNAME_FORMAT "%s-%s-%04d"

#define MCD_FNAME_FIXED_LEN MCD_FNAME_DATE_LEN + MCD_FNAME_NUM_LEN + 2 // Doesn't include null-termination or extension. 

// Data Date Stamp
#define MCD_DATE_FORMAT "%y%m%d"
#define MCD_DATESTAMP_MAXSIZE 6

#define MCD_TIME_FORMAT "%H:%m:%s, %a, %b %d, %Y"
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
extern int CVICALLBACK IdleAndGetData(void *functionData);
extern void CVICALLBACK discardIdleAndGetData(int poolHandle, int functionID, unsigned int event, int value, void *callbackData);

extern int run_experiment(PPROGRAM *p);

extern double *get_data(TaskHandle aTask, int np, int nc, int nt, double sr, int *error, int safe);
extern int prepare_next_step(PPROGRAM *p);
extern int prepare_next_step_safe(PPROGRAM *p);

/*********************** File I/O ************************/
extern int initialize_tdm(void);
extern int initialize_tdm_safe(int CE_lock, int TDM_lock);
extern int save_data(double *data, double **avg);
extern int save_data_safe(double *data, double **avg);

extern char *make_cstep_str(PPROGRAM *p, int cind, int avg, int *ev);
extern int initialize_mcd(char *fname, char *basefname, unsigned int num, PPROGRAM *p, unsigned int nc, time_t start, int64 hash);
extern int update_avg_data_mcd(char *fname, PPROGRAM *p, double *data, int cind, int nc, int64 hash);
extern int save_data_mcd(char *fname, PPROGRAM *p, double *data, int cind, int nc, time_t done);


extern int load_experiment(char *filename);
extern int load_experiment_safe(char *filename);
extern double *load_data(char *filename, int lindex, PPROGRAM *p, int avg, int nch, int *rv);
extern double *load_data_safe(char *filename, int lindex, PPROGRAM *p, int avg, int nch, int *rv);
extern double *load_data_tdm(int lindex, DDCFileHandle file, DDCChannelGroupHandle mcg, DDCChannelHandle *chs, PPROGRAM *p, int nch, int *rv);
extern double *load_data_tdm_safe(int lindex, DDCFileHandle file, DDCChannelGroupHandle mcg, DDCChannelHandle *chs, PPROGRAM *p, int nch, int *rv);

extern int get_ddc_channel_groups(DDCFileHandle file, char **names, int num, DDCChannelGroupHandle *cgs);
extern int get_ddc_channel_groups_safe(DDCFileHandle file, char **names, int num, DDCChannelGroupHandle *cgs);

extern void select_data_item(void);
extern void select_directory(char *path);
extern void select_file(char *path);
extern void load_file_info(char *path, char *old_path);
extern void load_file_info_safe(char *path, char *old_path);
extern void update_file_info(char *path);
extern void update_file_info_safe(char *path);

/********************* UI Interfaces *********************/
extern void add_data_path_to_recent(char *path);
extern void add_data_path_to_recent_safe(char *path);
extern void load_data_popup(void);

extern int plot_data(double *data, int np, double sr, int nc);
extern int plot_data_safe(double *data, int np, double sr, int nc);
extern void clear_plots(void);
extern void clear_plots_safe(void);
extern int change_phase(int chan, double phase, int order);
extern int change_phase_safe(int chan, double phase, int order);

extern void update_experiment_nav(void);
extern void update_experiment_nav_safe(void);
extern void update_transients(void);
extern void update_transients_safe(void);
extern void set_nav_from_lind(int lind, int panel, int avg);
extern void set_nav_from_lind_safe(int lind, int panel, int avg);
extern void set_data_from_nav(int panel);
extern void set_data_from_nav_safe(int panel);

extern int calculate_num_transients(int cind, int ind, int nt);
extern int get_selected_ind(int panel, int t_inc, int *step);
extern int get_selected_ind_safe(int panel, int t_inc, int *step);
extern int calculate_num_transients(int cind, int ind, int nt);

extern void change_fid_chan_col(int num);
extern void change_fid_chan_col_safe(int num);
extern void change_spec_chan_col(int num);
extern void change_spec_chan_col_safe(int num);
extern void change_fid_gain(int num);
extern void change_fid_gain_safe(int num);
extern void change_fid_offset(int num);
extern void change_fid_offset_safe(int num);
extern void change_spec_gain(int num);
extern void change_spec_gain_safe(int num);
extern void change_spec_offset(int num);
extern void change_spec_offset_safe(int num);

extern void toggle_fid_chan(int num);
extern void toggle_fid_chan_safe(int num);
extern void toggle_spec_chan(int num);
extern void toggle_spec_chan_safe(int num);

extern void update_chan_ring(int panel, int ctrl, int chans[]);
extern void update_fid_chan_box(void);
extern void update_fid_chan_box_safe(void);
extern void update_spec_chan_box(void);
extern void update_spec_chan_box_safe(void);
extern void update_spec_fft_chan(void);
extern void update_spec_fft_chan_safe(void);

extern int get_current_fname(char *path, char *fname, int next, time_t *cdate);
extern char *get_full_path(char *path, char *fname, char *ext, int *ev);

extern int get_devices(void);
extern int get_devices_safe(void);
extern int load_AO_info(void);
extern int load_AO_info_safe(void);
extern int load_DAQ_info(void);
extern int load_DAQ_info_safe(int UIDC_lock, int UIPC_lock, int DAQ_lock);

extern void toggle_ic(void);
extern void toggle_ic_safe(void);
extern void add_chan(char *label, int val);
extern void add_chan_safe(char *label, int val);
extern void remove_chan(int val);
extern void remove_chan_safe(int val);
extern void change_chan(void);
extern void change_range(void);
extern void change_range_safe(void);


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

