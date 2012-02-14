 /**************************** Preprocessing *****************************/
// General info about the program
#define MC_VERSION 1.0						// The current version

//// Root
// Elements
#define MCXML_RETAG "SSRoot"				// Root element tag
#define MCXML_PREFS "Preferences"			// Preferences element
#define MCXML_GEN "General"					// General Info
#define MCXML_PPC "PulseProgConfig"			// Pulse program config info
#define MCXML_DDISP "DataDisp"				// Data display info

// Attributes
#define MCXML_VER "Version"					// Version title.

//// Preferences
// Elements
#define MCXML_NPSRAT "NPSRATPrefs"			// Preferences on what happens when you change np, sr or at
#define MCXML_TVIEW "TransientView"			// Preference for what transient to show on data acquisition.

//// General
#define MCXML_ACTIVETAB "ActiveTab"			// Active tab index.
#define MCXML_DFBNAME "BaseDataFName" 		// Base filename for data files
#define MCXML_DFPATH "DataPath"				// Data path
#define MCXML_DLPATH "DataLoadPath"			// Data path for loading data files.
#define MCXML_DDESC "DataDescription"		// Data file description
#define MCXML_PPATH "ProgPath"				// Program path.
#define MCXML_LFINFO "LoadFileInfo"			// Load file info -> boolean

//// PulseProgConfig
// Elements
#define MCXML_DEV "Device"					// Device name
#define MCXML_PBDEV "PulseBlaster_Dev"		// Spincore device index - int
#define MCXML_CHANSON "ChansOn"				// Acquisition channels on (pas la chanson)
#define MCXML_CHANNAMES "ChanNames"			// The names of the channels that are on.
#define MCXML_CHANRANGE "ChanRanges"		// Ranges of the channels - ints.
#define MCXML_CURCHAN "CurrentChan"			// Current channel selected - has index.
#define MCXML_COUNTCHAN "CounterChan"		// Counter channel
#define MCXML_TRIGCHAN "TrigChannel"		// Trigger channel
#define MCXML_BROKETTLS "BrokenTTLS"		// Broken TTLS - flags

// Attributes
#define MCXML_TRIGTTL "TriggerTTL"			// The trigger ttl.
#define MCXML_INDEX "Index"					// Index attribute of anything that takes indexes
#define MCXML_NUM "Number"					// Number attribute for anything with a number
#define MCXML_TRIGEDGE "TrigEdge"			// Trigger edge - rising or falling

//// Data Display
// Elements
#define MCXML_FID "FID"						// Parent element for all FID info
#define MCXML_SPEC "Spectrum"				// Parent element for all Spectrum info
#define MCXML_COLORS "Colors"				// Colors for the channels
#define MCXML_GAINS "Gains"					// Gains for the channels
#define MCXML_OFF "Offsets"					// Offsets for the channels
			  
// Attributes
#define MCXML_AUTOSCALE "Autoscale"			// Whether or not autoscale is on.
/************************* Function Declarations *************************/
// UI Manipulation
extern int load_ui(char *uifname);
extern void setup_broken_ttls(void);
extern void setup_broken_ttls_safe(void);

extern void ppconfig_popout(void);
extern int CVICALLBACK ppconfig_popin(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

// Struct Initialization
extern void initialize_program(void);
extern void initialize_data(void);
extern void initialize_uicontrols(void);
extern void initialize_ce(void);
extern int *get_broken_ttl_ctrls(void);

// File I/O
extern int save_session(char *filename);
extern int load_session(char *filename);
extern void display_xml_error(int err);


