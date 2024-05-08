#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "mazesolver.hpp"
// Entry to the maze should be at top right (0,63) and exit from the maze should be at bottom left (63,0)

void solver_main(int size, short* maze, char solving_algorithm[MAX_ARG_LEN], MPI_Comm comm, int start, int end){
    int rank;
    MPI_Comm_rank(comm, &rank);
    if (strcmp(solving_algorithm, "dfs") == 0){
        solveUsingDFS(size, maze, comm, start, end);
    } else if (strcmp(solving_algorithm, "dijkstra") == 0){
        solveUsingDijkstra(size, maze, comm, start, end);
    }
    else {
        printf("Invalid solving algorithm\n");
    }

}