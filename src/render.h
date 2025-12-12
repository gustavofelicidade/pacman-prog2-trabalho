#pragma once

struct GameState;

void render_game(const struct GameState* game);
void render_menu(const struct GameState* game);
void render_title_screen(const struct GameState* game);
void render_ranking_screen(const struct GameState* game);
