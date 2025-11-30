#pragma once

#include <stdbool.h>

struct GameState;

bool save_game(const struct GameState* game, const char* path);
bool load_game(struct GameState* game, const char* path);
