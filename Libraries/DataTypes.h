#ifndef DATA_TYPES_H
#define DATA_TYPES_H


typedef struct dstor {
	// Structure for storing data (in caches and such).	
	int nt;					// Number of transients
	int maxds;				// Maximum dimension step
	int np;					// Number of points
	int nc;					// Number of channels
	int nd;					// Number of dimensions.
	int *dim_step;			// Dimension step for de-indexing
	
	double ***data;			// 3D Array for the data [nt][maxds][np*nc] 
	double ****data4;		// 4D Array for the data [nt][maxds][nc][np]
							// This should actually point to the same
							// place in memory as the 3D array, so only
							// free the top two layers.
	
	int valid;
	
} dstor;

typedef struct dheader {
	// Type for containing information from a data header
	// Much of this information will be stored in a CEXP.
	
	char *filename; 			// Path of the data
	char *expname;				// Experiment name
	char *desc;					// Experiment description
	int num;					// Indexing of experiments with the same name
	unsigned __int64 hash;		// Unique identifier
	
	char *dstamp;				// Date stamp
	time_t tstarted;			// Time started
	time_t tdone;				// Time the last part of the experiment finished
	
	unsigned int nchans;		// Number of channels.
	unsigned int cind;			// Current index.
	unsigned int *maxsteps; 	// Maximum steps (see PPROGRAM)
	
	int valid;
} dheader;

#endif
