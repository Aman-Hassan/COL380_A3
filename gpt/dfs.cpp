#include "dfs.hpp"
#include <mpi.h>
#include <stack>
#include <iostream>

std::vector<std::vector<char>> DFS::solveMaze(const std::vector<std::vector<char>>& maze) {
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

    std::stack<std::pair<int, int>> stk;
    std::set<std::pair<int, int>> visited;

    // Start from entry point
    stk.push({startRow, cols - 1});
    visited.insert({startRow, cols - 1});

    // DFS traversal
    while (!stk.empty()) {
        auto [x, y] = stk.top();
        stk.pop();

        // Check if exit reached
        if (x == rows - 1 && y == 0) break;

        // Randomize directions
        std::vector<int> directions = {0, 1, 2, 3};
        std::random_shuffle(directions.begin(), directions.end());

        for (int dir : directions) {
            int newX = x, newY = y;
            if (dir == 0) newX--; // Up
            else if (dir == 1) newY++; // Right
            else if (dir == 2) newX++; // Down
            else newY--; // Left

            if (isValidMove(newX, newY, rows, cols) && visited.find({newX, newY}) == visited.end()) {
                stk.push({newX, newY});
                visited.insert({newX, newY});
            }
        }
    }

    // Mark solution path
    while (!stk.empty()) {
        auto [x, y] = stk.top();
        stk.pop();
        solution[x][y] = 'P';
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

bool DFS::isValidMove(int x, int y, int rows, int cols) {
    return (x >= 0 && x < rows && y >= 0 && y < cols && maze[x][y] == ' ');
}
