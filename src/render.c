#include "render.h"
#include "game.h"
#include "raylib.h"

void render_game(const GameState* game) {
    (void)game;
    // TODO: desenhar mapa, entidades e HUD.
    DrawText("TODO: renderizar jogo", 20, 20, 20, RAYWHITE);
}

void render_menu(const GameState* game) {
    if (game->menu.status != MENU_OPEN) return;
    DrawText("TODO: menu (TAB)", 20, WINDOW_HEIGHT - 30, 18, YELLOW);
}
