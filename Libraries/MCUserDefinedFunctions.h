/*****************    Global Variables   ******************/
extern int lock_pb;
extern int lock_DAQ;
extern int lock_tdm;
extern int lock_uidc;
extern int lock_uipc;
extern int lock_ce;
extern int lock_af;

/*************************** Function Declarations ***************************/
DeclareThreadSafeScalarVar(int, QuitUpdateStatus); 
DeclareThreadSafeScalarVar(int, QuitIdle);
DeclareThreadSafeScalarVar(int, DoubleQuitIdle);
DeclareThreadSafeScalarVar(int, Status);
DeclareThreadSafeScalarVar(int, Running);
DeclareThreadSafeScalarVar(int, Initialized);
DeclareThreadSafeScalarVar(int, PhaseUpdating);

/***************** Function Declarations ******************/

