#pragma once
#include <string>
#include "sqlite3.h"

namespace Shiro {
    struct Player;

    namespace Records {
        const std::string filename = "shiromino.sqlite";
        struct List {
            sqlite3 *db;
        };
    }
}

//void scoredb_checkfill_default_sectiontimes(Shiro::Records::List *records);

void scoredb_init(Shiro::Records::List *records, const char *filename);
void scoredb_terminate(Shiro::Records::List *records);

Shiro::Records::List *scoredb_create(const char *filename);
void scoredb_destroy(Shiro::Records::List *records);

void scoredb_create_player(Shiro::Records::List *records, Shiro::Player *out_player, const char *playerName);
void scoredb_update_player(Shiro::Records::List *records, Shiro::Player *p);

void scoredb_add_live_sectiontime(Shiro::Records::List *records, Shiro::Player* p, int mode, int startlevel, int endlevel, int time);
void scoredb_clear_live_sectiontimes(Shiro::Records::List *records);

int scoredb_get_sectiontime(Shiro::Records::List *records, int mode, int startlevel);
void scoredb_add_sectiontime(Shiro::Records::List *records, Shiro::Player* p, int mode, int grade, int startlevel, int endlevel, int time);

void scoredb_add_replay(Shiro::Records::List *records, Shiro::Player* p, struct replay *r);

int scoredb_get_replay_count(Shiro::Records::List *records, Shiro::Player* p);

// Get list of replay descriptors (no replay data)
struct replay *scoredb_get_replay_list(Shiro::Records::List *records, Shiro::Player *p, int *out_replayCount);

void scoredb_delete_replay(Shiro::Records::List *records, int replay_id);
void scoredb_get_full_replay(Shiro::Records::List *records, struct replay *out_replay, int replay_id);
void scoredb_get_full_replay_by_condition(Shiro::Records::List *records, struct replay *out_replay, int mode);