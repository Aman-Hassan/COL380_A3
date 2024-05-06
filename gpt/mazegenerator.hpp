#ifndef MAZEGENERATOR_HPP
#define MAZEGENERATOR_HPP

#include <vector>
#include <cstdlib>

class MazeGenerator {
public:
    virtual std::vector<std::vector<char>> generateMaze(int rows, int cols) = 0;
};

#endif // MAZEGENERATOR_HPP
