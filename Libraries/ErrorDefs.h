/***********************************************************
*  														   *
*            		Error Indexing						   *
* 														   *
***********************************************************/
#ifndef ERROR_DEFS_H
#define ERROR_DEFS_H

// Index
#define MCF_ERR 1
#define MCD_ERR 2
#define MCPP_ERR 3
#define MCEX_ERR 4
#define MCSS_ERR 5

// FSave Errors
#define MCF_ERR_TITLE "File Saving Error"
#define MCF_ERR_MAX -10751
#define MCF_ERR_MIN -11000

// Data Saving
#define MCD_ERR_TITLE "Data Saving Error"		
#define MCD_ERR_MAX -11001
#define MCD_ERR_MIN -11250

// Pulse Program
#define MCPP_ERR_TITLE "Pulse Program Error"	
#define MCPP_ERR_MAX -11251
#define MCPP_ERR_MIN -11500

// Current Experiment
#define MCEX_ERR_TITLE "Current Experiment Error"
#define MCEX_ERR_MAX -11501
#define MCEX_ERR_MIN -11750

// Session Saving
#define MCSS_ERR_TITLE "Session Saving Error"
#define MCSS_ERR_MAX -11751
#define MCSS_ERR_MIN -12000

/***********************************************************
*  														   *
*            		File Saving Errors					   *
* 														   *
***********************************************************/

#define MCF_ERR_INVALID_MAX_BYTES -10751
#define MCF_ERR_FSAVE_NOT_FOUND -10752
#define MCF_ERR_BAD_FSAVE_REPLACEMENT_NAME -10753
#define MCF_ERR_FSAVE_TYPE_MISMATCH -10754
#define MCF_ERR_FSAVE_SIZE_MISMATCH -10755
#define MCF_ERR_CANNOT_TRUNCATE -10756
#define MCF_ERR_NOFILE -10757
#define MCF_ERR_BADFNAME -10758
#define MCF_ERR_NODATA -10759
#define MCF_ERR_CUST_NOENTRIES -10760
#define MCF_ERR_INVALID_TYPE -10761
#define MCF_ERR_FLOC_NAME -10762
#define MCF_ERR_FLOC_TYPE -10763
#define MCF_ERR_FLOC_SIZE -10764
#define MCF_ERR_NOFLOCS -10765
#define MCF_ERR_FILEREAD -10766
#define MCF_ERR_FILEWRITE -10767
#define MCF_ERR_NOSTRING -10768
#define MCF_ERR_FS_NOTYPE -10769
#define MCF_ERR_FS_NOSIZE -10770
#define MCF_ERR_FS_BADCONTENTS -10771
#define MCF_ERR_NOARRAY -10772
#define MCF_ERR_NOFSAVE -10773
#define MCF_ERR_EMPTYVAL -10774
#define MCF_ERR_FS_NONS -10775
#define MCF_ERR_FS_NONAME -10776

#define MCF_ERR_INVALID_MAX_BYTES_STR "Invalid maximum bytes paramater passed."
#define MCF_ERR_FSAVE_NOT_FOUND_STR "Requested fsave not found. This may not be an error."
#define MCF_ERR_BAD_FSAVE_REPLACEMENT_NAME_STR "FSAVE names do not match, cannot replace safely!"
#define MCF_ERR_FSAVE_TYPE_MISMATCH_STR "FSAVE types do not match. This may not be an error."
#define MCF_ERR_FSAVE_SIZE_MISMATCH_STR "FSAVE sizes do not match. This may not be an error."
#define MCF_ERR_CANNOT_TRUNCATE_STR "Cannot truncate file."
#define MCF_ERR_NOFILE_STR "Invalid file passed to function."
#define MCF_ERR_BADFNAME_STR "Invalid file name."
#define MCF_ERR_NODATA_STR "Required data were not passed to function."
#define MCF_ERR_CUST_NOENTRIES_STR "No struct entries provided for custom FSAVE type"
#define MCF_ERR_INVALID_TYPE_STR "Invalid FSAVE type passed to function."
#define MCF_ERR_FLOC_NAME_STR "Couldn't read field name."
#define MCF_ERR_FLOC_TYPE_STR "Couldn't read field type."
#define MCF_ERR_FLOC_SIZE_STR "Could read field size."
#define MCF_ERR_NOFLOCS_STR "No fields found in specified file."
#define MCF_ERR_FILEREAD_STR "Failed to properly read file."
#define MCF_ERR_FILEWRITE_STR "Failed to properly write to file."
#define MCF_ERR_NOSTRING_STR "No string was provided to the function."
#define MCF_ERR_FS_NOTYPE_STR "No type was found for the fsave structure."
#define MCF_ERR_FS_NOSIZE_STR "No size was found for the fsave structure."
#define MCF_ERR_FS_BADCONTENTS_STR "The contents of the fsave structure were not properly read."
#define MCF_ERR_NOARRAY_STR "Required array not passed to function."
#define MCF_ERR_NOFSAVE_STR "Invalid FSAVE struct passed to function."
#define MCF_ERR_EMPTYVAL_STR "The value for the fsave was not provided, or size was 0."
#define MCF_ERR_FS_NONS_STR "FSAVE has malformed header - missing name length."
#define MCF_ERR_FS_NONAME_STR "FSAVE has malformed header - malformed name."

/***********************************************************
*  														   *
*            		Data Saving Errors					   *
* 														   *
***********************************************************/
// Return values
#define MCD_ERR_NOFILENAME -11001
#define MCD_ERR_NOFILE -11002
#define MCD_ERR_NOPROG -11003
#define MCD_ERR_FILEWRITE -11004
#define MCD_ERR_FILEREAD -11005
#define MCD_ERR_NODATA -11006
#define MCD_ERR_NOAVGDATA -11007
#define MCD_ERR_BADCIND -11008
#define MCD_ERR_NOPATH -11009
#define MCD_ERR_NOINPUTCHANS -11010
#define MCD_ERR_NOSTEPSTR -11011
#define MCD_ERR_INVALIDSTR -11012

// Strings
#define MCD_ERR_NOFILENAME_STR "No data filename provided."
#define MCD_ERR_NOFILE_STR "No data file provided, or file name is broken."
#define MCD_ERR_NOPROG_STR "No program provided."
#define MCD_ERR_FILEWRITE_STR "Error writing data file."
#define MCD_ERR_FILEREAD_STR "Error reading data file."
#define MCD_ERR_NODATA_STR "No data were provided to the file for writing."
#define MCD_ERR_NOAVGDATA_STR "No average data found in existing file."
#define MCD_ERR_BADCIND_STR "Malformed current index passed to function."
#define MCD_ERR_NOPATH_STR "No path was provided to a function that requires one."
#define MCD_ERR_NOINPUTCHANS_STR "No input channels are available on this device."
#define MCD_ERR_NOSTEPSTR_STR "No step title was provided to a function which requires one."
#define MCD_ERR_INVALIDSTR_STR "An invalid string caused an error."

/***********************************************************
*  														   *
*            Pulse Program Error Values					   *
* 														   *
***********************************************************/
// Return values
#define MCPP_ERR_NOFILE -11251
#define MCPP_ERR_NOPROG -11252
#define MCPP_ERR_TEMP_FILE_NAME -11254
#define MCPP_ERR_TEMP_FILE -11255
#define MCPP_ERR_FILE_MOVING -11256
#define MCPP_ERR_FILEWRITE -11257
#define MCPP_ERR_FILEREAD -11258
#define MCPP_ERR_NOHEADER -11259
#define MCPP_ERR_NOINSTRS -11260
#define MCPP_ERR_NOAOUT -11261
#define MCPP_ERR_NOND -11262
#define MCPP_ERR_NOSKIP -11263
#define MCPP_ERR_FLOC_TYPE -11265
#define MCPP_ERR_FLOC_SIZE -11266
#define MCPP_ERR_NOFLOCS -11267
#define MCPP_ERR_MALFORMED_FNAME -11268
#define MCPP_ERR_FILE_NOPROG -11269
#define MCPP_ERR_FILE_NOPROPS -11270
#define MCPP_ERR_FILE_NOINSTRS -11271
#define MCPP_ERR_PROG_PROPS_LABELS -11272
#define MCPP_ERR_INVALID_NC_STRING -11275
#define MCPP_ERR_FIELDSMISSING -11276
#define MCPP_ERR_INVALIDTMODE -11277
#define MCPP_ERR_NOARRAY -11278
#define MCPP_ERR_FS_NOTYPE -11279
#define MCPP_ERR_FS_NOSIZE -11280
#define MCPP_ERR_FS_BADCONTENTS -11281
#define MCPP_ERR_NOSTRING -11282

// Strings
#define MCPP_ERR_NOFILE_STR "No filename was provided."
#define MCPP_ERR_NOPROG_STR "PPROGRAM structure variable missing or NULL."
#define MCPP_ERR_TEMP_FILE_NAME_STR "Failed to find temporary file name."
#define MCPP_ERR_TEMP_FILE_STR "Failed to open temporary file."
#define MCPP_ERR_FILE_MOVING_STR "Failed to move file to provided location."
#define MCPP_ERR_FILEWRITE_STR "Failed to write all values to file."
#define MCPP_ERR_FILEREAD_STR "Failed to read all values from file."
#define MCPP_ERR_NOHEADER_STR "Failed to generate header."
#define MCPP_ERR_NOINSTRS_STR "Failed to generate instructions array."
#define MCPP_ERR_NOAOUT_STR "Failed to generate analog output array."
#define MCPP_ERR_NOND_STR "Failed to generate multidimensional instrs array."
#define MCPP_ERR_NOSKIP_STR "Failed to generate skip properties."
#define MCPP_ERR_MALFORMED_FNAME_STR "Malformed field name."
#define MCPP_ERR_FILE_NOPROG_STR "Program not found in specified file."
#define MCPP_ERR_FILE_NOPROPS_STR "Program properties not found in program."
#define MCPP_ERR_FILE_NOINSTRS_STR "Program properties contains no instructions."
#define MCPP_ERR_PROG_PROPS_LABELS_STR "Program property labels were missing or invalid."
#define MCPP_ERR_INVALID_NC_STRING_STR "Invalid linearized newline-delimited string array found."
#define MCPP_ERR_FIELDSMISSING_STR "Necessary fields are missing from a required item in PPROGRAM."
#define MCPP_ERR_INVALIDTMODE_STR "Invalid transient indexing mode in PPROGRAM"
#define MCPP_ERR_NOARRAY_STR "Null array passed to pulse program function"
#define MCPP_ERR_FS_NOTYPE_STR "No type provided to fsave structure."
#define MCPP_ERR_FS_NOSIZE_STR "No size provided for fsave structure."
#define MCPP_ERR_FS_BADCONTENTS_STR "Bad contents passed to fsave structure."
#define MCPP_ERR_NOSTRING_STR "Required string was not present."

/***********************************************************
*  														   *
*            	Experiment Running Errors				   *
* 														   *
***********************************************************/
// Return values
#define MCEX_ERR_NOCEXP -11501
#define MCEX_ERR_NOPROG -11502
#define MCEX_ERR_NOSTEPS -11503
#define MCEX_ERR_INVALIDTMODE -11504
#define MCEX_ERR_NOFNAME -11505

// Strings
#define MCEX_ERR_NOCEXP_STR "Missing or invalid CEXP structure passed to the function."
#define MCEX_ERR_NOPROG_STR "Missing or invalid program in the current experiment."
#define MCEX_ERR_NOSTEPS_STR "CEXP is missing the steps indexing array."
#define MCEX_ERR_INVALIDTMODE_STR "Invalid transient indexing mode in CEXP"
#define MCEX_ERR_NOFNAME_STR "No filename has been added to the CEXP struct."

/***********************************************************
*  														   *
*           Sessopm Savomg Error Values					   *
* 														   *
***********************************************************/
// Return values
#define MCSS_ERR_NOOUT -11751
#define MCSS_ERR_NOATTNAME -11752

// Strings
#define MCSS_ERR_NOOUT_STR "No output pointer was specified."
#define MCSS_ERR_NOATTNAME_STR "No attribute name passed to function."

#endif

