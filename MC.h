
/************** Static Function Declarations **************/

/************** Global Variable Declarations **************/
extern int timer;
extern int nPoints;
extern int Transients;
extern int ttl_trigger;
extern int ninstructions;
extern int n_inst_disp;
extern int inst[1000];
extern char defaultsloc[50];
extern char savedsessionloc[50];
extern PPROGRAM *running_prog;
extern int get_status;
extern int started;
extern int update_thread;
extern int idle_thread;
extern int initialized;
extern int lock_pb;
extern int cont_mode;
extern int lock_pp;
extern int cinst[1000];
extern char errorstring[200];
extern int lock_DAQ;
extern int lock_plot;

/************** Global Function Declarations **************/
extern void __cdecl AutoscalingOnOff(int menuBar,
                                     int menuItem, void *callbackData,
                                     int panel);
extern void __cdecl BrokenTTLs(int menuBar,
                               int menuItem, void *callbackData,
                               int panel);
extern int __cdecl Change_Scan(int panel,
                               int control, int event, void *callbackData,
                               int eventData1, int eventData2);
extern int __cdecl Change_Trigger(int panel,
                                  int control, int event, void *callbackData,
                                  int eventData1, int eventData2);
extern int __cdecl ChangeAcquisitionChannel(int panel,
                                            int control, int event,
                                            void *callbackData,
                                            int eventData1, int eventData2);
extern int __cdecl ChangeAcquisitionTime(int panel,
                                         int control, int event,
                                         void *callbackData, int eventData1,
                                         int eventData2);
extern int __cdecl ChangeChannelGain(int panel,
                                     int control, int event, void *callbackData,
                                     int eventData1, int eventData2);
extern int __cdecl ChangeCounterEdge(int panel,
                                     int control, int event, void *callbackData,
                                     int eventData1, int eventData2);
extern int __cdecl ChangeDevice(int panel,
                                int control, int event, void *callbackData,
                                int eventData1, int eventData2);
extern int __cdecl ChangeDimension(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl ChangeFIDChanColor(int panel,
                                      int control, int event, void *callbackData,
                                      int eventData1, int eventData2);
extern int __cdecl ChangeFIDChannel(int panel,
                                    int control, int event, void *callbackData,
                                    int eventData1, int eventData2);
extern int __cdecl ChangeFIDChanPrefs(int panel,
                                      int control, int event, void *callbackData,
                                      int eventData1, int eventData2);
extern int __cdecl ChangeFIDGain(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl ChangeFIDOffset(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl ChangeFInstrData(int panel,
                                    int control, int event, void *callbackData,
                                    int eventData1, int eventData2);
extern int __cdecl ChangeIncInstrData(int panel,
                                      int control, int event, void *callbackData,
                                      int eventData1, int eventData2);
extern int __cdecl ChangeInputMinMax(int panel,
                                     int control, int event, void *callbackData,
                                     int eventData1, int eventData2);
extern int __cdecl ChangeInstDelay(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl ChangeInstrVary(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl ChangeIntInstr(int panel,
                                  int control, int event, void *callbackData,
                                  int eventData1, int eventData2);
extern void __cdecl ChangeNDPointMenu(int menuBar,
                                      int menuItem, void *callbackData,
                                      int panel);
extern int __cdecl ChangeNPoints(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl ChangeNumSteps(int panel,
                                  int control, int event, void *callbackData,
                                  int eventData1, int eventData2);
extern int __cdecl ChangePhaseCorrectionOrder(int panel,
                                              int control, int event,
                                              void *callbackData,
                                              int eventData1, int eventData2);
extern int __cdecl ChangePhaseKnob(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl ChangePolyFitOrder(int panel,
                                      int control, int event, void *callbackData,
                                      int eventData1, int eventData2);
extern int __cdecl ChangePolySubtract(int panel,
                                      int control, int event, void *callbackData,
                                      int eventData1, int eventData2);
extern int __cdecl ChangeSampleRate(int panel,
                                    int control, int event, void *callbackData,
                                    int eventData1, int eventData2);
extern int __cdecl ChangeSpecChanPrefs(int panel,
                                       int control, int event,
                                       void *callbackData, int eventData1,
                                       int eventData2);
extern int __cdecl ChangeSpectrumChan(int panel,
                                      int control, int event, void *callbackData,
                                      int eventData1, int eventData2);
extern int __cdecl ChangeSpectrumChanColor(int panel,
                                           int control, int event,
                                           void *callbackData,
                                           int eventData1, int eventData2);
extern int __cdecl ChangeSpectrumChannel(int panel,
                                         int control, int event,
                                         void *callbackData, int eventData1,
                                         int eventData2);
extern int __cdecl ChangeSpectrumGain(int panel,
                                      int control, int event, void *callbackData,
                                      int eventData1, int eventData2);
extern int __cdecl ChangeSpectrumOffset(int panel,
                                        int control, int event,
                                        void *callbackData, int eventData1,
                                        int eventData2);
extern int __cdecl ChangeTransients(int panel,
                                    int control, int event, void *callbackData,
                                    int eventData1, int eventData2);
extern void __cdecl ChangeTransientView(int menuBar,
                                        int menuItem, void *callbackData,
                                        int panel);
extern int __cdecl ChangeTriggerEdge(int panel,
                                     int control, int event, void *callbackData,
                                     int eventData1, int eventData2);
extern int __cdecl ChangeTUnits(int panel,
                                int control, int event, void *callbackData,
                                int eventData1, int eventData2);
extern int __cdecl ChangeViewingTransientNumSpec(int panel,
                                                 int control, int event,
                                                 void *callbackData,
                                                 int eventData1,
                                                 int eventData2);
extern int __cdecl ColorVal(int panel,
                            int control, int event, void *callbackData,
                            int eventData1, int eventData2);
extern int __cdecl ContinuousRunCallback(int panel,
                                         int control, int event,
                                         void *callbackData, int eventData1,
                                         int eventData2);
extern int __cdecl ControlHidden(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl DeleteInstructionCallback(int panel,
                                             int control, int event,
                                             void *callbackData,
                                             int eventData1, int eventData2);
extern int __cdecl DirectorySelect(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl FIDGraphClick(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl InstNumChange(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl InstrCallback(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl InstrDataCallback(int panel,
                                     int control, int event, void *callbackData,
                                     int eventData1, int eventData2);
extern void __cdecl LoadConfigurationFromFile(int menuBar,
                                              int menuItem, void *callbackData,
                                              int panel);
extern void __cdecl LoadDataMenu(int menuBar,
                                 int menuItem, void *callbackData,
                                 int panel);
extern int __cdecl LoadProgram(int panel,
                               int control, int event, void *callbackData,
                               int eventData1, int eventData2);
extern void __cdecl LoadProgramMenu(int menuBar,
                                    int menuItem, void *callbackData,
                                    int panel);
extern int __cdecl MoveInst(int panel,
                            int control, int event, void *callbackData,
                            int eventData1, int eventData2);
extern int __cdecl MoveInstr(int panel,
                             int control, int event, void *callbackData,
                             int eventData1, int eventData2);
extern void __cdecl NewAcquisitionMenu(int menuBar,
                                       int menuItem, void *callbackData,
                                       int panel);
extern int __cdecl NewProgram(int panel,
                              int control, int event, void *callbackData,
                              int eventData1, int eventData2);
extern void __cdecl NewProgramMenu(int menuBar,
                                   int menuItem, void *callbackData,
                                   int panel);
extern int __cdecl NumDimensionCallback(int panel,
                                        int control, int event,
                                        void *callbackData, int eventData1,
                                        int eventData2);
extern void __cdecl PanGraph(int menuBar,
                             int menuItem, void *callbackData,
                             int panel);
extern int __cdecl QuitCallback(int panel,
                                int control, int event, void *callbackData,
                                int eventData1, int eventData2);
extern void __cdecl QuitCallbackMenu(int menuBar,
                                     int menuItem, void *callbackData,
                                     int panel);
extern void __cdecl SaveConfig(int menuBar,
                               int menuItem, void *callbackData,
                               int panel);
extern void __cdecl SaveConfigToFile(int menuBar,
                                     int menuItem, void *callbackData,
                                     int panel);
extern int __cdecl SaveProgram(int panel,
                               int control, int event, void *callbackData,
                               int eventData1, int eventData2);
extern void __cdecl SaveProgramMenu(int menuBar,
                                    int menuItem, void *callbackData,
                                    int panel);
extern int __cdecl SpectrumCallback(int panel,
                                    int control, int event, void *callbackData,
                                    int eventData1, int eventData2);
extern int __cdecl StartProgram(int panel,
                                int control, int event, void *callbackData,
                                int eventData1, int eventData2);
extern int __cdecl StopProgram(int panel,
                               int control, int event, void *callbackData,
                               int eventData1, int eventData2);
extern int __cdecl ToggleND(int panel,
                            int control, int event, void *callbackData,
                            int eventData1, int eventData2);
extern void __cdecl UpdateDAQMenuCallback(int menuBar,
                                          int menuItem, void *callbackData,
                                          int panel);
extern void __cdecl ViewProgramChart(int menuBar,
                                     int menuItem, void *callbackData,
                                     int panel);
extern void __cdecl ZoomGraph(int menuBar,
                              int menuItem, void *callbackData,
                              int panel);
extern void *PtrToPtr64(const void *p);
extern void *Ptr64ToPtr(const void *p);
extern void *HandleToHandle64(const void *h);
extern void *Handle64ToHandle(const void *h);
extern PVOID RtlSecureZeroMemory(PVOID ptr,
                                 SIZE_T cnt);
extern int get_nd_state(int panel, int control);
extern int __cdecl CalculateAverage(int panel,
                                    int control, int event, void *callbackData,
                                    int eventData1, int eventData2);
extern int __cdecl ChangeCursorMode(int panel,
                                    int control, int event, void *callbackData,
                                    int eventData1, int eventData2);
extern int __cdecl ChangeDelay(int panel,
                               int control, int event, void *callbackData,
                               int eventData1, int eventData2);
extern int __cdecl ChangeDim(int panel,
                             int control, int event, void *callbackData,
                             int eventData1, int eventData2);
extern int __cdecl ChangeDimPoint(int panel,
                                  int control, int event, void *callbackData,
                                  int eventData1, int eventData2);
extern int __cdecl ChangeFDelay(int panel,
                                int control, int event, void *callbackData,
                                int eventData1, int eventData2);
extern int __cdecl ChangeFTimeDataUnits(int panel,
                                        int control, int event,
                                        void *callbackData, int eventData1,
                                        int eventData2);
extern int __cdecl ChangeIncrement(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl ChangeIncTimeDataUnits(int panel,
                                          int control, int event,
                                          void *callbackData, int eventData1,
                                          int eventData2);
extern int __cdecl ChangeInitDelay(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl ChangeInitTimeDataUnits(int panel,
                                           int control, int event,
                                           void *callbackData,
                                           int eventData1, int eventData2);
extern int __cdecl ChangeITUnits(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl ChangeNumCycles(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl ChangePhaseCycleFreq(int panel,
                                        int control, int event,
                                        void *callbackData, int eventData1,
                                        int eventData2);
extern int __cdecl ChangePhaseCycleInstr(int panel,
                                         int control, int event,
                                         void *callbackData, int eventData1,
                                         int eventData2);
extern int __cdecl ChangeRMS(int panel,
                             int control, int event, void *callbackData,
                             int eventData1, int eventData2);
extern int __cdecl ChangeViewingTransientFID(int panel,
                                             int control, int event,
                                             void *callbackData,
                                             int eventData1, int eventData2);
extern int __cdecl CursorVisible(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl DisplayRMSBoundsCallback(int panel,
                                            int control, int event,
                                            void *callbackData,
                                            int eventData1, int eventData2);
extern int __cdecl PhaseCycleCallback(int panel,
                                      int control, int event, void *callbackData,
                                      int eventData1, int eventData2);
extern int __cdecl SetRMSBorders(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl ShowHidden(int panel,
                              int control, int event, void *callbackData,
                              int eventData1, int eventData2);
extern int __cdecl TestButtonCallback(int panel,
                                      int control, int event, void *callbackData,
                                      int eventData1, int eventData2);
extern int __cdecl UpdateChannels(int panel,
                                  int control, int event, void *callbackData,
                                  int eventData1, int eventData2);
extern int __cdecl WriteToFile(int panel,
                               int control, int event, void *callbackData,
                               int eventData1, int eventData2);
extern int file_exists(char *filename);
extern int get_value_from_file(char *filename,
                               int line, char *string, void *value);
extern int generate_recent_experiments_menu(void);
extern int load_data_file(char *loadfile);
extern int generate_recent_programs_menu(void);
extern int add_recent_program(char *fname);
extern int load_DAQ_info(void);
extern int Load_TTLs(int num);
extern int change_np_or_sr(int np_sr_at);
extern int change_fid_or_fft_chan(int panel,
                                  int control, int fid_or_fft);
extern int isin(int source, int *array, int sizeofarray);
extern int setup_DAQ_task(TaskHandle *acquiretask,
                          TaskHandle *countertask);
extern double pulse_program(PPROGRAM *p);
extern int update_status(int status);
extern int get_data(PPROGRAM *p);
extern int change_fid(int plot);
extern int change_spectrum(int plot);
extern int average_from_file(char *filename,
                             double *input, int np);
extern int get_devices(void);
extern int change_phase(double phase, int order,
                        int chan);
extern int InitializeQuitUpdateStatus(void);
extern void UninitializeQuitUpdateStatus(void);
extern int *GetPointerToQuitUpdateStatus(void);
extern void ReleasePointerToQuitUpdateStatus(void);
extern void SetQuitUpdateStatus(int val);
extern int GetQuitUpdateStatus(void);
extern double log2(double value);
extern int pb_init_safe(int verbose);
extern int pb_start_programming_safe(int verbose,
                                     int device);
extern int pb_stop_programming_safe(int verbose);
extern int pb_inst_pbonly_safe(int verbose,
                               unsigned int flags, int inst, int inst_data,
                               double length);
extern int pb_close_safe(int verbose);
extern int pb_read_status_safe(int verbose);
extern int pb_start_safe(int verbose);
extern int pb_stop_safe(int verbose);
extern int __cdecl UpdateStatus(void *functiondata);
extern int __cdecl IdleAndGetData(void *functionData);
extern void __cdecl discardIdleAndGetData(int poolHandle,
                                          int functionID, unsigned int event,
                                          int value, void *callbackData);
extern int InitializeQuitIdle(void);
extern void UninitializeQuitIdle(void);
extern int *GetPointerToQuitIdle(void);
extern void ReleasePointerToQuitIdle(void);
extern void SetQuitIdle(int val);
extern int GetQuitIdle(void);
extern int InitializeStatus(void);
extern void UninitializeStatus(void);
extern int *GetPointerToStatus(void);
extern void ReleasePointerToStatus(void);
extern void SetStatus(int val);
extern int GetStatus(void);
extern int clear_instruction(int num);
extern int load_fid_data(char *filename,
                         int np, double *fid);
extern int load_fft_data(char *filename,
                         int np, double *realchan, double *imagchan);
extern int clear_plots(void);
extern int get_num_points(char *filename);
extern double get_sampling_rate(char *filename);
extern int get_num_transients_completed(char *filename);
extern int plot_fid_data(double *data,
                         int chan, int np, int t, double sr);
extern int plot_fft_data(double *realchan,
                         double *imagchan, int chan, int np, double sr,
                         int t);
extern int load_data(void);
extern int get_fid_or_fft_fname(char *fname,
                                int t, int c, int fid_or_fft);
extern int insert_transient(int t);
extern int fft(double *in, double *rc, double *ic,
               int np);
extern int change_fid_and_fft(int t);
extern int change_viewing_transient(int panel,
                                    int control);
extern int switch_transient_index(int t);
extern int get_t_units_pair(int panel,
                            int control);
extern int change_precision(int panel,
                            int control);
extern int update_final_val(int panel);
extern int fix_number_of_dimensions(void);
extern int change_dimension(int panel);
extern int change_increment(int panel);
extern int change_num_steps(int panel);
extern int change_nc_or_cf(int panel);
extern int get_number_of_dimension_points(int dim);
extern int populate_dim_points(void);
extern int get_num_varying_instr(void);
extern int phase_cycle_n_transients(int panel);
extern int update_dim_point(int dim,
                            int np);
extern int write_data(double *fid, int np,
                      int chan, int nc, int t, int nt, PPROGRAM *p,
                      char *filename, char *pathname);
extern int exp_is_1d(char *loadfile);
extern int load_data_1d(char *loadfile);
extern int load_data_nd(int nd, char *loadfile);
extern double calculate_units(double val);
extern int getCursorValues(void);
extern int update_dim_point_control(int *steps,
                                    int nd);
extern char *itoa(int in);
extern int Load_ND_Labels(int d);
extern int Load_ND_Val(int d);
extern int get_folder_name(char *pathname,
                           char *pathout);
extern int get_num_dimensions(char *filename);
extern int get_points_completed(char *fname,
                                int nd, int *points);
extern int change_ND_point(void);
extern int update_current_dim_point(int *point,
                                    int nd);
extern int update_channel_gain(void);
extern int get_view_mode(void);
extern int get_num_channels(char *filename);
extern int polynomial_fit(double *y,
                          int np, int order, double *output);
extern int channel_on(int channel, int fid_or_fft);
extern int poly_subtract(double *data,
                         int c, int np);
extern int channel_control(int channel,
                           int fid_or_fft, int *out);
extern int all_panels(int num);
extern int all_controls(int panel, int num);
extern int change_gain(int panel, int control,
                       int chan, int fid_or_fft);
extern int change_offset(int panel, int control,
                         int chan, int fid_or_fft);
extern int save_configuration_to_file(char *fname);
extern int load_configuration_from_file(char *filename);
extern int copy_file(char *from, char *to);
extern int change_poly(int chan);
extern int load_DAQ_info_locked(void);
extern int setup_DAQ_task_locked(TaskHandle *acquiretask,
                                 TaskHandle *countertask);
extern void __cdecl SelectRecentData(int menuBarHandle,
                                     int menuItemID, void *callbackData,
                                     int panelHandle);
extern void __cdecl SelectRecentProgram(int menuBarHandle,
                                        int menuItemID, void *callbackData,
                                        int panelHandle);
extern int add_recent_experiment(char *fname);
extern int change_fid_and_fft_locked(int t);
extern int __cdecl QuitGraph(int panel,
                             int control, int event, void *callbackData,
                             int eventData1, int eventData2);
extern int zoom_graph(int panel, int control,
                      int in, int xy);
extern int pan_graph(int panel, int control,
                     int dir);
extern int fit_graph(int panel, int control,
                     int xy);
extern int run_hotkey(int panel, int control,
                      int eventData1, int eventData2);
extern int setup_broken_ttls(void);
extern int __cdecl QuitBrokenTTL(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl ToggleBrokenTTL(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int InitializeDoubleQuitIdle(void);
extern void UninitializeDoubleQuitIdle(void);
extern int *GetPointerToDoubleQuitIdle(void);
extern void ReleasePointerToDoubleQuitIdle(void);
extern void SetDoubleQuitIdle(int val);
extern int GetDoubleQuitIdle(void);
extern int swap_ttls(int to, int from);
extern int ttls_in_use(PPROGRAM *p, int *ttls);
extern int toggle_RMS_borders(int panel,
                              int on);
extern int getMousePositionOnGraph(int panel,
                                   int control, double *x, double *y);
extern int RMS_click(int panel, int control,
                     double x, double y);
extern int RMS_click_up(int panel, int control,
                        double x, double y);
extern int RMS_mouse_move(int panel,
                          int control, double x, double y);
extern int calculate_RMS(int panel, int control);
extern int setup_RMS(int panel, double rms1,
                     double rms2);
extern char *temporary_filename(char *dir,
                                char *extension);
extern int cursor_click_up(int panel,
                           int control, int mode, double x, double y);
extern char *temp_file(char *extension);
extern int update_nd_state(int panel,
                           int control, int state);
extern int update_instr_data_nd(int panel,
                                int change);
extern double calculate_instr_length(int instr);
extern double get_and_set_old_units(int panel,
                                    int control);
extern int get_precision(double val, int total_num);
extern int load_ui(char *uifname);
extern void initialize_uicontrols(void);
