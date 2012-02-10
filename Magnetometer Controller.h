/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2012. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  AOConP                           1
#define  AOConP_NumAOuts                  2       /* control type: numeric, callback function: ChangeNumAOuts */
#define  AOConP_InitV                     3       /* control type: textMsg, callback function: (none) */
#define  AOConP_IncLab                    4       /* control type: textMsg, callback function: (none) */
#define  AOConP_FinalLab                  5       /* control type: textMsg, callback function: (none) */
#define  AOConP_StepsLab                  6       /* control type: textMsg, callback function: (none) */
#define  AOConP_DimLab                    7       /* control type: textMsg, callback function: (none) */
#define  AOConP_NDOnLab                   8       /* control type: textMsg, callback function: (none) */
#define  AOConP_DeviceLab                 9       /* control type: textMsg, callback function: (none) */
#define  AOConP_OutputLab                 10      /* control type: textMsg, callback function: (none) */
#define  AOConP_AOTitle                   11      /* control type: textMsg, callback function: (none) */

#define  AOInstPan                        2
#define  AOInstPan_AOutChan               2       /* control type: ring, callback function: ChangeAOutChan */
#define  AOInstPan_DimRing                3       /* control type: ring, callback function: ChangeAOChanDim */
#define  AOInstPan_ChanDev                4       /* control type: ring, callback function: ChangeAODev */
#define  AOInstPan_ChanValFin             5       /* control type: numeric, callback function: (none) */
#define  AOInstPan_ChanIncVal             6       /* control type: numeric, callback function: (none) */
#define  AOInstPan_InitChanVal            7       /* control type: numeric, callback function: ChangeAOVal */
#define  AOInstPan_NDToggle               8       /* control type: LED, callback function: (none) */
#define  AOInstPan_ExpressionCtrl         9       /* control type: string, callback function: ModifyAOChanInstr */
#define  AOInstPan_ChanNumSteps           10      /* control type: numeric, callback function: ChangeChanNumSteps */
#define  AOInstPan_xButton                11      /* control type: pictButton, callback function: DeleteAOInstr */

#define  BrokenTTLs                       3
#define  BrokenTTLs_TTL23                 2       /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL22                 3       /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL21                 4       /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL20                 5       /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL19                 6       /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL18                 7       /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL17                 8       /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL16                 9       /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL15                 10      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL14                 11      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL13                 12      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL12                 13      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL11                 14      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL10                 15      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL9                  16      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL8                  17      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL7                  18      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL6                  19      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL2                  20      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL5                  21      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL1                  22      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL4                  23      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL0                  24      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_TTL3                  25      /* control type: LED, callback function: ToggleBrokenTTL */
#define  BrokenTTLs_Exit                  26      /* control type: command, callback function: BrokenTTLsExit */
#define  BrokenTTLs_ClearAll              27      /* control type: command, callback function: BrokenTTLsClearAll */

#define  CurrentLoc                       4
#define  CurrentLoc_TransientNum          2       /* control type: ring, callback function: ChangeViewingTransient */
#define  CurrentLoc_ID8                   3       /* control type: textMsg, callback function: (none) */
#define  CurrentLoc_ID7                   4       /* control type: textMsg, callback function: (none) */
#define  CurrentLoc_ID6                   5       /* control type: textMsg, callback function: (none) */
#define  CurrentLoc_ID5                   6       /* control type: textMsg, callback function: (none) */
#define  CurrentLoc_ID4                   7       /* control type: textMsg, callback function: (none) */
#define  CurrentLoc_ID3                   8       /* control type: textMsg, callback function: (none) */
#define  CurrentLoc_ID2                   9       /* control type: textMsg, callback function: (none) */
#define  CurrentLoc_ID1                   10      /* control type: textMsg, callback function: (none) */
#define  CurrentLoc_IDVal8                11      /* control type: ring, callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal7                12      /* control type: ring, callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal6                13      /* control type: ring, callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal5                14      /* control type: ring, callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal4                15      /* control type: ring, callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal3                16      /* control type: ring, callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal2                17      /* control type: ring, callback function: DatChangeIDPos */
#define  CurrentLoc_IDVal1                18      /* control type: ring, callback function: DatChangeIDPos */

#define  EditFunc                         5
#define  EditFunc_QUITBUTTON              2       /* control type: command, callback function: FuncEditQuit */
#define  EditFunc_NewFunc                 3       /* control type: command, callback function: FENewFunc */
#define  EditFunc_Save                    4       /* control type: command, callback function: FuncEditSave */
#define  EditFunc_SaveAndClose            5       /* control type: command, callback function: FuncEditSaveAndClose */
#define  EditFunc_DelayInstr              6       /* control type: numeric, callback function: FEChangeDelayInstr */
#define  EditFunc_InstrDataInstr          7       /* control type: numeric, callback function: FEChangeInstrDataInstr */
#define  EditFunc_NumInst                 8       /* control type: numeric, callback function: InstNumChange */
#define  EditFunc_Trigger_TTL             9       /* control type: numeric, callback function: Change_Trigger */
#define  EditFunc_EnableDelay             10      /* control type: LED, callback function: FEEnableDelay */
#define  EditFunc_EnableInstrData         11      /* control type: LED, callback function: FEEnableInstrData */

#define  HiddenVals                       6
#define  HiddenVals_PolySubtractOnOffRing 2       /* control type: ring, callback function: (none) */
#define  HiddenVals_PolyOrderValues       3       /* control type: ring, callback function: (none) */
#define  HiddenVals_PhaseCorrectionValues 4       /* control type: table, callback function: (none) */
#define  HiddenVals_ChannelOffsets        5       /* control type: table, callback function: (none) */
#define  HiddenVals_PlotIDs               6       /* control type: table, callback function: (none) */
#define  HiddenVals_ChannelGains          7       /* control type: table, callback function: (none) */
#define  HiddenVals_ATConstant            8       /* control type: ring, callback function: (none) */
#define  HiddenVals_TransientView         9       /* control type: ring, callback function: (none) */
#define  HiddenVals_SRConstant            10      /* control type: ring, callback function: (none) */
#define  HiddenVals_NPConstant            11      /* control type: ring, callback function: (none) */
#define  HiddenVals_LastProgramLoc        12      /* control type: string, callback function: (none) */
#define  HiddenVals_ControlDimmed         13      /* control type: tree, callback function: (none) */
#define  HiddenVals_ControlHidden         14      /* control type: tree, callback function: ControlHidden */
#define  HiddenVals_RecentData            15      /* control type: ring, callback function: (none) */
#define  HiddenVals_RecentPrograms        16      /* control type: ring, callback function: (none) */
#define  HiddenVals_SpectrumChanColor     17      /* control type: ring, callback function: ColorVal */
#define  HiddenVals_FIDChanColor          18      /* control type: ring, callback function: ColorVal */
#define  HiddenVals_TTLBroken             19      /* control type: ring, callback function: (none) */

#define  MainPanel                        7
#define  MainPanel_MainTabs               2       /* control type: tab, callback function: PopoutTab */
#define  MainPanel_Start                  3       /* control type: command, callback function: StartProgram */
#define  MainPanel_Stop                   4       /* control type: command, callback function: StopProgram */
#define  MainPanel_QUITBUTTON             5       /* control type: command, callback function: QuitCallback */
#define  MainPanel_CurrentFile            6       /* control type: string, callback function: (none) */
#define  MainPanel_Filename               7       /* control type: string, callback function: (none) */
#define  MainPanel_DirectorySelect        8       /* control type: pictButton, callback function: DirectorySelect */
#define  MainPanel_Running                9       /* control type: LED, callback function: (none) */
#define  MainPanel_Waiting                10      /* control type: LED, callback function: (none) */
#define  MainPanel_Stopped                11      /* control type: LED, callback function: (none) */
#define  MainPanel_Path                   12      /* control type: string, callback function: (none) */
#define  MainPanel_IsRunning              13      /* control type: LED, callback function: (none) */
#define  MainPanel_PBStatus               14      /* control type: textMsg, callback function: (none) */
#define  MainPanel_ProgDesc               15      /* control type: textBox, callback function: (none) */
#define  MainPanel_DataDescription        16      /* control type: textBox, callback function: (none) */
#define  MainPanel_SPLITTER_2             17      /* control type: splitter, callback function: (none) */
#define  MainPanel_SPLITTER               18      /* control type: splitter, callback function: (none) */
#define  MainPanel_CurrentProgRing        19      /* control type: ring, callback function: (none) */
#define  MainPanel_COMMANDBUTTON          20      /* control type: command, callback function: (none) */
#define  MainPanel_DataDirectory          21      /* control type: listBox, callback function: ChangeDataBox */
#define  MainPanel_LoadInfoMode           22      /* control type: LED, callback function: ChangeLoadInfoMode */

#define  MDInstr                          8
#define  MDInstr_InstrNum                 2       /* control type: numeric, callback function: (none) */
#define  MDInstr_Instructions             3       /* control type: ring, callback function: (none) */
#define  MDInstr_VaryInstr                4       /* control type: LED, callback function: ChangeInstrVary */
#define  MDInstr_FInstrData               5       /* control type: numeric, callback function: ChangeInitOrFinal */
#define  MDInstr_InitInstrData            6       /* control type: numeric, callback function: ChangeInitOrFinal */
#define  MDInstr_IncInstrData             7       /* control type: numeric, callback function: ChangeInc */
#define  MDInstr_FTime                    8       /* control type: numeric, callback function: ChangeInitOrFinal */
#define  MDInstr_FTimeUnits               9       /* control type: ring, callback function: ChangeNDTimeUnits */
#define  MDInstr_InitDisplay              10      /* control type: numeric, callback function: (none) */
#define  MDInstr_FDisplay                 11      /* control type: numeric, callback function: (none) */
#define  MDInstr_IncDisplay               12      /* control type: numeric, callback function: (none) */
#define  MDInstr_IncTime                  13      /* control type: numeric, callback function: ChangeInc */
#define  MDInstr_IncTimeUnits             14      /* control type: ring, callback function: ChangeNDTimeUnits */
#define  MDInstr_InitTime                 15      /* control type: numeric, callback function: ChangeInitOrFinal */
#define  MDInstr_InitTimeUnits            16      /* control type: ring, callback function: ChangeNDTimeUnits */
#define  MDInstr_NumSteps                 17      /* control type: numeric, callback function: ChangeNumSteps */
#define  MDInstr_Dimension                18      /* control type: ring, callback function: ChangeDimension */
#define  MDInstr_IncDataExpression        19      /* control type: string, callback function: EditExpression */
#define  MDInstr_IncDelayExpression       20      /* control type: string, callback function: EditExpression */

#define  PPConfigP                        9
#define  PPConfigP_VaryingLabel           2       /* control type: textMsg, callback function: (none) */
#define  PPConfigP_NumStepsLabel          3       /* control type: textMsg, callback function: (none) */
#define  PPConfigP_FinalLabel             4       /* control type: textMsg, callback function: (none) */
#define  PPConfigP_IncrementLabel         5       /* control type: textMsg, callback function: (none) */
#define  PPConfigP_InitialLabel           6       /* control type: textMsg, callback function: (none) */
#define  PPConfigP_InstrLabel             7       /* control type: textMsg, callback function: (none) */

#define  PPPanel                          10
#define  PPPanel_PhaseCycleLabel          2       /* control type: textMsg, callback function: (none) */
#define  PPPanel_ScanLabel                3       /* control type: textMsg, callback function: (none) */
#define  PPPanel_InstrLabel               4       /* control type: textMsg, callback function: (none) */
#define  PPPanel_DelayLabel               5       /* control type: textMsg, callback function: (none) */
#define  PPPanel_InstrNumLabel            6       /* control type: textMsg, callback function: (none) */
#define  PPPanel_TTLsLabel                7       /* control type: textMsg, callback function: (none) */

#define  PulseInstP                       11
#define  PulseInstP_InstNum               2       /* control type: numeric, callback function: MoveInst */
#define  PulseInstP_Instructions          3       /* control type: ring, callback function: InstrCallback */
#define  PulseInstP_Instr_Data            4       /* control type: numeric, callback function: InstrDataCallback */
#define  PulseInstP_TTL23                 5       /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL22                 6       /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL21                 7       /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL20                 8       /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL19                 9       /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL18                 10      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL17                 11      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL16                 12      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL15                 13      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL14                 14      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL13                 15      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL12                 16      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL11                 17      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL10                 18      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL9                  19      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL8                  20      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL7                  21      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL6                  22      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL5                  23      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL4                  24      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL3                  25      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL2                  26      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL1                  27      /* control type: LED, callback function: (none) */
#define  PulseInstP_TTL0                  28      /* control type: LED, callback function: (none) */
#define  PulseInstP_InstDelay             29      /* control type: numeric, callback function: ChangeInstDelay */
#define  PulseInstP_TimeUnits             30      /* control type: ring, callback function: ChangeTUnits */
#define  PulseInstP_Scan                  31      /* control type: LED, callback function: Change_Scan */
#define  PulseInstP_UpButton              32      /* control type: pictButton, callback function: MoveInstButton */
#define  PulseInstP_DownButton            33      /* control type: pictButton, callback function: MoveInstButton */
#define  PulseInstP_PhaseCycleLevel       34      /* control type: ring, callback function: ChangePhaseCycleLevel */
#define  PulseInstP_PhaseCycleStep        35      /* control type: ring, callback function: ChangePhaseCycleStep */
#define  PulseInstP_NumCycles             36      /* control type: numeric, callback function: InstrChangeCycleNum */
#define  PulseInstP_PhaseCyclingOn        37      /* control type: LED, callback function: PhaseCycleInstr */
#define  PulseInstP_xButton               38      /* control type: pictButton, callback function: DeleteInstructionCallback */
#define  PulseInstP_CollapseButton        39      /* control type: pictButton, callback function: CollapsePhaseCycle */
#define  PulseInstP_ExpandButton          40      /* control type: pictButton, callback function: ExpandPhaseCycle */

     /* tab page panel controls */
#define  AOutTab_NDimensionalOn           2       /* control type: LED, callback function: ToggleND */
#define  AOutTab_NumDimensions            3       /* control type: numeric, callback function: NumDimensionCallback */

     /* tab page panel controls */
#define  FID_Graph                        2       /* control type: graph, callback function: FIDGraphClick */
#define  FID_CursorY                      3       /* control type: numeric, callback function: (none) */
#define  FID_CursorX                      4       /* control type: numeric, callback function: (none) */
#define  FID_Chan8                        5       /* control type: LED, callback function: ToggleFIDChan */
#define  FID_Chan7                        6       /* control type: LED, callback function: ToggleFIDChan */
#define  FID_Chan6                        7       /* control type: LED, callback function: ToggleFIDChan */
#define  FID_Chan5                        8       /* control type: LED, callback function: ToggleFIDChan */
#define  FID_Chan4                        9       /* control type: LED, callback function: ToggleFIDChan */
#define  FID_Chan3                        10      /* control type: LED, callback function: ToggleFIDChan */
#define  FID_Chan2                        11      /* control type: LED, callback function: ToggleFIDChan */
#define  FID_Chan1                        12      /* control type: LED, callback function: ToggleFIDChan */
#define  FID_ChanLabel                    13      /* control type: textMsg, callback function: (none) */
#define  FID_ChanPrefs                    14      /* control type: ring, callback function: ChangeFIDChanPrefs */
#define  FID_PolySubtract                 15      /* control type: LED, callback function: ChangePolySubtract */
#define  FID_PolyFitOrder                 16      /* control type: numeric, callback function: ChangePolyFitOrder */
#define  FID_ChannelBox                   17      /* control type: deco, callback function: (none) */
#define  FID_PolyFitLabel                 18      /* control type: textMsg, callback function: (none) */
#define  FID_Offset                       19      /* control type: numeric, callback function: ChangeFIDOffset */
#define  FID_Gain                         20      /* control type: numeric, callback function: ChangeFIDGain */
#define  FID_ChanColor                    21      /* control type: color, callback function: ChangeFIDChanColor */
#define  FID_Autoscale                    22      /* control type: LED, callback function: (none) */

     /* tab page panel controls */
#define  PPConfig_SaveProgram             2       /* control type: command, callback function: SaveProgram */
#define  PPConfig_NTransients             3       /* control type: numeric, callback function: ChangeTransients */
#define  PPConfig_LoadProgram             4       /* control type: command, callback function: LoadProgram */
#define  PPConfig_NDimensionalOn          5       /* control type: LED, callback function: ToggleND */
#define  PPConfig_EstimatedTime           6       /* control type: numeric, callback function: (none) */
#define  PPConfig_NPoints                 7       /* control type: numeric, callback function: ChangeNP_AT_SR */
#define  PPConfig_SampleRate              8       /* control type: numeric, callback function: ChangeNP_AT_SR */
#define  PPConfig_Device                  9       /* control type: ring, callback function: ChangeDevice */
#define  PPConfig_CounterChan             10      /* control type: ring, callback function: (none) */
#define  PPConfig_TriggerEdge             11      /* control type: ring, callback function: (none) */
#define  PPConfig_Trigger_Channel         12      /* control type: ring, callback function: (none) */
#define  PPConfig_AcquisitionChannel      13      /* control type: ring, callback function: ChangeAcquisitionChannel */
#define  PPConfig_AcquisitionTime         14      /* control type: numeric, callback function: ChangeNP_AT_SR */
#define  PPConfig_NumDimensions           15      /* control type: numeric, callback function: NumDimensionCallback */
#define  PPConfig_NumChans                16      /* control type: numeric, callback function: (none) */
#define  PPConfig_ChannelGain             17      /* control type: ring, callback function: ChangeCurrentChan */
#define  PPConfig_SkipCondition           18      /* control type: LED, callback function: SetupSkipCondition */
#define  PPConfig_SkipConditionExpr       19      /* control type: string, callback function: EditSkipCondition */
#define  PPConfig_ChannelRange            20      /* control type: ring, callback function: ChangeChannelRange */
#define  PPConfig_TransAcqMode            21      /* control type: ring, callback function: (none) */
#define  PPConfig_PBDeviceSelect          22      /* control type: ring, callback function: ChangePBDevice */

     /* tab page panel controls */
#define  PulseProg_TransientNum           2       /* control type: ring, callback function: ProgChangeIDPos */
#define  PulseProg_ID8                    3       /* control type: textMsg, callback function: (none) */
#define  PulseProg_ID7                    4       /* control type: textMsg, callback function: (none) */
#define  PulseProg_ID6                    5       /* control type: textMsg, callback function: (none) */
#define  PulseProg_ID5                    6       /* control type: textMsg, callback function: (none) */
#define  PulseProg_ID4                    7       /* control type: textMsg, callback function: (none) */
#define  PulseProg_ID3                    8       /* control type: textMsg, callback function: (none) */
#define  PulseProg_ID2                    9       /* control type: textMsg, callback function: (none) */
#define  PulseProg_ID1                    10      /* control type: textMsg, callback function: (none) */
#define  PulseProg_IDVal8                 11      /* control type: ring, callback function: ProgChangeIDPos */
#define  PulseProg_IDVal7                 12      /* control type: ring, callback function: ProgChangeIDPos */
#define  PulseProg_IDVal6                 13      /* control type: ring, callback function: ProgChangeIDPos */
#define  PulseProg_IDVal5                 14      /* control type: ring, callback function: ProgChangeIDPos */
#define  PulseProg_IDVal4                 15      /* control type: ring, callback function: ProgChangeIDPos */
#define  PulseProg_IDVal3                 16      /* control type: ring, callback function: ProgChangeIDPos */
#define  PulseProg_IDVal2                 17      /* control type: ring, callback function: ProgChangeIDPos */
#define  PulseProg_IDVal1                 18      /* control type: ring, callback function: ProgChangeIDPos */
#define  PulseProg_NumInst                19      /* control type: numeric, callback function: InstNumChange */
#define  PulseProg_SaveProgram            20      /* control type: command, callback function: SaveProgram */
#define  PulseProg_NewProgram             21      /* control type: command, callback function: NewProgram */
#define  PulseProg_LoadProgram            22      /* control type: command, callback function: LoadProgram */
#define  PulseProg_Trigger_TTL            23      /* control type: numeric, callback function: Change_Trigger */
#define  PulseProg_ContinuousRun          24      /* control type: LED, callback function: ContinuousRunCallback */
#define  PulseProg_PhaseCycles            25      /* control type: numeric, callback function: ChangeNumCycles */
#define  PulseProg_ResetDims              26      /* control type: command, callback function: ResetIDPos */

     /* tab page panel controls */
#define  Spectrum_Graph                   2       /* control type: graph, callback function: SpectrumCallback */
#define  Spectrum_CursorY                 3       /* control type: numeric, callback function: (none) */
#define  Spectrum_CursorX                 4       /* control type: numeric, callback function: (none) */
#define  Spectrum_Channel                 5       /* control type: ring, callback function: ChangeSpectrumChannel */
#define  Spectrum_Chan8                   6       /* control type: LED, callback function: ToggleSpecChan */
#define  Spectrum_Chan7                   7       /* control type: LED, callback function: ToggleSpecChan */
#define  Spectrum_Chan6                   8       /* control type: LED, callback function: ToggleSpecChan */
#define  Spectrum_Chan5                   9       /* control type: LED, callback function: ToggleSpecChan */
#define  Spectrum_Chan4                   10      /* control type: LED, callback function: ToggleSpecChan */
#define  Spectrum_Chan3                   11      /* control type: LED, callback function: ToggleSpecChan */
#define  Spectrum_Chan2                   12      /* control type: LED, callback function: ToggleSpecChan */
#define  Spectrum_Chan1                   13      /* control type: LED, callback function: ToggleSpecChan */
#define  Spectrum_ChanLabel               14      /* control type: textMsg, callback function: (none) */
#define  Spectrum_ChannelBox              15      /* control type: deco, callback function: (none) */
#define  Spectrum_ChanPrefs               16      /* control type: ring, callback function: ChangeSpecChanPrefs */
#define  Spectrum_PolySubtract            17      /* control type: LED, callback function: ChangePolySubtract */
#define  Spectrum_PolyFitOrder            18      /* control type: numeric, callback function: ChangePolyFitOrder */
#define  Spectrum_PhaseCorrectionOrder    19      /* control type: ring, callback function: ChangePhaseCorrectionOrder */
#define  Spectrum_PhaseKnob               20      /* control type: scale, callback function: ChangePhaseKnob */
#define  Spectrum_Offset                  21      /* control type: numeric, callback function: ChangeSpectrumOffset */
#define  Spectrum_Gain                    22      /* control type: numeric, callback function: ChangeSpectrumGain */
#define  Spectrum_ChanColor               23      /* control type: color, callback function: ChangeSpectrumChanColor */
#define  Spectrum_Autoscale               24      /* control type: LED, callback function: (none) */
#define  Spectrum_PolyFitLabel            25      /* control type: textMsg, callback function: (none) */


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
int  CVICALLBACK ChangeAOChanDim(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeAODev(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeAOutChan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeAOVal(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeChannelRange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChangeChanNumSteps(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
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
int  CVICALLBACK ChangeNumAOuts(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
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
int  CVICALLBACK DeleteAOInstr(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
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
int  CVICALLBACK ModifyAOChanInstr(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
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
