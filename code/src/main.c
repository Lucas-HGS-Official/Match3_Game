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
bool matched[BOARD_SIZE * BOARD_SIZE] = { 0 };
float fallOffset[BOARD_SIZE * BOARD_SIZE] = { 0 };

int score = 0;
Vector2 selectedTile = { -1,-1 };

Vector2 gridOrigin;
Texture2D background;
Font scoreFont;
float fallSpeed = 8.f;

char random_tile(void);
void init_board(void);
bool find_matches(void);
void resolve_matches(void);

int main(void) {
    SetRandomSeed(time(NULL));
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Match3");
    SetTargetFPS(60);

    background = LoadTexture("resources/background.jpg");
    scoreFont = LoadFontEx("resources/04b03.ttf", SCORE_FONT_SIZE,NULL,0);

    init_board();
    Vector2 mouseCoords = { 0 };

    while(!WindowShouldClose()) {

        mouseCoords = GetMousePosition();
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int x = (mouseCoords.x - gridOrigin.x)/TILE_SIZE;
            int y = (mouseCoords.y - gridOrigin.y)/TILE_SIZE;

            if(x>=0 && x<BOARD_SIZE  &&  y>=0 && y<BOARD_SIZE) {
                selectedTile = (Vector2) { x, y };
            }
        }

        for(int y=0; y<BOARD_SIZE; y++) {
            for(int x=0; x<BOARD_SIZE; x++) {
                if(fallOffset[(y*BOARD_SIZE) + x] > 0) {
                    fallOffset[(y*BOARD_SIZE) + x] -= fallSpeed;
                    if(fallOffset[(y*BOARD_SIZE) + x] < 0) {fallOffset[(y*BOARD_SIZE) + x] = 0; }
                }
            }
        }

        if(find_matches()) {
            resolve_matches();
        }

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

                    if(board[(y*BOARD_SIZE) + x] != ' ') {
                        DrawTextEx(
                            GetFontDefault(), 
                            TextFormat("%c", board[(BOARD_SIZE * y) + x]),
                            (Vector2) {
                                rect.x + 12,
                                rect.y + 8 -fallOffset[(BOARD_SIZE * y) + x],
                            },
                            20.f, 1, 
                            matched[(y*BOARD_SIZE) + x] ? GREEN : WHITE
                        );
                    }
                }
            }

            DrawTextEx(
                scoreFont,
                TextFormat("SCORE: %d", score),
                (Vector2) { 20, 20 },
                SCORE_FONT_SIZE, 1.f, YELLOW
            );

            if (selectedTile.x >= 0) {
                DrawRectangleLinesEx(
                    (Rectangle) {
                        .x = gridOrigin.x +(selectedTile.x*TILE_SIZE),
                        .y = gridOrigin.y + (selectedTile.y*TILE_SIZE),
                        .width = TILE_SIZE,
                        .height = TILE_SIZE,
                    }, 2, YELLOW
                );
            }
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

bool find_matches() {
    bool found = false;

    for(int x=0; x<BOARD_SIZE; x++) {
        for(int y=0; y<BOARD_SIZE; y++) {
            matched[(y*BOARD_SIZE) + x] = false;
        }
    }

    for(int x=0; x<BOARD_SIZE - 2; x++) {
        for(int y=0; y<BOARD_SIZE; y++) {

            char t = board[(y*BOARD_SIZE) + x];

            if(
                t == board[(y*BOARD_SIZE) + (x+1)] &&
                t == board[(y*BOARD_SIZE) + (x+2)]
            ) {
                matched[(y*BOARD_SIZE) + x] = matched[(y*BOARD_SIZE) + (x+1)] = matched[(y*BOARD_SIZE) + (x+2)] = true;
                score +=10;
                found = true;
            }
        }
    }

    for(int x=0; x<BOARD_SIZE; x++) {
        for(int y=0; y<BOARD_SIZE - 2; y++) { 

            char t = board[(y*BOARD_SIZE) + x];

            if(
                t == board[((y+1)*BOARD_SIZE) + x] &&
                t == board[((y+2)*BOARD_SIZE) + x]
            ) {
                matched[(y*BOARD_SIZE) + x] = matched[((y+1)*BOARD_SIZE) + x] = matched[((y+2)*BOARD_SIZE) + x] = true;
                score +=10;
                found = true;
            }
        }
    }

    return found;
}


void resolve_matches() {
    for(int x=0; x<BOARD_SIZE; x++) {
        int writeY = BOARD_SIZE-1;
        for(int y=BOARD_SIZE-1; y>=0; y--) {
            if(!matched[(y*BOARD_SIZE) + x]) {
                if(y != writeY) {
                    board[(writeY*BOARD_SIZE) + x] = board[(y*BOARD_SIZE) + x];
                    fallOffset[(writeY*BOARD_SIZE) + x] = (writeY - y) * TILE_SIZE;
                    board[(y*BOARD_SIZE) + x] = ' ';
                }
                writeY--;
            }
        }

        while(writeY >=0) {
            board[(writeY*BOARD_SIZE) + x] = random_tile();
            fallOffset[(writeY*BOARD_SIZE) + x] = (writeY + 1) * TILE_SIZE;
            writeY--;
        }
    } 
}