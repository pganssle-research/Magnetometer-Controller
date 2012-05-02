/********************************************************************************
*                              Public Interface 								*
********************************************************************************/
#ifndef DATA_LIB_H
#define DATA_LIB_H

#include <PulseProgramTypes.h>

typedef struct dheader {
	// Type for containing information from a data header
	// Much of this information will be stored in a CEXP.
	
	char *filename; 			// Path of the data
	char *expname;				// Experiment name
	int num;					// Indexing of experiments with the same name
	__int64 hash;				// Unique identifier
	
	char *dstamp;				// Date stamp
	time_t tstarted;			// Time started
	time_t tdone;				// Time the last part of the experiment finished
	
	unsigned int nchans;		// Number of channels.
	unsigned int cind;			// Current index.
	unsigned int *maxsteps; 	// Maximum steps (see PPROGRAM)
	
	int valid;
} dheader;
	

/*************************** Function Declarations ***************************/
/************** Running Experiment Functions *************/

extern int CVICALLBACK IdleAndGetData(void *functionData);
extern void CVICALLBACK discardIdleAndGetData(int poolHandle, int functionID, unsigned int event, int value, void *callbackData);

/*********************** File I/O ************************/
extern int load_experiment(char *filename, int prog);
extern int load_experiment_safe(char *filename, int prog);

extern int load_experiment_tdm(char *filename);
extern int load_experiemnt_tdm_safe(char *filename);

extern double ****load_all_data_file(FILE *f, PPROGRAM *p, dheader *dhead, int *ev);
extern double *load_data_fname(char *filename, int lindex, PPROGRAM *p, int *ev);
extern double *load_data_file(FILE *f, int lindex, PPROGRAM *p, int *ev);

extern double *load_data(char *filename, int lindex, PPROGRAM *p, int avg, int nch, int *rv);
extern double *load_data_safe(char *filename, int lindex, PPROGRAM *p, int avg, int nch, int *rv);

// Data parsing
double ****free_data_4d(double ****data, int nt, int ds, int nc);

dheader load_dataheader_file(FILE *f, int *ev);
dheader free_dh(dheader *d);
dheader null_dh(void);

extern int *parse_cstep(char *step, int *ev);

// File Navigation
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

extern void set_fid_chan(int num, int on);
extern void set_fid_chan_safe(int num, int on);

extern void toggle_fid_chan(int num);
extern void toggle_fid_chan_safe(int num);

extern void set_spec_chan(int num, int on);
extern void set_spec_chan_safe(int num, int on);

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

extern int toggle_ic_ind(int ind);
extern int toggle_ic_ind_safe(int ind);

extern void toggle_ic(void);
extern void toggle_ic_safe(void);

extern void add_chan(char *label, int val);
extern void add_chan_safe(char *label, int val);

extern void remove_chan(int val);
extern void remove_chan_safe(int val);

extern void change_chan(void);

extern void change_range(void);
extern void change_range_safe(void);

#endif
