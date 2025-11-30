#pragma once

#include <stdbool.h>
#include "entity.h"

#define MAP_ROWS 20
#define MAP_COLS 40

typedef struct {
    int rows;
    int cols;
    char* cells;          // rows * cols
    Position pacmanStart;
    int ghostCount;
    Position* ghostStarts;
    Position* portals;
    int portalCount;
    int pelletsInitial;
    int pelletsRemaining;
} Map;

bool map_load(Map* map, const char* path);
void map_free(Map* map);
char map_get(const Map* map, int row, int col);
void map_set(Map* map, int row, int col, char value);
bool map_in_bounds(const Map* map, int row, int col);
