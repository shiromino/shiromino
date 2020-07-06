#pragma once
#include "Player.h"
#include <sqlite3.h>
namespace Shiro {
    struct RecordList {
        sqlite3 *db;
    };
}
struct replay;

void scoredb_init(Shiro::RecordList *records, const char *filename);
void scoredb_terminate(Shiro::RecordList *records);

Shiro::RecordList *scoredb_create(const char *filename);
void scoredb_destroy(Shiro::RecordList *records);

void scoredb_create_player(Shiro::RecordList *records, Shiro::Player *out_player, const char *playerName);
void scoredb_update_player(Shiro::RecordList *records, Shiro::Player *p);

void scoredb_add(Shiro::RecordList *records, Shiro::Player* p, struct replay *r);

int scoredb_get_replay_count(Shiro::RecordList *records, Shiro::Player* p);

// Get list of replay descriptors (no replay data)
struct replay *scoredb_get_replay_list(Shiro::RecordList *records, Shiro::Player *p, int *out_replayCount);

void scoredb_get_full_replay(Shiro::RecordList *records, struct replay *out_replay, int replay_id);
void scoredb_get_full_replay_by_condition(Shiro::RecordList *records, struct replay *out_replay, int mode);