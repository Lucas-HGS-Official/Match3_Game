#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "raylib.h"

#define BOARD_SIZE 8
#define TILE_SIZE 42
#define TILE_TYPES 5

const int SCORE_FONT_SIZE = 32;
const char TILE_CHARS[TILE_TYPES] = { '#', '@', '$', '%', '&' };
char board[BOARD_SIZE * BOARD_SIZE];

int score = 0;

Vector2 gridOrigin;
Texture2D background;
Font scoreFont;

char random_tile(void);
void init_board(void);

int main(void) {
    SetRandomSeed(time(NULL));
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Match3");
    SetTargetFPS(60);

    background = LoadTexture("resources/background.jpg");
    scoreFont = LoadFontEx("resources/04b03.ttf", SCORE_FONT_SIZE,NULL,0);

    init_board();

    while(!WindowShouldClose()) {
        BeginDrawing();
        {
            ClearBackground(RAYWHITE);

            DrawTexturePro(
                background,
                (Rectangle) {
                    0,0, background.width, background.height,
                },
                (Rectangle) {
                    0,0, GetScreenWidth(), GetScreenHeight(),
                },
                (Vector2) { 0,0 },
                0.f,
                WHITE
            );

            for(int x = 0; x < BOARD_SIZE; x++) {
                for(int y = 0; y < BOARD_SIZE; y++) {
                    Rectangle rect = {
                        .x = gridOrigin.x +(x*TILE_SIZE),
                        .y = gridOrigin.y + (y*TILE_SIZE),
                        .width = TILE_SIZE,
                        .height = TILE_SIZE,
                    };

                    DrawRectangleLinesEx(rect, 1, DARKGRAY);

                    Vector2 pos = {
                        .x = x*TILE_SIZE +12,
                        .y = y*TILE_SIZE +8,
                    };

                    DrawTextEx(
                        GetFontDefault(), 
                        TextFormat("%c", board[(BOARD_SIZE * y) + x]),
                        (Vector2) { rect.x + 12, rect.y +8 },
                        20.f, 1, WHITE
                    );
                }
            }

            DrawTextEx(
                scoreFont,
                TextFormat("SCORE: %d", score),
                (Vector2) { 20, 20 },
                SCORE_FONT_SIZE, 1.f, YELLOW
            );
        }
        EndDrawing();
    }

    UnloadTexture(background);
    UnloadFont(scoreFont);
    CloseWindow();

    return 0;
}



char random_tile() {
    return TILE_CHARS[GetRandomValue(0, TILE_TYPES-1)];
}

void init_board() {
    for(int x = 0; x < BOARD_SIZE; x++) {
        for(int y = 0; y < BOARD_SIZE; y++) {
            board[(BOARD_SIZE * y) + x] = random_tile();
        }
    }

    int gridWidth = BOARD_SIZE * TILE_SIZE;
    int gridHeight = BOARD_SIZE * TILE_SIZE;

    gridOrigin = (Vector2) {
        .x = (GetScreenWidth() - gridWidth) / 2,
        .y = (GetScreenHeight() - gridHeight) / 2,
    };
}