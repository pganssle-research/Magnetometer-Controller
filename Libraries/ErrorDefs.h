/***********************************************************
*  														   *
*            Pulse Program Error Values					   *
* 														   *
***********************************************************/
// Span
#define MCPP_ERR_MAX -11880
#define MCPP_ERR_MIN -11905

// Return values
#define MCPP_ERR_NOFILE -11880
#define MCPP_ERR_NOPROG -11881
#define MCPP_ERR_NOSTRING -11882
#define MCPP_ERR_TEMP_FILE_NAME -11883
#define MCPP_ERR_TEMP_FILE -11884
#define MCPP_ERR_FILE_MOVING -11885
#define MCPP_ERR_FILEWRITE -11886
#define MCPP_ERR_FILEREAD -11887
#define MCPP_ERR_NOHEADER -11888
#define MCPP_ERR_NOINSTRS -11889
#define MCPP_ERR_NOAOUT -11890
#define MCPP_ERR_NOND -11891
#define MCPP_ERR_NOSKIP -11892
#define MCPP_ERR_FLOC_NAME -11893
#define MCPP_ERR_FLOC_TYPE -11894
#define MCPP_ERR_FLOC_SIZE -11895
#define MCPP_ERR_NOFLOCS -11896
#define MCPP_ERR_MALFORMED_FNAME -11897
#define MCPP_ERR_FILE_NOPROG -11898
#define MCPP_ERR_FILE_NOPROPS -11899
#define MCPP_ERR_FILE_NOINSTRS -11900
#define MCPP_ERR_PROG_PROPS_LABELS -11901
#define MCPP_ERR_CUST_NOENTRIES -11902
#define MCPP_ERR_INVALID_TYPE -11903
#define MCPP_ERR_INVALID_NC_STRING -11904
#define MCPP_ERR_FIELDSMISSING -11905

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
