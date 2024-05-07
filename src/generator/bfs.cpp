#include <mpi.h>
#include <vector>
#include <queue>
#include <set>
#include <unordered_map>
#include <random>
#include <algorithm>

#include "bfs.hpp"

// - Nodes of the maze are represented by an integer, 64*row + col
// - We make macros to access row no. and col no., neighbors, etc.
// - make sure that the neighbors macros give valid output, i.e. they don't go out of bounds
// - Also need a check whether a node is valid or not
// - We can store edges as 64x64 1d array of char (8bit) where last 4 bits value representing whether node is connected to left/right/down/up neighbors
// | 0 | 0 | 0 | visited | left | right | up | down | -> 8 bits


// Function to generate a maze using BFS and MPI
// @param size: The size of the maze
// @param maze: The array of nodes of the maze, each char is | 0 | 0 | selected | visited | left | right | up | down |
void generateMazeUsingBFS(int size, char *maze, MPI_Comm comm){
    // Get the rank and size of the communicator
    int rank, commSize;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &commSize);
    
    // Get random start node
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, size * size - 1);
    int start = dis(gen);
    

    // define the frontier and visited set

    std::vector<int> global_frontier;
    
    // For each process
    std::vector<int> local_frontier; // current frontier for each process
    std::vector<int> next_local_frontier; // next frontier for each process
    // We would also need to maintain a list/vector of neighbour_nodes added (along with with neighbour they are) so that the final maze matrix can be updated before next loop
    std::unordered_map<int, int> neighbour_nodes_added; // key: neighbour_node, value: node from which it was added
    
    // if rank is 0, add the start node to the frontier
    if (rank == 0){
        global_frontier.push_back(start);
        // Broadcast the global frontier
        MPI_Bcast(global_frontier.data(), global_frontier.size(), MPI_INT, 0, comm);
        
    }
    // Start the loop
    while (true){
        // If the global frontier is empty, break
        if (global_frontier.size() == 0){
            break;
        }
        // Set the visted bit of the nodes in the global frontier
        for (int node : global_frontier){
            SET_VISITED(maze[node]);
        }

        // MPI sync
        MPI_Barrier(comm);

        // Clear the next local frontier
        local_frontier.clear();

        // Split the global frontier into local frontiers per proc
        int local_frontier_size = global_frontier.size() / commSize;
        for (int i = 0; i < local_frontier_size; i++){
            local_frontier.push_back(global_frontier[i + rank * local_frontier_size]);
        }

        // For each node in the local frontier, add the neighbors to the next local frontier if they are not visited
        // Set them as visited for current proc (we dont want to check the nodes again in case they are a neighbour of another node in the local frontier)
        for (int node : local_frontier){
            int neighbour_node;
            // Add the neighbors to the next local frontier if they are not visited
            if ((neighbour_node = LEFT_NODE(node, size)) != -1 && !GET_VISITED(maze[neighbour_node])){
                SET_VISITED(maze[neighbour_node]);
                next_local_frontier.push_back(neighbour_node);
                neighbour_nodes_added[neighbour_node] = node;
            }
            if ((neighbour_node = RIGHT_NODE(node, size)) != -1 && !GET_VISITED(maze[neighbour_node])){
                SET_VISITED(maze[neighbour_node]); 
                next_local_frontier.push_back(neighbour_node);
                neighbour_nodes_added[neighbour_node] = node;
            }
            if ((neighbour_node = UP_NODE(node, size)) != -1 && !GET_VISITED(maze[neighbour_node])){
                SET_VISITED(maze[neighbour_node]); 
                next_local_frontier.push_back(neighbour_node);
                neighbour_nodes_added[neighbour_node] = node;
            }
            if ((neighbour_node = DOWN_NODE(node, size)) != -1 && !GET_VISITED(maze[neighbour_node])){
                SET_VISITED(maze[neighbour_node]);
                next_local_frontier.push_back(neighbour_node);
                neighbour_nodes_added[neighbour_node] = node;
            }
        }


        
        // // do reduction to merge all the next local frontiers of each proc into the global frontier
        // MPI_Allreduce(next_local_frontier.data(), global_frontier.data(), next_local_frontier.size(), MPI_INT, MPI_SUM, comm);

        // Do reduction to merge all next local frontiers of each proc into the global frontier - do this in proc 0
        // Then shuffle the global frontier, set visited bit and then broadcast
        if (rank == 0){
            // Make a set instead of vector (to avoid duplicates)
            std::set <int> global_frontier_temp;
            for (int i = 1; i < commSize; i++){
                std::vector<int> temp(local_frontier_size);
                MPI_Recv(temp.data(), local_frontier_size, MPI_INT, i, 0, comm, MPI_STATUS_IGNORE);
                global_frontier_temp.insert(temp.begin(), temp.end());
                // global_frontier.insert(global_frontier.end(), temp.begin(), temp.end());
            }
            // Convert the set to vector
            global_frontier.assign(global_frontier_temp.begin(), global_frontier_temp.end());
            std::random_shuffle(global_frontier.begin(), global_frontier.end());
            MPI_Bcast(global_frontier.data(), global_frontier.size(), MPI_INT, 0, comm);
        } else {
            MPI_Send(next_local_frontier.data(), next_local_frontier.size(), MPI_INT, 0, 0, comm);
        }

        // Now we need to get the neighbour_nodes_added from all the processes and update the maze for each of them
        // We can do this by sending the neighbour_nodes_added from each process to process 0 and Broadcasting it to all processes
        // Then each process can update the maze accordingly
        // We would probably need to create a new MPI data type for the unordered_map
        //! CHECK IF THE BELOW PART IS PROPERLY CREATED
        MPI_Datatype MPI_UNORDERED_MAP;
        MPI_Type_contiguous(2, MPI_INT, &MPI_UNORDERED_MAP);
        MPI_Type_commit(&MPI_UNORDERED_MAP);
        std::unordered_map<int, int> neighbour_nodes_added_global;
        if (rank == 0){
            for (int i = 1; i < commSize; i++){
                std::unordered_map<int, int> temp;
                MPI_Recv(&temp, 1, MPI_UNORDERED_MAP, i, 0, comm, MPI_STATUS_IGNORE);
                neighbour_nodes_added_global.insert(temp.begin(), temp.end());
            }
            neighbour_nodes_added_global.insert(neighbour_nodes_added.begin(), neighbour_nodes_added.end());
            MPI_Bcast(&neighbour_nodes_added_global, 1, MPI_UNORDERED_MAP, 0, comm);
        } else {
            MPI_Send(&neighbour_nodes_added, 1, MPI_UNORDERED_MAP, 0, 0, comm);
        }

        // Update the maze for each process
        for (auto it = neighbour_nodes_added_global.begin(); it != neighbour_nodes_added_global.end(); it++){
            int neighbour_node = it->first;
            int node = it->second;
            if (LEFT_NODE(node, size) == neighbour_node){
                SET_RIGHT(maze[node]);
                SET_LEFT(maze[neighbour_node]);
            } else if (RIGHT_NODE(node, size) == neighbour_node){
                SET_LEFT(maze[node]);
                SET_RIGHT(maze[neighbour_node]);
            } else if (UP_NODE(node, size) == neighbour_node){
                SET_DOWN(maze[node]);
                SET_UP(maze[neighbour_node]);
            } else if (DOWN_NODE(node, size) == neighbour_node){
                SET_UP(maze[node]);
                SET_DOWN(maze[neighbour_node]);
            }
        }

    }

    // The tree has now been generated and is stored in the maze

}