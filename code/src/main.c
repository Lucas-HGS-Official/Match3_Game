#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "raylib.h"

#define BOARD_SIZE 8
#define TILE_SIZE 42
#define TILE_TYPES 5
#define MAX_SCORE_POPUPS 32

typedef enum {
    STATE_IDLE,
    STATE_ANIMATING,
    STATE_MATCH_DELAY,
} TileState_t;

typedef struct {
    Vector2 pos;
    int amount;
    float lifetime;
    float alpha;
    bool isActive;
} ScorePopup_t;

const int SCORE_FONT_SIZE = 32;
const float MATCH_DELAY_DURATION = 0.2f;

const char TILE_CHARS[TILE_TYPES] = { '#', '@', '$', '%', '&' };

char board[BOARD_SIZE * BOARD_SIZE];
bool matched[BOARD_SIZE * BOARD_SIZE] = { 0 };
float fallOffset[BOARD_SIZE * BOARD_SIZE] = { 0 };

int score = 0;
float fallSpeed = 8.f;
float matchDelayTimer = 0.f;
Vector2 selectedTile = { -1,-1 };
Vector2 gridOrigin;
Texture2D background;
Font scoreFont;
Music bgm;
Sound matchSFX;
TileState_t tileState;
ScorePopup_t scorePopups[MAX_SCORE_POPUPS] = { 0 };

char random_tile(void);
void init_board(void);
bool find_matches(void);
void resolve_matches(void);
void swap_tiles(int x1, int y1, int x2, int y2);
bool are_tiles_adjancent(Vector2 a, Vector2 b);
void add_score_popup(int x,int y,int amount, Vector2 gridOrigin);

int main(void) {
    SetRandomSeed(time(NULL));
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Match3");
    SetTargetFPS(60);

    InitAudioDevice();

    background = LoadTexture("resources/background.jpg");
    scoreFont = LoadFontEx("resources/04b03.ttf", SCORE_FONT_SIZE,NULL,0);
    bgm = LoadMusicStream("resources/prismx27s-edge-246705.mp3");
    matchSFX = LoadSound("resources/match.mp3");

    PlayMusicStream(bgm);

    init_board();
    Vector2 mouseCoords = { 0 };

    while(!WindowShouldClose()) {

        UpdateMusicStream(bgm);

        mouseCoords = GetMousePosition();
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && tileState == STATE_IDLE) {
            int x = (mouseCoords.x - gridOrigin.x)/TILE_SIZE;
            int y = (mouseCoords.y - gridOrigin.y)/TILE_SIZE;

            if(x>=0 && x<BOARD_SIZE  &&  y>=0 && y<BOARD_SIZE) {
                Vector2 currentTile = (Vector2) { x, y };

                if(selectedTile.x < 0) {
                    selectedTile = currentTile;
                } else {

                    if(are_tiles_adjancent(selectedTile, currentTile)) {
                        swap_tiles(selectedTile.x, selectedTile.y, currentTile.x, currentTile.y);
                        if(find_matches()) {
                            resolve_matches();
                        } else {
                            swap_tiles(selectedTile.x, selectedTile.y, currentTile.x, currentTile.y);
                        }
                    }

                    selectedTile = (Vector2) { -1, -1 };
                } 
            }
        }

        if(tileState == STATE_ANIMATING) {
            bool stillAnimating = false;
            
            for(int y=0; y<BOARD_SIZE; y++) {
                for(int x=0; x<BOARD_SIZE; x++) {

                    if(fallOffset[(y*BOARD_SIZE) + x] > 0) {
                        fallOffset[(y*BOARD_SIZE) + x] -= fallSpeed;

                        if(fallOffset[(y*BOARD_SIZE) + x] < 0) {
                            fallOffset[(y*BOARD_SIZE) + x] = 0;
                        } else {
                            stillAnimating = true;
                        }
                    }
                }
            }

            if(!stillAnimating) {
                tileState = STATE_MATCH_DELAY;
                matchDelayTimer = MATCH_DELAY_DURATION;
            }
        }

        if(tileState == STATE_MATCH_DELAY) {
            matchDelayTimer -= GetFrameTime();

            if(matchDelayTimer <= 0.f) {
                
                if(find_matches()) {
                    resolve_matches();
                } else {
                    tileState = STATE_IDLE;
                }
            }
        }

        for(int i=0;i<MAX_SCORE_POPUPS; i++) {
            if(scorePopups[i].isActive) {
                scorePopups[i].lifetime -= GetFrameTime();
                scorePopups[i].pos.y -= 30*GetFrameTime();
                scorePopups[i].alpha = scorePopups[i].lifetime;

                if(scorePopups[i].lifetime <= 0.f) {
                    scorePopups[i].isActive = false;
                }
            }
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

            DrawRectangle(
                gridOrigin.x,
                gridOrigin.y,
                BOARD_SIZE * TILE_SIZE,
                BOARD_SIZE * TILE_SIZE,
                Fade(DARKGRAY, .6f)
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

            for(int i=0;i<MAX_SCORE_POPUPS;i++) {
                if(scorePopups[i].isActive) {
                    Color c = Fade(YELLOW, scorePopups[i].alpha);
                    DrawText(
                        TextFormat("+%d", scorePopups[i].amount),
                        scorePopups[i].pos.x,
                        scorePopups[i].pos.y,
                        20,
                        c
                    );
                }
            }
        }
        EndDrawing();
    }

    StopMusicStream(bgm);
    UnloadMusicStream(bgm);
    UnloadSound(matchSFX);
    UnloadTexture(background);
    UnloadFont(scoreFont);


    CloseAudioDevice();
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
    
    if(find_matches()) {
        resolve_matches();
    } else {
        tileState = STATE_IDLE;
    }
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
                PlaySound(matchSFX);

                add_score_popup(x, y, 10, gridOrigin);
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
                PlaySound(matchSFX);

                add_score_popup(x, y, 10, gridOrigin);
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

    tileState = STATE_ANIMATING;
}

void swap_tiles(int x1, int y1, int x2, int y2) {
    char temp = board[(y1*BOARD_SIZE) + x1];
    board[(y1*BOARD_SIZE) + x1] = board[(y2*BOARD_SIZE) + x2];
    board[(y2*BOARD_SIZE) + x2] = temp;
}

bool are_tiles_adjancent(Vector2 a, Vector2 b) {
    return (abs((int)a.x - (int)b.x) + abs((int)a.y - (int)b.y)) == 1;
}

void add_score_popup(int x,int y,int amount, Vector2 gridOrigin) {
    for(int i=0; i<MAX_SCORE_POPUPS; i++) {
        if(!scorePopups[i].isActive) {
            scorePopups[i] = (ScorePopup_t) {
                .pos.x = gridOrigin.x + x * TILE_SIZE + TILE_SIZE / 2,
                .pos.y = gridOrigin.y + y * TILE_SIZE + TILE_SIZE / 2,
                .amount = amount,
                .lifetime = 1.f,
                .alpha = 1.f,
                .isActive = true,
            };
            break;
        }
    }
}