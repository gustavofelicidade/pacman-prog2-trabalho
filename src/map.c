#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int idx(const Map* map, int row, int col) {
    return row * map->cols + col;
}

bool map_in_bounds(const Map* map, int row, int col) {
    return row >= 0 && row < map->rows && col >= 0 && col < map->cols;
}

char map_get(const Map* map, int row, int col) {
    if (!map_in_bounds(map, row, col)) return ' ';
    return map->cells[idx(map, row, col)];
}

void map_set(Map* map, int row, int col, char value) {
    if (!map_in_bounds(map, row, col)) return;
    map->cells[idx(map, row, col)] = value;
}

bool map_load(Map* map, const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return false;

    map->rows = MAP_ROWS;
    map->cols = MAP_COLS;
    int total = map->rows * map->cols;
    map->cells = (char*)malloc(total);
    if (!map->cells) {
        fclose(f);
        return false;
    }

    map->ghostStarts = NULL;
    map->portals = NULL;
    map->ghostCount = 0;
    map->portalCount = 0;
    map->pelletsInitial = 0;
    map->pelletsRemaining = 0;

    char line[128];
    int r = 0;
    while (r < map->rows && fgets(line, sizeof(line), f)) {
        for (int c = 0; c < map->cols; c++) {
            char ch = line[c];
            map_set(map, r, c, ch);
            if (ch == 'P') {
                map->pacmanStart.row = r;
                map->pacmanStart.col = c;
            } else if (ch == 'F') {
                map->ghostCount++;
            } else if (ch == 'T') {
                map->portalCount++;
            } else if (ch == '.' || ch == 'o') {
                map->pelletsInitial++;
            }
        }
        r++;
    }

    fclose(f);

    if (map->ghostCount > 0) {
        map->ghostStarts = (Position*)malloc(sizeof(Position) * map->ghostCount);
    }
    if (map->portalCount > 0) {
        map->portals = (Position*)malloc(sizeof(Position) * map->portalCount);
    }

    int g = 0;
    int p = 0;
    for (int row = 0; row < map->rows; row++) {
        for (int col = 0; col < map->cols; col++) {
            char ch = map_get(map, row, col);
            if (ch == 'F' && map->ghostStarts) {
                map->ghostStarts[g].row = row;
                map->ghostStarts[g].col = col;
                g++;
            } else if (ch == 'T' && map->portals) {
                map->portals[p].row = row;
                map->portals[p].col = col;
                p++;
            }
        }
    }

    map->pelletsRemaining = map->pelletsInitial;
    return true;
}

void map_free(Map* map) {
    free(map->cells);
    free(map->ghostStarts);
    free(map->portals);
    map->cells = NULL;
    map->ghostStarts = NULL;
    map->portals = NULL;
    map->ghostCount = 0;
    map->portalCount = 0;
    map->pelletsInitial = 0;
    map->pelletsRemaining = 0;
    map->rows = 0;
    map->cols = 0;
}
