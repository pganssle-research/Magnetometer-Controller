/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2011. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  BrokenTTLs                       1
#define  BrokenTTLs_TTL23                 2       /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL22                 3       /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL21                 4       /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL20                 5       /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL19                 6       /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL18                 7       /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL17                 8       /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL16                 9       /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL15                 10      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL14                 11      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL13                 12      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL12                 13      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL11                 14      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL10                 15      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL9                  16      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL8                  17      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL7                  18      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL6                  19      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL2                  20      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL5                  21      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL1                  22      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL4                  23      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL0                  24      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL3                  25      /* callback function: ToggleBrokenTTL */
#define  BrokenTTLs_Exit                  26      /* callback function: BrokenTTLsExit */
#define  BrokenTTLs_ClearAll              27      /* callback function: BrokenTTLsClearAll */

#define  CurrentLoc                       2
#define  CurrentLoc_TransientNum          2       /* callback function: ChangeViewingTransient */
#define  CurrentLoc_ID8                   3
#define  CurrentLoc_ID7                   4
#define  CurrentLoc_ID6                   5
#define  CurrentLoc_ID5                   6
#define  CurrentLoc_ID4                   7
#define  CurrentLoc_ID3                   8
#define  CurrentLoc_ID2                   9
#define  CurrentLoc_ID1                   10
#define  CurrentLoc_IDVal8                11      /* callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal7                12      /* callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal6                13      /* callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal5                14      /* callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal4                15      /* callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal3                16      /* callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal2                17      /* callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal1                18      /* callback function: DatChangeIDPos */

#define  EditFunc                         3
#define  EditFunc_QUITBUTTON              2       /* callback function: FuncEditQuit */
#define  EditFunc_NewFunc                 3       /* callback function: FENewFunc */
#define  EditFunc_Save                    4       /* callback function: FuncEditSave */
#define  EditFunc_SaveAndClose            5       /* callback function: FuncEditSaveAndClose */
#define  EditFunc_DelayInstr              6       /* callback function: FEChangeDelayInstr */
#define  EditFunc_InstrDataInstr          7       /* callback function: FEChangeInstrDataInstr */
#define  EditFunc_NumInst                 8       /* callback function: InstNumChange */
#define  EditFunc_Trigger_TTL             9       /* callback function: Change_Trigger */
#define  EditFunc_EnableDelay             10      /* callback function: FEEnableDelay */
#define  EditFunc_EnableInstrData         11      /* callback function: FEEnableInstrData */

#define  HiddenVals                       4
#define  HiddenVals_PolySubtractOnOffRing 2
#define  HiddenVals_PolyOrderValues       3
#define  HiddenVals_PhaseCorrectionValues 4
#define  HiddenVals_ChannelOffsets        5
#define  HiddenVals_PlotIDs               6
#define  HiddenVals_ChannelGains          7
#define  HiddenVals_ATConstant            8
#define  HiddenVals_TransientView         9
#define  HiddenVals_SRConstant            10
#define  HiddenVals_NPConstant            11
#define  HiddenVals_LastProgramLoc        12
#define  HiddenVals_ControlDimmed         13
#define  HiddenVals_ControlHidden         14      /* callback function: ControlHidden */
#define  HiddenVals_RecentData            15
#define  HiddenVals_RecentPrograms        16
#define  HiddenVals_SpectrumChanColor     17      /* callback function: ColorVal */
#define  HiddenVals_FIDChanColor          18      /* callback function: ColorVal */
#define  HiddenVals_TTLBroken             19

#define  MainPanel                        5
#define  MainPanel_MainTabs               2       /* callback function: PopoutTab */
#define  MainPanel_Start                  3       /* callback function: StartProgram */
#define  MainPanel_Stop                   4       /* callback function: StopProgram */
#define  MainPanel_QUITBUTTON             5       /* callback function: QuitCallback */
#define  MainPanel_CurrentFile            6
#define  MainPanel_Filename               7
#define  MainPanel_DirectorySelect        8       /* callback function: DirectorySelect */
#define  MainPanel_Running                9
#define  MainPanel_Waiting                10
#define  MainPanel_Stopped                11
#define  MainPanel_Path                   12
#define  MainPanel_IsRunning              13
#define  MainPanel_PBStatus               14
#define  MainPanel_ProgDesc               15
#define  MainPanel_DataDescription        16
#define  MainPanel_SPLITTER_2             17
#define  MainPanel_SPLITTER               18
#define  MainPanel_CurrentProgRing        19
#define  MainPanel_COMMANDBUTTON          20
#define  MainPanel_DataDirectory          21      /* callback function: ChangeDataBox */
#define  MainPanel_LoadInfoMode           22      /* callback function: ChangeLoadInfoMode */

#define  MDInstr                          6
#define  MDInstr_InstrNum                 2
#define  MDInstr_Instructions             3
#define  MDInstr_VaryInstr                4       /* callback function: ChangeInstrVary */
#define  MDInstr_FInstrData               5       /* callback function: ChangeInitOrFinal */
#define  MDInstr_InitInstrData            6       /* callback function: ChangeInitOrFinal */
#define  MDInstr_IncInstrData             7       /* callback function: ChangeInc */
#define  MDInstr_FTime                    8       /* callback function: ChangeInitOrFinal */
#define  MDInstr_FTimeUnits               9       /* callback function: ChangeNDTimeUnits */
#define  MDInstr_InitDisplay              10
#define  MDInstr_FDisplay                 11
#define  MDInstr_IncDisplay               12
#define  MDInstr_IncTime                  13      /* callback function: ChangeInc */
#define  MDInstr_IncTimeUnits             14      /* callback function: ChangeNDTimeUnits */
#define  MDInstr_InitTime                 15      /* callback function: ChangeInitOrFinal */
#define  MDInstr_InitTimeUnits            16      /* callback function: ChangeNDTimeUnits */
#define  MDInstr_NumSteps                 17      /* callback function: ChangeNumSteps */
#define  MDInstr_Dimension                18      /* callback function: ChangeDimension */
#define  MDInstr_IncDataExpression        19      /* callback function: EditExpression */
#define  MDInstr_IncDelayExpression       20      /* callback function: EditExpression */

#define  PPConfigP                        7
#define  PPConfigP_VaryingLabel           2
#define  PPConfigP_NumStepsLabel          3
#define  PPConfigP_FinalLabel             4
#define  PPConfigP_IncrementLabel         5
#define  PPConfigP_InitialLabel           6
#define  PPConfigP_InstrLabel             7

#define  PPPanel                          8
#define  PPPanel_PhaseCycleLabel          2
#define  PPPanel_ScanLabel                3
#define  PPPanel_InstrLabel               4
#define  PPPanel_DelayLabel               5
#define  PPPanel_InstrNumLabel            6
#define  PPPanel_TTLsLabel                7

#define  PulseInstP                       9
#define  PulseInstP_InstNum               2       /* callback function: MoveInst */
#define  PulseInstP_Instructions          3       /* callback function: InstrCallback */
#define  PulseInstP_Instr_Data            4       /* callback function: InstrDataCallback */
#define  PulseInstP_TTL23                 5
#define  PulseInstP_TTL22                 6
#define  PulseInstP_TTL21                 7
#define  PulseInstP_TTL20                 8
#define  PulseInstP_TTL19                 9
#define  PulseInstP_TTL18                 10
#define  PulseInstP_TTL17                 11
#define  PulseInstP_TTL16                 12
#define  PulseInstP_TTL15                 13
#define  PulseInstP_TTL14                 14
#define  PulseInstP_TTL13                 15
#define  PulseInstP_TTL12                 16
#define  PulseInstP_TTL11                 17
#define  PulseInstP_TTL10                 18
#define  PulseInstP_TTL9                  19
#define  PulseInstP_TTL8                  20
#define  PulseInstP_TTL7                  21
#define  PulseInstP_TTL6                  22
#define  PulseInstP_TTL5                  23
#define  PulseInstP_TTL4                  24
#define  PulseInstP_TTL3                  25
#define  PulseInstP_TTL2                  26
#define  PulseInstP_TTL1                  27
#define  PulseInstP_TTL0                  28
#define  PulseInstP_InstDelay             29      /* callback function: ChangeInstDelay */
#define  PulseInstP_TimeUnits             30      /* callback function: ChangeTUnits */
#define  PulseInstP_Scan                  31      /* callback function: Change_Scan */
#define  PulseInstP_UpButton              32      /* callback function: MoveInstButton */
#define  PulseInstP_DownButton            33      /* callback function: MoveInstButton */
#define  PulseInstP_PhaseCycleLevel       34      /* callback function: ChangePhaseCycleLevel */
#define  PulseInstP_PhaseCycleStep        35      /* callback function: ChangePhaseCycleStep */
#define  PulseInstP_NumCycles             36      /* callback function: InstrChangeCycleNum */
#define  PulseInstP_PhaseCyclingOn        37      /* callback function: PhaseCycleInstr */
#define  PulseInstP_xButton               38      /* callback function: DeleteInstructionCallback */
#define  PulseInstP_CollapseButton        39      /* callback function: CollapsePhaseCycle */
#define  PulseInstP_ExpandButton          40      /* callback function: ExpandPhaseCycle */

     /* tab page panel controls */
#define  FID_Graph                        2       /* callback function: FIDGraphClick */
#define  FID_CursorY                      3
#define  FID_CursorX                      4
#define  FID_Chan8                        5       /* callback function: ToggleFIDChan */
#define  FID_Chan7                        6       /* callback function: ToggleFIDChan */
#define  FID_Chan6                        7       /* callback function: ToggleFIDChan */
#define  FID_Chan5                        8       /* callback function: ToggleFIDChan */
#define  FID_Chan4                        9       /* callback function: ToggleFIDChan */
#define  FID_Chan3                        10      /* callback function: ToggleFIDChan */
#define  FID_Chan2                        11      /* callback function: ToggleFIDChan */
#define  FID_Chan1                        12      /* callback function: ToggleFIDChan */
#define  FID_ChanLabel                    13
#define  FID_ChanPrefs                    14      /* callback function: ChangeFIDChanPrefs */
#define  FID_PolySubtract                 15      /* callback function: ChangePolySubtract */
#define  FID_PolyFitOrder                 16      /* callback function: ChangePolyFitOrder */
#define  FID_ChannelBox                   17
#define  FID_PolyFitLabel                 18
#define  FID_Offset                       19      /* callback function: ChangeFIDOffset */
#define  FID_Gain                         20      /* callback function: ChangeFIDGain */
#define  FID_ChanColor                    21      /* callback function: ChangeFIDChanColor */
#define  FID_Autoscale                    22

     /* tab page panel controls */
#define  PPConfig_SaveProgram             2       /* callback function: SaveProgram */
#define  PPConfig_NTransients             3       /* callback function: ChangeTransients */
#define  PPConfig_LoadProgram             4       /* callback function: LoadProgram */
#define  PPConfig_NDimensionalOn          5       /* callback function: ToggleND */
#define  PPConfig_EstimatedTime           6
#define  PPConfig_NPoints                 7       /* callback function: ChangeNP_AT_SR */
#define  PPConfig_SampleRate              8       /* callback function: ChangeNP_AT_SR */
#define  PPConfig_Device                  9       /* callback function: ChangeDevice */
#define  PPConfig_CounterChan             10
#define  PPConfig_TriggerEdge             11
#define  PPConfig_Trigger_Channel         12
#define  PPConfig_AcquisitionChannel      13      /* callback function: ChangeAcquisitionChannel */
#define  PPConfig_AcquisitionTime         14      /* callback function: ChangeNP_AT_SR */
#define  PPConfig_NumDimensions           15      /* callback function: NumDimensionCallback */
#define  PPConfig_NumChans                16
#define  PPConfig_ChannelGain             17      /* callback function: ChangeCurrentChan */
#define  PPConfig_SkipCondition           18      /* callback function: SetupSkipCondition */
#define  PPConfig_SkipConditionExpr       19      /* callback function: EditSkipCondition */
#define  PPConfig_ChannelRange            20      /* callback function: ChangeChannelRange */
#define  PPConfig_TransAcqMode            21
#define  PPConfig_PBDeviceSelect          22      /* callback function: ChangePBDevice */

     /* tab page panel controls */
#define  PulseProg_TransientNum           2       /* callback function: ProgChangeIDPos */
#define  PulseProg_ID8                    3
#define  PulseProg_ID7                    4
#define  PulseProg_ID6                    5
#define  PulseProg_ID5                    6
#define  PulseProg_ID4                    7
#define  PulseProg_ID3                    8
#define  PulseProg_ID2                    9
#define  PulseProg_ID1                    10
#define  PulseProg_IDVal8                 11      /* callback function: ProgChangeIDPos */
#define  PulseProg_IDVal7                 12      /* callback function: ProgChangeIDPos */
#define  PulseProg_IDVal6                 13      /* callback function: ProgChangeIDPos */
#define  PulseProg_IDVal5                 14      /* callback function: ProgChangeIDPos */
#define  PulseProg_IDVal4                 15      /* callback function: ProgChangeIDPos */
#define  PulseProg_IDVal3                 16      /* callback function: ProgChangeIDPos */
#define  PulseProg_IDVal2                 17      /* callback function: ProgChangeIDPos */
#define  PulseProg_IDVal1                 18      /* callback function: ProgChangeIDPos */
#define  PulseProg_NumInst                19      /* callback function: InstNumChange */
#define  PulseProg_SaveProgram            20      /* callback function: SaveProgram */
#define  PulseProg_NewProgram             21      /* callback function: NewProgram */
#define  PulseProg_LoadProgram            22      /* callback function: LoadProgram */
#define  PulseProg_Trigger_TTL            23      /* callback function: Change_Trigger */
#define  PulseProg_ContinuousRun          24      /* callback function: ContinuousRunCallback */
#define  PulseProg_PhaseCycles            25      /* callback function: ChangeNumCycles */
#define  PulseProg_ResetDims              26      /* callback function: ResetIDPos */

     /* tab page panel controls */
#define  Spectrum_Graph                   2       /* callback function: SpectrumCallback */
#define  Spectrum_CursorY                 3
#define  Spectrum_CursorX                 4
#define  Spectrum_Channel                 5       /* callback function: ChangeSpectrumChannel */
#define  Spectrum_Chan8                   6       /* callback function: ToggleSpecChan */
#define  Spectrum_Chan7                   7       /* callback function: ToggleSpecChan */
#define  Spectrum_Chan6                   8       /* callback function: ToggleSpecChan */
#define  Spectrum_Chan5                   9       /* callback function: ToggleSpecChan */
#define  Spectrum_Chan4                   10      /* callback function: ToggleSpecChan */
#define  Spectrum_Chan3                   11      /* callback function: ToggleSpecChan */
#define  Spectrum_Chan2                   12      /* callback function: ToggleSpecChan */
#define  Spectrum_Chan1                   13      /* callback function: ToggleSpecChan */
#define  Spectrum_ChanLabel               14
#define  Spectrum_ChannelBox              15
#define  Spectrum_ChanPrefs               16      /* callback function: ChangeSpecChanPrefs */
#define  Spectrum_PolySubtract            17      /* callback function: ChangePolySubtract */
#define  Spectrum_PolyFitOrder            18      /* callback function: ChangePolyFitOrder */
#define  Spectrum_PhaseCorrectionOrder    19      /* callback function: ChangePhaseCorrectionOrder */
#define  Spectrum_PhaseKnob               20      /* callback function: ChangePhaseKnob */
#define  Spectrum_Offset                  21      /* callback function: ChangeSpectrumOffset */
#define  Spectrum_Gain                    22      /* callback function: ChangeSpectrumGain */
#define  Spectrum_ChanColor               23      /* callback function: ChangeSpectrumChanColor */
#define  Spectrum_Autoscale               24
#define  Spectrum_PolyFitLabel            25


     /* Menu Bars, Menus, and Menu Items: */

#define  MainMenu                         1
#define  MainMenu_File                    2
#define  MainMenu_File_New                3
#define  MainMenu_File_New_SUBMENU        4
#define  MainMenu_File_New_NewAcquisition 5       /* callback function: NewAcquisitionMenu */
#define  MainMenu_File_New_NewProgram     6       /* callback function: NewProgramMenu */
#define  MainMenu_File_Save               7
#define  MainMenu_File_Save_SUBMENU       8
#define  MainMenu_File_Save_SaveData      9
#define  MainMenu_File_Save_SaveProgram   10      /* callback function: SaveProgramMenu */
#define  MainMenu_File_Load               11
#define  MainMenu_File_Load_SUBMENU       12
#define  MainMenu_File_Load_LoadData      13      /* callback function: LoadDataMenu */
#define  MainMenu_File_Load_LoadRecentData 14
#define  MainMenu_File_Load_SEPARATOR_2   15
#define  MainMenu_File_Load_LoadProgram   16      /* callback function: LoadProgramMenu */
#define  MainMenu_File_Load_LoadRecentProgram 17
#define  MainMenu_File_SEPARATOR          18
#define  MainMenu_File_Quit               19      /* callback function: QuitCallbackMenu */
#define  MainMenu_View                    20
#define  MainMenu_View_ProgramChart       21      /* callback function: ViewProgramChart */
#define  MainMenu_View_TransView          22
#define  MainMenu_View_TransView_SUBMENU  23
#define  MainMenu_View_TransView_ViewLatestTrans 24 /* callback function: ChangeTransientView */
#define  MainMenu_View_TransView_ViewAverage 25   /* callback function: ChangeTransientView */
#define  MainMenu_View_TransView_ViewNoUpdate 26  /* callback function: ChangeTransientView */
#define  MainMenu_View_ChangeDimension    27      /* callback function: ChangeNDPointMenu */
#define  MainMenu_SetupMenu               28
#define  MainMenu_SetupMenu_UpdateDAQ     29      /* callback function: UpdateDAQMenuCallback */
#define  MainMenu_SetupMenu_SEPARATOR_3   30
#define  MainMenu_SetupMenu_SaveCurrentConfig 31  /* callback function: SaveConfig */
#define  MainMenu_SetupMenu_SaveConfig    32      /* callback function: SaveConfigToFile */
#define  MainMenu_SetupMenu_LoadConfigFromFile 33 /* callback function: LoadConfigurationFromFile */
#define  MainMenu_SetupMenu_SEPARATOR_4   34
#define  MainMenu_SetupMenu_BrokenTTLsMenu 35     /* callback function: BrokenTTLsMenu */

#define  RCMenus                          2
#define  RCMenus_AcquisitionTime          2
#define  RCMenus_AcquisitionTime_SampleRate 3
#define  RCMenus_AcquisitionTime_NumPoints 4
#define  RCMenus_PopoutMenu               5
#define  RCMenus_PopoutMenu_ReleaseTab    6
#define  RCMenus_PopoutMenu_ReleaseWindowSubMenu 7
#define  RCMenus_PopoutMenu_ReleaseWindowSubMenu_SUBMENU 8
#define  RCMenus_PopoutMenu_ReleaseWindowSubMenu_ReleaseConfineFID 9
#define  RCMenus_PopoutMenu_ReleaseWindowSubMenu_ReleaseConfineSpec 10
#define  RCMenus_PopoutMenu_ReleaseWindowSubMenu_ReleaseConfinePProg 11
#define  RCMenus_PopoutMenu_ReleaseWindowSubMenu_ReleaseConfinePPConf 12
#define  RCMenus_GraphMenu                13
#define  RCMenus_GraphMenu_AutoScaling    14      /* callback function: AutoscalingOnOff */
#define  RCMenus_GraphMenu_ZoomGraphIn    15      /* callback function: ZoomGraph */
#define  RCMenus_GraphMenu_ZoomGraphOut   16      /* callback function: ZoomGraph */
#define  RCMenus_GraphMenu_PanRight       17      /* callback function: PanGraph */
#define  RCMenus_GraphMenu_PanLeft        18      /* callback function: PanGraph */
#define  RCMenus_GraphMenu_PanUp          19      /* callback function: PanGraph */
#define  RCMenus_GraphMenu_PanDown        20      /* callback function: PanGraph */
#define  RCMenus_GraphMenu_FitHorizontally 21     /* callback function: AutoscalingOnOff */
#define  RCMenus_GraphMenu_FitGraphVertically 22  /* callback function: AutoscalingOnOff */
#define  RCMenus_SampleRate               23
#define  RCMenus_SampleRate_NumPoints     24
#define  RCMenus_SampleRate_AcquisitionTime 25
#define  RCMenus_NumPoints                26
#define  RCMenus_NumPoints_AcquisitionTime 27
#define  RCMenus_NumPoints_SampleRate     28


     /* Callback Prototypes: */

void CVICALLBACK AutoscalingOnOff(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK BrokenTTLsClearAll(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BrokenTTLsExit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK BrokenTTLsMenu(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK Change_Scan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Change_Trigger(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeAcquisitionChannel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeChannelRange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeCurrentChan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeDataBox(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeDevice(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeDimension(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeFIDChanColor(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeFIDChanPrefs(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeFIDGain(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeFIDOffset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeInc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeInitOrFinal(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeInstDelay(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeInstrVary(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeLoadInfoMode(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK ChangeNDPointMenu(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK ChangeNDTimeUnits(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeNP_AT_SR(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeNumCycles(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeNumSteps(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangePBDevice(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangePhaseCorrectionOrder(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangePhaseCycleLevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangePhaseCycleStep(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangePhaseKnob(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangePolyFitOrder(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangePolySubtract(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeSpecChanPrefs(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeSpectrumChanColor(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeSpectrumChannel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeSpectrumGain(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeSpectrumOffset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeTransients(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK ChangeTransientView(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK ChangeTUnits(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeViewingTransient(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CollapsePhaseCycle(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ColorVal(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ContinuousRunCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ControlHidden(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DatChangeIDPos(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DeleteInstructionCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DirectorySelect(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK EditExpression(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK EditSkipCondition(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ExpandPhaseCycle(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FEChangeDelayInstr(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FEChangeInstrDataInstr(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FEEnableDelay(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FEEnableInstrData(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FENewFunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FIDGraphClick(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FuncEditQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FuncEditSave(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FuncEditSaveAndClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK InstNumChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK InstrCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK InstrChangeCycleNum(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK InstrDataCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK LoadConfigurationFromFile(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK LoadDataMenu(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK LoadProgram(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK LoadProgramMenu(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK MoveInst(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveInstButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK NewAcquisitionMenu(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK NewProgram(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK NewProgramMenu(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK NumDimensionCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK PanGraph(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK PhaseCycleInstr(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PopoutTab(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ProgChangeIDPos(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK QuitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK QuitCallbackMenu(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK ResetIDPos(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK SaveConfig(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK SaveConfigToFile(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK SaveProgram(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK SaveProgramMenu(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK SetupSkipCondition(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SpectrumCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK StartProgram(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK StopProgram(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ToggleBrokenTTL(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ToggleFIDChan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ToggleND(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ToggleSpecChan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK UpdateDAQMenuCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK ViewProgramChart(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK ZoomGraph(int menubar, int menuItem, void *callbackData, int panel);


#ifdef __cplusplus
    }
#endif
