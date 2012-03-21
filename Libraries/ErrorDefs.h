/***********************************************************
*  														   *
*            		Error Indexing						   *
* 														   *
***********************************************************/
// Index
#define MCD_ERR 1
#define MCPP_ERR 2
#define MCEX_ERR 3

// Data Saving
#define MCD_ERR_TITLE "Data Saving Error"		
#define MCD_ERR_MAX -11000
#define MCD_ERR_MIN -11250

// Pulse Program
#define MCPP_ERR_TITLE "Pulse Program Error"	
#define MCPP_ERR_MAX -11251
#define MCPP_ERR_MIN -11500

// Current Experiment
#define MCEX_ERR_TITLE "Current Experiment Error"
#define MCEX_ERR_MAX -11501
#define MCEX_ERR_MIN -11750

/***********************************************************
*  														   *
*            		Data Saving Errors					   *
* 														   *
***********************************************************/
// Return values
#define MCD_ERR_NOFILENAME -11000
#define MCD_ERR_NOFILE -11001
#define MCD_ERR_NOPROG -11002
#define MCD_ERR_FILEWRITE -11003
#define MCD_ERR_FILEREAD -11004
#define MCD_ERR_NODATA -11005
#define MCD_ERR_NOAVGDATA -11006

// Strings
#define MCD_ERR_NOFILENAME_STR "No data filename provided."
#define MCD_ERR_NOFILE_STR "No data file provided, or file name is broken."
#define MCD_ERR_NOPROG_STR "No program provided."
#define MCD_ERR_FILEWRITE_STR "Error writing data file."
#define MCD_ERR_FILEREAD_STR "Error reading data file."
#define MCD_ERR_NODATA_STR "No data were provided to the file for writing."
#define MCD_ERR_NOAVGDATA_STR "No average data found in existing file."

/***********************************************************
*  														   *
*            Pulse Program Error Values					   *
* 														   *
***********************************************************/
// Return values
#define MCPP_ERR_NOFILE -11251
#define MCPP_ERR_NOPROG -11252
#define MCPP_ERR_NOSTRING -11253
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
#define MCPP_ERR_FLOC_NAME -11264
#define MCPP_ERR_FLOC_TYPE -11265
#define MCPP_ERR_FLOC_SIZE -11266
#define MCPP_ERR_NOFLOCS -11267
#define MCPP_ERR_MALFORMED_FNAME -11268
#define MCPP_ERR_FILE_NOPROG -11269
#define MCPP_ERR_FILE_NOPROPS -11270
#define MCPP_ERR_FILE_NOINSTRS -11271
#define MCPP_ERR_PROG_PROPS_LABELS -11272
#define MCPP_ERR_CUST_NOENTRIES -11273
#define MCPP_ERR_INVALID_TYPE -11274
#define MCPP_ERR_INVALID_NC_STRING -11275
#define MCPP_ERR_FIELDSMISSING -11276
#define MCPP_ERR_INVALIDTMODE -11277
#define MCPP_ERR_NOARRAY -11278


// Strings
#define MCPP_ERR_NOFILE_STR "No filename was provided."
#define MCPP_ERR_NOPROG_STR "PPROGRAM structure variable missing or NULL."
#define MCPP_ERR_NOSTRING_STR "No string was provided to the function."
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
#define MCPP_ERR_FLOC_NAME_STR "Couldn't read field name."
#define MCPP_ERR_FLOC_TYPE_STR "Couldn't read field type."
#define MCPP_ERR_FLOC_SIZE_STR "Could read field size."
#define MCPP_ERR_NOFLOCS_STR "No fields found in specified file."
#define MCPP_ERR_MALFORMED_FNAME_STR "Malformed field name."
#define MCPP_ERR_FILE_NOPROG_STR "Program not found in specified file."
#define MCPP_ERR_FILE_NOPROPS_STR "Program properties not found in program."
#define MCPP_ERR_FILE_NOINSTRS_STR "Program properties contains no instructions."
#define MCPP_ERR_PROG_PROPS_LABELS_STR "Program property labels were missing or invalid."
#define MCPP_ERR_CUST_NOENTRIES_STR "No struct entries provided for custom FSAVE type"
#define MCPP_ERR_INVALID_TYPE_STR "Invalid FSAVE type passed to function."
#define MCPP_ERR_INVALID_NC_STRING_STR "Invalid linearized newline-delimited string array found."
#define MCPP_ERR_FIELDSMISSING_STR "Necessary fields are missing from a required item in PPROGRAM."
#define MCPP_ERR_INVALIDTMODE_STR "Invalid transient indexing mode in PPROGRAM"
#define MCPP_ERR_NOARRAY_STR "Null array passed to pulse program function"

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

// Strings
#define MCEX_ERR_NOCEXP_STR "Missing or invalid CEXP structure passed to the function."
#define MCEX_ERR_NOPROG_STR "Missing or invalid program in the current experiment."
#define MCEX_ERR_NOSTEPS_STR "CEXP is missing the steps indexing array."
#define MCEX_ERR_INVALIDTMODE_STR "Invalid transient indexing mode in CEXP"
