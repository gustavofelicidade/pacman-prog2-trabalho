#pragma once

#include <stdbool.h>
#include "map.h"
#include "entity.h"
#include "menu.h"

#define TILE_SIZE 40
#define HUD_HEIGHT 40
#define WINDOW_WIDTH (MAP_COLS * TILE_SIZE)
#define WINDOW_HEIGHT (MAP_ROWS * TILE_SIZE + HUD_HEIGHT)
#define PACMAN_START_LIVES 3

typedef struct GameState {
    Map map;
    Pacman pacman;
    Ghost* ghosts;
    int ghostCount;
    int pelletsRemaining;
    int level;
    int score;
    int lives;
    float powerTimer;
    bool paused;
    bool running;
    MenuState menu;
} GameState;

bool game_init(GameState* game, const char* firstMapPath, int ghostCount);
void game_shutdown(GameState* game);
bool game_load_level(GameState* game, const char* mapPath);
void game_update(GameState* game, float dt);
void game_draw(const GameState* game);
