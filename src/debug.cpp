
#include "defs.hpp"
#include <stdio.h>

void print_maze(char* maze, int size){
    printf("Printing maze\n");
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            printf("%c", maze[i * size + j] == 0x00 ? 'W' : 'C');
        }
        printf("\n");
    }
}

void print_edges(char* edges, int size){
    printf("Printing edges\n");
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            // print the last 8 bits of the edge
            printf("%x ", edges[i * size + j] & 0xFF);
        }
        printf("\n");
    }
}

// - Nodes of the maze are represented by an integer, 64*row + col
// - We make macros to access row no. and col no., neighbors, etc.
// - make sure that the neighbors macros give valid output, i.e. they don't go out of bounds
// - Also need a check whether a node is valid or not
// - We can store edges as 64x64 1d array of char (8bit) where last 4 bits value representing whether node is connected to left/right/down/up neighbors
// | 0 | 0 | C/W | visited | left | right | up | down | -> 8 bits

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

// ┴ -  Box drawings light up and horizontal
// ┼ -  Box drawings light vertical and horizontal
// ┬ -  Box drawings light down and horizontal
// ┤ -  Box drawings light vertical and left
// ├ -  Box drawings light vertical and right
// ─ -  Box drawings light horizontal
// │ -  Box drawings light vertical
// ┘ -  Box drawings light up and left
// ┐ -  Box drawings light down and left
// └ -  Box drawings light up and right
// ┌ -  Box drawings light down and right
// ╶ -  Box drawings light left
// ╴ -  Box drawings light right
// ╵ -  Box drawings light up
// ╷ -  Box drawings light down
// · - center dot
void print_maze_visual(char* edges, int size){
    printf("Printing maze\n");
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            char edge = edges[i * size + j];
            if (GET_LEFT(edge) == LEFT && GET_RIGHT(edge) == RIGHT && GET_UP(edge) == UP && GET_DOWN(edge) == DOWN){
                printf("┼");
            } else if (GET_LEFT(edge) == LEFT && GET_RIGHT(edge) == RIGHT && GET_UP(edge) == UP){
                printf("┴");
            } else if (GET_LEFT(edge) == LEFT && GET_RIGHT(edge) == RIGHT && GET_DOWN(edge) == DOWN){
                printf("┬");
            } else if (GET_LEFT(edge) == LEFT && GET_UP(edge) == UP && GET_DOWN(edge) == DOWN){
                printf("┤");
            } else if (GET_RIGHT(edge) == RIGHT && GET_UP(edge) == UP && GET_DOWN(edge) == DOWN){
                printf("├");
            } else if (GET_LEFT(edge) == LEFT && GET_RIGHT(edge) == RIGHT){
                printf("─");
            } else if (GET_UP(edge) == UP && GET_DOWN(edge) == DOWN){
                printf("│");
            } else if (GET_LEFT(edge) == LEFT && GET_UP(edge) == UP){
                printf("┘");
            } else if (GET_LEFT(edge) == LEFT && GET_DOWN(edge) == DOWN){
                printf("┐");
            } else if (GET_RIGHT(edge) == RIGHT && GET_UP(edge) == UP){
                printf("└");
            } else if (GET_RIGHT(edge) == RIGHT && GET_DOWN(edge) == DOWN){
                printf("┌");
            } else if (GET_LEFT(edge) == LEFT){
                printf("╴");
            } else if (GET_RIGHT(edge) == RIGHT){
                printf("╶");
            } else if (GET_UP(edge) == UP){
                printf("╵");
            } else if (GET_DOWN(edge) == DOWN){
                printf("╷");
            } else {
                printf("·");
            }
        }
        printf("\n");
    }

}

void print_visited(char* edges, int size){
    printf("Printing visited\n");
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            if (IS_VISITED(edges[i * size + j])){
                printf("V");
            } else {
                printf("X");
            }
        }
        printf("\n");
    }
}