#ifndef VICTIMSELECT_H
#define VICTIMSELECT_H 1

#if defined(__VS_ORIG__)

inline void vsinit(int rank, int size)
{
}

inline char * vsdescript(void)
{
#if defined(__SS_HALF__)
	return "MPI Workstealing (Half Round Robin)";
#else
	return "MPI Workstealing (Round Robin)";
#endif
}

inline int selectvictim(int rank, int size, int last)
{
	last = (last + 1) % size;
	if(last == rank) last = (last + 1) % size;
	return last;
}

#elif defined(__VS_RAND__)

inline void vsinit(int rank, int size)
{
	srand(rank);
}

inline char * vsdescript(void)
{
#if defined(__SS_HALF__)
	return "MPI Workstealing (Half Rand)";
#else
	return "MPI Workstealing (Rand)";
#endif
}

inline int selectvictim(int rank, int size, int last)
{
	do {
		last = rand()%size;
	} while(last == rank);
	return last;
}

#elif defined(__VS_GSLUI__)
#include <gsl/gsl_rng.h>
int *victims;
gsl_rng *rng;

inline char * vsdescript()
{
#if defined(__SS_HALF__)
	return "MPI Workstealing (Half GSL Uniform Int)";
#else
	return "MPI Workstealing (GSL Uniform Int)";
#endif
}

inline void vsinit(int rank, int size)
{
	int i,j;
	// allocate & init the victim array
	victims = malloc((size -1) * sizeof(int));
	for(i = 0, j = 0; i < size; i++)
		if(i != rank)
			victims[j++] = i;
	// init an rng, using the rank-th number from the global sequence as seed
	gsl_rng_env_setup();
	const gsl_rng_type *T = gsl_rng_default;
	rng = gsl_rng_alloc(T);
	for(i = 0; i < rank; i++)
		gsl_rng_uniform_int(rng,UINT_MAX);
	gsl_rng_set(rng,gsl_rng_uniform_int(rng,UINT_MAX));
}

inline int selectvictim(int rank, int size, int last)
{
	last = gsl_rng_uniform_int(rng,size -1);
	last = victims[last];
	return last;
}

#elif defined(__VS_GSLRD__)
#include <mpi-ext.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

int *victims;
double *weights;
gsl_rng *rng;
gsl_ran_discrete_t *table;

inline void vsinit(int rank, int size)
{
	int i,j;
	// allocate & init the victim array
	victims = malloc((comm_size -1) * sizeof(int));
	for(i = 0, j = 0; i < comm_size; i++)
		if(i != comm_rank)
			victims[j++] = i;
	// compute weights, using the tofu coords
	int mx,my,mz,ma,mb,mc;
	FJMPI_Topology_sys_rank2xyzabc(comm_rank,&mx,&my,&mz,&ma,&mb,&mc);
	weights = malloc((comm_size -1) * sizeof(double));
	for(i = 0 ; i < comm_size-1; i++)
	{
		//find coords of this rank
		int x,y,z,a,b,c;
		FJMPI_Topology_sys_rank2xyzabc(victims[i],&x,&y,&z,&a,&b,&c);
		// compute euclidian distance between nodes
		double d = pow(mx - x,2) + pow(my - y,2) + pow(mz - z,2) + pow(ma - a,2) + pow(mb - b,2)
			+ pow(mc - c,2);
#if defined(__VS_FIX__)
		if(d < 1.0) d = 1.0;
#endif
		d = sqrt(d);
		weights[i] = 1.0/d;
	}
	// init an rng, using the rank-th number from the global sequence as seed
	gsl_rng_env_setup();
	const gsl_rng_type *T = gsl_rng_default;
	rng = gsl_rng_alloc(T);
	for(i = 0; i < rank; i++)
		gsl_rng_uniform_int(rng,UINT_MAX);
	gsl_rng_set(rng,gsl_rng_uniform_int(rng,UINT_MAX));
	table = gsl_ran_discrete_preproc(comm_size-1,weights);
}

inline char * vsdescript(void)
{
#if defined(__SS_HALF__)
	return "MPI Workstealing (Half GSL Distance Tofu)";
#else
	return "MPI Workstealing (GSL Tofu Weights)";
#endif
}

inline int selectvictim(int rank, int size, int last)
{
	last = gsl_ran_discrete(rng,table);
	last_steal = victims[last_steal];
	return last;
}

#else
#error "You forgot to select a victim selection"
#endif /* Strategy selection */

#endif /* VICTIMSELECT_H */
