/********************************************************************
*   																*
*				Session Saving Library - Private					*
*																	*
********************************************************************/
#ifndef SAVE_SESSION_PRIV_H
#define SAVE_SESSION_PRIV_H

// This includes only functions which are not exposed to other libraries
#include <cvixml.h> 
#include <PulseProgramTypes.h>
#include <Version.h>

/**************************** Preprocessing *****************************/


/********************************************************
 *						Root							*
 ********************************************************
 Element Tree:
 <MCXML_ROOT
  *MCXML_VER
  *MCMXL_VNUM>
 	
  	<MCXML_GEN />
	
	<MCXML_PREFS />
	
	<MCXML_PPC />
	
	<MCXML_DDISP />
  
 </MCXML_ROOT>

*******************************************************/
// Element
#define MCXML_ROOT "Root"					// The root element

// Attributes
#define MCXML_VER "Version"					// Version title
#define MCXML_VNUM "VersionNum"				// Version number

// Reused Elements
#define MCXML_CHANNEL "Channel"				// Element representing a channel - value is the name.

// Reused Attributes
#define MCXML_NUM "Number"					
#define MCXML_ON "On"
#define MCXML_INDEX "Index"
#define MCXML_NAME "Name"
#define MCXML_CURCHAN "CurrentChan"			// Current channel selected - has index.

/********************************************************
 *						General							*
 ********************************************************
 Element Tree:
 <MCXML_GEN
  *MCXML_ACTIVETAB>
 
 	<MCXML_DATAFILE
	 *MCXML_BFNAME
	 *MCXML_FPATH
	 *MCXML_LPATH
	 *MCXML_LFINFO>
		
	 	- Data Description -
	
	</MCXML_DATAFILE>
	
	<MCXML_PROGFILE
	 *MCXML_LPATH 
	 *MCXML_FPATH />  -> Only for now - add similar rich text.
  
 </MCXML_GEN>

*******************************************************/

// Elements
#define MCXML_GEN "General"					// General Info
#define MCXML_DATAFILE "DataFile"			// Info about the data file
#define MCXML_PROGFILE "ProgFile"			// Info about the prog file

// Attributes
#define MCXML_ACTIVETAB "ActiveTab"			// Active tab index.

#define MCXML_BFNAME "BaseFName"			// Base File Name
#define MCXML_FPATH "FilePath"				// File's full path
#define MCXML_LPATH "LoadPath"				// Path to load new things from
#define MCXML_LFINFO "LoadFileInfo"			// Load file info -> boolean

/********************************************************
 *					Preferences							*
 ********************************************************
 Element Tree:
 <MCXML_PREFS
  *MCXML_TVIEW>
 
 	<MCXML_NPSRAT
	 *MCXML_NP
	 *MCXML_SR
	 *MCXML_AT />
	 
 </MCXML_PREFS>

*******************************************************/
// Elements
#define MCXML_PREFS "Preferences"			// Preferences element

#define MCXML_NPSRAT "NPSRATPrefs"			// Preferences on what happens when you change np, sr or at

// Attributes
#define MCXML_TVIEW "TransientView"			// Preference for what transient to show on data acquisition.

#define MCXML_NPPREF "np"
#define MCXML_SRPREF "sr"
#define MCXML_ATPREF "at"


/********************************************************
 *			Pulse Program Configuration					*
 ********************************************************
 Element Tree:
 <MCXML_PPC>
 
   	<MCMXL_PBDEV
     *MCXML_INDEX
	 *MCXML_TRIGTTL
	 *MCXML_BROKENTTLS />
   
	<MCXML_DEV		    -> Currently used device
	 *MCXML_NAME 		-> Name of the device
     *MCXML_INDEX
	 *MCXML_NCHANSON  	-> Number of channels on
	 *MCXML_CURCHAN >	-> Index of the current channel
	
		<MCXML_TRIGCHAN
	  	 *MCXML_INDEX
	 	 *MCXML_TRIGEDGE>
			- Trigger Channel Name -
		</MCXML_TRIGCHAN>
		
		<MCXML_COUNTCHAN
		 *MCXML_INDEX>
			-Counter channel name-
		</MCXML_COUNTCHAN>
		
		<MCXML_CHANNEL
	 	 *MCXML_RANGE
	 	 *MCXML_INDEX>
	 		-Channel Name-
		</MCXML_CHANNEL>
	 
	</MCXML_DEVICE>
 
 </MCXML_PPC>

*******************************************************/
// Elements
#define MCXML_PPC "PulseProgConfig"

#define MCXML_DEV "Device"					// Device name
#define MCXML_COUNTCHAN "CounterChan"		// Counter channel
#define MCXML_TRIGCHAN "TrigChannel"		// Trigger channel

// Attributes
#define MCXML_TRIGTTL "TriggerTTL"			// The trigger ttl.
#define MCXML_PBDEV "PulseBlaster_Dev"		// Spincore device index - int
#define MCXML_BROKENTTLS "BrokenTTLS"		// Broken TTLS - flags
#define MCXML_NCHANSON "NumChansOn"			// Number of channels on
#define MCXML_RANGE "Range"					// Channel range
#define MCXML_TRIGEDGE "TrigEdge"			// Trigger edge - rising or falling

/********************************************************
 *					Data Display						*
 ********************************************************
 Element Tree:
 <MCXML_DATADISP
  *MCXML_POLYON 			-> If polynomial fitting is on
  *MCXML_POLYORD>			-> Polynomial fit order
 	
  	<MCXML_FID
 	 *MCXML_NCHANSON 			-> Number of channels displayed
	 *MCXML_AUTOSCALE
	 *MCXML_CURCHAN>
		<MCXML_CHANNEL
		 *MCXML_INDEX
		 *MCXML_ON			-> If it should be displayed
		 *MCXML_COLOR
		 *MCXML_GAIN
		 *MCXML_OFFSET> -Channel Name, If Applicable- </MCXML_CHANNEL>
	</MCXML_FID>
	
	<MCXML_SPEC
	*MCXML_NCHANSON 			-> Number of channels displayed
	*MCXML_AUTOSCALE		
	*MCXML_NPHASEORDERS
	*MCMXL_CURCHAN>
		<MCXML_CHANNEL
		 *MCXML_INDEX
		 *MCXML_ON
		 *MCXML_COLOR
		 *MCXML_GAIN
		 *MCXML_OFFSET>
		 	<MCXML_PHASE
			 *MCXML_PHASEORDER>-Correction-</MCXML_PHASE>
		</MCXML_CHANNEL>
	</MCXML_SPEC>
	
	<MCXML_EP>
		<MCXML_AGAIN
		*MCXML_ON>--Value--</MCXML_AGAIN>
		
		<MCXML_RES
		*MCXML_ON>--Value--</MCXML_RES>
	
		<MCXML_X
		*MCXML_UNITS
		*MCXML_UBASE 
		*MCXML_UBASEN />
		
		<MCXML_Y
		*MCXML_UNITS
		*MCXML_UBASE 
		*MCXML_UBASEN />
		
		<MCXML_F
		*MCXML_UNITS
		*MCXML_UBASE 
		*MCXML_UBASEN />
		
		<MCXML_S
		*MCXML_UNITS
		*MCXML_UBASE 
		*MCXML_UBASEN />
		
		<MCXML_CAL
		*MCXML_UNITS
		*MCXML_UBASE
		*MCXML_UBASEN > --Value-- </MCXML_CAL>
	</MCXML_EP>
</MCXML_DATADISP>

*******************************************************/
// Elements
#define MCXML_DDISP "DataDisp"				// Data display info

#define MCXML_FID "FID"						// Parent element for all FID info
#define MCXML_SPEC "Spectrum"				// Parent element for all Spectrum info

#define MCXML_PHASE "Phase"					// Element representing a phase of a given order.

#define MCXML_EP "ExperimentParams"

#define MCXML_AGAIN "AmpGain"
#define MCXML_RESVAL "ResistorVal"
#define MCXML_X "X"
#define MCXML_Y "Y"
#define MCXML_F "F"
#define MCXML_S "S"
#define MCXML_CAL "Calibration"

// Attributes
#define MCXML_AUTOSCALE "Autoscale"			// Whether or not autoscale is on.

// Spectrum Attributes 
#define MCXML_NPHASEORDERS "NPhaseOrders"	// Number of phase orders
#define MCXML_POLYON "PolyOn"				// Polynomial fitting on -bool
#define MCXML_POLYORD "PolyOrd" 			// Polynomial fitting order

// Channel attributes					// Also has attribute "Num", for index
#define MCXML_COLOR "Color"					// Colors for the channels
#define MCXML_GAIN "Gain"					// Gains for the channels
#define MCXML_OFFSET "Offset"				// Offsets for the channels

// Phase Attributes
#define MCXML_PHASEORDER "Order"			// Phase order

// Unit Attributres
#define MCXML_UNITS "Units"					// Name of the unit
#define MCXML_UBASE "UnitBase"				// Base of the unit -> numeric
#define MCXML_UBASEN "UnitBaseName"			// Name of the unit base

/************************* Function Declarations *************************/    
extern char *get_element_val(CVIXMLElement elem, char *buff, int *blen, int *ev);
extern char *get_attribute_val(CVIXMLElement elem, char *att_name, char *buff, int *blen, int *ev);
extern int get_attribute_float_val(CVIXMLElement elem, char *att_name, float *val, float dflt_val);
extern int get_attribute_int_val(CVIXMLElement elem, char *att_name, int *val, int dflt_val);


#endif

