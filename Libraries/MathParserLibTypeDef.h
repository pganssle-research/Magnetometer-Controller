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
