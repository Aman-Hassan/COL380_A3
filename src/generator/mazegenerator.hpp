#include "defs.hpp"
#include "bfs.hpp"
#include "kruskal.hpp"
//! Function prototypes for maze generation - NOT FINAL
short* init_graph(int size);
void init_maze(int size, short* maze);
short* generator_main(int size, char solving_algorithm[MAX_ARG_LEN], MPI_Comm comm);