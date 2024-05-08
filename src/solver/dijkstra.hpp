#include <mpi.h>
#include "defs.hpp"
void solveUsingDijkstra(int size, short* maze, MPI_Comm comm, int start, int end);