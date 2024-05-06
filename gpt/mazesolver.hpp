#ifndef MAZESOLVER_HPP
#define MAZESOLVER_HPP

#include <vector>

class MazeSolver {
public:
    virtual std::vector<std::vector<char>> solveMaze(const std::vector<std::vector<char>>& maze) = 0;
};

#endif // MAZESOLVER_HPP
