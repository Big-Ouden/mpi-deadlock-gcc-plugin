#include "mpi_collectives.h"

enum mpi_collective_code get_mpi_call_code(gimple* stmt)
{
    if (!is_gimple_call(stmt)) {
        return LAST_AND_UNUSED_MPI_COLLECTIVE_CODE;
    }

    tree called_fun = gimple_call_fndecl(stmt);
    if (NULL == called_fun) {
        return LAST_AND_UNUSED_MPI_COLLECTIVE_CODE;
    }

    std::string fun_name = std::string(IDENTIFIER_POINTER(DECL_NAME(called_fun)));

    for (int i = MPI_INIT; i < LAST_AND_UNUSED_MPI_COLLECTIVE_CODE; ++i) {
        std::string mpi = std::string(mpi_collective_name[i]);
        if (fun_name.compare(mpi) == 0) {
            return mpi_collective_code(i);
        }
    }

    return LAST_AND_UNUSED_MPI_COLLECTIVE_CODE;
}
