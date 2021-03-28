#pragma once
#include "Player.h"
#include <sqlite3.h>
#include <string>
namespace Shiro::Records {
    const std::string filename = "shiromino.sqlite";
    struct List {
        sqlite3 *db;
    };
}
struct replay;

void scoredb_init(Shiro::Records::List *records, const char *filename);
void scoredb_terminate(Shiro::Records::List *records);

Shiro::Records::List *scoredb_create(const char *filename);
void scoredb_destroy(Shiro::Records::List *records);

void scoredb_create_player(Shiro::Records::List *records, Shiro::Player *out_player, const char *playerName);
void scoredb_update_player(Shiro::Records::List *records, Shiro::Player *p);

void scoredb_add(Shiro::Records::List *records, Shiro::Player* p, struct replay *r);

int scoredb_get_replay_count(Shiro::Records::List *records, Shiro::Player* p);

// Get list of replay descriptors (no replay data)
struct replay *scoredb_get_replay_list(Shiro::Records::List *records, Shiro::Player *p, int *out_replayCount);

void scoredb_get_full_replay(Shiro::Records::List *records, struct replay *out_replay, int replay_id);
void scoredb_get_full_replay_by_condition(Shiro::Records::List *records, struct replay *out_replay, int mode);