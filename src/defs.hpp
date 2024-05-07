#ifndef DEFS_H
#define DEFS_H

// - Nodes of the maze are represented by an integer, 64*row + col
// - We make macros to access row no. and col no., neighbors, etc.
// - make sure that the neighbors macros give valid output, i.e. they don't go out of bounds
// - Also need a check whether a node is valid or not
// - We can store edges as 64x64 1d array of char (8bit) where last 4 bits value representing whether node is connected to left/right/down/up neighbors
// | 0 | 0 | C/W | visited | left | right | up | down | -> 8 bits
#define ROW(node, n) (node / n)
#define COL(node, n) (node % n)
#define NODE(row, col, n) (n*row + col)
#define LEFT_NODE(node, n) (COL(node, n) > 0 ? node - 1 : -1)
#define RIGHT_NODE(node, n) (COL(node, n) < n-1 ? node + 1 : -1)
#define UP_NODE(node, n) (ROW(node, n) > 0 ? node - n : -1)
#define DOWN_NODE(node, n) (ROW(node, n) < n-1 ? node + n : -1)

// Define macros which represent position of the edges in the 4-bit value, to directly to & and | operationd
#define LEFT 0x08
#define RIGHT 0x04
#define UP 0x02
#define DOWN 0x01

// Define macros for visited
#define VISITED 0x10

// To set the bits
#define SET_LEFT(edges) (edges |= LEFT)
#define SET_RIGHT(edges) (edges |= RIGHT)
#define SET_UP(edges) (edges |= UP)
#define SET_DOWN(edges) (edges |= DOWN)
#define SET_VISITED(edges) (edges |= VISITED)

// to unset the bits
#define UNSET_LEFT(edges) (edges &= ~LEFT)
#define UNSET_RIGHT(edges) (edges &= ~RIGHT)
#define UNSET_UP(edges) (edges &= ~UP)
#define UNSET_DOWN(edges) (edges &= ~DOWN)
#define UNSET_ALL(edges) (edges &= 0x00)

// to unset the visited bit
#define UNSET_VISITED(edges) (edges & ~VISITED)

// to get the edges
#define GET_LEFT(edges) (edges & LEFT)
#define GET_RIGHT(edges) (edges & RIGHT)
#define GET_UP(edges) (edges & UP)
#define GET_DOWN(edges) (edges & DOWN)

// to get the visited bit
#define GET_VISITED(edges) (edges & VISITED)

// Check if node is C or W
#define IS_C(edges) (edges & 0x20)
#define IS_W(edges) (!(edges & 0x20))

// Set node as C or W
#define SET_C(edges) (edges |= 0x20)
#define SET_W(edges) (edges &= ~0x20)

#define IS_VALID_NODE(node, n) (ROW(node, n) >= 0 && ROW(node, n) < n && COL(node, n) >= 0 && COL(node, n) < n)


#define MAX_ARG_LEN 16



#endif // DEFS_H