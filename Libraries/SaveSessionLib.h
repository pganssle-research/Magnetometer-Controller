/************************* Function Declarations *************************/
#ifndef SAVE_SESSION_LIB_H
#define SAVE_SESSION_LIB_H

#include <cvirte.h>
#include <UIControls.h>

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
extern void initialize_ec(void);
extern int *get_broken_ttl_ctrls(void);

// File I/O
extern EP save_ep(void);
extern void load_ep(EP ep);
extern EP null_ep(void);
extern EP free_ep(EP *ep);

EP uiep;

extern int save_session(char *filename, int safe);
extern int load_session(char *filename, int safe);

extern void display_xml_error(int err);

#endif
