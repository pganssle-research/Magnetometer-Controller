//////////////////////////////////////////////////////////////////////
// 																	//
//	      				 Error Library								//
//					Paul Ganssle, 03/05/2012						//
//																	//
//	This library is intended to contain all the basic error-handling//
// like message popups and such. ErrorDefs.h contains the defs for 	//
// all the errors and their associated strings. It can be used 		//
// independently from this library.									//
//////////////////////////////////////////////////////////////////////

#include <ErrorDefs.h>
#include <ErrorLib.h>

char *get_err_string(int err_val) {
	if(err_val >= MCPP_ERR_MIN && err_val <= MCPP_ERR_MAX) {
		// Pulse program return values
		switch(err_val) {
			case MCPP_ERR_NOFILE:
				return MCPP_ERR_NOFILE_STR;
			case MCPP_ERR_NOPROG:
				return MCPP_ERR_NOPROG_STR;
			case MCPP_ERR_NOSTRING:
				return MCPP_ERR_NOSTRING_STR;
			case MCPP_ERR_TEMP_FILE_NAME:
				return MCPP_ERR_TEMP_FILE_NAME_STR;
			case MCPP_ERR_TEMP_FILE:
				return MCPP_ERR_TEMP_FILE_STR;
			case MCPP_ERR_FILE_MOVING:
				return MCPP_ERR_FILE_MOVING_STR;
			case MCPP_ERR_FILEWRITE:
				return MCPP_ERR_FILEWRITE_STR;
			case MCPP_ERR_FILEREAD:
				return MCPP_ERR_FILEREAD_STR;
			case MCPP_ERR_NOHEADER:
				return MCPP_ERR_NOHEADER_STR;
			case MCPP_ERR_NOINSTRS:
				return MCPP_ERR_NOINSTRS_STR;
			case MCPP_ERR_NOAOUT:
				return MCPP_ERR_NOAOUT_STR;
			case MCPP_ERR_NOND:
				return MCPP_ERR_NOND_STR;
			case MCPP_ERR_NOSKIP:
				return MCPP_ERR_NOSKIP_STR;
			case MCPP_ERR_FLOC_NAME:
				return MCPP_ERR_FLOC_NAME_STR;
			case MCPP_ERR_FLOC_TYPE:
				return MCPP_ERR_FLOC_TYPE_STR;
			case MCPP_ERR_FLOC_SIZE:
				return MCPP_ERR_FLOC_SIZE_STR;
			case MCPP_ERR_NOFLOCS:
				return MCPP_ERR_NOFLOCS_STR;
			case MCPP_ERR_MALFORMED_FNAME:
				return MCPP_ERR_MALFORMED_FNAME_STR;
			case MCPP_ERR_FILE_NOPROG:
				return MCPP_ERR_FILE_NOPROG_STR;
			case MCPP_ERR_FILE_NOINSTRS:
				return MCPP_ERR_FILE_NOINSTRS_STR;
			case MCPP_ERR_PROG_PROPS_LABELS:
				return MCPP_ERR_PROG_PROPS_LABELS_STR;
			case MCPP_ERR_CUST_NOENTRIES:
				return MCPP_ERR_CUST_NOENTRIES_STR;
			case MCPP_ERR_INVALID_TYPE:
				return MCPP_ERR_INVALID_TYPE_STR;
			case MCPP_ERR_INVALID_NC_STRING:
				return MCPP_ERR_INVALID_NC_STRING_STR;
			case MCPP_ERR_FIELDSMISSING:
				return MCPP_ERR_FIELDSMISSING_STR;
		}
	}
	
	return "Unknown error.";
}
