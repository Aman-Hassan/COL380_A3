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
void generateTreeUsingKruskal(char *maze, int size, MPI_Comm comm) {
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

    // Braodcast parent and tree_height to all processes
    MPI_Bcast(parent.data(), n, MPI_INT, 0, comm);
    MPI_Bcast(tree_height.data(), n, MPI_INT, 0, comm);

    // Create a list of edges (node pairs) and their weights
    std::vector<std::tuple<int, int, int>> edges; // This is local to each process -> contains the edges that are connected to the nodes in the process 
    //TODO: The above edges vector could probably be somehow replaced by our already exisiting graph sturcture -> the top 8 bits out of the 16 bits represents the "weight" of a node
    
    for (int i = 0; i < n; i++) {
        if (GET_LEFT(maze[i])) {
            int weight = std::max(IS_C(maze[i]), IS_C(maze[LEFT_NODE(i, size)])); // The weight of the edge is the maximum of the two nodes connected by the edge
            edges.emplace_back(i, LEFT_NODE(i, size), weight);
        }
        if (GET_RIGHT(maze[i])) {
            int weight = std::max(IS_C(maze[i]), IS_C(maze[RIGHT_NODE(i, size)]));
            edges.emplace_back(i, RIGHT_NODE(i, size), weight);
        }
        if (GET_UP(maze[i])) {
            int weight = std::max(IS_C(maze[i]), IS_C(maze[UP_NODE(i, size)]));
            edges.emplace_back(i, UP_NODE(i, size), weight);
        }
        if (GET_DOWN(maze[i])) {
            int weight = std::max(IS_C(maze[i]), IS_C(maze[DOWN_NODE(i, size)]));
            edges.emplace_back(i, DOWN_NODE(i, size), weight);
        }
    }

    // Sort edges by weight
    std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return std::get<2>(a) < std::get<2>(b);
    });

    // Scatter edges to all processes
    int edgeCount = edges.size();
    int *edgeBuffer = nullptr;
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
    MPI_Scatter(edgeBuffer, edgeCount / commSize * 3, MPI_INT, MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, 0, comm);
    if (rank == 0) {
        localEdgeCount = edgeCount / commSize * 3;
        localEdges = edgeBuffer;
    } else {
        MPI_Scatter(nullptr, 0, MPI_DATATYPE_NULL, MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, 0, comm);
        localEdgeCount = edgeCount / commSize * 3;
        localEdges = new int[localEdgeCount];
        MPI_Scatter(edgeBuffer, localEdgeCount, MPI_INT, localEdges, localEdgeCount, MPI_INT, 0, comm);
    }

    // Kruskal's algorithm (parallel)
    std::unordered_set<int> mstNodes;
    for (int i = 0; i < localEdgeCount; i += 3) {
        int u = localEdges[i];
        int v = localEdges[i + 1];
        if (find(u) != find(v)) {
            merge(u, v);
            mstNodes.insert(u);
            mstNodes.insert(v);
        }
    }

    // Gather MST nodes from all processes
    int *mstNodeBuffer = nullptr;
    int mstNodeCount;
    MPI_Gather(&mstNodes.size(), 1, MPI_INT, MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, 0, comm);
    if (rank == 0) {
        mstNodeCount = 0;
        for (int i = 0; i < commSize; i++) {
            int tmp;
            MPI_Gather(&mstNodes.size(), 1, MPI_INT, &tmp, 1, MPI_INT, i, comm);
            mstNodeCount += tmp;
        }
        mstNodeBuffer = new int[mstNodeCount];
    }

    std::vector<int> localMstNodes(mstNodes.begin(), mstNodes.end());
    MPI_Gatherv(localMstNodes.data(), localMstNodes.size(), MPI_INT, mstNodeBuffer, nullptr, nullptr, MPI_INT, 0, comm);

    // Update the maze with MST nodes
    if (rank == 0) {
        for (int i = 0; i < mstNodeCount; i++) {
            SET_C(maze[mstNodeBuffer[i]]);
        }
        delete[] mstNodeBuffer;
    }

    if (rank != 0) {
        delete[] localEdges;
    } else {
        delete[] edgeBuffer;
    }
}