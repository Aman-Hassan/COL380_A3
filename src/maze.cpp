#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "defs.hpp"
#include "mazegenerator.hpp"
#include "mazesolver.hpp"

bool parse_inputs(int argc, char* argv[], char* generation_algorithm, char* solving_algorithm) {
    for (int i = 1; i < argc; ++i) {
        char* arg = argv[i];
        if (strcmp(arg, "-g") == 0) {
            if (i + 1 < argc) {
                strcpy(generation_algorithm, argv[++i]);
                for (char* c = generation_algorithm; *c; ++c) {
                    *c = tolower(*c);
                }
            } else {
                fprintf(stderr, "Error: Missing argument for -g\n");
                return false;
            }
        } else if (strcmp(arg, "-s") == 0) {
            if (i + 1 < argc) {
                strcpy(solving_algorithm, argv[++i]);
                for (char* c = solving_algorithm; *c; ++c) {
                    *c = tolower(*c);
                }
            } else {
                fprintf(stderr, "Error: Missing argument for -s\n");
                return false;
            }
        } else {
            fprintf(stderr, "Error: Unknown argument %s\n", arg);
            return false;
        }
    }

    if (strlen(generation_algorithm) == 0 || strlen(solving_algorithm) == 0) {
        fprintf(stderr, "Error: Missing required arguments\n");
        return false;
    }

    if (strcmp(generation_algorithm, "bfs") != 0 && strcmp(generation_algorithm, "kruskal") != 0) {
        fprintf(stderr, "Error: Invalid generation algorithm '%s'\n", generation_algorithm);
        return false;
    }

    if (strcmp(solving_algorithm, "dfs") != 0 && strcmp(solving_algorithm, "dijkstra") != 0) {
        fprintf(stderr, "Error: Invalid solving algorithm '%s'\n", solving_algorithm);
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    char generation_algorithm[MAX_ARG_LEN];
    char solving_algorithm[MAX_ARG_LEN];

    if (my_rank == 0) {
        if (!parse_inputs(argc, argv, generation_algorithm, solving_algorithm)) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        // //print the arguments
        // printf("From rank %d: generation_algorithm: %s\n", my_rank, generation_algorithm);
        // printf("From rank %d: solving_algorithm: %s\n", my_rank, solving_algorithm);
    }

    // Broadcast the parsed arguments to all processes
    MPI_Bcast(generation_algorithm, MAX_ARG_LEN, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(solving_algorithm, MAX_ARG_LEN, MPI_CHAR, 0, MPI_COMM_WORLD);

    // // for debugging purposes
    // if (my_rank != 0) {
    //     printf("From rank %d: generation_algorithm: %s\n", my_rank, generation_algorithm);
    //     printf("From rank %d: solving_algorithm: %s\n", my_rank, solving_algorithm);
    // }

    // Generate the maze
    generator_main(64, generation_algorithm, MPI_COMM_WORLD);
    
    MPI_Finalize();
    return 0;
}