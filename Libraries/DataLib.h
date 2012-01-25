/********************************** Includes *********************************/
#define MCTD_MAINDATA "DataGroup"			// The main data group name.

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

DeclareThreadSafeScalarVar(int, QuitUpdateStatus); 
DeclareThreadSafeScalarVar(int, QuitIdle);
DeclareThreadSafeScalarVar(int, DoubleQuitIdle);
DeclareThreadSafeScalarVar(int, Status);

/************** Running Experiment Functions *************/
extern int CVICALLBACK IdleAndGetData(void *functionData);
extern void CVICALLBACK discardIdleAndGetData(int poolHandle, int functionID, unsigned int event, int value, void *callbackData);

extern int run_experiment(PPROGRAM *p);

extern double *get_data(PPROGRAM *p, int *error);
extern int prepare_next_step(PPROGRAM *p);

/*********************** File I/O ************************/
extern int initialize_tdms(void);
extern int save_data(double *data);

extern int load_experiment(char *filename);
extern double *load_data(int lindex, int *rv);
extern double *load_data_tdms(int lindex, TDMSFileHandle file, TDMSChannelGroupHandle mcg, TDMSChannelHandle *chs, PPROGRAM *p, int cind, int nch, int *rv);

extern int write_data(void);
/********************* UI Interfaces *********************/
extern void add_data_path_to_recent(char *path);
extern void load_data_popup(void);

extern int plot_data(double *data, int np, double sr, int nc);
extern int change_phase(int chan, double phase, int order);
extern void update_experiment_nav(void);
extern void update_transients(void);
extern int calculate_num_transients(int cind, int ind, int nt);
extern void clear_plots(void);
extern int get_selected_ind(int panel, int t_inc, int *step);
extern int calculate_num_transients(int cind, int ind, int nt);

extern void change_fid_chan_col(int num);
extern void change_spec_chan_col(int num);
extern void toggle_fid_chan(int num);
extern void toggle_spec_chan(int num);
extern void update_spec_fft_chan(void);
extern void update_spec_chan_box(void);
extern void update_fid_chan_box(void);
extern void update_chan_ring(int panel, int ctrl, int chans[]);
extern void change_fid_gain(int num);
extern void change_spec_gain(int num);
extern void change_fid_offset(int num);
extern void change_spec_offset(int num);

extern int get_current_fname(char *path, char *fname);

extern int get_devices(void);
extern int load_DAQ_info(void);

extern void toggle_ic(void);
extern void add_chan(char *label, int val);
extern void remove_chan(int val);
extern void change_chan(void);
extern void change_range(void);


/****************** Device Interaction *******************/
extern int setup_DAQ_task(void);
extern int clear_DAQ_task(void);

