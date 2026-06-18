#include <mpi.h>

#define NEW(x, val)                                                            \
  int __priv__##x = (val);                                                     \
  int *x = &__priv__##x

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  NEW(a, 0);
  NEW(b, 1);
  NEW(c, 3);

  if (a == b) {
    if (b == c) {
      MPI_Barrier(MPI_COMM_WORLD);
      goto next;
    }
    MPI_Barrier(MPI_COMM_WORLD);
  } else {
    MPI_Barrier(MPI_COMM_WORLD);
  }

next:
  MPI_Finalize();

  return 0;
}
