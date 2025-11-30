#include "save.h"
#include "game.h"
#include <stdio.h>

bool save_game(const GameState* game, const char* path) {
    (void)game;
    (void)path;
    // TODO: serializar estado em binário.
    return false;
}

bool load_game(GameState* game, const char* path) {
    (void)game;
    (void)path;
    // TODO: ler estado salvo em binário.
    return false;
}
