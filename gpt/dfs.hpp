#ifndef DFS_HPP
#define DFS_HPP

#include "mazesolver.hpp"
#include <stack>

class DFS : public MazeSolver {
public:
    std::vector<std::vector<char>> solveMaze(const std::vector<std::vector<char>>& maze) override;
private:
    bool isValidMove(int x, int y, int rows, int cols);
};

#endif // DFS_HPP
