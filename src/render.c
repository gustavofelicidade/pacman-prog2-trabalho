#include "render.h"
#include "game.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>

static const Color WALL_COLOR = {0, 82, 204, 255};
static const Color FLOOR_COLOR = {15, 15, 15, 255};

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
    for (int row = 0; row < map->rows; row++) {
        for (int col = 0; col < map->cols; col++) {
            char cell = map_get(map, row, col);
            Rectangle rect = tile_rect(row, col);
            DrawRectangleRec(rect, FLOOR_COLOR);

            switch (cell) {
                case '#':
                    DrawRectangleRec(rect, WALL_COLOR);
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
    float radius = (float)TILE_SIZE / 2.2f;
    DrawCircleV(center, radius, color);

    Direction mouthDir = pacman->dir;
    if (mouthDir == DIR_NONE) mouthDir = DIR_RIGHT;
    float mouthAngle = 40.0f;
    float centerAngle = 0.0f;
    switch (mouthDir) {
        case DIR_RIGHT: centerAngle = 0.0f; break;
        case DIR_LEFT: centerAngle = 180.0f; break;
        case DIR_UP: centerAngle = -90.0f; break;
        case DIR_DOWN: centerAngle = 90.0f; break;
        default: break;
    }
    float start = centerAngle - mouthAngle;
    float end = centerAngle + mouthAngle;
    float sectorStart = fmodf(start + 360.0f, 360.0f);
    float sectorEnd = fmodf(end + 360.0f, 360.0f);
    float delta = sectorEnd - sectorStart;
    if (delta < 0) delta += 360.0f;
    if (delta > 0) {
        DrawCircleSector(center, radius + 1.0f, sectorStart, sectorStart + delta, 30, FLOOR_COLOR);
    }

    float eyeAngleDeg = centerAngle + 90.0f;
    if (mouthDir == DIR_RIGHT || mouthDir == DIR_NONE) {
        eyeAngleDeg = centerAngle - 90.0f;
    }
    float eyeAngleRad = eyeAngleDeg * PI / 180.0f;
    Vector2 eyePos = {
        center.x + cosf(eyeAngleRad) * radius * 0.4f,
        center.y + sinf(eyeAngleRad) * radius * 0.4f
    };
    DrawCircleV(eyePos, radius * 0.15f, BLACK);
}

static void draw_ghost_shape(Vector2 center, bool vulnerable) {
    Color base = vulnerable ? WHITE : RED;
    float radius = (float)TILE_SIZE / 2.3f;
    float headRadius = radius;
    Vector2 headCenter = {center.x, center.y - radius * 0.2f};
    DrawCircleV(headCenter, headRadius, base);

    Rectangle body = {
        .x = headCenter.x - headRadius,
        .y = headCenter.y,
        .width = headRadius * 2.0f,
        .height = radius * 1.2f
    };
    DrawRectangleRec(body, base);

    float waveRadius = headRadius / 3.0f;
    for (int i = 0; i < 3; i++) {
        Vector2 waveCenter = {
            body.x + waveRadius + i * (waveRadius * 2.0f),
            body.y + body.height - waveRadius
        };
        DrawCircleV(waveCenter, waveRadius, base);
    }

    Color eyeWhite = vulnerable ? (Color){200, 200, 200, 255} : RAYWHITE;
    Color pupil = vulnerable ? (Color){100, 100, 100, 255} : DARKBLUE;
    Vector2 leftEye = {center.x - headRadius * 0.35f, headCenter.y - headRadius * 0.2f};
    Vector2 rightEye = {center.x + headRadius * 0.35f, headCenter.y - headRadius * 0.2f};
    DrawCircleV(leftEye, headRadius * 0.3f, eyeWhite);
    DrawCircleV(rightEye, headRadius * 0.3f, eyeWhite);
    DrawCircleV((Vector2){leftEye.x + 4, leftEye.y}, headRadius * 0.15f, pupil);
    DrawCircleV((Vector2){rightEye.x + 4, rightEye.y}, headRadius * 0.15f, pupil);
}

static void draw_ghosts(const GameState* game) {
    if (!game->ghosts) return;
    for (int i = 0; i < game->ghostCount; i++) {
        const Ghost* ghost = &game->ghosts[i];
        if (!ghost->alive) continue;
        Vector2 center = tile_center(ghost->pos.row, ghost->pos.col);
        draw_ghost_shape(center, ghost->vulnerable);
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

    if (game->hudMessageTimer > 0.0f && game->hudMessage[0] != '\0') {
        DrawText(game->hudMessage, 820, (int)hudY + 10, 18, YELLOW);
    }
}

static void draw_end_overlay(const GameState* game) {
    GamePhase overlayPhase = game->phase;
    if (overlayPhase == GAME_PHASE_ENTER_SCORE) {
        overlayPhase = game->postPhase;
    }
    if (overlayPhase != GAME_PHASE_VICTORY && overlayPhase != GAME_PHASE_GAMEOVER) return;
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.75f));
    const char* title = (overlayPhase == GAME_PHASE_VICTORY) ? "Vitoria!" : "Game Over";
    const char* subtitle = (overlayPhase == GAME_PHASE_VICTORY)
        ? "N: Novo jogo  R: Ranking  Q: Sair"
        : "N: Reiniciar  R: Ranking  Q: Sair";
    int titleWidth = MeasureText(title, 48);
    int subWidth = MeasureText(subtitle, 24);
    int centerX = WINDOW_WIDTH / 2;
    DrawText(title, centerX - titleWidth / 2, WINDOW_HEIGHT / 2 - 60, 48, GOLD);
    DrawText(subtitle, centerX - subWidth / 2, WINDOW_HEIGHT / 2, 24, RAYWHITE);
}

static void render_name_entry_overlay(const GameState* game) {
    if (game->phase != GAME_PHASE_ENTER_SCORE) return;
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.8f));
    const char* title = "Novo recorde!";
    char scoreText[64];
    snprintf(scoreText, sizeof(scoreText), "Pontuacao: %06d", game->pendingRankingScore);
    const char* hint = "Digite seu nome e pressione ENTER (ESC para ignorar)";
    int centerX = WINDOW_WIDTH / 2;
    int y = WINDOW_HEIGHT / 2 - 80;
    int titleWidth = MeasureText(title, 42);
    DrawText(title, centerX - titleWidth / 2, y, 42, GOLD);
    DrawText(scoreText, centerX - MeasureText(scoreText, 24) / 2, y + 60, 24, RAYWHITE);
    DrawText(hint, centerX - MeasureText(hint, 20) / 2, y + 100, 20, LIGHTGRAY);

    Rectangle inputRect = {
        .x = (float)(centerX - 200),
        .y = (float)(y + 140),
        .width = 400.0f,
        .height = 50.0f
    };
    DrawRectangleRec(inputRect, (Color){20, 20, 20, 255});
    DrawRectangleLinesEx(inputRect, 2.0f, GOLD);
    const char* name = (game->nameEntryLen > 0) ? game->nameEntry : "_";
    DrawText(name, centerX - MeasureText(name, 28) / 2, (int)inputRect.y + 10, 28, RAYWHITE);
}

void render_title_screen(const GameState* game) {
    (void)game;
    const char* title = "PAC-MAN Prog II";
    const char* subtitle = "Trabalho Pratico - Turma 2025/2";
    int centerX = WINDOW_WIDTH / 2;
    int titleWidth = MeasureText(title, 48);
    DrawText(title, centerX - titleWidth / 2, 160, 48, YELLOW);
    DrawText(subtitle, centerX - MeasureText(subtitle, 20) / 2, 220, 20, LIGHTGRAY);

    const char* options[] = {
        "[N] Novo jogo",
        "[C] Carregar jogo salvo",
        "[R] Ver ranking",
        "[Q] Sair"
    };
    for (int i = 0; i < 4; i++) {
        DrawText(options[i], centerX - 160, 300 + i * 40, 24, RAYWHITE);
    }
}

void render_ranking_screen(const GameState* game) {
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.7f));
    const char* title = "Ranking de Pontuacoes";
    int centerX = WINDOW_WIDTH / 2;
    DrawText(title, centerX - MeasureText(title, 36) / 2, 80, 36, GOLD);

    for (int i = 0; i < RANKING_MAX_ENTRIES; i++) {
        const RankingEntry* entry = &game->ranking.entries[i];
        char line[64];
        snprintf(line, sizeof(line), "%2d. %-10s %6d", i + 1, entry->name, entry->score);
        DrawText(line, centerX - 150, 150 + i * 28, 24, RAYWHITE);
    }

    const char* hint = "[ESC] Voltar  [N] Novo jogo  [Q] Sair";
    DrawText(hint, centerX - MeasureText(hint, 20) / 2, WINDOW_HEIGHT - 80, 20, LIGHTGRAY);
}

void render_game(const GameState* game) {
    draw_map_tiles(game);
    draw_pacman(&game->pacman);
    draw_ghosts(game);
    draw_hud(game);
    draw_end_overlay(game);
    render_name_entry_overlay(game);
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
