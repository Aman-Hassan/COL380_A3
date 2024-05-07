#include <mpi.h>
#include <vector>
#include <queue>
#include <set>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <cstddef>
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
    int start, loop_iter = 0;
    
    //& debugging statments -> remove later
    //printf("Rank: %d, Size: %d\n", rank, commSize);
    
    // define the frontier and visited set
    std::vector<int> global_frontier;
    
    // For each process
    std::vector<int> local_frontier; // current frontier for each process
    std::vector<int> next_local_frontier; // next frontier for each process
    // We would also need to maintain a list/vector of neighbour_nodes added (along with with neighbour they are) so that the final maze matrix can be updated before next loop
    std::unordered_map<int, int> neighbour_nodes_added; // key: neighbour_node, value: node from which it was added
    

    // if rank is 0, add the start node to the frontier
    if (rank == 0){
        // Get random start node
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, size * size - 1);
        start = dis(gen);
        
        global_frontier.push_back(start);
    }

    // Broadcast the size of the global frontier
    int global_frontier_size;
    if (rank == 0) {
        global_frontier_size = global_frontier.size();
    }
    MPI_Bcast(&global_frontier_size, 1, MPI_INT, 0, comm);

    // Resize the global_frontier vector on non-root processes
    if (rank != 0) {
        global_frontier.resize(global_frontier_size);
    }

    // Broadcast the global frontier data
    MPI_Bcast(global_frontier.data(), global_frontier_size, MPI_INT, 0, comm);

    // & debugging statments -> remove later
    //printf("Rank: %d, Start: %d\n", rank, global_frontier[0]);

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
        next_local_frontier.clear();

        // Split the global frontier into local frontiers per proc
        int local_frontier_size = global_frontier.size() / commSize + (rank < global_frontier.size() % commSize ? 1 : 0);
        for (int i = 0; i < local_frontier_size; i++){
            local_frontier.push_back(global_frontier[i + rank * local_frontier_size]);
        }

        // For each node in the local frontier, add the neighbors to the next local frontier if they are not visited
        // Set them as visited for current proc (we dont want to check the nodes again in case they are a neighbour of another node in the local frontier)
        
        // printing visited before local frontier update
        if (rank == 0){
            printf("Matrix BEFORE update of LOCAL FRONTIER, rank: %d, loop: %d\n", rank, loop_iter);
            print_maze_visual(maze, size);
            print_visited(maze, size);
        }
        for (int node : local_frontier) {
            //printf("Rank: %d, Loop: %d, Node: %d\n", rank, loop_iter, node);

            int neighbour_node;
            // Add the neighbors to the next local frontier if they are not visited
            if ((neighbour_node = LEFT_NODE(node, size)) != -1 && !IS_VISITED(maze[neighbour_node])) {
                // print is visited
                // printf() IS_VISITED(maze[neighbour_node])
                printf("IS_VISITED(maze[neighbour_node]): %d\n", IS_VISITED(maze[neighbour_node]));
                next_local_frontier.push_back(neighbour_node);
                neighbour_nodes_added[neighbour_node] = node;
                //print the neigbour node bits before update
                printf("left node not visited yet");
                printf("Rank: %d, Loop: %d, Node: %d, Neighbour: %d, Neighbour bits: %x\n", rank, loop_iter, node, neighbour_node, maze[neighbour_node] & 0xFF);
                SET_VISITED(maze[neighbour_node]);
                // print the neighbour node bits after update
                printf("Rank: %d, Loop: %d, Node: %d, Neighbour: %d, Neighbour bits: %x\n", rank, loop_iter, node, neighbour_node, maze[neighbour_node] & 0xFF);
                
            }
            if ((neighbour_node = RIGHT_NODE(node, size)) != -1 && !IS_VISITED(maze[neighbour_node])) {
                printf("IS_VISITED(maze[neighbour_node]): %d\n", IS_VISITED(maze[neighbour_node]));
                next_local_frontier.push_back(neighbour_node);
                neighbour_nodes_added[neighbour_node] = node;
                SET_VISITED(maze[neighbour_node]);
            }
            if ((neighbour_node = UP_NODE(node, size)) != -1 && !IS_VISITED(maze[neighbour_node])) {
                printf("IS_VISITED(maze[neighbour_node]): %d\n", IS_VISITED(maze[neighbour_node]));
                next_local_frontier.push_back(neighbour_node);
                neighbour_nodes_added[neighbour_node] = node;
                SET_VISITED(maze[neighbour_node]);
            }
            if ((neighbour_node = DOWN_NODE(node, size)) != -1 && !IS_VISITED(maze[neighbour_node])) {
                printf("IS_VISITED(maze[neighbour_node]): %d\n", IS_VISITED(maze[neighbour_node]));
                next_local_frontier.push_back(neighbour_node);
                neighbour_nodes_added[neighbour_node] = node;
                SET_VISITED(maze[neighbour_node]);
            }
        }
        // print size of next local frontier
        printf("Rank: %d, Loop: %d, next_local_frontier size: %d\n", rank, loop_iter, next_local_frontier.size());


        // Clear the global frontier
        global_frontier.clear();

        // Next part is to merge the next local frontiers of each proc into the global frontier

        // // do reduction to merge all the next local frontiers of each proc into the global frontier
        // MPI_Allreduce(next_local_frontier.data(), global_frontier.data(), next_local_frontier.size(), MPI_INT, MPI_SUM, comm);

        // Do reduction to merge all next local frontiers of each proc into the global frontier - do this in proc 0
        // Then shuffle the global frontier, set visited bit and then broadcast
        //! For now, we're just sending all info directly to proc 0 and then manually creating the global_frontier (no standard MPI merge function) 
        //! This is ok for now because there are only going to be 4 processes   
        if (rank == 0) {
            //printf("Rank: %d, Loop: %d\n", rank, loop_iter);
            // Make a set instead of vector (to avoid duplicates)
            std::set<int> global_frontier_temp;

            // Insert the next local frontier of proc 0
            global_frontier_temp.insert(next_local_frontier.begin(), next_local_frontier.end());
            
            // Receive the next local frontiers of other processes and insert them into the set
            for (int i = 1; i < commSize; i++) {
                int temp_size;
                MPI_Recv(&temp_size, 1, MPI_INT, i, 0, comm, MPI_STATUS_IGNORE);
                std::vector<int> temp(temp_size);
                MPI_Recv(temp.data(), temp_size, MPI_INT, i, 0, comm, MPI_STATUS_IGNORE);
                global_frontier_temp.insert(temp.begin(), temp.end());
            }
            // Convert the set to vector
            global_frontier.assign(global_frontier_temp.begin(), global_frontier_temp.end());
            std::random_shuffle(global_frontier.begin(), global_frontier.end());
        } else {
            int next_local_frontier_size = next_local_frontier.size();
            MPI_Send(&next_local_frontier_size, 1, MPI_INT, 0, 0, comm);
            MPI_Send(next_local_frontier.data(), next_local_frontier_size, MPI_INT, 0, 0, comm);
        }

        // Broadcast the size of the global_frontier
        int global_frontier_size;
        if (rank == 0) {
            global_frontier_size = global_frontier.size();
            //printf("Rank: %d, global_frontier_size: %d\n", rank, global_frontier_size);
        }

        MPI_Bcast(&global_frontier_size, 1, MPI_INT, 0, comm);

        // MPI sync
        MPI_Barrier(comm);

        // Resize the global_frontier vector on non-root processes
        if (rank != 0) {
            global_frontier.resize(global_frontier_size);
        }

        // Broadcast the global_frontier data
        //printf("Broadcasting global_frontier\n");
        MPI_Bcast(global_frontier.data(), global_frontier_size, MPI_INT, 0, comm);

        //& debugging statments -> remove later
        //printf("Rank: %d, start after iteration: %d\n", rank, global_frontier[0]);

        // Now we need to get the neighbour_nodes_added from all the processes and update the maze for each of them
        // We can do this by sending the neighbour_nodes_added from each process to process 0 and Broadcasting it to all processes
        // Then each process can update the maze accordingly
        // We would probably need to create a new MPI data type for the unordered_map
        //! CHECK IF THE BELOW PART IS PROPERLY CREATED
        // Define a struct that has standard layout
        struct Pair {
            int first;
            int second;

            bool operator<(const Pair& other) const {
                return (first < other.first) || (first == other.first && second < other.second);
            }
        };

        // Create a custom MPI datatype for Pair
        MPI_Datatype MPI_PAIR;
        int blocklengths[] = {1, 1};
        MPI_Aint displacements[] = {offsetof(Pair, first), offsetof(Pair, second)};
        MPI_Datatype types[] = {MPI_INT, MPI_INT};
        MPI_Type_create_struct(2, blocklengths, displacements, types, &MPI_PAIR);
        MPI_Type_commit(&MPI_PAIR);

        // Convert the std::pair<int, int> to Pair
        std::vector<Pair> neighbour_nodes(neighbour_nodes_added.size());
        std::transform(neighbour_nodes_added.begin(), neighbour_nodes_added.end(), neighbour_nodes.begin(),
                    [](const std::pair<int, int>& p) { return Pair{p.first, p.second}; });

        if (rank == 0) {
            std::set<Pair> neighbour_nodes_added_global;
            std::set<int> neighbour_nodes_seen;

            // Before inserting to global, check if the neighbour_nodes are already seen
            // do that by checking if the first element of the pair is already in the set
            for (auto it = neighbour_nodes.begin(); it != neighbour_nodes.end(); it++) {
                if (neighbour_nodes_seen.find(it->first) == neighbour_nodes_seen.end()) {
                    neighbour_nodes_added_global.insert(*it);
                    neighbour_nodes_seen.insert(it->first);
                }
            }
            // // Insert the neighbour_nodes of proc 0
            // neighbour_nodes_added_global.insert(neighbour_nodes.begin(), neighbour_nodes.end());
            // // insert into nodes seen the first element of all pairs
            // neighbour_nodes_seen.insert(neighbour_nodes.begin(), neighbour_nodes.end());

            // Receive the neighbour_nodes of other processes and insert them into the set
            for (int i = 1; i < commSize; i++) {
                int temp_size;
                MPI_Recv(&temp_size, 1, MPI_INT, i, 0, comm, MPI_STATUS_IGNORE);
                std::vector<Pair> temp(temp_size);
                MPI_Recv(temp.data(), temp_size, MPI_PAIR, i, 0, comm, MPI_STATUS_IGNORE);
                // similar to before, check if the first element of the pair is already in the set
                for (auto it = temp.begin(); it != temp.end(); it++) {
                    if (neighbour_nodes_seen.find(it->first) == neighbour_nodes_seen.end()) {
                        neighbour_nodes_added_global.insert(*it);
                        neighbour_nodes_seen.insert(it->first);
                    }
                }

                // neighbour_nodes_added_global.insert(temp.begin(), temp.end());
            }

            // Convert the set to vector
            neighbour_nodes.assign(neighbour_nodes_added_global.begin(), neighbour_nodes_added_global.end());
        } else {
            int neighbour_nodes_size = neighbour_nodes.size();
            MPI_Send(&neighbour_nodes_size, 1, MPI_INT, 0, 0, comm);
            MPI_Send(neighbour_nodes.data(), neighbour_nodes_size, MPI_PAIR, 0, 0, comm);
        }

        // Broadcast the size of the neighbour_nodes
        int neighbour_nodes_size;
        if (rank == 0) {
            neighbour_nodes_size = neighbour_nodes.size();
        }

        MPI_Bcast(&neighbour_nodes_size, 1, MPI_INT, 0, comm);

        // Resize the neighbour_nodes vector on non-root processes
        if (rank != 0) {
            neighbour_nodes.resize(neighbour_nodes_size);
        }

        // Broadcast the neighbour_nodes data
        MPI_Bcast(neighbour_nodes.data(), neighbour_nodes_size, MPI_PAIR, 0, comm);

        // Free the custom MPI datatype
        MPI_Type_free(&MPI_PAIR);
        
        // before edges update
        if (rank == 0){
            printf("Matrix BEFORE update of edges, rank: %d, loop: %d\n", rank, loop_iter);
            print_maze_visual(maze, size);
            print_visited(maze, size);

        }

        // Update the maze for each process
        for (auto it = neighbour_nodes.begin(); it != neighbour_nodes.end(); it++){
            int neighbour_node = it->first;
            int node = it->second;
            if (LEFT_NODE(node, size) == neighbour_node){
                // if the left of node is neighbour_node
                SET_LEFT(maze[node]);
                SET_RIGHT(maze[neighbour_node]);
            } else if (RIGHT_NODE(node, size) == neighbour_node){
                SET_RIGHT(maze[node]);
                SET_LEFT(maze[neighbour_node]);
            } else if (UP_NODE(node, size) == neighbour_node){
                SET_UP(maze[node]);
                SET_DOWN(maze[neighbour_node]);
            } else if (DOWN_NODE(node, size) == neighbour_node){
                SET_DOWN(maze[node]);
                SET_UP(maze[neighbour_node]);
            }
        }

        // print maze for proc 0 
        if (rank == 0){
            printf("Matrix AFTER update of edges, rank: %d, loop: %d\n", rank, loop_iter);
            print_maze_visual(maze, size);
            print_visited(maze, size);
        }

        loop_iter++;
        
        // if (loop_iter >= 10)
        //     break;
    }

    // The tree has now been generated and is stored in the maze

}