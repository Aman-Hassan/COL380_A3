#include "bfs.hpp"
#include <mpi.h>
#include <vector>
#include <queue>
#include <set>

std::vector<std::vector<char>> BFS::generateMaze(int rows, int cols) {
    std::vector<std::vector<char>> maze(rows, std::vector<char>(cols, ' '));
    std::queue<std::pair<int, int>> q;
    std::set<std::pair<int, int>> visited;

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rowsPerProcess = rows / size;
    int startRow = rank * rowsPerProcess;
    int endRow = startRow + rowsPerProcess;
    if (rank == size - 1) endRow = rows;

    // Start from top-right corner
    int startCol = cols - 1;
    if (rank == 0) q.push({0, cols - 1});
    visited.insert({0, cols - 1});

    // Perform BFS to generate maze
    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        // Randomize directions
        std::vector<int> directions = {0, 1, 2, 3};
        std::random_shuffle(directions.begin(), directions.end());

        for (int dir : directions) {
            int newX = x, newY = y;
            if (dir == 0) newX--; // Up
            else if (dir == 1) newY++; // Right
            else if (dir == 2) newX++; // Down
            else newY--; // Left

            if (isValid(newX, newY, rows, cols) && visited.find({newX, newY}) == visited.end()) {
                if (newX >= startRow && newX < endRow) {
                    maze[newX][newY] = '*'; // Carve passage
                    q.push({newX, newY});
                }
                visited.insert({newX, newY});
            }
        }
    }

    // Gather maze from all processes
    std::vector<std::vector<char>> allMaze(rows, std::vector<char>(cols, ' '));
    MPI_Allgather(&maze[startRow][0], rowsPerProcess * cols, MPI_CHAR, &allMaze[0][0], rowsPerProcess * cols, MPI_CHAR, MPI_COMM_WORLD);

    if (rank == 0) {
        allMaze[0][cols - 1] = 'S'; // Entry
        allMaze[rows - 1][0] = 'E'; // Exit
    }

    return allMaze;
}

bool BFS::isValid(int x, int y, int rows, int cols) {
    return (x >= 0 && x < rows && y >= 0 && y < cols);
}
