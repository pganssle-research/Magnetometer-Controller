//////////////////////////////////////////////////////////////////////
// 																	//
//	      General Operations and Multithreading Library				//
//					Paul Ganssle, 11\/02/2011						//
//																	//
//	This library is intended to deal with all the stuff that's used	//
//  in basically all of the other libraries, UI stuff, mulithreading//
//  stuff, etc.														//
// 																	//
//////////////////////////////////////////////////////////////////////

// Includes
#include <utility.h>
#include "toolbox.h"
#include <ansi_c.h>
#include <General.h>

//////////////////////////////////////////////////////////////
// 															//
//					Array Utilities							//
// 															//
//////////////////////////////////////////////////////////////

int string_in_array(char **array, char *string, int size) {
	// Returns the first string which is a perfect match
	// for a string in the array array. Returns -1 on failure,
	// returns index in array of size "size" on success.
	
	int i;
	for(i = 0; i < size; i++) {
		if(strcmp(string, array[i]) == 0)
			return i;
	}
	
	return -1;
}

int *strings_in_array(char **array, char **strings, int size1, int size2) {
	// Returns an array of size "size2" indicating where the strings in
	// the array of strings **strings is located in the array **array.
	// Elements will be -1 if the string isn't in array, they'll be 
	// the index value otherwise.
	//
	// size1 is the size of array.
	// size2 is the size of strings
	// Returns NULL on error.
	// Output needs to me freed.
	
	if(size1 == 0 || size2 == 0)
		return NULL;
	
	int j;
	int *out = malloc(sizeof(int)*size2);
	int *found = calloc(size2, sizeof(int)), nfound = 0;   
	memset(out, -1, sizeof(int)*size2); // Initialize to -1.

	for(int i = 0; i < size1; i++) {
		for(j = 0; j < size2; j++) {
			if(found[j]) { continue; }
			
			if(strcmp(strings[j], array[i]) == 0) {
				found[j] = 1;
				out[j] = i;
				nfound++;
			}
		}
		
		if(nfound == size2)
			break;
	}
	
	free(found);
	return out;
}

int int_in_array (int *array, int val, int size) {
	// Determine if an int is in an array, and if so where.
	// Returns -1 or the index.

	if(array == NULL)
		return -1;
	
	for(int i = 0; i < size; i++) {
		if(array[i] == val) { return i; }
	}
	
	return -1;
}

int double_in_array (double *array, double val, int size) {
	if(array == NULL) { return -1; }
	
	for(int i = 0; i < size; i++) {
		if(array[i] == val) { return i; }
	}
	
	return -1;
}

int constant_array_int(int *array, int size) {
	// Give this an array of ints and the size of the array
	// and it checks to see if the array is the same everywhere.
	
	for(int i=1; i<size; i++) {
		if(array[i] != array[0])	// If it's not the same as the first one
			return 0;				// it's not the same everywhere.
	}

	return 1;
}

int constant_array_double(double *array, int size) {
	// Give this an array of doubles and the size of the array
	// and it checks to see if the array is the same everywhere.
	
	for(int i=1; i<size; i++) {
		if(array[i] != array[0])	// If it's not the same as the first one
			return 0;				// it's not the same everywhere.
	}

	return 1;
}

void remove_array_item(int *array, int index, int num_items) {
	// Removes the item at index from *array, an array of size num_items
	// This does not re-allocate space for the array, so if the item you
	// are removing is the last one, it doesn't do anything, otherwise
	// it moves everything over one.
	
	for(int i=index; i<num_items-1; i++)
		array[i] = array[i+1];
	
}

void remove_array_item_void(void *array, int index, int num_items, int type) {
	// Same as remove_array_item, but for any type.
	// Pass 0 for double
	// Pass 1 for char
	// Pass 2 for int*
	// Pass 3 for double*
	
	if(type == 0) {
		double *ar = (double*)array;
	 	for(int i=index; i<num_items-1; i++)
			ar[i] = ar[i+1];
	} else if(type == 1) {
		char *ar = (char*)array;
		for(int i=index; i<num_items-1; i++)
			ar[i] = ar[i+1];
	} else if(type == 2) {
		int **ar = (int**)array;
		for(int i=index; i<num_items-1; i++)
			ar[i] = ar[i+1];
	} else if(type == 3) {
		double **ar = (double**)array;
		for(int i=index; i<num_items-1; i++)
			ar[i] = ar[i+1];
	}

}


//////////////////////////////////////////////////////////////
// 															//
//					String Utils							//
// 															//
//////////////////////////////////////////////////////////////

char *generate_nc_string(char **strings, int numstrings, int *len) {
	// We're going to store arrays of strings as a single newline-delimited char array. 
	// len returns the full length of the array with newlines and \0. 
	//
	// The array is malloced, so you need to free it when you are done.
	
	if(strings == NULL) { 
		char *s = calloc(3, sizeof(char));
		strcpy(s, " \n");
		return s;
	}
	
	int i, l=0;
	for(i = 0; i < numstrings; i++) {
		if(strings[i] == NULL || strlen(strings[i]) == 0) { 
			l += 2; 
		} else { 
			l += strlen(strings[i])+1;	// Add one for the newline, not null.;
		}
	}	
	
	char *out = malloc(++l);	// Adds one for the null. (Keeping the trailing \n)
	out[0] = '\0';
	if(len != NULL) { *len = l; }
	
	// Generate the array
	for(i = 0; i < numstrings; i++) {
		if(strings[i] != NULL && strlen(strings[i]) > 0) { 
			strcat(out, strings[i]);
		} else {
			strcat(out, " ");
		}
		
		strcat(out, "\n");
	}
	
	return out;
}

char **get_nc_strings(char *string, int *ns) {
	// Performs the reverse operation of generate_nc_string ns should be your best guess as
	// to how many strings there are. If you don't know, a value of 0 or less defaults to 10.

	int n = 0, g = ((ns != NULL) && (*ns > 0))?*ns:10, s = g, i;
	char **out = malloc(sizeof(char*)*s), *p = strtok(string, "\n");

	while(p != NULL) {
		i = n++;	// Index is one less than n.
		if(n > g) {	// Increment the counter.
			g += s;		// Get more space.
			out = realloc(out, sizeof(char*)*g);
		}
								 
		// Put the string in the array.
		out[i] = malloc(strlen(p)+1);

		if(p[0] == ' ') {
			strcpy(out[i], "");
		} else {
			strcpy(out[i], p);
		}
		
		p = strtok(NULL, "\n");
	}
	
	if(n <= 0) {
		free(out);
		return NULL;
	}
	
	out = realloc(out, sizeof(char*)*n);	// Eliminate excess.
	
	if(ns != NULL)
		ns[0] = n;	// This is the number of strings.
	
	return out;
}

char **generate_char_num_array(int first, int last, int *elems) {
	// Generates an array of strings that are just numbers, starting with
	// first, ending with last.
	// *c needs to be freed. *elems is the number of items in
	// the list - make sure to iterate through c and free all the elements
	
	// Some people don't understand that the last thing comes AFTER the
	// first thing, fix it if that's a problem.
	if(first > last) {
		int buff = first;
		first = last;
		last = buff;
	}
	
	// Need the length of the longest entry.
	int maxlen;
	if(last == 0 && last == first)			// log10(0) is undefined.
		maxlen = 2;		
	if(last <= 0)
		maxlen=(int)log10(abs(first))+2; 	// Need to allocate space for the sign
	else									// and the null termination
		maxlen=(int)log10(abs(last))+1;
	
	int elements = last-first+1;
	char **c = malloc(sizeof(char*)*elements);
	for(int i=0; i<elements; i++) {
		c[i] = malloc(maxlen+1);
		sprintf(c[i], "%d", first+i);
	}
	
	if(elems != NULL)
		elems[0] = elements;
	
	return c;
}

char *generate_expression_error_message(char *err_message, int *pos, int size) { 
	// Pass this an error message and a cstep and it makes a nice little message
	// that displays the position of the error and the reason for it.
	// The output of this is dynamically allocated, so you need to free it.
	
	// Error checking
	if(err_message == NULL)
		return NULL;
	
	// Need to do some stuff to allocate the size of the message
	int max_num = 0;
	for(int i=0; i<size; i++) {
		if(pos[i] > max_num)
			max_num = pos[i];
	}
	
	int num_char_len = ((int)log10(max_num)+1)*size;	// Characters to allocate per number.
	int commas = 2*(size-1);				// How many commas to generate.
	char *text;
	
	if(size == 1)
		text = "\n\nGenerated at step ";
	else 
		text = "\n\nGenerated at point [";
	
	int total_len = num_char_len+commas+strlen(text)+strlen(err_message)+2;	// One for the null, one for either ] or .
	
	// Build the full output string.
	char *output = malloc(total_len);
	sprintf(output, "%s%s%d", err_message, text, pos[0]);
	
	for(int i=1; i<size; i++)
		sprintf(output, "%s%s%d", output, ", ", pos[i]);
	
	if(size == 1)
		strcat(output, "]");
	else
		strcat(output, ".");
	
	return output;
}

//////////////////////////////////////////////////////////////
// 															//
//					Memory Allocation						//
// 															//
//////////////////////////////////////////////////////////////

void *malloc_or_realloc(void *pointer, size_t size) {
	// Either malloc this or realloc it, depending on if it's null already.
	if(pointer == NULL) {
		return malloc(size);	
	} else {
		return realloc(pointer, size);	
	}
}

void *realloc_if_needed(void *array, int *len, int new_len, int inc) {
	// Reallocates "array" in chunks of size "inc" such that it is
	// greater than or equal to new_len. Return value is the new
	// length of the array. No change if new_len < len;
	
	int l = *len;
	
	if(new_len <= l) { return array; }
	
	if(inc <= 1) {
		l = new_len;
	} else {
		l += ((int)((new_len-l)/inc)+1)*inc;
	}
	
	array = realloc(array, l);
	*len = l;
	return array;
}

char **free_string_array (char **array, int size) {
	// Safely frees an array of strings
	// Make sure that string arrays are initialized to NULL if you
	// are going to use this function on them.
	//
	// Returns null array, so you use like this: 
	// array = free_string_array(array, size);
	
	if(array != NULL) {
		for(int i = 0; i < size; i++) {
			if(array[i] != NULL) { free(array[i]); }	
		}
	
		free(array);
	}
	
	return NULL;
}

double **free_doubles_array (double **array, int size) {
	// Safely frees an array of arrays of doubles
	// Make sure that arrays are initialized to NULL if you
	// are going to use this function on them.
	//
	// Returns null array, so you use like this: 
	// array = free_doubles_array(array, size);
	
	if(array != NULL) {
		for(int i = 0; i < size; i++) {
			if(array[i] != NULL) { free(array[i]); }	
		}
	
		free(array);
	}
	
	return NULL;
}

int **free_ints_array (int **array, int size) {
	// Safely frees an array of arrays of ints
	// Make sure that arrays are initialized to NULL if you
	// are going to use this function on them.
	//
	// Returns null array, so you use like this: 
	// array = free_intss_array(array, size);
	
	if(array != NULL) {
		for(int i = 0; i < size; i++) {
			if(array[i] != NULL) { free(array[i]); }	
		}
	
		free(array);
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////
// 															//
//					File I/O								//
// 															//
//////////////////////////////////////////////////////////////

int get_name(char *pathname, char *name, char *ending) {
	// Gets the base name without the ending. Pass NULL if you don't
	// care about the ending, pass the specified ending (no .) if you 
	// have one in mind, and pass "*" if the ending is not known.
	
	SplitPath(pathname, NULL, NULL, name);
	
	if(ending == NULL)
		return 0;
	
	if(ending[0] == '*') {
		for(int i = strlen(name)-1; i--; i >= 0) {
			if(name[i] == '.') {
				name[i] = '\0';
				break;
			}
		}
		
		return 0;
	}
	
	if(strlen(name) > strlen(ending) && strcmp(&name[strlen(name)-strlen(ending)], ending) == 0)
		name[strlen(name)-strlen(ending)] = '\0';
	
	return 0;
}

char *get_extension(char *pathname) {
	// Returns the extension, malloced. Finds the last index of ".", so this won't be
	// able to detect shit like "foo.tar.gz"
	//
	// Returns NULL if no extension is found, otherwise returns malloced string.
	
	if(pathname == NULL) { return NULL; }
	
	char *c = NULL;
	
	int i, l = strlen(pathname);
	
	if(l == 0) { return NULL; }
	
	for(i = l ; i > 0; i--) {
		if(pathname[i] == '.') { break; }
		if(pathname[i] == '\\') { return NULL; }
	}
	
	if(i == 0) { return NULL; }
	
	c = malloc(l-i+1);
	strcpy(c, pathname+l);
	
	return c;
}


char *temp_file(char *extension) {
	return temporary_filename(NULL, extension);
}

char *temporary_filename(char *dir, char *extension) {
	//Generates a random temporary file in the directory dir and returns the pathname.
	//If dir == NULL, it uses the directory TemporaryFiles in the project directory.

	char *filenameout = malloc(MAX_PATHNAME_LEN), *dirtok = malloc(MAX_PATHNAME_LEN);
	
	if(dir == NULL) {
		char *tempdir = malloc(MAX_PATHNAME_LEN);  
		GetProjectDir(tempdir);
		if(tempdir[strlen(tempdir)] != '\\')
		strcat(tempdir, "\\");
		strcat(tempdir, "TemporaryFiles");
		strcpy(dirtok, tempdir);
		strcpy(filenameout, tempdir);
		free(tempdir);
	} else {
		strcpy(dirtok, dir);
		strcpy(filenameout, dir);
	}
	
	if(!file_exists(dirtok))
	{
		char *createdir = malloc(MAX_PATHNAME_LEN), *tok = malloc(MAX_PATHNAME_LEN);
		tok = strtok(dirtok, "\\");
		strcpy(createdir, "");
		while(tok != NULL)
		{
			strcat(createdir, tok);
			strcat(createdir, "\\");
			if(!file_exists(createdir)) {
				MakeDir(createdir);
			}
			tok = strtok(NULL, "\\");
		}
	}
	
	srand(time(NULL));
	char *randtotext = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-\0";
	
	if(filenameout[strlen(filenameout)] != '\\')
		strcat(filenameout, "\\");
	
	int i, j = 0, r;
	while(j++ < 200)
	{
		r = rand()%6;
		for(i = 0; i< 18 + r; i++)
			sprintf(filenameout, "%s%c", filenameout, randtotext[rand()%strlen(randtotext)]);
		
		strcat(filenameout, extension);
		
		if(!file_exists(filenameout))
			break;
	}
	
	if(j == 1000) { return NULL; }
	
	FILE *f = fopen(filenameout, "wb+");
	fclose(f);
	
	return filenameout;
	
}

int file_exists(char *filename) {
	//Check if a file exists.
	return FileExists(filename, 0);
}


//////////////////////////////////////////////////////////////
// 															//
//						Sorting								//
// 															//
//////////////////////////////////////////////////////////////

int *add_item_to_sorted_array(int *array, int item, int size) {
	// Add an item where it belongs in a sorted array. (insertion sort)
	// Frees the old array and returns a new one. Update all pointers.
	int *new_array = malloc(sizeof(int)*(size+1)), i;
	for(i = 0; i < size; i++) {
		if(item < array[i]) {
			new_array[i] = item;
			break;
		}
		
		new_array[i] = array[i];
	}
	
	if(i < size) {
		memcpy(new_array+i+1, array+i, sizeof(int)*(size-i));
	} else {
		new_array[size] = item;
	}
	
	array = realloc(array, (++size)*sizeof(int));
	memcpy(array, new_array, size*sizeof(int));

	free(new_array);
	
	return array;
}

void sort_linked(int **array, int num_arrays, int size) {
	// Feed this an array of arrays of ints and it sorts by the first array.
	// Uses insertion sort, which is efficient for small data sets.
	// Sorts so that the first element is the smallest.
	// Pass an array of size size*sizeof(int) to *index if you'd like the index
	// from the sort, otherwise pass NULL.

	int i, j, k;
	
	int *buff = malloc(sizeof(int)*num_arrays); 	// Buffer array for the integers we're moving around.
	
	// The insertion sort.
	for(i=0; i<size; i++) {
		for(j=0; j<num_arrays; j++)					// Move the element into the buffer
			buff[j] = array[j][i];
		
		for(j=i; j>0; j--) {
			if(buff[0] >= array[0][j-1])
				break;
			else {
				for(k=0; k<num_arrays; k++) 
					array[k][j] = array[k][j-1];	// Everything moves over one
			}
		}
		
		// Put the element into the array now that it's in its proper spot
		for(k=0; k<num_arrays; k++) {
			array[k][j] = buff[k];
		}
	}
}

//////////////////////////////////////////////////////////////
// 															//
//					Linear Indexing							//
// 															//
//////////////////////////////////////////////////////////////

int get_lindex(int *cstep, int *maxsteps, int size)
{
	// Give this a position in space maxsteps, which is an array of size "size"
	// It returns the linear index of cstep
	// We'll assume you can figure out whether or not this is the end of the array
	
	if(cstep == NULL)
		return -3;			// Not varied.
	
	if(size == 0)
		return -2;			// Size of array is 0.
	
	int lindex = cstep[0];	// Start with the least signficant bit.
	int place = 1;
	for(int i=1; i<size; i++) {
		place*=maxsteps[i-1];		// This is the "place" place. In decimal, you have [10 10 10] = 1s, 10s, 100s.
		lindex+=cstep[i]*place;		// The same thing goes here. [3 4 2] = 1s place, 3s places, 12s place.
	}
	
	return lindex;
}

int get_cstep(int lindex, int *cstep, int *maxsteps, int size) {
	// Give this a linear index value and it will return an array of size "size"
	// containing the place in sampling space maxsteps that the linear index points to
	// cstep must already be allocated.
	if(size-- == 0)
		return -2; 				// Size cannot be 0.
	
	int i, place = 1;
	for(i = 0; i<size; i++)
		place*=maxsteps[i];		
	
	if(place == 0 || lindex < 0)
		return -2; 				// Size is 0 or other issue

	if(lindex == maxsteps[size]*place)
		return -1;				// End of array condition met
	
	int remainder = lindex, step;
	for(i=size; i>= 0; --i) {		// Start at the end and work backwards
		step = remainder;	// Initialize step to whatever's left. When we first get started, it's everything
		step /= place;		// Divide by the place. This number is by definition bigger than the product 
		cstep[i] = step;	// of all the numbers before it, so rounding down gets the most signficant digit

		remainder -= place*step;	// Get the remainder for the next iteration
		if(i>0)
			place /= maxsteps[i-1];	// Update the place
	}
	
	return 1;	// Successful.
}

//////////////////////////////////////////////////////////////
// 															//
//						Math								//
// 															//
//////////////////////////////////////////////////////////////
int calculate_units(double val) {
	// Returns the correct units.
	if(val == 0)
		return 0;
	
	int a = (int)((log10(fabs(val)))/3);
	if(a < 0)
		return 0;
	if(a > 3)
		return 3;
	else
		return a;
}

int get_precision (double val, int total_num) {
	// Feed this a value, then tell you how many numbers you want to display total.
	// This will give you the number of numbers to pad out after the decimal place.
	// If the number to be displayed has more digits after the decimal point than 
	// total_num, it returns 0.

	if(--total_num <= 1)  	// Always need one thing before the decimal place.
		return 0;
	
	if(val == 0)
		return total_num;

	int a = log10(fabs(val)); // The number of digits is the base-10 log, plus 1 - since we subtracted off the 1 from total_num, we're good..
	
	if(a >= total_num) // If we're already at capacity, we need nothing after the 0.
		return 0;
	
	return total_num-a; // Subtract off the number being used by the value, that's your answer.
}

int get_bits(int in, int start, int end) {
	// Get the bits in the span from start to end in integer in. Zero-based index.
	
	if(start > end || start < 0)
		return -1;
	
	if(end > 31)
		return -2; // Maxint
	
	return get_bits_in_place(in, start, end)>>start;
}

int get_bits_in_place(int in, int start, int end) {
	// Gets the bits you want and preserves them in place.
	if(start > end || start < 0)
		return -1;
	
	if(end > 31)
		return -2; // Maxint

	return (in & (1<<end+1)-(1<<start));
}

int move_bit_skip(int in, int skip, int to, int from) {
	// Same as move_bit, but this also skips anything high in "flags"
	if(((1<<to)|(1<<from)) & skip)
		return -1; // Error.
	
	int bits = move_bit(in, to, from); // Start off by not skipping anything.
	int i, toh = to>from, inc = toh?-1:1;

	// Now we go through bit by bit and swap each broken bit into its original
	// place. If to>from, that involves swapping with the bit 1 size bigger, 
	// if from > to, you swap with the bit 1 size smaller.
	for(i = to; toh?(i > from):(i < from); i+=inc) {
		if((1<<i) & skip)
			bits = move_bit(bits, i, i+inc); // Swap with the appropriate adjacent bit.
	}

	return bits;
}

int move_bit(int in, int to, int from) {
	// Move a bit from one place to another, shifting everything as you go
	// Two examples: 1101101, from 1->end or from end->1 (big-endian)
	
	if(to == from)
		return in;
	
	int toh = to > from;
	int high = toh?to:from, low = toh?from:to;
	
	// Start by getting only the stuff we care about.
	int bits = get_bits_in_place(in, low, high); // 1101101 -> 110110
	int mask = in - bits;
	bits = bits >> low;
	
	// Put the "from" flag into a buffer
	int from_flag = bits & 1<<(from-low); // 1->end: 0, end->1: 100000  
	
	// Subtract off the "from" flag
	bits -= from_flag; // 1e: 110110, e1: 010110
	
	// If the from flag is high, we need to multiply by 2, otherwise subtract
	bits = (toh ? bits/2 : bits*2); // 1e: 011011, e1: 101100
	
	// Now we transform from_flag to be a flag in the right spot.
	from_flag = toh?from_flag<<(to-from):from_flag>>(from-to); // 1e: 0->0, e1: 100000 -> 1
	
	// Finally we add it back in.
	bits += from_flag; 	// 1e: 011011->011011, e1: 101100->101101
	
	// Then we need to put it back in place.
	bits = bits << low;  // 1e: 0110110, e1: 1011010
	bits = mask|bits; // 1e: 0110111, e1: 1011011
	
	return bits; // Final results: 1101101 -> 1e: 0110111, e1: 1011011
}
