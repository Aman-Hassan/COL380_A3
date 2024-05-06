#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include "mazesolver.hpp"
#include <queue>
#include <vector>
#include <climits>

class Dijkstra : public MazeSolver {
public:
    std::vector<std::vector<char>> solveMaze(const std::vector<std::vector<char>>& maze) override;
};

#endif // DIJKSTRA_HPP
