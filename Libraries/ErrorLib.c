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

char *get_mc_error_title(unsigned int err_index) {
	switch(err_index) {
		case MCD_ERR:
			return MCD_ERR_TITLE;
		case MCPP_ERR:
			return MCPP_ERR_TITLE;
		case MCEX_ERR:
			return MCEX_ERR_TITLE;
	}
	
	return "Unknown Error";
}

char *get_err_string(int err_val, unsigned int err_type) {
	unsigned int type = err_type;
	
	if(type == 0) { 
		unsigned int type = is_mc_error(err_val);
	}
	
	if(type > 0) { 
		switch(type) {
			case MCD_ERR:
				// Data saving errors
				switch(err_val) {
					case MCD_ERR_NOFILE:
						return MCD_ERR_NOFILE_STR;
					case MCD_ERR_NOFILENAME:
						return MCD_ERR_NOFILENAME_STR;
					case MCD_ERR_NOPROG:
						return MCD_ERR_NOPROG_STR;
					case MCD_ERR_FILEWRITE:
						return MCD_ERR_FILEWRITE_STR;
					case MCD_ERR_FILEREAD:
						return MCD_ERR_FILEREAD_STR;
				}
				break;
			case MCPP_ERR:
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
					case MCPP_ERR_INVALIDTMODE:
						return MCPP_ERR_INVALIDTMODE_STR;
					case MCPP_ERR_NOARRAY:
						return MCPP_ERR_NOARRAY_STR;
				}
				break;
			case MCEX_ERR:
				switch(err_val) {
					case MCEX_ERR_NOCEXP:
						return MCEX_ERR_NOCEXP_STR;
					case MCEX_ERR_NOPROG:
						return MCEX_ERR_NOPROG_STR;
					case MCEX_ERR_NOSTEPS:
						return MCEX_ERR_NOSTEPS_STR;
					case MCEX_ERR_INVALIDTMODE:
						return MCEX_ERR_INVALIDTMODE_STR;
				}
				break;
		}
	}
	
	return "Unknown error.";
}

unsigned int is_mc_error(int ev) {
	if(ev >= MCD_ERR_MIN && ev <= MCD_ERR_MAX) {
		return MCD_ERR;	
	} else if(ev >= MCPP_ERR_MIN && ev <= MCPP_ERR_MAX) {
		return MCPP_ERR;
	} else if (ev >= MCEX_ERR_MIN && ev >= MCEX_ERR_MAX) {
		return MCEX_ERR;
	}
	
	return 0;
}
