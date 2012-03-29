// Error library
#define ERROR_LIB_H
extern void display_error(int err_val);

extern char *get_mc_error_title(unsigned int err_index); 
extern char *get_err_string(int err_val, unsigned int err_type);

extern unsigned int is_mc_error(int ev);
