/******************* Constant Definitions *******************/
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

#define MC_HIDDEN 1 
#define MC_DIMMED 2

#define MC_INIT 0
#define MC_INC 1
#define MC_FINAL 2

#define INSTR_HEIGHT 30
#define INSTR_GAP 5

/******************* Structure Definitions *******************/
// Structures for containing pointers to UI objects
// pc - > Structure for indexing the UI controls and panels related to pulse programming
struct {
	// Panels
	int PProgPan; 		// Pulse Program Controls Panel
	int PProgCPan; 		// Pulse Program Container Panel
	int PPConfigPan; 	// ND Controls Panel
	int PPConfigCPan; 	// ND Instruction Container
	int AOutPan;		// Analog output panel
	int AOutCPan;		// Analog output container panel (not in tab)

	int *inst; 			// Instruction Subpanel
	int *cinst; 		// ND Instruction Subpanel
	int *ainst;			// Analog output instruction.
	
	// Controls on the inst panels
	int ins_num;		// Instruction Number
	int instr; 			// Instruction (Continue, Stop, etc)
	int instr_d;		// Instruction data
	int delay;			// The instruction delay
	int delayu;			// The instruction delay units;
	int pcon;			// Phase cycling on/off LED
	int pcstep;			// Phase cycle step
	int pclevel;		// Phase cycle level
	int pcsteps;		// Number of phase cycle steps
	int scan;			// Scan control
	int TTLs[24]; 		// TTL Controls
	
	int uparrow;		// Up arrow
	int downarrow;		// Down arrow
	int xbutton;		// Delete control.
	int expandpc;		// Expand phase cycled instr.
	int collapsepc;		// Collapse phase cycled instr.

	//	Controls on the cinst panels
	int cins_num; 		// Instruction Num control on cinst
	int	cinstr;			// Instruction control on cinst
	
	int del_init; 		// Initial delay control
	int disp_init;		// Initial delay display (total time, double)
	int delu_init;		// Initial delay units control
	int	dat_init;		// Initial data
	
	int del_inc;		// Delay increment control
	int disp_inc;		// Increment display (total time, double)
	int delu_inc;		// Delay increment units control
	int	dat_inc;		// Data increment control
	
	int del_fin;		// Final delay control
	int disp_fin;		// Final display (total time, double)
	int	delu_fin;		// Final delay units control
	int dat_fin;		// Final data control
	
	int cexpr_delay;	// Delay increment expression control (text)
	int cexpr_data;		// Data increment expression control
	
	int nsteps;			// Number of dim steps control
	int dim;			// Number of dimensions control
	int vary;			// Vary instruction control

	//	Controls in the Pulse Program Tab
	int trig_ttl[2]; 	// Trigger TTL control
	int ninst[2];		// Number of instructions control
	int	rc[2];			// Run continuously control
	int numcycles[2];	// Number of phase cycles
	int trans[2];		// Current transient control
	int idlabs[8][2];	// ID labels
	int idrings[8][2];	// ID ring controls
	
	// Controls in the Pulse Program Config Tab
	int ndon[2];		// ND On LED
	int ndims[2];		// Number of dimensions control
	int tfirst[2];		// Transient acquisition mode
	int sr[2];			// Sampling rate control
	int nt[2];			// Number of transients control
	int np[2];			// Number of points control
	int at[2];			// Acquisition time control
	int dev[2];			// Device control
	int pbdev[2];		// Pulseblaster device ring.
	int nc[2];			// Number of channels control
	int ic[2];			// Input channels control
	int cc[2];			// Counter channel control
	int curchan[2];		// Current channel control
	int range[2];			// Maximum value control
	int trigc[2];		// Trigger channel ring control
	int	trige[2];		// Trigger edge ring control
	int skip[2];		// Skip condition on/off LED control
	int skiptxt[2];		// Skip condition control (text)
	
	int timeest[2];		// Estimated time control
	
	// Analog output window controls
	int anum[2];		// Number of analog output channels.
	
	int ainitval;		// Initial value for the output channel.
	int aincval;		// Increment value for the output channel.
	int aincexpr;		// Increment expression
	int afinval;		// Final value
	int asteps;			// Number of steps in the given dimension
	int adim;			// Dimension it varies along
	int andon;			// Whether or not it varies.
	int aodev;			// Analog output device.
	int aochan;			// Analog output channel.
	
	// What we need to load new panels.
	char *uifname;
	int pulse_inst;
	int md_inst;
	int a_inst;
} pc;

// dc -> Structure for indexing the UI controls/panels related to data acquisition
struct {
	// Panels
	int fid;			// FID panel
	int spec;			// Spectrum panel
	int cloc[2];		// Current location in acquisition space (implemented twice)
						// cloc[0] = FID, cloc[1] = Spectrum
	
	// Controls on the FID tab
	int fgraph;			// FID Graph control
	int fxpos;			// FID x position (double)
	int fypos;			// FID y position (double)
	int fauto;			// FID Autoscale LED
	
	int fchans[8];		// FID Array of channel LEDs.
	
	int fcring;			// FID Channel preferences ring control
	int fccol;			// FID Channel color numeric
	int fcgain;			// FID Channel gain preference (double)
	int fcoffset;		// FID Channel offset preference control (double)
	int fpolysub;		// FID Polynomial subtraction LED
	int fpsorder;		// FID Polynomial subtraction order control (int)
	
	
	// Controls on the Spectrum tab
	int sgraph;			// Spectrum Graph control
	int sxpos;			// Spectrum x position (double)
	int sypos;			// Spectrum y position (double)
	int sauto;			// Spectrum Autoscale LED
	
	int schans[8];		// Spectrum Array of channel LEDs.
	
	int scring;			// Spectrum Channel preferences ring control
	int sccol;			// Spectrum Channel color numeric
	int scgain;			// Spectrum Channel gain preference (double)
	int scoffset;		// Spectrum Channel offset preference control (double)
	int spolysub;		// Spectrum Polynomial subtraction LED
	int spsorder;		// Spectrum Polynomial subtraction order control (int)

	int sphase[2];			// Spectrum phase knob 
	int sphorder[2];		// Spectrum phase order
	int sfftring[2];		// Choose FFT Type
	
	
	// Controls on the location child panels
	int fcloc;			// Current location panel for fid
	int scloc;			// Current location panel for spec
	
	int	ctrans;			// Transient ring control
	int idrings[8];		// ID rings
	int idlabs[8];		// ID labels

	
} dc;

// mc -> Structure for indexing the main panel
struct {
	// Panels
	int mp;					// The main panel
	
	// Menu Bars
	int mainmenu;			// Main menu bar
	int rcmenu; 			// Right click menu
	
	// Main panel controls
	int mtabs[2];			// The main tabs.
	int pathb[2];			// Path selection button
	int path[2];			// Path text control
	int basefname[2];		// Base filename text control
	int cdfname[2];			// Current filename.
	int datadesc[2];		// Data description text box.
	int datafbox[2];		// Data file browser
	int ldatamode[2];		// Load file info from browser LED.
	
	int cprog[2];			// Current program ring control
	int cprogdesc[2];		// Current program description.
	
	int pbrun[2];			// Green PulseBlaster Status LED
	int pbwait[2];			// Yellow PulseBlaster Status LED
	int pbstop[2];			// Red Pulseblaster Status LED
	int mainstatus[2];		// Main status LED
	
	int startbut[2];		// Start button
	int stopbut[2];			// Stop button
	
	// Menu controls.
	// Parent menus:
	int file;				// File parent menu
	int view;				// View parent menu
	int setup;				// Setup parent menu
	
	// File controls
	int fnew;				// File > New (Submenu)
	int fnewacq;			// File > New > New Acquisition
	int fnewprog;			// File > New > New Program
	int fsave;				// File > Save (Submenu)
	int fsavedata;			// File > Save > Save Data
	int fsaveprog;			// File > Save > Save Program
	int fload;				// File > Load
	int floaddata;			// File > Load > Load Data
	int floadrecdat;		// File > Load > Load Recent Data (Submenu)
	int floadprog;			// File > Load > Load Program
	int floadrecprog;		// File > Load > Load Recent Program (Submenu)
	int fquit;				// File > Quit
	
	// View controls
	int vpchart;			// File > View Program Chart
	int vtview;				// File > Transient View (Submenu)
	int vtviewopts[3];		// File > Transient View > Average / View Latest Transient / No Change (that order)
	int vcstep;				// File > Change Dimension
	
	// Setup controls
	int supdaq;				// Setup > Update DAQ
	int ssaveconfig;		// Setup > Save Current Config
	int ssaveconfig_file;	// Setup > Save Current Config to File
	int sloadconfig;		// Setup > Load Config from File
	int sbttls;				// Setup > Setup Broken TTLS

} mc;

// uipc -> contains data about the pulse program side of the current configuration of the UI
struct {
	int ni;						// Number of instructions visible
	int max_ni;					// Maximum number of instructions that have been created
	int trigger_ttl;			// Current trigger TTL.
	int broken_ttls;			// Flags for if a given TTL is broken.
	double static_time;			// Static portion of the time estimate.
	double dynamic_time;		// The portion of the time est. from varied instrs.
	double total_time;			// Total time this should take.
	
	// Multidimensional info
	int nd;						// Number of indirect dimensions
	int max_nd;					// Maximum dimensions that have been stored
	int ndins;					// Number of instructions on in some dimension.
	int *dim_steps;				// Number of steps per dimension
	int *dim_ins;				// Instructions which vary along some dimension
	int *ins_dims;				// The dimension they vary along
	int *ins_state;				// The mode of operation (expression or linear)

	int **nd_data;				// The instr_data, Outermost size ->ins_dims
	double **nd_delays;			// The delays, in ns. Outermost size ->ndins
	
	int err_dat;				// If there was an error in the data evaluation, this is it
	int err_dat_size;			// Size of the sampling space when the error was generated
	int *err_dat_pos;			// The cstep it happened on.
	int err_del;				// Same as above, but with the delay evaluation.
	int err_del_size;			// Size of the sampling space when the error was generated.
	int *err_del_pos;			// Again, same.
	
	int real_n_steps;			// Real steps after skipping
	int max_n_steps;			// Maximum number of steps
	int skip_err;				// Skip error
	int skip_err_size;			// Size of the position array; 
	int *skip_err_pos;			// Position of the skip error.
	unsigned char *skip_locs;	// Skip locations.
	
	// Phase cycling info
	int nc;						// Number of cycles
	int max_nc;					// Maximum number of cycles that have been stored
	int ncins;					// Number of cycled instructions.
	int *cyc_steps; 			// Number of steps per cycle
	int *cyc_ins;				// Instructions which are cycled
	int *ins_cycs;				// The cycle level of each instruction
	
	int n_cyc_pans;				// Size of the cyc_pans array.
	int *max_cinstrs;			// Maximum number of cycled instructions per instruction
	int **cyc_pans;				// Handles for cycle panels in expanded mode.
	PINSTR ***c_instrs;			// The instructions for each phase cycle.
								// Outermost size is ninst, it's easiest that way and
								// the memory isn't much.
	
	// Analog Output info
	int anum;					// Number of analog output instructions.
	int max_anum;				// Maximum number of analog outputs.

	char *ppath;				// The default directory for the base program.
	
} uipc;

// uidc - > Not necessarily visible information about the ui.
struct {	   
	int nt;					// Number of transients
	int nd;					// Number of dimensions in the data section.
	double sr;				// Sampling rate of the data (needed for phase correction)
	
	int fnc;				// Number of FID channels on
	int snc;				// Number of Spectrum channels on
	int schan;				// Spectrum channel -> Real = 0, Imag = 1, Mag = 2
	int fchans[8];			// Which FID channels are on.
	int schans[8];			// Which spectrum channels are on
	int fplotids[8];		// What the plotids for each plot are. -1 = None
	int splotids[8][3];		// Same as above, but for spectrum (2D array for real, imag, mag)
	
	float sphase[8][3];		// The phase correction for the spectra, up to 2nd order
	
	int fcol[8];			// Colors for the FID
	int scol[8];			// Colors for the spectrum
	
	int disp_update;		// Display update pref: 0 = Show avg, 1 = Latest Transient, 2 = No Change
	
	float fgain[8];			// Gains for the FID
	float sgain[8];			// Gains for the spectrum
	float foff[8];			// Offsets for the FID.
	float soff[8];			// Offsets for the spectrum

	int devindex;			// Device index
	
	int *chans;				// Channels on array of flag ints, size is nchans / 32
	int nchans;				// Total number of channels available.
	int onchans;			// Number of channels on.
	
	float range[8];			// Ranges for each channel - 8 possible values.
	
	char *dlpath;					// Default data directory.
} uidc;

// ce -> Structure containing information about the currently running experiment.
struct {
	TaskHandle aTask; 		// Signal acquisition task
	TaskHandle cTask;		// Counter task.
	
	int update_thread;		// Update thread id.
	
	PPROGRAM *p;			// Current program
	
	int nchan;				// Number of channels.
	int t_first;			// Transients first or indirect aq first?
							// Three options: 	0 -> ID first, then advance transients.
							//					1 -> All transients first, then ID
							//					2 -> Phase Cycles first, then IDs, then repeat as necessary
	
	char *path;				// Full pathname of where to save the data
	char *fname;			// Experiment filename.
	char *desc;				// Description of the experiment.
	
	char *ctname;			// Counter task name
	char *atname;			// Acquisition task name
	
	char *ccname; 			// Counter channel name
	char **icnames; 		// Input channel names
	
	int ctset;				// Whether or not the counter task has been set
	int atset;				// Whether or not the acquisition task has been set.

	int ct;					// Current transient
	int *cstep;				// Current step

	int ninst;				// Number of instructions in this run
	PINSTR *ilist;			// List of instructions for this run.

	CVIAbsoluteTime tstart;	// Time started
	CVIAbsoluteTime tdone; 	// Time that the most recent part was completed.
	
	// File saving info
	int cind;					// Current index in acquisition
	
} ce;

// af - > Contains all the functions that have been set up.
struct {
	pfunc **pfuncs;			// The functions themselves.
	
	int nFuncs;				// Number of user defined functions
	int *func_locs;			// An index from instruction index to function 
} af; 
