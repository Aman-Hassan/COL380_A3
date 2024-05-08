#include <mpi.h>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <random>
#include "defs.hpp"
#include "kruskal.hpp"

// Union-Find data structure
std::vector<int> parent; //Stores the parent of each node in the tree (the disjoint set) -> //! Is it possible to not use this and do something more optmized with the graph structure we currently have (i.e. using our macros?)
std::vector<int> tree_height; // Stores the rank of each node in the tree (NOTE: This is not the process rank of mpi, rank here denotes the height of the tree rooted at the node) -> //! Is it possible to not use this and do something more optmized with the graph structure we currently have (i.e. using our macros?

// Find the root of a node (with path compression) -> works by recursively finding the parent of the node and then setting the parent of the node to the root
int find(int x) {
    if (parent[x] == x) return x;
    return parent[x] = find(parent[x]);
}

// Union two sets -> 
/* For disjoint sets, we can merge two sets by making the root of one set the child of the root of the other set. 
- This is done by comparing the heights of the trees rooted at the two sets. 
- If the heights are equal, we can make one the child of the other and increment the height of the root. 
- If the height of one is less than the other, we can make the root of the shorter tree the child of the root of the taller tree. 
- If the height of one is greater than the other, we can make the root of the taller tree the child of the root of the shorter tree.
*/
void merge(int x, int y) {
    int xroot = find(x);
    int yroot = find(y);
    if (xroot == yroot) return;
    if (tree_height[xroot] < tree_height[yroot]) {
        parent[xroot] = yroot;
    } else if (tree_height[xroot] > tree_height[yroot]) {
        parent[yroot] = xroot;
    } else {
        parent[yroot] = xroot;
        tree_height[xroot]++;
    }
}

// Generating the Tree using the Kruskal Algorithm
void generateTreeUsingKruskal(int size, short *maze, MPI_Comm comm) {
    int rank, commSize;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &commSize);

    // Initialize parent and tree_height vectors - only in process 0
    int n = size * size;
    if (rank == 0) {
        parent.resize(n);
        tree_height.resize(n);
        for (int i = 0; i < n; i++) {
            parent[i] = i; // Initially, each node is the root of its own tree (i.e. each of them are its own disjoint set)
            tree_height[i] = 0; // Hence the height of each tree is 0
        }
    }
    else{
        parent.resize(n);
        tree_height.resize(n);
    }

    // Braodcast parent and tree_height to all processes
    MPI_Bcast(parent.data(), n, MPI_INT, 0, comm);
    MPI_Bcast(tree_height.data(), n, MPI_INT, 0, comm);

    // Create a list of edges (node pairs) and their weights
    std::vector<std::tuple<int, int, int>> edges; // This is local to each process -> contains the edges that are connected to the nodes in the process 
    //TODO: The above edges vector could probably be somehow replaced by our already exisiting graph sturcture -> the top 8 bits out of the 16 bits represents the "weight" of a node
    
    // Set the local edges 
    for (int i = 0; i < n; i++) {
        int node = maze[i];
        int neighbour_node;
        if ((neighbour_node = LEFT_NODE(i, size)) != -1) {
            // printf("Rank: %d, Node: %d, Neighbour: %d\n", rank, i, neighbour_node);
            int weight = GET_EDGE_WEIGHT(node, neighbour_node);
            edges.emplace_back(i, neighbour_node, weight);
        }
        if ((neighbour_node = RIGHT_NODE(i,size)) != -1) {
            // printf("Rank: %d, Node: %d, Neighbour: %d\n", rank, i, neighbour_node);
            int weight = GET_EDGE_WEIGHT(node, neighbour_node);
            edges.emplace_back(i, neighbour_node, weight);
        }
        if ((neighbour_node = UP_NODE(i, size)) != -1) {
            // printf("Rank: %d, Node: %d, Neighbour: %d\n", rank, i, neighbour_node);
            int weight =  GET_EDGE_WEIGHT(node, neighbour_node);
            edges.emplace_back(i, neighbour_node, weight);
        }
        if ((neighbour_node = DOWN_NODE(i, size)) != -1) {
            // printf("Rank: %d, Node: %d, Neighbour: %d\n", rank, i, neighbour_node);
            int weight =  GET_EDGE_WEIGHT(node, neighbour_node);
            edges.emplace_back(i, neighbour_node, weight);
        }
    }

    // if (rank == 0) {
    //     // print edges
    //     printf("Edges:\n");
    //     for (auto edge : edges) {
    //         int u = std::get<0>(edge);
    //         int v = std::get<1>(edge);
    //         int w = std::get<2>(edge);
    //         fprintf(stderr, "Edge: %d %d %d\n", u, v, w);
    //     }
    // }
    //? The below code could be optimized parallelly
    // Sort local edges by weight
    std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return std::get<2>(a) < std::get<2>(b);
    });

    // Scatter edges to all processes
    int edgeCount = edges.size(); // Total number of edges in current process
    int *edgeBuffer = nullptr; // Buffer to store all the edges in rank 0
    if (rank == 0) {
        edgeBuffer = new int[edgeCount * 3];
        for (int i = 0; i < edgeCount; i++) {
            edgeBuffer[i * 3] = std::get<0>(edges[i]);
            edgeBuffer[i * 3 + 1] = std::get<1>(edges[i]);
            edgeBuffer[i * 3 + 2] = std::get<2>(edges[i]);
        }
    }

    int *localEdges = nullptr;
    int localEdgeCount;
    if (rank == 0) {
        localEdgeCount = edgeCount;
        localEdges = edgeBuffer;
    } else {
        localEdgeCount = (edgeCount / commSize) * 3;
        int remainder = (edgeCount % commSize) * 3;
        if (rank < remainder) {
            localEdgeCount += 3;
        }
        localEdges = new int[localEdgeCount];
    }

    int *rcvCounts = new int[commSize]; // Number of elements to receive from each process
    int *disps = new int[commSize]; // Displacements for scatterv
    for (int i = 0; i < commSize; i++) { // Compute the number of elements to receive from each process
        rcvCounts[i] = (edgeCount / commSize) * 3; // Each edge has 3 elements
        if (i < (edgeCount % commSize)) { // If there are any remaining edges
            rcvCounts[i] += 3; // add 3 more elements
        }
    }

    disps[0] = 0; // Displacement for the first process
    for (int i = 1; i < commSize; i++) {  // Compute the displacements for scatterv
        disps[i] = disps[i - 1] + rcvCounts[i - 1]; // Displacement for the current process is the sum of the number of elements received from all previous processes
    }

    MPI_Scatterv(edgeBuffer, rcvCounts, disps, MPI_INT, localEdges, localEdgeCount, MPI_INT, 0, comm); // Scatter the edges to all processes

    delete[] rcvCounts;
    delete[] disps;

    MPI_Barrier(comm);

    // Kruskal's algorithm (parallel)
    std::unordered_set<int> mstNodes; // Stores the nodes in the MST
    for (int i = 0; i < localEdgeCount; i += 3) { // Iterate over the local edges
        int u = localEdges[i]; // Get the first node of the edge
        int v = localEdges[i + 1]; // Get the second node of the edge
        if (find(u) != find(v)) { // If the nodes are not in the same set
            merge(u, v); // Merge the sets
            mstNodes.insert(u); // Add the nodes to the MST
            mstNodes.insert(v);
        }
    }

    // Gather MST nodes from all processes
    std::vector<int> localMstNodes(mstNodes.begin(), mstNodes.end()); // Convert the set to a vector
    int localMstNodesSize = localMstNodes.size(); // Get the size of the local MST nodes

    // Gather the sizes of localMstNodes from all processes
    std::vector<int> mstNodesSizes(commSize); // Stores the sizes of MST nodes from all processes
    MPI_Gather(&localMstNodesSize, 1, MPI_INT, mstNodesSizes.data(), 1, MPI_INT, 0, comm); // Gather the sizes of localMstNodes from all processes

    // Compute the total size of mstNodes
    int totalMstNodesSize = 0;
    if (rank == 0) {
        for (int size : mstNodesSizes) { // Compute the total size of mstNodes
            totalMstNodesSize += size; // Add the size of each process
        }
    }

    // Broadcast the total size to all processes
    MPI_Bcast(&totalMstNodesSize, 1, MPI_INT, 0, comm); // Broadcast the total size of mstNodes to all processes

    // Resize the localMstNodes vector to the total size and gather the data
    std::vector<int> gatheredMstNodes(totalMstNodesSize);
    int *recvBuffer = gatheredMstNodes.data(); // Buffer to store the gathered MST nodes
    int *recvCounts = mstNodesSizes.data(); // Number of elements to receive from each process
    int *displs = new int[commSize]; // Displacements for gatherv
    int disp = 0;
    for (int i = 0; i < commSize; i++) {
        displs[i] = disp; // Compute the displacements for gatherv
        disp += recvCounts[i]; // Displacement for the current process is the sum of the number of elements received from all previous processes
    }

    MPI_Gatherv(localMstNodes.data(), localMstNodesSize, MPI_INT,
                recvBuffer, recvCounts, displs, MPI_INT, 0, comm); // Gather the MST nodes from all processes

    delete[] displs;

    // Update the maze with the MST nodes (rank 0)
    if (rank == 0) {
       for (int i = 0; i < n; i++){
        if (RIGHT_NODE(i,size) != -1 && mstNodes.find(i) != mstNodes.end() && mstNodes.find(RIGHT_NODE(i,size)) != mstNodes.end()){
            SET_RIGHT(maze[i]);
            SET_LEFT(maze[RIGHT_NODE(i,size)]);
        }
        if (DOWN_NODE(i,size) != -1 && mstNodes.find(i) != mstNodes.end() && mstNodes.find(DOWN_NODE(i,size)) != mstNodes.end()){
            SET_DOWN(maze[i]);
            SET_UP(maze[DOWN_NODE(i,size)]);
        }
       }
    }

    if (rank != 0) {
        delete[] localEdges;
    } else {
        delete[] edgeBuffer;
    }

    // Showcase the tree generated
    if (rank == 0) {
        print_maze_visual(maze, size);
    }
}