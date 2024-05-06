#include "kruskal.hpp"
#include <algorithm>
#include <mpi.h>

class DisjointSet {
    std::vector<int> parent;
public:
    DisjointSet(int n) {
        parent.resize(n);
        for (int i = 0; i < n; ++i)
            parent[i] = i;
    }

    int find(int u) {
        if (parent[u] == u) return u;
        return parent[u] = find(parent[u]);
    }

    void merge(int u, int v) {
        parent[find(u)] = find(v);
    }

    bool isConnected(int u, int v) {
        return find(u) == find(v);
    }
};

std::vector<std::vector<char>> Kruskal::generateMaze(int rows, int cols) {
    std::vector<std::vector<char>> maze(rows, std::vector<char>(cols, '*'));

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rowsPerProcess = rows / size;
    int startRow = rank * rowsPerProcess;
    int endRow = startRow + rowsPerProcess;
    if (rank == size - 1) endRow = rows;

    // Initialize edges
    std::vector<std::pair<int, std::pair<int, int>>> edges;
    for (int i = startRow; i < endRow; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (i > 0) edges.push_back({rand(), {i * cols + j, (i - 1) * cols + j}}); // Up
            if (j > 0) edges.push_back({rand(), {i * cols + j, i * cols + (j - 1)}}); // Left
        }
    }

    // Gather all edges
    std::vector<std::vector<std::pair<int, std::pair<int, int>>>> allEdges(size);
    MPI_Allgather(&edges[0], edges.size() * sizeof(std::pair<int, std::pair<int, int>>), MPI_BYTE, &allEdges[0], edges.size() * sizeof(std::pair<int, std::pair<int, int>>), MPI_BYTE, MPI_COMM_WORLD);

    // Shuffle edges
    if (rank == 0) {
        std::vector<std::pair<int, std::pair<int, int>>> allEdgesCombined;
        for (const auto& edgeList : allEdges) {
            allEdgesCombined.insert(allEdgesCombined.end(), edgeList.begin(), edgeList.end());
        }
        std::random_shuffle(allEdgesCombined.begin(), allEdgesCombined.end());

        // Perform Kruskal's algorithm
        DisjointSet ds(rows * cols);
        for (auto edge : allEdgesCombined) {
            int u = edge.second.first, v = edge.second.second;
            int row1 = u / cols, col1 = u % cols;
            int row2 = v / cols, col2 = v % cols;

            if (!ds.isConnected(u, v)) {
                maze[row1][col1] = maze[row2][col2] = ' ';
                ds.merge(u, v);
            }
        }

        maze[0][cols - 1] = 'S'; // Entry
        maze[rows - 1][0] = 'E'; // Exit
    }

    // Broadcast maze to all processes
    for (int i = 0; i < rows; ++i) {
        MPI_Bcast(&maze[i][0], cols, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    return maze;
}
