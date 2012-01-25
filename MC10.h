/************** Static Function Declarations **************/
/************** Global Variable Declarations **************/
extern int update_thread;
extern int idle_thread;
extern int lock_pb;
extern int lock_pp;
extern int lock_DAQ;
extern int lock_plot;

/************** Global Function Declarations **************/
extern void *PtrToPtr64(const void *p);
extern void *Ptr64ToPtr(const void *p);
extern void *HandleToHandle64(const void *h);
extern void *Handle64ToHandle(const void *h);
extern void __cdecl AutoscalingOnOff(int menuBar,
                                     int menuItem, void *callbackData,
                                     int panel);
extern int __cdecl BrokenTTLsClearAll(int panel,
                                      int control, int event, void *callbackData,
                                      int eventData1, int eventData2);
extern int __cdecl BrokenTTLsExit(int panel,
                                  int control, int event, void *callbackData,
                                  int eventData1, int eventData2);
extern void __cdecl BrokenTTLsMenu(int menuBar,
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
extern int __cdecl ChangeInc(int panel,
                             int control, int event, void *callbackData,
                             int eventData1, int eventData2);
extern int __cdecl ChangeInitOrFinal(int panel,
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
extern void __cdecl ChangeNDPointMenu(int menuBar,
                                      int menuItem, void *callbackData,
                                      int panel);
extern int __cdecl ChangeNDTimeUnits(int panel,
                                     int control, int event, void *callbackData,
                                     int eventData1, int eventData2);
extern int __cdecl ChangeNumCycles(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl ChangeNumSteps(int panel,
                                  int control, int event, void *callbackData,
                                  int eventData1, int eventData2);
extern int __cdecl ChangePhaseCorrectionOrder(int panel,
                                              int control, int event,
                                              void *callbackData,
                                              int eventData1, int eventData2);
extern int __cdecl ChangePhaseCycleLevel(int panel,
                                         int control, int event,
                                         void *callbackData, int eventData1,
                                         int eventData2);
extern int __cdecl ChangePhaseCycleStep(int panel,
                                        int control, int event,
                                        void *callbackData, int eventData1,
                                        int eventData2);
extern int __cdecl ChangePhaseKnob(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl ChangePolyFitOrder(int panel,
                                      int control, int event, void *callbackData,
                                      int eventData1, int eventData2);
extern int __cdecl ChangePolySubtract(int panel,
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
extern int __cdecl ChangeViewingTransient(int panel,
                                          int control, int event,
                                          void *callbackData, int eventData1,
                                          int eventData2);
extern int __cdecl CollasePhaseCycle(int panel,
                                     int control, int event, void *callbackData,
                                     int eventData1, int eventData2);
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
extern int __cdecl DatChangeIDPos(int panel,
                                  int control, int event, void *callbackData,
                                  int eventData1, int eventData2);
extern int __cdecl DeleteInstructionCallback(int panel,
                                             int control, int event,
                                             void *callbackData,
                                             int eventData1, int eventData2);
extern int __cdecl DirectorySelect(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl EditExpression(int panel,
                                  int control, int event, void *callbackData,
                                  int eventData1, int eventData2);
extern int __cdecl EditSkipCondition(int panel,
                                     int control, int event, void *callbackData,
                                     int eventData1, int eventData2);
extern int __cdecl ExpandPhaseCycle(int panel,
                                    int control, int event, void *callbackData,
                                    int eventData1, int eventData2);
extern int __cdecl FEChangeDelayInstr(int panel,
                                      int control, int event, void *callbackData,
                                      int eventData1, int eventData2);
extern int __cdecl FEChangeInstrDataInstr(int panel,
                                          int control, int event,
                                          void *callbackData, int eventData1,
                                          int eventData2);
extern int __cdecl FEEnableDelay(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl FEEnableInstrData(int panel,
                                     int control, int event, void *callbackData,
                                     int eventData1, int eventData2);
extern int __cdecl FENewFunc(int panel,
                             int control, int event, void *callbackData,
                             int eventData1, int eventData2);
extern int __cdecl FIDGraphClick(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl FuncEditQuit(int panel,
                                int control, int event, void *callbackData,
                                int eventData1, int eventData2);
extern int __cdecl FuncEditSave(int panel,
                                int control, int event, void *callbackData,
                                int eventData1, int eventData2);
extern int __cdecl FuncEditSaveAndClose(int panel,
                                        int control, int event,
                                        void *callbackData, int eventData1,
                                        int eventData2);
extern int __cdecl InstNumChange(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl InstrCallback(int panel,
                                 int control, int event, void *callbackData,
                                 int eventData1, int eventData2);
extern int __cdecl InstrChangeCycleNum(int panel,
                                       int control, int event,
                                       void *callbackData, int eventData1,
                                       int eventData2);
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
extern int __cdecl MoveInstButton(int panel,
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
extern int __cdecl PhaseCycleInstr(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl ProgChangeIDPos(int panel,
                                   int control, int event, void *callbackData,
                                   int eventData1, int eventData2);
extern int __cdecl QuitCallback(int panel,
                                int control, int event, void *callbackData,
                                int eventData1, int eventData2);
extern void __cdecl QuitCallbackMenu(int menuBar,
                                     int menuItem, void *callbackData,
                                     int panel);
extern int __cdecl ResetIDPos(int panel,
                              int control, int event, void *callbackData,
                              int eventData1, int eventData2);
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
extern int __cdecl SetupSkipCondition(int panel,
                                      int control, int event, void *callbackData,
                                      int eventData1, int eventData2);
extern int __cdecl SpectrumCallback(int panel,
                                    int control, int event, void *callbackData,
                                    int eventData1, int eventData2);
extern int __cdecl StartProgram(int panel,
                                int control, int event, void *callbackData,
                                int eventData1, int eventData2);
extern int __cdecl StopProgram(int panel,
                               int control, int event, void *callbackData,
                               int eventData1, int eventData2);
extern int __cdecl ToggleBrokenTTL(int panel,
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
extern int __cdecl MoveInstButtonExpanded(int panel,
                                          int control, int event,
                                          void *callbackData, int eventData1,
                                          int eventData2);
extern int __cdecl ChangePhaseCycleStepExpanded(int panel,
                                                int control, int event,
                                                void *callbackData,
                                                int eventData1,
                                                int eventData2);
extern int InitializeQuitUpdateStatus(void);
extern void UninitializeQuitUpdateStatus(void);
extern int *GetPointerToQuitUpdateStatus(void);
extern void ReleasePointerToQuitUpdateStatus(void);
extern void SetQuitUpdateStatus(int val);
extern int GetQuitUpdateStatus(void);
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
extern int InitializeDoubleQuitIdle(void);
extern void UninitializeDoubleQuitIdle(void);
extern int *GetPointerToDoubleQuitIdle(void);
extern void ReleasePointerToDoubleQuitIdle(void);
extern void SetDoubleQuitIdle(int val);
extern int GetDoubleQuitIdle(void);
extern int __cdecl DeleteInstructionCallbackExpanded(int panel,
                                                     int control,
                                                     int event,
                                                     void *callbackData,
                                                     int eventData1,
                                                     int eventData2);
extern int __cdecl ChangeInstDelayExpanded(int panel,
                                           int control, int event,
                                           void *callbackData,
                                           int eventData1, int eventData2);
extern int __cdecl InstrCallbackExpanded(int panel,
                                         int control, int event,
                                         void *callbackData, int eventData1,
                                         int eventData2);
extern int __cdecl ChangeTUnitsExpanded(int panel,
                                        int control, int event,
                                        void *callbackData, int eventData1,
                                        int eventData2);
extern int __cdecl InstrDataCallbackExpanded(int panel,
                                             int control, int event,
                                             void *callbackData,
                                             int eventData1, int eventData2);
extern int __cdecl Change_ScanExpanded(int panel,
                                       int control, int event,
                                       void *callbackData, int eventData1,
                                       int eventData2);
extern int __cdecl ChangeNP_AT_SR(int panel,
                                  int control, int event, void *callbackData,
                                  int eventData1, int eventData2);
