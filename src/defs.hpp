#ifndef DEFS_H
#define DEFS_H

// - Nodes of the maze are represented by an integer, 64*row + col
// - We make macros to access row no. and col no., neighbors, etc.
// - make sure that the neighbors macros give valid output, i.e. they don't go out of bounds
// - Also need a check whether a node is valid or not
// - We can store edges as 64x64 1d array of short (16bit) where:
//   - last 4 bits value representing whether node is connected to left/right/down/up neighbors
//   - 5th bit representing whether the node is visited or not
//   - 6th bit representing whether the node is C or W
//   - 7th bit representing whether the node is in path or not (for solving the maze) denoted by P
//   - the top 8 bits representing the weight of the node
// | 0 | P | C/W | visited | left | right | up | down | -> bottom 8 bits
// |                       Weight                     | -> top 8 bits (NOTE: Needed only in kruskal and dijkstra, not in bfs or dfs)
// Now weight of an edge would thus be defined as the maximum of the two nodes connected by the edge

// Define macros to access the row and column of the node
#define ROW(node, n) (node / n)
#define COL(node, n) (node % n)
#define NODE(row, col, n) ((n)*(row) + (col))
#define LEFT_NODE(node, n) (COL(node, n) > 0 ? node - 1 : -1)
#define RIGHT_NODE(node, n) (COL(node, n) < n-1 ? node + 1 : -1)
#define UP_NODE(node, n) (ROW(node, n) > 0 ? node - n : -1)
#define DOWN_NODE(node, n) (ROW(node, n) < n-1 ? node + n : -1)

// Define macros for accessing the weight of a node
#define WEIGHT_MASK 0xFF00 // Mask for the top 8 bits (weight)
#define GET_NODE_WEIGHT(node) ((node & WEIGHT_MASK) >> 8) // Get the weight of the node
#define SET_NODE_WEIGHT(node, weight) (node = (node & ~WEIGHT_MASK) | ((weight << 8) & WEIGHT_MASK)) // Set the weight of the node

// Define macros for accessing the edge weight
//! Check if this makes sense
#define GET_EDGE_WEIGHT(node1, node2) (std::max(GET_NODE_WEIGHT(node1), GET_NODE_WEIGHT(node2))) // The weight of the edge is the maximum of the weights of two nodes connected by the edge

// Define macros which represent position of the edges in the 4-bit value, to directly to & and | operationd
#define LEFT 0x08
#define RIGHT 0x04
#define UP 0x02
#define DOWN 0x01

// Define macros for visited
#define VISITED 0x10
#define UNWALLED 0x20
#define PATH 0x40
#define VISITED_SOLVE 0x80

// To set the bits
#define SET_LEFT(edges) (edges |= LEFT)
#define SET_RIGHT(edges) (edges |= RIGHT)
#define SET_UP(edges) (edges |= UP)
#define SET_DOWN(edges) (edges |= DOWN)
#define SET_VISITED(edges) (edges |= VISITED)
#define SET_VISITED_SOLVE(edges) (edges |= VISITED_SOLVE)

// to unset the bits
#define UNSET_LEFT(edges) (edges &= ~LEFT)
#define UNSET_RIGHT(edges) (edges &= ~RIGHT)
#define UNSET_UP(edges) (edges &= ~UP)
#define UNSET_DOWN(edges) (edges &= ~DOWN)
#define UNSET_ALL(edges) (edges &= 0x00)

// to unset the visited bit
#define UNSET_VISITED(edges) (edges &= ~VISITED)

// to get the edges
#define GET_LEFT(edges) (edges & LEFT)
#define GET_RIGHT(edges) (edges & RIGHT)
#define GET_UP(edges) (edges & UP)
#define GET_DOWN(edges) (edges & DOWN)

// to get the visited bit
#define GET_VISITED(edges) (edges & VISITED)

// to check if visited is set, returns bool
#define IS_VISITED(edges) ((edges & VISITED) == VISITED)
#define IS_VISITED_SOLVE(edges) ((edges & VISITED_SOLVE) == VISITED_SOLVE)

// Check if node is C or W
// C - Connected, W - Wall
#define IS_C(edges) (edges & 0x20)
#define IS_W(edges) (!(edges & 0x20))

// Set node as C or W
#define SET_C(edges) (edges |= 0x20)
#define SET_W(edges) (edges &= ~0x20)

// Check if node is in path
#define IS_P(edges) (edges & 0x40)

// Set node as in path
#define SET_P(edges) (edges |= 0x40)

// Unset node as in path
#define UNSET_P(edges) (edges &= ~0x40)

// Check if node is valid
#define IS_VALID_NODE(node, n) (ROW(node, n) >= 0 && ROW(node, n) < n && COL(node, n) >= 0 && COL(node, n) < n)


#define MAX_ARG_LEN 16


// debug.cpp functions
void print_maze(short* maze, int size);
void print_edges(short* edges, int size);
void print_maze_visual(short* edges, int size);
void print_maze_complete(short* edges, int size);
void print_visited(short* edges, int size);
void print_visited_solve(short* edges, int size);

// maze.cpp functions

void print_maze_final(short* edges, int size, int start, int end);

#endif // DEFS_H