#include <stdio.h>
#include <time.h>

#define MCF_EOF -1
#define MCF_WRAP -2


#define MCF_ERR_INVALID_MAX_BYTES -10500
#define MCF_ERR_FSAVE_NOT_FOUND -10501
#define MCF_ERR_BAD_FSAVE_REPLACEMENT_NAME -10502;
#define MCF_ERR_FSAVE_TYPE_MISMATCH -10503;
#define MCF_ERR_FSAVE_SIZE_MISMATCH -10504;
#define MCF_ERR_CANNOT_TRUNCATE -10505;

#define MCF_ERR_INVALID_MAX_BYTES_STR "Invalid maximum bytes paramater passed."
#define MCF_ERR_FSAVE_NOT_FOUND_STR "Requested fsave not found. This may not be an error."
#define MCF_ERR_BAD_FSAVE_REPLACEMENT_NAME_STR "FSAVE names do not match, cannot replace safely!"
#define MCF_ERR_FSAVE_TYPE_MISMATCH_STR "FSAVE types do not match. This may not be an error."
#define MCF_ERR_FSAVE_SIZE_MISMATCH_STR "FSAVE sizes do not matc. This may not be an error."


// Data saving library prototypes.
// Types
typedef struct fsave {
	char *name;					// Name of the field
	unsigned int ns;			// Size of the name of the field
	
	union {
		char *c;				// Type 1
		unsigned char *uc;		// Type 2
		
		int *i;					// Type 3
		unsigned int *ui;		// Type 4
		
		float *f;				// Type 5
		double *d;				// Type 6
		
		time_t *t;				// Type 7
	} val;
	
	unsigned int type;
	unsigned int size;			// Total size (in bytes)
} fsave;

typedef struct flocs {
	char **name;
	
	unsigned int *size;
	unsigned long int *pos;
	unsigned char *type;
	
	int num;
} flocs;

// The Types
#define FS_NULL 0
#define FS_CHAR 1
#define FS_UCHAR 2
#define FS_INT 3
#define FS_UINT 4
#define FS_FLOAT 5
#define FS_DOUBLE 6
#define FS_TIME 7
#define FS_CONTAINER 32
#define FS_CUSTOM 64

/************ FSave Functions *************/
extern fsave make_fs(char *name);

extern int put_fs(fsave *fs, void *val, unsigned int type, unsigned int NumElements);
extern int put_fs_container(fsave *cont, fsave *fs, unsigned int NumElements);
extern int put_fs_custom(fsave *fs, void *val, unsigned int num_entries, unsigned int *types, char **names, unsigned int NumElements);

extern int get_fs_header_from_file(FILE *f, fsave *fs);
extern int get_fs_header_size_from_ns(int ns);
extern int get_fs_header_size_from_file(FILE *f);
extern int get_fs_header_size_from_char(char *array);

extern int overwrite_fsave_in_file(FILE *f, fsave *fs);

extern long find_fsave_in_file(FILE *f, char *fs_name, long max_bytes);
extern fsave read_fsave_from_file(FILE *f, int *ev);
extern fsave read_fsave_from_char(char *array, int *ev);
extern fsave *read_all_fsaves_from_char(char *array, int *ev, unsigned int *fs_size, size_t size);
extern fsave *read_all_fsaves_from_file(FILE *f, int *ev, unsigned int *fs_size, long max_bytes);

extern char *print_fs(char *array, fsave *fs);

extern size_t get_fs_type_size(unsigned int type);
extern size_t get_fs_strlen(fsave *fs);
extern int is_valid_fs_type(unsigned int type);   

extern fsave free_fsave(fsave *fs);
extern fsave *free_fsave_array(fsave *fs, int size);
extern fsave null_fs(void);

/************ FLocs Functions *************/ 
extern flocs read_flocs_from_file(FILE *f, int *ev, long max_bytes);
extern flocs read_flocs_from_char(char *array, int *ev, size_t size);

extern flocs null_flocs(void);
extern flocs free_flocs(flocs *fl);

