#ifndef MATH_PARSER_LIB_H
#define MATH_PARSER_LIB_H

#define C_INT 0
#define C_DOUBLE 1

typedef struct constants {
	// Structure for defining constants
	int num; // Number of elements
	int num_ints; // Number of integers
	int num_doubles; // Number of doubles
	char **c_names; // Constant names
	
	int *c_types; // Constant types
	int *c_locs; // Locations in their arrays
	
	int *c_ints; // Array of integers
	double *c_doubles; // Array of doubles
} constants;

extern void add_constant(constants *c,
                               char *name, int type, void *val);
extern void change_constant(constants *c, char *name, int type, void *val);
extern constants *malloc_constants(void);
extern void free_constants(constants *c);
extern int find_paren_contents(char *string,
                               char *outstring);
extern char *replace_chars(char *main_str,
                           char *reps, int s, int e, int *err_val);
extern char *del_char(char *input, int index);
extern char *strrep(char *str, char c1, char c2);
extern char *eval_funcs_and_constants(char *expr,
                                      constants *c, int *err_val,
                                      int n_pos);
extern double parse_math(char *expr, constants *c,
                         int *err_val, int s_pos);

extern int get_parse_error(int err_code, char *err_message);

#endif
