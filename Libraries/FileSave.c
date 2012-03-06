//////////////////////////////////////////////////////////////////////
// 																	//
//	      				File Saving Library							//
//					Paul Ganssle, 03/05/2012						//
//																	//
//	This library is intended to create a simple binary file that	//
// saves the data and the programs that are passed to it, rather 	//
// than relying on over-complicated TDM files and the like.			//
// 																	//
//////////////////////////////////////////////////////////////////////

#include <ansi_c.h>
#include <FileSave.h>
#include <ErrorDefs.h>
#include <ErrorLib.h>
#include <General.h>

/*************** FSave Functions ********************/
fsave make_fs(char *name) {
	fsave f = {
		.name = name,
		.ns = strlen(name)+1	// Include null character
	};

	return f;
}

int put_fs(fsave *fs, void *val, unsigned int type, unsigned int NumElements) {
	if(fs == NULL) { return -1; }
	if(NumElements <= 0) { return -2; }
	if(!is_valid_fs_type(type)) { return -3; }
	
	fs->type = type;
	fs->size = get_fs_type_size(type)*NumElements;
	
	// Don't try to cast a null pointer.
	if(val == NULL) { 
		fs->val.c = NULL; 
		return 0;
	}
	
	// May be able to remove this step by adding a void* in the union.
	switch(type) {
		case FS_CHAR:
		case FS_CONTAINER:
		case FS_CUSTOM:
			fs->val.c = (char *)val;
			break;
		case FS_UCHAR:
			fs->val.uc = (unsigned char *)val;
			break;
		case FS_INT:
			fs->val.i = (int *)val;
			break;
		case FS_UINT:
			fs->val.ui = (unsigned int *)val;
			break;
		case FS_FLOAT:
			fs->val.f = (float *)val;
			break;
		case FS_DOUBLE:
			fs->val.d = (double *)val;
			break;
		case FS_TIME:
			fs->val.t = (time_t *)val;
			break;
	}
	
	// Memcpy it over to a new array so val can be freed later
	void *nval = NULL;
	nval = malloc(fs->size);
	memcpy(nval, fs->val.c, fs->size);
	fs->val.c = nval;
	
	return 0;
}

int put_fs_container(fsave *cont, fsave *fs, unsigned int NumElements) {
	char *c = NULL;
	int rv = 0;
	
	cont->type = FS_CONTAINER;
	
	unsigned int cs = 0;
	for(int i = 0; i < NumElements; i++) {
		cs += get_fs_strlen(&fs[i]);		
	}
	
	cont->size = cs;
	c = malloc(cs);
	char *pos = c;
	
	for(int i = 0; i < NumElements; i++) { 
		pos = print_fs(pos, &fs[i]);
		if(pos == NULL) {
			rv = -1;
			goto error;
		}
	}
	
	cont->val.c = c;
	
	error:
	
	if(rv) { 
		if(c != NULL) { free(c); }
		cont->val.c = NULL;
	}
	
	return rv;
}

int put_fs_custom(fsave *fs, void *val, unsigned int num_entries, unsigned int *types, char **names, unsigned int NumElements) {
	// Puts the values in val (saved as char-array) into an FS_CUSTOM fsave.
	//
	// Inputs;
	// fs 			-> The fsave to which to save the values
	// val			-> A char array containing the full array of the things.
	// num_entries	-> How many subtypes in the custom type
	// types		-> Array containing the types (with FS_TYPE convention)
	// names		-> The names of each field
	// NumElements	-> The number of elements in the array.
	
	if(val == NULL) { return MCPP_ERR_NOSTRING; }
	if(num_entries == 0) { return MCPP_ERR_CUST_NOENTRIES; } 
	
	int rv = 0, i;
	unsigned int *ns = malloc(sizeof(unsigned int)*num_entries);
	unsigned char *ctypes = malloc(sizeof(unsigned char)*num_entries);
	char *c = NULL;
	
	fs->type = FS_CUSTOM;
	
	unsigned int cs = 0;
	size_t size = 0, nsizesum = 0, si = sizeof(unsigned int), si8 = sizeof(unsigned char);

	// Figure out the base size of the element, plus the lengths of all the names.
	for(i = 0; i < num_entries; i++) {
		if(!is_valid_fs_type(types[i])) { rv = MCPP_ERR_INVALID_TYPE; goto error; }
		
		if(names[i] != NULL) { 
			ns[i] = strlen(names[i])+1; 
		} else {
			ns[i] = 0;
		}
		
		nsizesum += ns[i];
		size += get_fs_type_size(types[i]);
		
		ctypes[i] = (unsigned char)types[i];
	}
	
	fs->size = size*NumElements + nsizesum; // The raw data contained in the fsave.
	fs->size += si*(num_entries + 1);		// Space for the name sizes, plus one for num_entries.
	fs->size += si8*num_entries; 			// Space for the type definitions
	
	
	// Create the char array.
	c = calloc(fs->size, 1);
	char *pos = c;
	
	memcpy(pos, &num_entries, si);	// Starts with the number of entries.
	pos += si;
	for(i = 0; i < num_entries; i++) {
		// Now the size of the name.
		memcpy(pos, &ns[i], si);
		pos += si;
		
		// Then the name itself
		if(ns[i] != 0) {
			memcpy(pos, names[i], ns[i]);
			pos += ns[i];
		}
		
		// Then the type, unsigned char
		memcpy(pos, &ctypes[i], si8);
		pos += si8;
	}
	
	// Now copy the value of the thing into the array.
	memcpy(pos, val, size*NumElements);
	fs->val.c = malloc(fs->size);
	
	// Move the array to the fsave and clear our pointer to it - don't free c.
	memcpy(fs->val.c, c, fs->size);
	c = NULL;
	
	error:
	// Some of these null checks are currently unnecessary, but I'm leaving them in
	// in case this function changes and we want to free these vars before this.
	if(c != NULL) { free(c); }
	if(ns != NULL) { free(ns); }
	if(ctypes != NULL) { free(ctypes); }
	
	if(rv != 0) {
		*fs = free_fsave(fs);
	}
	
	return rv;
}

fsave read_fsave_from_char(char *array, int *ev) {
	if(array == NULL) { *ev = MCPP_ERR_NOSTRING; return null_fs(); }
	
	int rv = 0;
	char *pos = array;
	fsave fs = null_fs();
	size_t si = sizeof(unsigned int), si8 = sizeof(unsigned char);
	
	// Name length
	memcpy(&(fs.ns), pos, si);
	pos += si;
	
	// Get the name
	if(fs.ns > 0) {
		fs.name = malloc(fs.ns);
		memcpy(fs.name, pos, fs.ns);
		pos += fs.ns;
		
		if(fs.name[fs.ns-1] != '\0') {
			rv = MCPP_ERR_MALFORMED_FNAME;
			goto error;
		}
		
	} else {
		fs.name = NULL;
	}
	
	// Get the type
	memcpy(&(fs.type), pos, si8);
	pos += si8;
	
	// Get the size
	memcpy(&(fs.size), pos, si);
	pos += si;
	
	// Get the contents
	if(fs.size < 1) {
		fs.val.c = NULL;	
	} else {
		fs.val.c = malloc(fs.size);
		memcpy(fs.val.c, pos, fs.size);
		pos +=fs.size;
	}
	
	error:
	if(rv) {
		fs = free_fsave(&fs);	
	}
	
	*ev = rv;
	return fs;
}

fsave *read_all_fsaves_from_char(char *array, int *ev, unsigned int *fs_size, size_t size) {
	if(array == NULL) { *ev = MCPP_ERR_NOSTRING; return NULL; }
	
	int rv =0, i;
	char *pos = array;
	fsave *fs = NULL;
	*fs_size = 0;
	unsigned int fss = 0;
	size_t si = sizeof(unsigned int), si8 = sizeof(unsigned char);
	
	fss = 1;
	for(i = 0; i < fss; i++) {
		fs = malloc_or_realloc(fs, sizeof(fsave)*fss);
		
		fs[i] = read_fsave_from_char(pos, &rv);
		if(rv != 0) { goto error; }
		
		pos += get_fs_strlen(&fs[i]);
		if((pos-array) >= size) { break; }
		fss++;
	}
	
	error:
	if(rv != 0) {
		free_fsave_array(fs, fss);
		fss = 0;
	}
	
	*ev = rv;
	*fs_size = fss;
	return fs;
}

char *print_fs(char *array, fsave *fs) {
	// Print an FS into an array and advance the array.
	// These will always have the same form:
	// UInt -> Length of the name of the thing
	// Char[] -> The name
	// UChar -> The type of the value (saved as UChar, cast from UInt);
	// UInt -> Size of the value in bytes					 
	// Char - > The value itself (implicit cast to Char via Union)
	
	if(array == NULL) { return NULL; }
	char *pos = array;
	
	size_t si = sizeof(unsigned int);
	
	// First the field name
	memcpy(pos, &(fs->ns), si);
	pos += si;
	
	memcpy(pos, fs->name, fs->ns);
	pos += fs->ns;
	
	// Now the field value.
	memcpy(pos, &(fs->type), sizeof(unsigned char));
	pos += sizeof(unsigned char);
	
	memcpy(pos, &(fs->size), si);
	pos += si;
	
	// If there are no vals, nothing to copy.
	if(fs->val.c == NULL) { 
		if(fs->type != FS_CONTAINER) { return NULL; }
		
		return pos;
	}
	
	memcpy(pos, fs->val.c, fs->size); 
	pos += fs->size;
	
	return pos;
}

size_t get_fs_type_size(unsigned int type) {
	switch(type) {
		case FS_CHAR:
		case FS_UCHAR:
		case FS_CUSTOM:
		case FS_CONTAINER:
			return sizeof(char);
		case FS_INT:
		case FS_UINT:
			return sizeof(int);
		case FS_FLOAT:
			return sizeof(float);
		case FS_DOUBLE:
			return sizeof(double);
	}
	return 0;
		
}

unsigned int get_fs_strlen(fsave *fs) {
	// Static sizes:	
	// UInt -> Length of name of thing
	// UInt -> Size of the value in bytes
	// UChar -> Type of the thing
	//
	// Dynamic values
	// fs.ns -> Name of the thing
	// fs.size -> Full size of the thing.
	
	return (sizeof(unsigned int)*2 + sizeof(unsigned char) + fs->ns + fs->size);
}

int is_valid_fs_type(unsigned int type) {
	switch(type) {
		case FS_CHAR:
		case FS_UCHAR:
		case FS_INT:
		case FS_UINT:
		case FS_FLOAT:
		case FS_DOUBLE:
		case FS_TIME:
		case FS_CONTAINER:
		case FS_CUSTOM:
			return 1;
	}
	
	return 0;
}

fsave free_fsave(fsave *fs) {
	if(fs != NULL && fs->val.c != NULL) {
		free(fs->val.c);
	}
	
	return null_fs();
}

fsave *free_fsave_array(fsave *fs, int size) {
	if(fs != NULL) {
		for(int i = 0; i < size; i++) { 
			if(fs[i].val.c != NULL) { free(fs[i].val.c); }
		}
		
		free(fs);
	}
	
	return NULL;
}

fsave null_fs() {
	fsave n = {.name = NULL, .ns = 0, .val.c = NULL, .type = FS_NULL, .size = 0};
	return n;
}

/*************** FLocs Functions ********************/ 
flocs read_flocs_from_file(FILE *f, int *ev) {
	// Create the flocs from a file, getting the list of containers
	int rv = 0;
	flocs fl = null_flocs();
	
	if(f == NULL) {
		rv = MCPP_ERR_NOFILE;
		goto error;
	}
	
	size_t nlen, clen;
	size_t si = sizeof(unsigned int), si8 = sizeof(unsigned char);
	unsigned int type;
	
	rewind(f);
	
	// Read the first bit.
	unsigned long int pos;
	size_t count = fread(&nlen, si, 1, f);
	if(count != 1 || feof(f)) {
		rv = MCPP_ERR_FILEREAD;
		goto error;
	}

	fl.num = 1;
	for(int i = 0; i < fl.num; i++) {
		// First read out the one we've already found.
		fl.name = malloc_or_realloc(fl.name, sizeof(char*)*fl.num);
		fl.size = malloc_or_realloc(fl.size, sizeof(unsigned int)*fl.num);
		fl.pos = malloc_or_realloc(fl.pos, sizeof(unsigned long int)*fl.num);
		fl.type = malloc_or_realloc(fl.type, sizeof(unsigned char)*fl.num);
		
		fl.name[i] = calloc(nlen, 1);

		count = fread(fl.name[i], 1, nlen, f);
		if(count != nlen) {
			rv = MCPP_ERR_FLOC_NAME;
			goto error;
		}
		
		count = fread(&(fl.type[i]), si8, 1, f);
		if(count != 1) { 
			rv = MCPP_ERR_FLOC_TYPE;
			goto error;
		}
		
		count = fread(&(fl.size[i]),si, 1, f);
		if(count != 1) {
			rv = MCPP_ERR_FLOC_SIZE;
			goto error;
		}
		
		fl.pos[i] = ftell(f);	// Save the position.
		
		// Now seek the next one.
		fseek(f, fl.size[i], SEEK_CUR);
		if(feof(f)) { break; }
		
		pos = ftell(f);
		count = fread(&nlen, si, 1, f);
		if(count != 1) {
			rv MCPP_ERR_FILEREAD;
			goto error;
		}
		
		fl.num++;
	}
	
	error:
	if(rv) {
		fl = free_flocs(&fl);
	}
	
	*ev = rv;
	return fl;
}

flocs read_flocs_from_char(char *array, int *ev, size_t size) {
	// Generates flocs from a char array you've already read out.
	if(array == NULL) { *ev = MCPP_ERR_NOSTRING; return null_flocs(); }
	
	// Indicator for reading from char array.
	int rv = 0;
	char *cpos = array;
	size_t si = sizeof(unsigned int), si8 = sizeof(unsigned char);
	unsigned int nlen;
	unsigned char type;
	flocs fl = null_flocs();
	
	fl.num = 1;
	
	// Read out all the flocs until there are no more, reallocating arrays as we go.
	for(int i = 0; i < fl.num; i++) {
		memcpy(&nlen, cpos, si);
		cpos += si;
		
		if(nlen > cpos-array+size) {
			rv = MCPP_ERR_MALFORMED_FNAME;
			goto error;
		}
		
		fl.name = malloc_or_realloc(fl.name, sizeof(char *)*fl.num);
		fl.pos = malloc_or_realloc(fl.pos, sizeof(unsigned long int)*fl.num);
		fl.size = malloc_or_realloc(fl.pos, si*fl.num);
		fl.type = malloc_or_realloc(fl.pos, si8*fl.num);
		
		if(nlen > 0) { 
			fl.name[i] = malloc(nlen);
		} else { 
			fl.name[i] = NULL;
		}
		
		memcpy(&(fl.name[i]), cpos, nlen);
		cpos += nlen;
		
		if(fl.name[nlen-1] != '\0') {
			rv = MCPP_ERR_MALFORMED_FNAME;
			goto error;
		}
		
		memcpy(&(fl.type[i]), cpos, si8);
		cpos += si8;
	
		memcpy(&(fl.size[i]), cpos, si);
		cpos += si;
		
		fl.pos[i] = (unsigned long int)(cpos-array);
		
		if((fl.pos[i] + fl.size[i]) >= size) { break; }
		
		cpos += fl.size[i];
		fl.num++;
	}
	
	error:
	if(rv != 0) {
		free_flocs(&fl);
		fl = null_flocs();
	}
	
	*ev = rv;
	return fl;
}

flocs null_flocs() {
	int rv = 0;
	flocs fl = {.name = NULL, .size = NULL, .pos = NULL, .type = NULL, .num = 0 }; 

	return fl;
}

flocs free_flocs(flocs *fl) {
	if(fl != NULL) { 
		if(fl->name != NULL) { free_string_array(fl->name, fl->num); }
		if(fl->size != NULL) { free(fl->size); }
		if(fl->pos != NULL) { free(fl->pos); }
		if(fl->type != NULL) { free(fl->type); }
	}
	
	return null_flocs();
}
