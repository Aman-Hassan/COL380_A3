#include <mpi.h>
#include <vector>
#include <queue>
#include <set>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <cstddef>
#include "dfs.hpp"

// - Nodes of the maze are represented by an integer, 64*row + col
// - We make macros to access row no. and col no., neighbors, etc.
// - make sure that the neighbors macros give valid output, i.e. they don't go out of bounds
// - Also need a check whether a node is valid or not
// - We can store edges as 64x64 1d array of short (16bit) where last 4 bits value representing whether node is connected to left/right/down/up neighbors
// - the last 8 bits - | visited_by_solver | in_path | not_walled | visited | left | right | up | down | 

void print_visited_solve2(short* edges, int size){
    // printf("Printing visited\n");
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            if (IS_VISITED_SOLVE(edges[i * size + j])){
                printf("V");
            } else {
                printf("X");
            }
        }
        printf("\n");
    }
}

// Function to generate a maze using BFS and MPI
// @param size: The size of the maze
// @param maze: The array of nodes of the maze, each last 8bits is | 0 | 0 | selected | visited | left | right | up | down |
void solveUsingDFS(int size, short* maze, MPI_Comm comm, int start, int end){
    // Get the rank and size of the communicator
    int rank, commSize;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &commSize);

    // define the frontier and visited set
    std::vector<int> global_frontier;

    // For each process
    std::vector<int> next_global_frontier; // next frontier for each process
    // We would also need to maintain a list/vector of local_neighbours_paired added (along with with neighbour they are) so that the final maze matrix can be updated before next loop
    std::unordered_map<int, int> parents; // key: neighbour_node, value: node from which it was added. Maps the child to the parent


    global_frontier.push_back(start);
    SET_VISITED_SOLVE(maze[start]);

    // This loop does BFS until we get frontier of size >= commSize, 
    // then we split the frontier nodes among the procs and they do local DFS
    // The BFS loop
    while ((int)(global_frontier.size()) < commSize){

        // If the global frontier is empty, break
        if (global_frontier.size() == 0){
            break;
        }
        // Clear the next local frontier
        next_global_frontier.clear();

        // For each node in the local frontier, add the neighbors to the next local frontier if they are not visited
        for (int parent : global_frontier) {

            int child;
            // Add the neighbors to the next local frontier if they are not visited and not marked as C
            if ((child = LEFT_NODE(parent, size)) != -1 && !IS_VISITED_SOLVE(maze[child]) && IS_C(maze[child])) {
                next_global_frontier.push_back(child);
                parents[child] = parent;
                SET_VISITED_SOLVE(maze[child]);
            }
            if ((child = RIGHT_NODE(parent, size)) != -1 && !IS_VISITED_SOLVE(maze[child]) && IS_C(maze[child])) {
                next_global_frontier.push_back(child);
                parents[child] = parent;
                SET_VISITED_SOLVE(maze[child]);
            }
            if ((child = UP_NODE(parent, size)) != -1 && !IS_VISITED_SOLVE(maze[child]) && IS_C(maze[child])) {
                next_global_frontier.push_back(child);
                parents[child] = parent;
                SET_VISITED_SOLVE(maze[child]);
            }
            if ((child = DOWN_NODE(parent, size)) != -1 && !IS_VISITED_SOLVE(maze[child]) && IS_C(maze[child])) {
                next_global_frontier.push_back(child);
                parents[child] = parent;
                SET_VISITED_SOLVE(maze[child]);
            }
        }

        // Update the global frontier
        global_frontier = next_global_frontier;
    }

    std::vector<int> local_frontier; // current frontier for each process
    int local_frontier_size = global_frontier.size() / commSize + (rank < (int)(global_frontier.size() % commSize) ? 1 : 0);
    for (int i = 0; i < local_frontier_size; i++){
        local_frontier.push_back(global_frontier[i + rank * local_frontier_size]);
    }
    
    // Could sync here but not necessary

    bool found = false;
    // DFS loop
    for (int node : local_frontier){
        std::vector<int> stack;
        stack.push_back(node);
        while (true) {
            if (stack.empty()) {
                break;
            }
            if (stack.back() == end){
                found = true;
                break;
            }   

            // Get the last element of the stack
            int current_node = stack.back();

            // For each of the 4 neighbors of the current node
            int child;
            if ((child = LEFT_NODE(current_node, size)) != -1 && !IS_VISITED_SOLVE(maze[child]) && IS_C(maze[child])) {
                stack.push_back(child);
                parents[child] = current_node;
                SET_VISITED_SOLVE(maze[child]);
                continue;
            }
            if ((child = RIGHT_NODE(current_node, size)) != -1 && !IS_VISITED_SOLVE(maze[child]) && IS_C(maze[child])) {
                stack.push_back(child);
                parents[child] = current_node;
                SET_VISITED_SOLVE(maze[child]);
                continue;
            }
            if ((child = UP_NODE(current_node, size)) != -1 && !IS_VISITED_SOLVE(maze[child]) && IS_C(maze[child])) {
                stack.push_back(child);
                parents[child] = current_node;
                SET_VISITED_SOLVE(maze[child]);
                continue;
            }
            if ((child = DOWN_NODE(current_node, size)) != -1 && !IS_VISITED_SOLVE(maze[child]) && IS_C(maze[child])) {
                stack.push_back(child);
                parents[child] = current_node;
                SET_VISITED_SOLVE(maze[child]);
                continue;
            }
            stack.pop_back();
        }
        if (found)
            break;
    }

    //? Maybe should give a signal to other procs to give up once any proc finds the end?

    int found_rank;
    if (found){
        // If found, update the PATH flags in maze
        int end_node = end;
        while (end_node != start){
            SET_P(maze[end_node]);
            end_node = parents[end_node];
        }
        found_rank = rank;
        // send found_rank to rank 0
        // if 0 found it, no need to send
        if (rank != 0)
            MPI_Send(&found_rank, 1, MPI_INT, 0, 0, comm);
    } else {
        // if not found, and rank 0, receive from the proc that found it
        if (rank == 0){
            MPI_Status status;
            MPI_Recv(&found_rank, 1, MPI_INT, MPI_ANY_SOURCE, 0, comm, &status);
        }
    }

    // Broadcast the found_rank from 0
    MPI_Bcast(&found_rank, 1, MPI_INT, 0, comm);
   
    // broadcast the maze from the proc that found the end
    MPI_Bcast(maze, size * size, MPI_SHORT, found_rank, comm);
}