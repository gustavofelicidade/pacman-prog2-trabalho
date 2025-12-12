#include "ranking.h"
#include <stdio.h>
#include <string.h>

void ranking_init(Ranking* ranking) {
    for (int i = 0; i < RANKING_MAX_ENTRIES; i++) {
        snprintf(ranking->entries[i].name, RANKING_NAME_LEN, "---");
        ranking->entries[i].score = 0;
    }
}

bool ranking_load(Ranking* ranking, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        ranking_init(ranking);
        return false;
    }
    size_t read = fread(ranking->entries, sizeof(RankingEntry), RANKING_MAX_ENTRIES, f);
    fclose(f);
    if (read != RANKING_MAX_ENTRIES) {
        ranking_init(ranking);
        return false;
    }
    return true;
}

bool ranking_save(const Ranking* ranking, const char* path) {
    FILE* f = fopen(path, "wb");
    if (!f) return false;
    size_t written = fwrite(ranking->entries, sizeof(RankingEntry), RANKING_MAX_ENTRIES, f);
    fclose(f);
    return written == RANKING_MAX_ENTRIES;
}

int ranking_position_for_score(const Ranking* ranking, int score) {
    for (int i = 0; i < RANKING_MAX_ENTRIES; i++) {
        if (score > ranking->entries[i].score) {
            return i;
        }
    }
    return -1;
}

void ranking_insert(Ranking* ranking, const char* name, int score) {
    int pos = ranking_position_for_score(ranking, score);
    if (pos < 0) return;
    for (int i = RANKING_MAX_ENTRIES - 1; i > pos; i--) {
        ranking->entries[i] = ranking->entries[i - 1];
    }
    RankingEntry entry = {0};
    strncpy(entry.name, name, RANKING_NAME_LEN - 1);
    entry.name[RANKING_NAME_LEN - 1] = '\0';
    entry.score = score;
    ranking->entries[pos] = entry;
}
