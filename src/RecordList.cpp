#include "RecordList.h"
#include "Debug.hpp"
#include "replay.h"
#include "Player.h"
#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sqlite3.h>
#define check_bind(db, bind_call) check((bind_call) == SQLITE_OK, "Could not bind parameter value: %s", sqlite3_errmsg((db)))

static const int MAX_PLAYER_NAME_LENGTH = 64;

void scoredb_init(Shiro::RecordList *records, const char *filename)
{
    try {
        int ret = sqlite3_open_v2(filename, &records->db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
        check(ret == SQLITE_OK, "Could not open/create sqlite database: %s", sqlite3_errmsg(records->db));

        const char enableForeignKeysSql[] =
            "PRAGMA foreign_keys = ON;";

        ret = sqlite3_exec(records->db, enableForeignKeysSql, NULL, NULL, NULL);
        check(ret == 0, "Could not enable foreign key constraints");

        const char createPlayerDbSql[] =
            "CREATE TABLE IF NOT EXISTS players ("
            "    playerId INTEGER PRIMARY KEY, "
            "    name VARCHAR(64) UNIQUE NOT NULL COLLATE NOCASE, "
            "    tetroCount INTEGER DEFAULT(0), "
            "    pentoCount INTEGER DEFAULT(0), "
            "    tetrisCount INTEGER DEFAULT(0)"
            ");";

        ret = sqlite3_exec(records->db, createPlayerDbSql, NULL, NULL, NULL);
        check(ret == 0, "Could not create players table: %s", sqlite3_errmsg(records->db));

        // TODO: Actually design the database. Replay table? Player table + related columns? Indexes?
        const char createTableSql[] =
            "CREATE TABLE IF NOT EXISTS scores ("
            "    scoreId INTEGER PRIMARY KEY, "
            "    playerId INTEGER NOT NULL, "
            "    mode INTEGER, "
            "    grade INTEGER, "
            "    startlevel INTEGER, "
            "    level INTEGER, "
            "    time INTEGER, "
            "    replay BLOB, "
            "    date INTEGER, "
            "    FOREIGN KEY(playerId) REFERENCES players(playerId) "
            ");";

        ret = sqlite3_exec(records->db, createTableSql, NULL, NULL, NULL);
        check(ret == 0, "Could not create scores table");

        std::cerr << "Opened record list \"" << filename << "\"" << std::endl;

        // TODO: Create a view with human-readable data
    }
    catch (const std::logic_error& error) {
    }
}

void scoredb_terminate(Shiro::RecordList *records)
{
    sqlite3_close(records->db);
}

Shiro::RecordList *scoredb_create(const char *filename)
{
    Shiro::RecordList *records = (Shiro::RecordList *) malloc(sizeof(Shiro::RecordList));
    scoredb_init(records, filename);

    return records;
}

void scoredb_destroy(Shiro::RecordList *records)
{
    scoredb_terminate(records);

    free(records);
}

void scoredb_create_player(Shiro::RecordList *records, Shiro::Player *out_player, const char *playerName)
{
    sqlite3_stmt *sql;
    try {
        const char insertPlayerSql[] =
            "INSERT OR IGNORE INTO players (name)"
            "VALUES (:playerName);";

        check(sqlite3_prepare_v2(records->db, insertPlayerSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

        size_t playerNameLength = 0;
        while (playerName[playerNameLength] != '\0' && ++playerNameLength < MAX_PLAYER_NAME_LENGTH);

        check(playerName != NULL && playerNameLength > 0, "Player name is invalid");
        check_bind(records->db, sqlite3_bind_text(sql,  sqlite3_bind_parameter_index(sql, ":playerName"), playerName, (int)playerNameLength, SQLITE_STATIC));

        int ret = sqlite3_step(sql);
        check(ret == SQLITE_DONE, "Could not insert value into players table: %s", sqlite3_errmsg(records->db));

        std::cerr << "Player \"" << playerName << "\" is in players table" << std::endl;

        sqlite3_finalize(sql);

        const char selectPlayerSql[] =
            "SELECT playerId, name, tetroCount, pentoCount, tetrisCount "
            "FROM players "
            "WHERE name = :playerName;";

        check(sqlite3_prepare_v2(records->db, selectPlayerSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

        check_bind(records->db, sqlite3_bind_text(sql,  sqlite3_bind_parameter_index(sql, ":playerName"), playerName, (int)playerNameLength, SQLITE_STATIC));

        ret = sqlite3_step(sql);
        check(ret == SQLITE_ROW, "Could not get player \"%s\" from players table: %s", playerName, sqlite3_errmsg(records->db));

        out_player->playerId    = sqlite3_column_int(sql,  0);
        out_player->name        = sqlite3_column_text(sql, 1);
        out_player->tetroCount  = sqlite3_column_int(sql,  2);
        out_player->pentoCount  = sqlite3_column_int(sql,  3);
        out_player->tetrisCount = sqlite3_column_int(sql,  4);
    }
    catch (const std::logic_error& error) {
    }

    sqlite3_finalize(sql);
}

void scoredb_update_player(Shiro::RecordList *records, Shiro::Player *p)
{
    sqlite3_stmt *sql;
    try {
        const char updatePlayerSql[] =
            "UPDATE players "
            "    SET tetroCount = :tetroCount, "
            "        pentoCount = :pentoCount, "
            "       tetrisCount = :tetrisCount "
            "WHERE playerId = :playerId;";

        check(sqlite3_prepare_v2(records->db, updatePlayerSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":tetroCount"), p->tetroCount));
        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":pentoCount"), p->pentoCount));
        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":tetrisCount"), p->tetrisCount));
        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":playerId"), p->playerId));

        const int ret = sqlite3_step(sql);
        check(ret == SQLITE_DONE, "Could not update players table for: %s", sqlite3_errmsg(records->db));
    }
    catch (const std::logic_error& error) {
    }

    sqlite3_finalize(sql);
}

void scoredb_add(Shiro::RecordList *records, Shiro::Player* p, struct replay *r)
{
    sqlite3_stmt *sql;
    try {
        std::string replayDescriptor = get_replay_descriptor(r);

        const char insertSql[] =
            "INSERT INTO scores (mode, playerId, grade, startLevel, level, time, replay, date) "
            "VALUES (:mode, :playerId, :grade, :startLevel, :level, :time, :replay, strftime('%s', 'now'));";

        check(sqlite3_prepare_v2(records->db, insertSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

        size_t replayLen = 0;
        uint8_t *replayData = generate_raw_replay(r, &replayLen);

        check_bind(records->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":mode"),       r->mode));
        check_bind(records->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":playerId"),   p->playerId));
        check_bind(records->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":grade"),      r->grade));
        check_bind(records->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":startLevel"), r->starting_level));
        check_bind(records->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":level"),      r->ending_level));
        check_bind(records->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":time"),       r->time));
        check_bind(records->db, sqlite3_bind_blob(sql, sqlite3_bind_parameter_index(sql, ":replay"),     replayData, (int)replayLen, SQLITE_STATIC));

        const int ret = sqlite3_step(sql);
        check(ret == SQLITE_DONE, "Could not insert value into scores table: %s", sqlite3_errmsg(records->db));

        std::cerr << "Wrote replay " << replayLen << ": " << replayDescriptor << std::endl;
    }
    catch (const std::logic_error& error) {
    }

    sqlite3_finalize(sql);
}

int scoredb_get_replay_count(Shiro::RecordList *records, Shiro::Player *p)
{
    sqlite3_stmt *sql;
    int replayCount = 0;
    try {
        const char getReplayCountSql[] =
            "SELECT COUNT(*) "
            "FROM scores "
            "WHERE playerId = :playerId;";

        check(sqlite3_prepare_v2(records->db, getReplayCountSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

        check_bind(records->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":playerId"), p->playerId));

        const int ret = sqlite3_step(sql);
        check(ret == SQLITE_ROW, "Could not get replay count: %s", sqlite3_errmsg(records->db));

        replayCount = sqlite3_column_int(sql, 0);
    }
    catch (const std::logic_error& error) {
    }

    sqlite3_finalize(sql);

    return replayCount;
}

struct replay *scoredb_get_replay_list(Shiro::RecordList *records, Shiro::Player *p, int *out_replayCount)
{
    sqlite3_stmt *sql;
    const int replayCount = scoredb_get_replay_count(records, p);
    struct replay *replayList = (struct replay *) malloc(sizeof(struct replay) * replayCount);
    try {
        // TODO: Pagination? Current interface expects a full list of replays
        const char getReplayListSql[] =
            "SELECT scoreId, mode, grade, startLevel, level, time, date "
            "FROM scores "
            "WHERE playerId = :playerId "
            "ORDER BY mode, level DESC, time;";

        check(sqlite3_prepare_v2(records->db, getReplayListSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

        check_bind(records->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":playerId"), p->playerId));

        for (int i = 0; i < replayCount; i++)
        {
            int ret = sqlite3_step(sql);
            check(ret == SQLITE_ROW, "Could not get replay: %s", sqlite3_errmsg(records->db));

            replayList[i].index          = sqlite3_column_int(sql, 0);
            replayList[i].mode           = sqlite3_column_int(sql, 1);
            replayList[i].grade          = sqlite3_column_int(sql, 2);
            replayList[i].starting_level = sqlite3_column_int(sql, 3);
            replayList[i].ending_level   = sqlite3_column_int(sql, 4);
            replayList[i].time           = sqlite3_column_int(sql, 5);
            replayList[i].date           = sqlite3_column_int(sql, 6);
        }
    }
    catch (const std::logic_error& error) {
    }
    sqlite3_finalize(sql);

    *out_replayCount = replayCount;
    return replayList;
}


void scoredb_get_full_replay(Shiro::RecordList *records, struct replay *out_replay, int replay_id)
{
    sqlite3_stmt *sql;
    try {
        const char *getReplaySql =
            "SELECT replay FROM scores "
            "WHERE scoreId = :scoreId;";

        check(sqlite3_prepare_v2(records->db, getReplaySql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":scoreId"), replay_id));

        const int ret = sqlite3_step(sql);
        check(ret == SQLITE_ROW, "Could not get replay: %s", sqlite3_errmsg(records->db));

        const int replayBufferLength = sqlite3_column_bytes(sql, 0);
        const uint8_t *replayBuffer = (const uint8_t *)sqlite3_column_blob(sql, 0);

        read_replay_from_memory(out_replay, replayBuffer, replayBufferLength);
    }
    catch (const std::logic_error& error) {
    }

    sqlite3_finalize(sql);
}

void scoredb_get_full_replay_by_condition(Shiro::RecordList *records, struct replay *out_replay, int mode)
{
    sqlite3_stmt *sql;
    try {
        const char *getReplaySql =
            "SELECT replay FROM scores "
            "WHERE mode = :mode "
            "ORDER BY grade DESC, level DESC, time, date "
            "LIMIT 1;";

        check(sqlite3_prepare_v2(records->db, getReplaySql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":mode"), mode));

        int ret = sqlite3_step(sql);
        check(ret == SQLITE_ROW, "Could not get replay: %s", sqlite3_errmsg(records->db));

        int replayBufferLength = sqlite3_column_bytes(sql, 0);
        const uint8_t *replayBuffer = (const uint8_t *)sqlite3_column_blob(sql, 0);

        read_replay_from_memory(out_replay, replayBuffer, replayBufferLength);
    }
    catch (const std::logic_error& error) {
    }

    sqlite3_finalize(sql);
}