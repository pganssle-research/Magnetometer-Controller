#define FILE_SAVE_H

#include <stdio.h>
#include <time.h>
#include <FileSaveTypes.h>

#define MCF_EOF -1
#define MCF_WRAP -2

// File I/O
extern int fwrite_fs(FILE *f, fsave *fs);
extern int fadd_fs_to_container(FILE *f, fsave *fs);
extern int fappend_data_to_fs(FILE *f, void *data, unsigned int data_size);
extern int overwrite_fsave_in_file(FILE *f, fsave *fs);
extern int find_and_overwrite_fsave_in_file(FILE *f, fsave *fs, unsigned int max_bytes);
extern int replace_fsaves(FILE *f, char *parent, fsave *list, int nfs, int *found);

/************ FSave Functions *************/
extern fsave make_fs(char *name);

extern int put_fs(fsave *fs, void *val, unsigned int type, unsigned int NumElements);
extern int put_fs_container(fsave *cont, fsave *fs, unsigned int NumElements);
extern int put_fs_custom(fsave *fs, void *val, unsigned int num_entries, unsigned int *types, char **names, unsigned int NumElements);

extern int get_fs_header_from_file(FILE *f, fsave *fs);
extern int get_fs_header_size_from_ns(int ns);
extern int get_fs_header_size_from_file(FILE *f);
extern int get_fs_header_size_from_char(char *array);

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

