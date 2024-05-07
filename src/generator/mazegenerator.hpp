#include "defs.hpp"
#include "bfs.hpp"
#include "kruskal.hpp"
//! Function prototypes for maze generation - NOT FINAL
short* init_graph(int size);
short* init_maze(int size);
void generator_main(int size, char solving_algorithm[MAX_ARG_LEN], MPI_Comm comm);