#include "game.h"
#include "render.h"
#include "save.h"
#include <stdlib.h>

bool game_load_level(GameState* game, const char* mapPath) {
    map_free(&game->map);
    if (!map_load(&game->map, mapPath)) return false;

    game->pelletsRemaining = game->map.pelletsRemaining;
    game->powerTimer = 0.0f;

    game->pacman.pos = game->map.pacmanStart;
    game->pacman.dir = DIR_NONE;
    game->pacman.pendingDir = DIR_NONE;
    game->pacman.powered = false;
    game->pacman.powerTimeLeft = 0.0f;

    free(game->ghosts);
    game->ghostCount = game->map.ghostCount;
    if (game->ghostCount > 0) {
        game->ghosts = (Ghost*)malloc(sizeof(Ghost) * game->ghostCount);
        for (int i = 0; i < game->ghostCount; i++) {
            game->ghosts[i].pos = game->map.ghostStarts[i];
            game->ghosts[i].dir = DIR_NONE;
            game->ghosts[i].vulnerable = false;
            game->ghosts[i].vulnerableTimeLeft = 0.0f;
            game->ghosts[i].alive = true;
        }
    } else {
        game->ghosts = NULL;
    }

    return true;
}

bool game_init(GameState* game, const char* firstMapPath, int ghostCount) {
    game->level = 1;
    game->score = 0;
    game->lives = PACMAN_START_LIVES;
    game->paused = false;
    game->running = true;
    game->menu.status = MENU_HIDDEN;
    game->menu.pendingAction = MENU_ACTION_NONE;
    game->menu.selectedIndex = 0;
    game->ghosts = NULL;
    game->ghostCount = ghostCount;
    game->pelletsRemaining = 0;
    game->powerTimer = 0.0f;

    game->map = (Map){0};
    return game_load_level(game, firstMapPath);
}

void game_shutdown(GameState* game) {
    free(game->ghosts);
    game->ghosts = NULL;
    map_free(&game->map);
}

void game_update(GameState* game, float dt) {
    (void)dt;
    (void)game;
    // TODO: implementar lógica de jogo, input, menu e movimentações.
}

void game_draw(const GameState* game) {
    render_game(game);
    render_menu(game);
}
