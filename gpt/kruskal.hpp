#ifndef KRUSKAL_HPP
#define KRUSKAL_HPP

#include "mazegenerator.hpp"
#include <vector>
#include <cstdlib>

class Kruskal : public MazeGenerator {
public:
    std::vector<std::vector<char>> generateMaze(int rows, int cols) override;
};

#endif // KRUSKAL_HPP
