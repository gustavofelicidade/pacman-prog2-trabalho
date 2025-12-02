#include "save.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int level;
    int score;
    int lives;
    int pelletsRemaining;
    int pelletsInitial;
    int mapRows;
    int mapCols;
    int ghostCount;
    int portalCount;
    Position pacmanPos;
    Position pacmanStart;
    int pacmanDir;
    int pacmanPending;
    int pacmanPowered;
    float pacmanPowerTimeLeft;
    float pacmanMoveTimer;
    char currentMapPath[128];
} SaveHeader;

typedef struct {
    Position pos;
    int dir;
    int vulnerable;
    float vulnerableTimeLeft;
    int alive;
    float moveTimer;
} SaveGhostState;

static bool write_data(const void* ptr, size_t size, size_t count, FILE* f) {
    return fwrite(ptr, size, count, f) == count;
}

static bool read_data(void* ptr, size_t size, size_t count, FILE* f) {
    return fread(ptr, size, count, f) == count;
}

bool save_game(const GameState* game, const char* path) {
    FILE* f = fopen(path, "wb");
    if (!f) return false;

    SaveHeader header = {
        .level = game->level,
        .score = game->score,
        .lives = game->lives,
        .pelletsRemaining = game->pelletsRemaining,
        .pelletsInitial = game->map.pelletsInitial,
        .mapRows = game->map.rows,
        .mapCols = game->map.cols,
        .ghostCount = game->ghostCount,
        .portalCount = game->map.portalCount,
        .pacmanPos = game->pacman.pos,
        .pacmanStart = game->map.pacmanStart,
        .pacmanDir = game->pacman.dir,
        .pacmanPending = game->pacman.pendingDir,
        .pacmanPowered = game->pacman.powered ? 1 : 0,
        .pacmanPowerTimeLeft = game->pacman.powerTimeLeft,
        .pacmanMoveTimer = game->pacman.moveTimer
    };
    snprintf(header.currentMapPath, sizeof(header.currentMapPath), "%s", game->currentMapPath);

    size_t totalCells = (size_t)game->map.rows * (size_t)game->map.cols;
    bool ok = write_data(&header, sizeof(SaveHeader), 1, f) &&
              write_data(game->map.cells, sizeof(char), totalCells, f);

    if (ok && game->map.ghostCount > 0 && game->map.ghostStarts) {
        ok = write_data(game->map.ghostStarts, sizeof(Position), game->map.ghostCount, f);
    }

    if (ok && game->map.portalCount > 0 && game->map.portals) {
        ok = write_data(game->map.portals, sizeof(Position), game->map.portalCount, f);
    }

    if (ok && game->ghostCount > 0 && game->ghosts) {
        for (int i = 0; i < game->ghostCount && ok; i++) {
            SaveGhostState state = {
                .pos = game->ghosts[i].pos,
                .dir = game->ghosts[i].dir,
                .vulnerable = game->ghosts[i].vulnerable ? 1 : 0,
                .vulnerableTimeLeft = game->ghosts[i].vulnerableTimeLeft,
                .alive = game->ghosts[i].alive ? 1 : 0,
                .moveTimer = game->ghosts[i].moveTimer
            };
            ok = write_data(&state, sizeof(SaveGhostState), 1, f);
        }
    }

    fclose(f);
    return ok;
}

static bool allocate_map_structures(Map* map, int rows, int cols, int ghostCount, int portalCount) {
    size_t totalCells = (size_t)rows * (size_t)cols;
    map->rows = rows;
    map->cols = cols;
    map->cells = (char*)malloc(totalCells);
    if (!map->cells) return false;

    map->ghostCount = ghostCount;
    map->portalCount = portalCount;
    map->ghostStarts = NULL;
    map->portals = NULL;
    if (ghostCount > 0) {
        map->ghostStarts = (Position*)malloc(sizeof(Position) * ghostCount);
        if (!map->ghostStarts) {
            free(map->cells);
            map->cells = NULL;
            return false;
        }
    }
    if (portalCount > 0) {
        map->portals = (Position*)malloc(sizeof(Position) * portalCount);
        if (!map->portals) {
            free(map->cells);
            map->cells = NULL;
            free(map->ghostStarts);
            map->ghostStarts = NULL;
            return false;
        }
    }
    return true;
}

bool load_game(GameState* game, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;

    SaveHeader header;
    bool ok = read_data(&header, sizeof(SaveHeader), 1, f);
    if (!ok) {
        fclose(f);
        return false;
    }

    map_free(&game->map);
    free(game->ghosts);
    game->ghosts = NULL;

    if (!allocate_map_structures(&game->map, header.mapRows, header.mapCols,
                                 header.ghostCount, header.portalCount)) {
        fclose(f);
        return false;
    }

    size_t totalCells = (size_t)header.mapRows * (size_t)header.mapCols;
    ok = read_data(game->map.cells, sizeof(char), totalCells, f);
    if (ok && header.ghostCount > 0) {
        ok = read_data(game->map.ghostStarts, sizeof(Position), header.ghostCount, f);
    }
    if (ok && header.portalCount > 0) {
        ok = read_data(game->map.portals, sizeof(Position), header.portalCount, f);
    }

    if (!ok) {
        fclose(f);
        map_free(&game->map);
        return false;
    }

    game->ghostCount = header.ghostCount;
    if (game->ghostCount > 0) {
        game->ghosts = (Ghost*)malloc(sizeof(Ghost) * game->ghostCount);
        if (!game->ghosts) {
            fclose(f);
            map_free(&game->map);
            return false;
        }
        for (int i = 0; i < game->ghostCount; i++) {
            SaveGhostState state;
            if (!read_data(&state, sizeof(SaveGhostState), 1, f)) {
                fclose(f);
                map_free(&game->map);
                free(game->ghosts);
                game->ghosts = NULL;
                return false;
            }
            game->ghosts[i].pos = state.pos;
            game->ghosts[i].dir = state.dir;
            game->ghosts[i].vulnerable = state.vulnerable;
            game->ghosts[i].vulnerableTimeLeft = state.vulnerableTimeLeft;
            game->ghosts[i].alive = state.alive;
            game->ghosts[i].moveTimer = state.moveTimer;
        }
    } else {
        game->ghosts = NULL;
    }

    fclose(f);

    game->map.pacmanStart = header.pacmanStart;
    game->map.pelletsRemaining = header.pelletsRemaining;
    game->map.pelletsInitial = header.pelletsInitial;

    game->level = header.level;
    game->score = header.score;
    game->lives = header.lives;
    game->pacman.pos = header.pacmanPos;
    game->pacman.dir = header.pacmanDir;
    game->pacman.pendingDir = header.pacmanPending;
    game->pacman.powered = header.pacmanPowered != 0;
    game->pacman.powerTimeLeft = header.pacmanPowerTimeLeft;
    game->pacman.moveTimer = header.pacmanMoveTimer;
    game->pelletsRemaining = header.pelletsRemaining;
    snprintf(game->currentMapPath, sizeof(game->currentMapPath), "%s", header.currentMapPath);

    game->running = true;
    game->paused = false;
    game->menu.status = MENU_HIDDEN;
    game->menu.pendingAction = MENU_ACTION_NONE;
    game->menu.selectedIndex = 0;

    return true;
}
