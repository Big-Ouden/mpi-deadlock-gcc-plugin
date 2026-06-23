#include <mpi.h>

#define NEW(x, val)                                                            \
  int __priv__##x = (val);                                                     \
  int *x = &__priv__##x

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  NEW(a, 0);
  NEW(b, 0);
  MPI_Barrier(MPI_COMM_WORLD);

  if (a == b) {
    MPI_Barrier(MPI_COMM_WORLD);
  } else {
    MPI_Barrier(MPI_COMM_WORLD);
  }

  MPI_Finalize();

  return 0;
}
