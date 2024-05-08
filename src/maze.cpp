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

void print_maze_final(short* edges, int size, int start, int end){
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            int node = NODE(i, j, size);
            if (IS_W(edges[node])){
                // a) * for wall cells in the maze,
                printf("*");
            } else if (node == start){
                // (d) S for the entry cell
                printf("S");
            } else if (node == end){
                // (e) E for the exit cell
                printf("E");
            } else if (IS_P(edges[node])){
                // (c) P for non-wall cells in the maze in solution path,
                printf("P");
            } else if (IS_C(edges[node])){
                // (b) space for non-wall cells in the maze not in solution path,
                printf(" ");
            } else {
                printf("?");
            }
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int size = 64;
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    char generation_algorithm[MAX_ARG_LEN];
    char solving_algorithm[MAX_ARG_LEN];

    if (my_rank == 0) {
        if (!parse_inputs(argc, argv, generation_algorithm, solving_algorithm)) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Broadcast the parsed arguments to all processes
    MPI_Bcast(generation_algorithm, MAX_ARG_LEN, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(solving_algorithm, MAX_ARG_LEN, MPI_CHAR, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD); //? Barrier to make sure all processes have received the arguments -> Is this needed?

    int start = NODE(0, size-1, size);
    int end = NODE(size-1, 0, size);

    // Generate the maze
    short* maze = generator_main(size, generation_algorithm, MPI_COMM_WORLD);
    // printf("Maze generated\n");
    // if (my_rank == 0)
        // print_maze_complete(maze, size);
    solver_main(size, maze, solving_algorithm, MPI_COMM_WORLD, start, end);
    // printf("Maze solved\n");

    MPI_Barrier(MPI_COMM_WORLD);

    if (my_rank == 0)
        print_maze_final(maze, size, start, end);

    free(maze);
    
    MPI_Finalize();
    return 0;
}