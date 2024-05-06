#include "dijkstra.hpp"
#include <mpi.h>
#include <vector>
#include <queue>
#include <climits>
#include <iostream>

std::vector<std::vector<char>> Dijkstra::solveMaze(const std::vector<std::vector<char>>& maze) {
    std::vector<std::vector<char>> solution = maze;

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows = maze.size();
    int cols = maze[0].size();
    int rowsPerProcess = rows / size;
    int startRow = rank * rowsPerProcess;
    int endRow = startRow + rowsPerProcess;
    if (rank == size - 1) endRow = rows;

    // Initialize distances and visited array
    std::vector<std::vector<int>> dist(rows, std::vector<int>(cols, INT_MAX));
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::priority_queue<std::pair<int, std::pair<int, int>>, std::vector<std::pair<int, std::pair<int, int>>>, std::greater<std::pair<int, std::pair<int, int>>>> pq;

    // Start from entry point
    pq.push({0, {startRow, cols - 1}});
    dist[startRow][cols - 1] = 0;

    // Dijkstra's algorithm
    while (!pq.empty()) {
        auto [d, pos] = pq.top();
        pq.pop();
        int x = pos.first, y = pos.second;

        if (visited[x][y]) continue;
        visited[x][y] = true;

        // Randomize directions
        std::vector<int> directions = {0, 1, 2, 3};
        std::random_shuffle(directions.begin(), directions.end());

        for (int dir : directions) {
            int newX = x, newY = y;
            if (dir == 0) newX--; // Up
            else if (dir == 1) newY++; // Right
            else if (dir == 2) newX++; // Down
            else newY--; // Left

            if (isValidMove(newX, newY, rows, cols) && !visited[newX][newY] && dist[newX][newY] > dist[x][y] + 1) {
                dist[newX][newY] = dist[x][y] + 1;
                pq.push({dist[newX][newY], {newX, newY}});
            }
        }
    }

    // Mark solution path
    for (int i = startRow; i < endRow; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (visited[i][j])
                solution[i][j] = 'P';
        }
    }

    // Gather solution from all processes
    std::vector<std::vector<char>> allSolution(rows, std::vector<char>(cols, ' '));
    MPI_Gather(&solution[startRow][0], rowsPerProcess * cols, MPI_CHAR, &allSolution[0][0], rowsPerProcess * cols, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        allSolution[0][cols - 1] = 'S'; // Entry
        allSolution[rows - 1][0] = 'E'; // Exit
    }

    // Error handling
    int error = MPI_SUCCESS;
    if (MPI_SUCCESS != (error = MPI_Barrier(MPI_COMM_WORLD))) {
        std::cerr << "MPI_Barrier error on rank " << rank << ": " << error << std::endl;
        MPI_Abort(MPI_COMM_WORLD, error);
    }

    return allSolution;
}

bool Dijkstra::isValidMove(int x, int y, int rows, int cols) {
    return (x >= 0 && x < rows && y >= 0 && y < cols && maze[x][y] == ' ');
}
