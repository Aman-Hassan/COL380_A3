#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <random>

#include "mazegenerator.hpp"

/* Basic Implementation Idea
^ - According to assignment instructions, we have to assign each cell in 64x64 maze as either "wall" cell or "non-wall" cell
^ - Generated maze should be perfect maze

* - Instead of the typical Adjacency List/Adjaceny Matrix representations we just add nodes as short (from which there are macros for getting row/col and its neighbours)
* - We can store the edges as a 8-bit short, where each bit represents whether the node is connected to the left, right, up, down neighbors and other necessary info like visited, etc.

Main Idea:
* - Intial graph of 32x32 nodes where all neighbours are connected ( should we prbly give weights?)
* - From a randomly selected root among these nodes we'll run BFS/Kruskal to generate a Spanning Tree (Min spanning in case of Kruskal)
* - Now to generate the 64*64 maze we do the following:
*   - Initialize maze to be all walls
*   - Every node in the spanning tree is made as C (i.e. non-wall cell) in the maze
*   - Every edge in the spanning tree i.e. an edge between 2 vertices in the spanning tree is made to be a C (i.e. non-wall cell) in the maze
*   - Now this would have created a 63x63 maze
*   - To make it 64x64 we finally add a new column and a new row at the end of the 63x63 maze
*   - Finally we need to add a path to the end of the maze (i.e. the cell (63, 63)) from the cell (62, 62) [The rest is already connected] -> Possibly just make (62, 63) or (63, 62) [but not both] as C
*   - Optionally we could add alternate C and W cells in the last added column and row to make the maze more complex (C should only be alternatively added from 0 to 61) [62nd one would be C depending on above step and 63rd one is the end cell which by defualt is C]

So basically we'll
1) Initalize 32x32 graph (maybe with weights)
2) Pass this to BFS/Kruskal depending on input from mazegnerator
3) Get the spanning tree
4) Convert this spanning tree to a 64x64 maze
5) Add a path from (62, 62) to (63, 63)
6) Optionally add more complexity by adding alternate C and W cells in the last row and column

*/

//! Possibly need to include weights -> would have to change macro's and the way we store edges
short* init_graph(int size){
    // create random weight for node
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 255); // since we have 8 bits for weight
    short* edges = (short*)malloc(size * size * sizeof(short));
    for (int i = 0; i < size * size; i++){
        edges[i] = 0x00; // Nothing is set, (But while doing bfs/kruskal we'll assume fully connected i.e. we wont be using the GET_LEFT, GET_RIGHT, etc. macros)
        SET_NODE_WEIGHT(edges[i], dis(gen)); // Set the weight of the node
    }
    return edges;
}

// Initialize the maze with all walls
short* init_maze(int size){
    // create random weight for node
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 255); // since we have 8 bits for weight
    short* maze = (short*)malloc(size * size * sizeof(short));
    for (int i = 0; i < size * size; i++){
        maze[i] = 0x00; // All walls (i.e. no connections at all)
    }
    return maze;

}


void expand_edges_to_maze(int size, short* edges, short* maze){
    // Now we need to convert the edges to the maze
    // Every (i, j) in the edges should be converted to (2*i, 2*j) in the maze and should be made as C
    // if there is a connection between say (i, j) and (i, j+1) then (2*i, 2*j+1) should be made as C 
    // else if there is a connection between say (i, j) and (i+1, j) then (2*i+1, 2*j) should be made as C
    // We should also change the corresponding edge bit values in the maze
    int edges_size = (size + 1) / 2;
    for (int i = 0; i < edges_size; i++){
        for (int j = 0; j < edges_size; j++){
            int node_edges_ind = i * edges_size + j; // index in the edges array
            int node_maze_ind = 2 * i * size + 2 * j; // index in the maze array
            maze[node_maze_ind] = edges[node_edges_ind]; // Every edge that was there in spanning tree is also there in the maze
            SET_C(maze[node_maze_ind]); // Set the C bit in maze array for the node
            if (GET_RIGHT(edges[node_edges_ind])){
                SET_C(maze[node_maze_ind + 1]); // Set the C bit in maze array for the right neighbour
            }
            if (GET_DOWN(edges[i * edges_size + j])){
                SET_C(maze[node_maze_ind + size]); // Set the C bit in maze array for the down neighbour
            }
        }
    }

    // Now we need to add the last row and column
    // We can just add the last row and column as walls
    for (int i = 0; i < size; i++){
        maze[size * (size - 1) + i] = 0x00; // Last row as walls
        maze[size * i + size - 1] = 0x00; // Last column as walls
    }

    // But we need a path from (62, 62) to (63, 63)
    // We can just make (62, 63) or (63, 62) as C
    if (maze[size * (size-2) + (size-1)] == 0x00){
        SET_C(maze[size * (size-2) + (size-1)]);
    } else {
        SET_C(maze[size * (size-1) + (size-2)]);
    }

    SET_C(maze[size * (size-1) + (size-1)]); // The end cell is always C

    // Optionally add more complexity by adding alternate C and W cells in the last row and column
    // for last row
    for (int i = 0; i < size - 2; i++){
        if (i % 2 == 0){
            SET_C(maze[size * (size - 1) + i]);
        }
    }

    // for last column
    for (int i = 0; i < size - 2; i++){
        if (i % 2 == 0){
            SET_C(maze[size * i + size - 1]);
        }
    }



}

void generator_main(int size, char solving_algorithm[MAX_ARG_LEN], MPI_Comm comm){
    int rank;
    MPI_Comm_rank(comm, &rank);

    int graph_size = (size + 1) / 2; // The size of the graph (i.e. the number of nodes in the graph)
    //! Cannot do the below now because of the random generation of weights -> This would cause each process to have different weights for the same nodes
    // short* edges = init_graph(graph_size); // Generates the initial graph with all neighbours connected (shrinked graph) -> size + 1 for odd sizes
    short* edges = (short*)malloc(graph_size * graph_size * sizeof(short));

    // broadcast one initialized graph from rank 0 to all other processes
    if (rank == 0){
        edges = init_graph(graph_size);
    }

    MPI_Bcast(edges, graph_size * graph_size, MPI_SHORT, 0, comm);

    if (strcmp(solving_algorithm, "bfs") == 0){
        generateTreeUsingBFS(graph_size, edges, comm);
    } else if (strcmp(solving_algorithm, "kruskal") == 0){
        generateTreeUsingKruskal(graph_size, edges, comm);
    }
    else {
        printf("Invalid solving algorithm\n");
    }

    // edges now contain the (min) spanning tree
    // Now we need to convert this to a 64x64 maze
    // We can do this by initializing a 64x64 maze with all walls
    if (rank == 0){
        short* maze = init_maze(size);
        // print_edges(edges, (size + 1) / 2); // For debugging purposes
        expand_edges_to_maze(size, edges, maze);
        // printing the final obtained maze
        print_maze_complete(maze, size);
        free(maze);
    }
    free(edges);
}

