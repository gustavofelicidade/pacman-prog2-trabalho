#include "render.h"
#include "game.h"
#include "raylib.h"
#include <stdio.h>

static Rectangle tile_rect(int row, int col) {
    Rectangle r = {
        .x = (float)(col * TILE_SIZE),
        .y = (float)(row * TILE_SIZE),
        .width = (float)TILE_SIZE,
        .height = (float)TILE_SIZE
    };
    return r;
}

static Vector2 tile_center(int row, int col) {
    Vector2 v = {
        (float)(col * TILE_SIZE + TILE_SIZE / 2),
        (float)(row * TILE_SIZE + TILE_SIZE / 2)
    };
    return v;
}

static void draw_map_tiles(const GameState* game) {
    const Map* map = &game->map;
    const Color wallColor = (Color){0, 82, 204, 255};
    const Color floorColor = (Color){15, 15, 15, 255};

    for (int row = 0; row < map->rows; row++) {
        for (int col = 0; col < map->cols; col++) {
            char cell = map_get(map, row, col);
            Rectangle rect = tile_rect(row, col);
            DrawRectangleRec(rect, floorColor);

            switch (cell) {
                case '#':
                    DrawRectangleRec(rect, wallColor);
                    break;
                case '.': {
                    Vector2 center = tile_center(row, col);
                    DrawCircleV(center, 4.0f, RAYWHITE);
                    break;
                }
                case 'o': {
                    Vector2 center = tile_center(row, col);
                    DrawCircleV(center, (float)TILE_SIZE / 2.5f, GREEN);
                    break;
                }
                case 'T':
                    DrawRectangleRec(rect, MAGENTA);
                    break;
                default:
                    break;
            }
        }
    }
}

static void draw_pacman(const Pacman* pacman) {
    Vector2 center = tile_center(pacman->pos.row, pacman->pos.col);
    Color color = pacman->powered ? GOLD : YELLOW;
    DrawCircleV(center, (float)TILE_SIZE / 2.2f, color);
}

static void draw_ghosts(const GameState* game) {
    if (!game->ghosts) return;
    for (int i = 0; i < game->ghostCount; i++) {
        const Ghost* ghost = &game->ghosts[i];
        if (!ghost->alive) continue;
        Vector2 center = tile_center(ghost->pos.row, ghost->pos.col);
        Color color = ghost->vulnerable ? WHITE : RED;
        DrawCircleV(center, (float)TILE_SIZE / 2.4f, color);
    }
}

static void draw_hud(const GameState* game) {
    const float hudY = (float)(MAP_ROWS * TILE_SIZE);
    Rectangle hudRect = {0, hudY, WINDOW_WIDTH, HUD_HEIGHT};
    DrawRectangleRec(hudRect, (Color){25, 25, 25, 255});
    DrawLine(0, (int)hudY, WINDOW_WIDTH, (int)hudY, DARKGRAY);

    char text[64];
    snprintf(text, sizeof(text), "Vidas: %d", game->lives);
    DrawText(text, 10, (int)hudY + 10, 18, RAYWHITE);

    snprintf(text, sizeof(text), "Score: %06d", game->score);
    DrawText(text, 170, (int)hudY + 10, 18, RAYWHITE);

    snprintf(text, sizeof(text), "Nivel: %d", game->level);
    DrawText(text, 370, (int)hudY + 10, 18, RAYWHITE);

    snprintf(text, sizeof(text), "Pellets: %d/%d", game->pelletsRemaining, game->map.pelletsInitial);
    DrawText(text, 520, (int)hudY + 10, 18, RAYWHITE);
}

void render_game(const GameState* game) {
    draw_map_tiles(game);
    draw_pacman(&game->pacman);
    draw_ghosts(game);
    draw_hud(game);
}

void render_menu(const GameState* game) {
    if (game->menu.status != MENU_OPEN) return;

    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.6f));

    DrawText("MENU", 60, 60, 32, YELLOW);
    static const char* options[] = {
        "Novo jogo (N)",
        "Carregar (C)",
        "Salvar (S)",
        "Sair (Q)",
        "Voltar (V)"
    };
    const int optionCount = (int)(sizeof(options) / sizeof(options[0]));
    int highlight = game->menu.selectedIndex;
    if (highlight < 0) highlight = 0;
    if (highlight >= optionCount) highlight = optionCount - 1;

    for (int i = 0; i < optionCount; i++) {
        Color color = (i == highlight) ? GOLD : RAYWHITE;
        DrawText(options[i], 80, 120 + i * 32, 24, color);
    }
}
