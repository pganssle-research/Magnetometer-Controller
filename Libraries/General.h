/********************************** Includes *********************************/
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

/************** General Utilities *************/ 
extern int string_in_array(char **array, char *string, int size);
extern int *strings_in_array(char **array, char **strings, int size1, int size2);
extern int int_in_array(int *array, int val, int size);
extern int *add_item_to_sorted_array(int *array, int item, int size);

extern char **free_string_array(char **array, int size);
extern double **free_doubles_array(double **array, int size);
extern int **free_ints_array(int **array, int size);

extern void *malloc_or_realloc(void *pointer, size_t size);

/************** Safe Pulseblaster Functions *************/
extern int pb_init_safe(int verbose);
extern int pb_start_programming_safe(int verbose, int device);
extern int pb_stop_programming_safe(int verbose);
extern int pb_inst_pbonly_safe(int verbose, unsigned int flags, int inst, int inst_data, double length);
extern int pb_close_safe(int verbose);
extern int pb_read_status_safe(int verbose);
extern int pb_start_safe(int verbose);
extern int pb_stop_safe(int verbose);
