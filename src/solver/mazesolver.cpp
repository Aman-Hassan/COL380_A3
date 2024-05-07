#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "mazesolver.hpp"

void solver_main(int size, char* edges, char solving_algorithm[MAX_ARG_LEN], MPI_Comm comm){
    int rank;
    MPI_Comm_rank(comm, &rank);
    if (strcmp(solving_algorithm, "dfs") == 0){
        // solveUsingDFS(size, edges, comm);
    } else if (strcmp(solving_algorithm, "dijkstra") == 0){
        // solveUsingDijkstra(size, edges, comm);
    }
    else {
        printf("Invalid solving algorithm\n");
    }

}