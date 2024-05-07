#include "defs.hpp"
#include "bfs.hpp"
#include "kruskal.hpp"
//! Function prototypes for maze generation - NOT FINAL
char* init_graph(int size);
char* init_maze(int size);
void generator_main(int size, char solving_algorithm[MAX_ARG_LEN], MPI_Comm comm);