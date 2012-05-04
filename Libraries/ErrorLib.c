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

#include <userint.h>
#include <ErrorDefs.h>
#include <ErrorLib.h>

void display_error(int err_val) {
	unsigned int type = is_mc_error(err_val);
	
	MessagePopup(get_mc_error_title(type), get_err_string(err_val, type));
}

char *get_mc_error_title(unsigned int err_index) {
	switch(err_index) {
		case MCF_ERR:
			return MCF_ERR_TITLE;
		case MCD_ERR:
			return MCD_ERR_TITLE;
		case MCPP_ERR:
			return MCPP_ERR_TITLE;
		case MCEX_ERR:
			return MCEX_ERR_TITLE;
		case MCSS_ERR:
			return MCSS_ERR_TITLE;
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
			case MCF_ERR:
				switch(err_val) {
					case MCF_ERR_INVALID_MAX_BYTES:
						return MCF_ERR_INVALID_MAX_BYTES_STR;
					case MCF_ERR_FSAVE_NOT_FOUND:
						return MCF_ERR_FSAVE_NOT_FOUND_STR;
					case MCF_ERR_BAD_FSAVE_REPLACEMENT_NAME:
						return MCF_ERR_BAD_FSAVE_REPLACEMENT_NAME_STR;
					case MCF_ERR_FSAVE_TYPE_MISMATCH:
						return MCF_ERR_FSAVE_TYPE_MISMATCH_STR;
					case MCF_ERR_FSAVE_SIZE_MISMATCH:
						return MCF_ERR_FSAVE_SIZE_MISMATCH_STR;
					case MCF_ERR_CANNOT_TRUNCATE:
						return MCF_ERR_CANNOT_TRUNCATE_STR;
					case MCF_ERR_NOFILE:
						return MCF_ERR_NOFILE_STR;
					case MCF_ERR_BADFNAME:
						return MCF_ERR_BADFNAME_STR;
					case MCF_ERR_NODATA:
						return MCF_ERR_NODATA_STR;
					case MCF_ERR_CUST_NOENTRIES:
						return MCF_ERR_CUST_NOENTRIES_STR;
					case MCF_ERR_INVALID_TYPE:
						return MCF_ERR_INVALID_TYPE_STR;
					case MCF_ERR_FLOC_NAME:
						return MCF_ERR_FLOC_NAME_STR;
					case MCF_ERR_FLOC_TYPE:
						return MCF_ERR_FLOC_TYPE_STR;
					case MCF_ERR_NOFLOCS:
						return MCF_ERR_NOFLOCS_STR;
					case MCF_ERR_FILEREAD:
						return MCF_ERR_FILEREAD_STR;
					case MCF_ERR_FILEWRITE:
						return MCF_ERR_FILEWRITE_STR;
					case MCF_ERR_NOSTRING:
						return MCF_ERR_NOSTRING_STR;
					case MCF_ERR_FS_NOTYPE:
						return MCF_ERR_FS_NOTYPE_STR;
					case MCF_ERR_FS_NOSIZE:
						return MCF_ERR_FS_NOSIZE_STR;
					case MCF_ERR_FS_BADCONTENTS:
						return MCF_ERR_FS_BADCONTENTS_STR;
					case MCF_ERR_NOARRAY:
						return MCF_ERR_NOARRAY_STR;
					case MCF_ERR_NOFSAVE:
						return MCF_ERR_NOFSAVE_STR;
					case MCF_ERR_EMPTYVAL:
						return MCF_ERR_EMPTYVAL_STR;
					case MCF_ERR_FS_NONS:
						return MCF_ERR_FS_NONS_STR;
					case MCF_ERR_FS_NONAME:
						return MCF_ERR_FS_NONAME_STR;
					case MCF_ERR_EOF:
						return MCF_ERR_EOF_STR;
				}
				break;
			case MCD_ERR:
				// Data saving errors
				switch(err_val) {
					case MCD_ERR_NOFILENAME:
						return MCD_ERR_NOFILENAME_STR;
					case MCD_ERR_NOFILE:
						return MCD_ERR_NOFILE_STR;
					case MCD_ERR_NOPROG:
						return MCD_ERR_NOPROG_STR;
					case MCD_ERR_FILEWRITE:
						return MCD_ERR_FILEWRITE_STR;
					case MCD_ERR_FILEREAD:
						return MCD_ERR_FILEREAD_STR;
					case MCD_ERR_NODATA:
						return MCD_ERR_NODATA_STR;
					case MCD_ERR_NOAVGDATA:
						return MCD_ERR_NOAVGDATA_STR;
					case MCD_ERR_BADCIND:
						return MCD_ERR_BADCIND_STR;
					case MCD_ERR_NOPATH:
						return MCD_ERR_NOPATH_STR;
					case MCD_ERR_NOINPUTCHANS:
						return MCD_ERR_NOINPUTCHANS_STR;
					case MCD_ERR_NOSTEPSTR:
						return MCD_ERR_NOSTEPSTR_STR;
					case MCD_ERR_INVALIDSTR:
						return MCD_ERR_INVALIDSTR_STR;
					case MCD_ERR_INVALID_EXTENSION:
						return MCD_ERR_INVALID_EXTENSION_STR;
					case MCD_ERR_BADHEADER:
						return MCD_ERR_BADHEADER_STR;
					case MCD_ERR_MALFORMED_PROG:
						return MCD_ERR_MALFORMED_PROG_STR;
					case MCD_ERR_NOCHANS:
						return MCD_ERR_NOCHANS_STR;
				}
				break;
			case MCPP_ERR:
				// Pulse program return values
				switch(err_val) {
					case MCPP_ERR_NOFILE:
						return MCPP_ERR_NOFILE_STR;
					case MCPP_ERR_NOPROG:
						return MCPP_ERR_NOPROG_STR;
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
					case MCPP_ERR_MALFORMED_FNAME:
						return MCPP_ERR_MALFORMED_FNAME_STR;
					case MCPP_ERR_FILE_NOPROG:
						return MCPP_ERR_FILE_NOPROG_STR;
					case MCPP_ERR_FILE_NOINSTRS:
						return MCPP_ERR_FILE_NOINSTRS_STR;
					case MCPP_ERR_PROG_PROPS_LABELS:
						return MCPP_ERR_PROG_PROPS_LABELS_STR;
					case MCPP_ERR_INVALID_NC_STRING:
						return MCPP_ERR_INVALID_NC_STRING_STR;
					case MCPP_ERR_FIELDSMISSING:
						return MCPP_ERR_FIELDSMISSING_STR;
					case MCPP_ERR_INVALIDTMODE:
						return MCPP_ERR_INVALIDTMODE_STR;
					case MCPP_ERR_NOARRAY:
						return MCPP_ERR_NOARRAY_STR;
					case MCPP_ERR_FS_NOTYPE:
						return MCPP_ERR_FS_NOTYPE_STR;
					case MCPP_ERR_FS_NOSIZE:
						return MCPP_ERR_FS_NOSIZE_STR;
					case MCPP_ERR_FS_BADCONTENTS:
						return MCPP_ERR_FS_BADCONTENTS_STR;
					case MCPP_ERR_NOSTRING:
						return MCPP_ERR_NOSTRING_STR;
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
			case MCSS_ERR:
				switch(err_val) {
					case MCSS_ERR_NOOUT:
						return MCSS_ERR_NOOUT_STR;
					case MCSS_ERR_NOATTNAME:
						return MCSS_ERR_NOATTNAME_STR;
				}
				break;
		}
	}
	
	return "Unknown error.";
}

unsigned int is_mc_error(int ev) {
	if(ev >= MCF_ERR_MIN && ev <= MCF_ERR_MAX) {
		return MCF_ERR;
	} else if(ev >= MCD_ERR_MIN && ev <= MCD_ERR_MAX) {
		return MCD_ERR;	
	} else if(ev >= MCPP_ERR_MIN && ev <= MCPP_ERR_MAX) {
		return MCPP_ERR;
	} else if (ev >= MCEX_ERR_MIN && ev >= MCEX_ERR_MAX) {
		return MCEX_ERR;
	} else if (ev >= MCSS_ERR_MIN && ev <= MCSS_ERR_MAX) {
		return MCSS_ERR;	
	}
	
	return 0;
}
