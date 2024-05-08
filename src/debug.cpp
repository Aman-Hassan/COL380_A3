
#include "defs.hpp"
#include <stdio.h>


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
void print_maze_visual(short* edges, int size){
    // printf("Printing maze\n");
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            short edge = edges[i * size + j];
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

void print_maze_complete(short* edges, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int node = NODE(i, j, size);
            short edge = edges[node];

            if (IS_W(edge)) {
                printf("█"); // Print a character to represent an opaque block
            } else {
                int left_node = LEFT_NODE(node, size);
                int right_node = RIGHT_NODE(node, size);
                int up_node = UP_NODE(node, size);
                int down_node = DOWN_NODE(node, size);

                int has_left = (left_node != -1 && IS_C(edges[left_node]) == 0x20);
                int has_right = (right_node != -1 && IS_C(edges[right_node]) == 0x20);
                int has_up = (up_node != -1 && IS_C(edges[up_node]) == 0x20);
                int has_down = (down_node != -1 && IS_C(edges[down_node]) == 0x20);

                if (has_left && has_right && has_up && has_down) {
                    printf("┼");
                } else if (has_left && has_right && has_up) {
                    printf("┴");
                } else if (has_left && has_right && has_down) {
                    printf("┬");
                } else if (has_left && has_up && has_down) {
                    printf("┤");
                } else if (has_right && has_up && has_down) {
                    printf("├");
                } else if (has_left && has_right) {
                    printf("─");
                } else if (has_up && has_down) {
                    printf("│");
                } else if (has_left && has_up) {
                    printf("┘");
                } else if (has_left && has_down) {
                    printf("┐");
                } else if (has_right && has_up) {
                    printf("└");
                } else if (has_right && has_down) {
                    printf("┌");
                } else if (has_left) {
                    printf("╴");
                } else if (has_right) {
                    printf("╶");
                } else if (has_up) {
                    printf("╵");
                } else if (has_down) {
                    printf("╷");
                } else {
                    printf("·");
                }
            }
        }
        printf("\n");
    }
}

void print_maze(short* maze, int size){
    // printf("Printing maze\n");
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            // printf("%c", maze[i * size + j] == 0x00 ? 'W' : 'C');
            short node = maze[i * size + j];
            if (IS_C(node) == 0x20){
                printf("□");

                // // if on top edge, print top wall
                // if (i == 0){
                //     printf("▔");
                // // if on left edge, print left wall
                // } else if (j == 0){
                //     printf("▏");
                // // if on right edge, print right wall
                // } else if (j == size - 1){
                //     printf("▕");
                // // if on bottom edge, print bottom wall
                // } else if (i == size - 1){
                //     printf("▁");
                // } else {
                //     printf("□");
                // }
            } else {
                // printf("█");
                printf("X");
            }
        }
        printf("\n");
    }
}

void print_edges(short* edges, int size){
    // printf("Printing edges\n");
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            // print the last 8 bits of the edge
            printf("%x ", edges[i * size + j] & 0xFF);
        }
        printf("\n");
    }
}

void print_visited(short* edges, int size){
    // printf("Printing visited\n");
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


void print_visited_solve(short* edges, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int node = NODE(i, j, size);
            short edge = edges[node];

            if (IS_W(edge)) {
                printf("█"); // Print a character to represent an opaque block
            } else {
                int left_node = LEFT_NODE(node, size);
                int right_node = RIGHT_NODE(node, size);
                int up_node = UP_NODE(node, size);
                int down_node = DOWN_NODE(node, size);

                int has_left = (left_node != -1 && IS_VISITED_SOLVE(edges[left_node]));
                int has_right = (right_node != -1 && IS_VISITED_SOLVE(edges[right_node]));
                int has_up = (up_node != -1 && IS_VISITED_SOLVE(edges[up_node]));
                int has_down = (down_node != -1 && IS_VISITED_SOLVE(edges[down_node]));

                if (has_left && has_right && has_up && has_down) {
                    printf("┼");
                } else if (has_left && has_right && has_up) {
                    printf("┴");
                } else if (has_left && has_right && has_down) {
                    printf("┬");
                } else if (has_left && has_up && has_down) {
                    printf("┤");
                } else if (has_right && has_up && has_down) {
                    printf("├");
                } else if (has_left && has_right) {
                    printf("─");
                } else if (has_up && has_down) {
                    printf("│");
                } else if (has_left && has_up) {
                    printf("┘");
                } else if (has_left && has_down) {
                    printf("┐");
                } else if (has_right && has_up) {
                    printf("└");
                } else if (has_right && has_down) {
                    printf("┌");
                } else if (has_left) {
                    printf("╴");
                } else if (has_right) {
                    printf("╶");
                } else if (has_up) {
                    printf("╵");
                } else if (has_down) {
                    printf("╷");
                } else {
                    printf("·");
                }
            }
        }
        printf("\n");
    }
    printf("\n");
}