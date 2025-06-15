#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "raylib.h"

#define BOARD_SIZE 8
#define TILE_SIZE 42
#define TILE_TYPES 5

const char TILE_CHARS[TILE_TYPES] = { '#', '@', '$', '%', '&' };

char board[BOARD_SIZE * BOARD_SIZE];

char random_tile();
void init_board();

int main(void) {
    SetRandomSeed(26581);

    return 0;
}



char random_tile() {
    return TILE_CHARS[GetRandomValue(0, TILE_TYPES-1)];
}

void init_board() {
    for(int x = 0; x < BOARD_SIZE; x++) {
        for(int y = 0; y < BOARD_SIZE; y++) {
            board[(BOARD_SIZE * y) + x] = random_tile;
        }
    }
}