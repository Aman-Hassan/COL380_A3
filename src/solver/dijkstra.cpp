#include <mpi.h>
#include <vector>
#include <queue>
#include <set>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <cstddef>

#include "dijkstra.hpp"

void solveUsingDijkstra(int size, short* maze, MPI_Comm comm, int start, int end){
    // 
    // printf("Solving using Dijkstra\n");
    // Get the rank and size of the communicator
    int rank, commSize;
    int found_rank;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &commSize);

    // define the frontier and visited set
    std::vector<int> global_frontier;

    // For each process
    std::vector<int> local_frontier; // current frontier for each process
    std::vector<int> next_local_frontier; // next frontier for each process
    // We would also need to maintain a list/vector of local_neighbours_paired added (along with with neighbour they are) so that the final maze matrix can be updated before next loop
    std::unordered_map<int, int> local_neighbours; // key: neighbour_node, value: node from which it was added
    std::unordered_map<int, int> parent_map;


    // if rank is 0, add the start node to the frontier
    if (rank == 0){
        global_frontier.push_back(start);
    }

    // Broadcast the size of the global frontier
    int global_frontier_size;
    if (rank == 0)
        global_frontier_size = global_frontier.size();
    MPI_Bcast(&global_frontier_size, 1, MPI_INT, 0, comm);

    // Resize the global_frontier vector on non-root processes
    if (rank != 0) {
        global_frontier.resize(global_frontier_size);
    }

    // Broadcast the global frontier data
    MPI_Bcast(global_frontier.data(), global_frontier_size, MPI_INT, 0, comm);
    bool found = false;
    while (true){
        

        // printing the tree generated step by step
        // if (rank == 0){
        //     print_maze_visual(maze, size);
        //     printf("\n");
        // }
        
        // If the global frontier is empty, break
        if (global_frontier.size() == 0){
            break;
        }

        // Set the visted bit of the nodes in the global frontier
        for (int node : global_frontier){
            SET_VISITED_SOLVE(maze[node]);
        }
        
        if (IS_VISITED_SOLVE(maze[end])){
            found = true;
            break;
        }

        // Clear the next local frontier
        local_frontier.clear();
        next_local_frontier.clear();

        // Split the global frontier into local frontiers per proc
        int local_frontier_size = global_frontier.size() / commSize + (rank < (int)(global_frontier.size() % commSize) ? 1 : 0);
        for (int i = 0; i < local_frontier_size; i++){
            local_frontier.push_back(global_frontier[i + rank * local_frontier_size]);
        }

        // For each node in the local frontier, add the neighbors to the next local frontier if they are not visited
        // Set them as visited for current proc (we dont want to check the nodes again in case they are a neighbour of another node in the local frontier)

        for (int parent : local_frontier) {

            int child;
            // Add the neighbors to the next local frontier if they are not visited
            if ((child = LEFT_NODE(parent, size)) != -1 && !IS_VISITED_SOLVE(maze[child])) {
                if (child == end) found_rank = rank;
                next_local_frontier.push_back(child);
                local_neighbours[child] = parent;
                SET_VISITED_SOLVE(maze[child]);
            }
            if ((child = RIGHT_NODE(parent, size)) != -1 && !IS_VISITED_SOLVE(maze[child])) {
                if (child == end) found_rank = rank;
                next_local_frontier.push_back(child);
                local_neighbours[child] = parent;
                SET_VISITED_SOLVE(maze[child]);
            }
            if ((child = UP_NODE(parent, size)) != -1 && !IS_VISITED_SOLVE(maze[child])) {
                if (child == end) found_rank = rank;
                next_local_frontier.push_back(child);
                local_neighbours[child] = parent;
                SET_VISITED_SOLVE(maze[child]);
            }
            if ((child = DOWN_NODE(parent, size)) != -1 && !IS_VISITED_SOLVE(maze[child])) {
                if (child == end) found_rank = rank;
                next_local_frontier.push_back(child);
                local_neighbours[child] = parent;
                SET_VISITED_SOLVE(maze[child]);
            }
        }

        // Clear the global frontier
        global_frontier.clear();

        // Next part is to merge the next local frontiers of each proc into the global frontier
        if (rank == 0) {
            // Make a set instead of vector (to avoid duplicates)
            std::set<int> global_frontier_temp;

            // Insert the next local frontier of proc 0
            global_frontier_temp.insert(next_local_frontier.begin(), next_local_frontier.end());
            
            // Receive the next local frontiers of other processes and insert them into the set
            for (int i = 1; i < commSize; i++) {

            // receive the size of the message to receive next
            int temp_size;
            MPI_Recv(&temp_size, 1, MPI_INT, i, 0, comm, MPI_STATUS_IGNORE);
            
            // receive the next_local_frontier of each process
            std::vector<int> temp(temp_size);
            MPI_Recv(temp.data(), temp_size, MPI_INT, i, 0, comm, MPI_STATUS_IGNORE);

            // insert to global_frontier_temp set to avoid duplicates
            global_frontier_temp.insert(temp.begin(), temp.end());
            }

            // Convert the set to vector
            global_frontier.assign(global_frontier_temp.begin(), global_frontier_temp.end());
            std::random_shuffle(global_frontier.begin(), global_frontier.end());

            // Combine and broadcast the local_neighbours hashmap
            // std::unordered_map<int, int> combined_parents;
            // combined_parents.insert(local_neighbours.begin(), local_neighbours.end());
            // MPI_Bcast(&combined_parents, 1, MPI_INT, 0, comm);

        } else {
            // for other processes
            int next_local_frontier_size = next_local_frontier.size();
            // send size of message
            MPI_Send(&next_local_frontier_size, 1, MPI_INT, 0, 0, comm);
            // send the next_local_frontier
            MPI_Send(next_local_frontier.data(), next_local_frontier_size, MPI_INT, 0, 0, comm);

            // send the local_neighbours hashmap
            // MPI_Bcast(&local_neighbours, 1, MPI_INT, 0, comm);
        }

        // Broadcast the size of the global_frontier
        int global_frontier_size;
        if (rank == 0) {
            global_frontier_size = global_frontier.size();
        }

        // Broadcast the size of the global_frontier
        MPI_Bcast(&global_frontier_size, 1, MPI_INT, 0, comm);

        // Resize the global_frontier vector on non-root processes
        if (rank != 0) {
            global_frontier.resize(global_frontier_size);
        }

        // Broadcast the global_frontier data
        MPI_Bcast(global_frontier.data(), global_frontier_size, MPI_INT, 0, comm);
    

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

        // local_neighbours - hashmap of child : parent, added by each process
        // local_neighbours_paired - vector of pairs of child : parent, to be sent to process 0
        // global_neighbours - final set of all neighbours to be broadcasted

        std::vector<Pair> global_neighbours;

        // Convert the std::pair<int, int> from the hashmap to Pair
        std::vector<Pair> local_neighbours_paired(local_neighbours.size());
        std::transform(local_neighbours.begin(), local_neighbours.end(), local_neighbours_paired.begin(),
                    [](const std::pair<int, int>& p) { return Pair{p.first, p.second}; });

        if (rank == 0) {
            // temporary one to hold all the neighbours from all processes, to 
            std::set<Pair> global_neighbours_temp;
            // neighbours_seen - set of all neighbours added by all processes (to avoid duplicates)
            std::set<int> neighbours_seen;

            // just for proc 0
            for (auto it = local_neighbours_paired.begin(); it != local_neighbours_paired.end(); it++) {
                if (neighbours_seen.find(it->first) == neighbours_seen.end()) {
                    // checking if the first element of the pair (the child) is already in the set
                    // if not seen, insert into the set and the global_neighbours_temp
                    global_neighbours_temp.insert(*it);
                    neighbours_seen.insert(it->first);
                    // We do this checking to prevent cycles in the maze, where different parents from different processes try to add the same child
                }
            }

            // Receive the local_neighbours_paired of other processes and insert them into the set
            for (int i = 1; i < commSize; i++) {
                int temp_size;
                MPI_Recv(&temp_size, 1, MPI_INT, i, 0, comm, MPI_STATUS_IGNORE);
                std::vector<Pair> temp(temp_size);
                MPI_Recv(temp.data(), temp_size, MPI_PAIR, i, 0, comm, MPI_STATUS_IGNORE);

                // similar to before, check if the first element of the pair is already in the set
                for (auto it = temp.begin(); it != temp.end(); it++) {
                    if (neighbours_seen.find(it->first) == neighbours_seen.end()) {
                        global_neighbours_temp.insert(*it);
                        neighbours_seen.insert(it->first);
                    }
                }
            }

            // Moving the values of the temp set to the main global_neighbours vector
            global_neighbours.assign(global_neighbours_temp.begin(), global_neighbours_temp.end());

        } else {
            int global_neighbours_size = local_neighbours_paired.size();
            MPI_Send(&global_neighbours_size, 1, MPI_INT, 0, 0, comm);
            MPI_Send(local_neighbours_paired.data(), global_neighbours_size, MPI_PAIR, 0, 0, comm);
        }

        // Broadcast the size of the local_neighbours_paired
        int global_neighbours_size;
        if (rank == 0) {
            global_neighbours_size = global_neighbours.size();
        }

        MPI_Bcast(&global_neighbours_size, 1, MPI_INT, 0, comm);

        // Resize the local_neighbours_paired vector on non-root processes
        if (rank != 0) {
            global_neighbours.resize(global_neighbours_size);
        }

        // Broadcast the local_neighbours_paired data
        MPI_Bcast(global_neighbours.data(), global_neighbours_size, MPI_PAIR, 0, comm);

        // Free the custom MPI datatype
        MPI_Type_free(&MPI_PAIR);

    
        // Add global_neighbours pairs to parent_map if it is not already present
        for (auto it = global_neighbours.begin(); it != global_neighbours.end(); it++) {
            if (parent_map.find(it->first) == parent_map.end()) {
                parent_map[it->first] = it->second;
            }
        }

    }
    

    if (rank == found_rank){
        // If found, update the PATH flags in maze
        int end_node = end;
        while (end_node != start){
            SET_P(maze[end_node]);
            end_node = parent_map[end_node];
        }

        // send found_rank to rank 0
        // if 0 found it, no need to send
        if (rank != 0)
            MPI_Send(&found_rank, 1, MPI_INT, 0, 0, comm);
    }

    if (rank == 0 && found_rank != 0){
        MPI_Status status;
        MPI_Recv(&found_rank, 1, MPI_INT, MPI_ANY_SOURCE, 0, comm, &status);
    }

    // Broadcast the found_rank from 0
    MPI_Bcast(&found_rank, 1, MPI_INT, 0, comm);
   
    // broadcast the maze from the proc that found the end
    MPI_Bcast(maze, size * size, MPI_SHORT, found_rank, comm);
    // printf("Rank %d finished\n", rank);

}