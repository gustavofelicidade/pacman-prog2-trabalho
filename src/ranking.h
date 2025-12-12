#pragma once

#include <stdbool.h>

#define RANKING_MAX_ENTRIES 10
#define RANKING_NAME_LEN 16

typedef struct {
    char name[RANKING_NAME_LEN];
    int score;
} RankingEntry;

typedef struct {
    RankingEntry entries[RANKING_MAX_ENTRIES];
} Ranking;

void ranking_init(Ranking* ranking);
bool ranking_load(Ranking* ranking, const char* path);
bool ranking_save(const Ranking* ranking, const char* path);
int ranking_position_for_score(const Ranking* ranking, int score);
void ranking_insert(Ranking* ranking, const char* name, int score);
