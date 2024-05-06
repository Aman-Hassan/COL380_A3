#ifndef BFS_HPP
#define BFS_HPP

#include "mazegenerator.hpp"
#include <queue>
#include <set>

class BFS : public MazeGenerator {
public:
    std::vector<std::vector<char>> generateMaze(int rows, int cols) override;
private:
    bool isValid(int x, int y, int rows, int cols);
};

#endif // BFS_HPP
