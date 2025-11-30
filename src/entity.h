#pragma once

#include <stdbool.h>

typedef struct {
    int row;
    int col;
} Position;

typedef enum {
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef struct {
    Position pos;
    Direction dir;
    Direction pendingDir;
    bool powered;
    float powerTimeLeft;
} Pacman;

typedef struct {
    Position pos;
    Direction dir;
    bool vulnerable;
    float vulnerableTimeLeft;
    bool alive;
} Ghost;
