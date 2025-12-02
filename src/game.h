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
#define PACMAN_SPEED_BLOCKS_PER_SEC 4.0f
#define GHOST_SPEED_BLOCKS_PER_SEC 4.0f
#define GHOST_SPEED_VULNERABLE 3.0f
#define PACMAN_STEP_INTERVAL (1.0f / PACMAN_SPEED_BLOCKS_PER_SEC)
#define GHOST_STEP_INTERVAL (1.0f / GHOST_SPEED_BLOCKS_PER_SEC)
#define GHOST_STEP_INTERVAL_VULNERABLE (1.0f / GHOST_SPEED_VULNERABLE)
#define POWER_MODE_DURATION 8.0f

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
    char currentMapPath[128];
} GameState;

bool game_init(GameState* game, const char* firstMapPath, int ghostCount);
void game_shutdown(GameState* game);
bool game_load_level(GameState* game, const char* mapPath);
void game_update(GameState* game, float dt);
void game_draw(const GameState* game);
