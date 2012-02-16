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
#include <analysis.h>
#include <cvixml.h>  
#include <NIDAQmx.h>
#include <toolbox.h>
#include <userint.h>
#include <ansi_c.h>
#include <formatio.h>  
#include <spinapi.h>					// SpinCore functions

#include <PulseProgramTypes.h>
#include <cvitdms.h>
#include <cviddc.h> 
#include <UIControls.h>					// For manipulating the UI controls
#include <MathParserLib.h>				// For parsing math
#include <DataLib.h>
#include <MCUserDefinedFunctions.h>		// Main UI functions
#include <PulseProgramLib.h>			// Prototypes and type definitions for this file
#include <SaveSessionLib.h>
#include <General.h>

//////////////////////////////////////////////////////////////
// 															//
//					General Utilities						//
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


void *malloc_or_realloc(void *pointer, size_t size) {
	// Either malloc this or realloc it, depending on if it's null already.
	if(pointer == NULL) {
		return malloc(size);	
	} else {
		return realloc(pointer, size);	
	}
}

//////////////////////////////////////////////////////////////
// 															//
//					Thread Safe Functions					//
// 															//
//////////////////////////////////////////////////////////////

/******************** Safe PulseBlaster Functions *********************/ 
int pb_initialize(int verbose) {
	int rv = pb_init();
	pb_set_clock(100.0);
	
	if(rv < 0 && verbose)
		MessagePopup("Error Initializing PulseBlaster", pb_get_error());
	
	if(rv >= 0)
		SetInitialized(1);

	return rv;
}

int pb_init_safe (int verbose)
{
	CmtGetLock(lock_pb);
	int rv = pb_initialize(verbose);
	CmtReleaseLock(lock_pb);
	
	return rv;
}

int pb_start_programming_safe(int verbose, int device)
{
	CmtGetLock(lock_pb);
	int rv = pb_start_programming(device);
	CmtReleaseLock(lock_pb);
	
	if(rv != 0 && verbose)
		MessagePopup("Error Starting PulseBlaster Programming", pb_get_error());

	return rv;
}

int pb_stop_programming_safe(int verbose)
{
	int rv = pb_stop_programming();
	CmtReleaseLock(lock_pb);
	
	if(rv <0 && verbose)
		MessagePopup("Error", pb_get_error());

	return rv;
}

int pb_inst_pbonly_safe (int verbose, unsigned int flags, int inst, int inst_data, double length)
{
	CmtGetLock(lock_pb);
	int rv = pb_inst_pbonly(flags, inst, inst_data, length);
	CmtReleaseLock(lock_pb);
		
	if(rv <0 && verbose)
		MessagePopup("Error Programming PulseBlaster", pb_get_error());

	return rv;
}
	

int pb_close_safe (int verbose)
{
	CmtGetLock(lock_pb);
	int rv = pb_close();
	CmtReleaseLock(lock_pb);
	if(rv < 0 && verbose)
		MessagePopup("Error Closing PulseBlaster", pb_get_error());
	
	SetInitialized(0);
		
	return rv;
}

int pb_read_status_or_error(int verbose) {
	int rv = 0;
	if(!GetInitialized()) {
		rv = pb_init();
		if(rv < 0)
			goto error;
	}

	rv = pb_read_status();
	
	error:
	if((rv <= 0 || rv > 32) && verbose)
	{
		int i, length = (int)((log10(abs(rv))/log10(2))+1);
		char *status = malloc(length+1);
		
		for(i = 0; i<length; i++)
			if(rv & (int)pow(2, i))
				status[length-i-1] = '1';
			else
				status[length-i-1] = '0';
		status[length] = '\0';
		
		
		char *err = pb_get_error();		// No need to free, not malloced.
		char *err_str = malloc(strlen(err)+strlen(status)+strlen(", Status: \n")+2);
		
		sprintf(err_str, "%s, Status: %s\n", err, status);
		MessagePopup("Read Pulseblaster Status Error", err_str);
		
		free(err_str);
	}
	
	return rv;
}

int pb_read_status_safe(int verbose)
{
	CmtGetLock(lock_pb);
	int rv = pb_read_status_or_error(verbose);
	CmtReleaseLock(lock_pb);

	return rv;
}

int pb_start_safe(int verbose)
{
	int rv = 0;

	if(!GetInitialized()) {
		rv = pb_init_safe(verbose);
		if(rv < 0)  { return rv; }
	}
	
	CmtGetLock(lock_pb);
	rv = pb_start();
	CmtReleaseLock(lock_pb);

	if(rv < 0 && verbose)
		MessagePopup("PulseBlaster Start Error", pb_get_error());

	return rv;
}

int pb_stop_safe(int verbose)
{
	
	CmtGetLock(lock_pb);
	int rv = pb_stop();
	CmtReleaseLock(lock_pb);
	
	if(rv < 0 && verbose)
		MessagePopup("PulseBlaster Stop Error", pb_get_error());
	
	return rv;
}
