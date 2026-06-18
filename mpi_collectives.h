#ifndef MPI_COLLECTIVES_H
#define MPI_COLLECTIVES_H

#include <string>
#include <gcc-plugin.h>
#include <tree.h>
#include <gimple.h>
#include <gimple-iterator.h>

/* Enum to represent the collective operations */
enum mpi_collective_code {
#define DEFMPICOLLECTIVES( CODE, NAME ) CODE,
#include "MPI_collectives.def"
	LAST_AND_UNUSED_MPI_COLLECTIVE_CODE
#undef DEFMPICOLLECTIVES
} ;

/* Name of each MPI collective operations */
#define DEFMPICOLLECTIVES( CODE, NAME ) NAME,
const char *const mpi_collective_name[] = {
#include "MPI_collectives.def"
} ;
#undef DEFMPICOLLECTIVES
enum mpi_collective_code get_mpi_call_code(gimple* stmt);
#endif
