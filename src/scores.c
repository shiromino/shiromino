#include "scores.h"
#include "replay.h"

#include "sqlite3.h"

#include "zed_dbg.h"

#include <stdlib.h>
#include <stdio.h>

#define check_bind(db, bind_call) check((bind_call) == SQLITE_OK, "Could not bind parameter value: %s", sqlite3_errmsg((db)));

static const int MAX_PLAYER_NAME_LENGTH = 64;

void scoredb_init(struct scoredb *s, const char *filename)
{
    int ret = sqlite3_open_v2(filename, &s->db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    check(ret == SQLITE_OK, "Could not open/create sqlite database: %s", sqlite3_errmsg(s->db));

    const char enableForeignKeysSql[] =
        "PRAGMA foreign_keys = ON;";

    ret = sqlite3_exec(s->db, enableForeignKeysSql, NULL, NULL, NULL);
    check(ret == 0, "Could not enable foreign key constraints");

    const char createPlayerDbSql[] =
        "CREATE TABLE IF NOT EXISTS players ("
        "    playerId INTEGER PRIMARY KEY, "
        "    name VARCHAR(64) UNIQUE NOT NULL COLLATE NOCASE, "
        "    tetroCount INTEGER DEFAULT(0), "
        "    pentoCount INTEGER DEFAULT(0), "
        "    tetrisCount INTEGER DEFAULT(0)"
        ");";

    ret = sqlite3_exec(s->db, createPlayerDbSql, NULL, NULL, NULL);
    check(ret == 0, "Could not create players table: %s", sqlite3_errmsg(s->db));
    
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

    ret = sqlite3_exec(s->db, createTableSql, NULL, NULL, NULL);
    check(ret == 0, "Could not create scores table");

    log_info("Opened scoredb %s", filename);
    
    // TODO: Create a view with human-readable data
    
 error:
    return;
}

void scoredb_terminate(struct scoredb *s)
{
    sqlite3_close(s->db);
}

struct scoredb *scoredb_create(const char *filename)
{   
    struct scoredb *s = malloc(sizeof(struct scoredb));
    scoredb_init(s, filename);

    return s;
}

void scoredb_destroy(struct scoredb *s)
{
    scoredb_terminate(s);

    free(s);
}

void scoredb_create_player(struct scoredb *s, struct player *out_player, const char *playerName)
{
    const char insertPlayerSql[] =
        "INSERT OR IGNORE INTO players (name)"
        "VALUES (:playerName);";

    sqlite3_stmt *sql;
    check(sqlite3_prepare_v2(s->db, insertPlayerSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(s->db));

    size_t playerNameLength = strnlen(playerName, MAX_PLAYER_NAME_LENGTH);

    check(playerName != NULL && playerNameLength > 0, "Player name is invalid");
    check_bind(s->db, sqlite3_bind_text(sql,  sqlite3_bind_parameter_index(sql, ":playerName"), playerName, playerNameLength, SQLITE_STATIC));

    int ret = sqlite3_step(sql);
    check(ret == SQLITE_DONE, "Could not insert value into players table: %s", sqlite3_errmsg(s->db));

    log_info("Player \"%s\" is in players table", playerName);

    sqlite3_finalize(sql);

    const char selectPlayerSql[] =
        "SELECT playerId, name, tetroCount, pentoCount, tetrisCount "
        "FROM players "
        "WHERE name = :playerName;";
 
    check(sqlite3_prepare_v2(s->db, selectPlayerSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(s->db));
    
    check_bind(s->db, sqlite3_bind_text(sql,  sqlite3_bind_parameter_index(sql, ":playerName"), playerName, playerNameLength, SQLITE_STATIC));

    ret = sqlite3_step(sql);
    check(ret == SQLITE_ROW, "Could not get player \"%s\" from players table: %s", playerName, sqlite3_errmsg(s->db));

    out_player->playerId    = sqlite3_column_int(sql,  0);
    out_player->name        = sqlite3_column_text(sql, 1);
    out_player->tetroCount  = sqlite3_column_int(sql,  2);
    out_player->pentoCount  = sqlite3_column_int(sql,  3);
    out_player->tetrisCount = sqlite3_column_int(sql,  4);

 error:
    sqlite3_finalize(sql);
}

void scoredb_update_player(struct scoredb *s, struct player *p)
{
    const char updatePlayerSql[] =
        "UPDATE players "
        "    SET tetroCount = :tetroCount, "
        "        pentoCount = :pentoCount, "
        "       tetrisCount = :tetrisCount "
        "WHERE playerId = :playerId;";

    sqlite3_stmt *sql;
    check(sqlite3_prepare_v2(s->db, updatePlayerSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(s->db));
    
    check_bind(s->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":tetroCount"), p->tetroCount));
    check_bind(s->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":pentoCount"), p->pentoCount));
    check_bind(s->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":tetrisCount"), p->tetrisCount));
    check_bind(s->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":playerId"), p->playerId));

    const int ret = sqlite3_step(sql);
    check(ret == SQLITE_DONE, "Could not update players table for: %s", sqlite3_errmsg(s->db));

 error:
    sqlite3_finalize(sql);
}

#define REPLAY_DESCRIPTOR_BUF_SIZE 64
void scoredb_add(struct scoredb *s, struct player* p, struct replay *r)
{
    char replayDescriptor[REPLAY_DESCRIPTOR_BUF_SIZE];

    get_replay_descriptor(r, replayDescriptor, REPLAY_DESCRIPTOR_BUF_SIZE);

    const char insertSql[] =
        "INSERT INTO scores (mode, playerId, grade, startLevel, level, time, replay, date) "
        "VALUES (:mode, :playerId, :grade, :startLevel, :level, :time, :replay, strftime('%s', 'now'));";

    sqlite3_stmt *sql;
    check(sqlite3_prepare_v2(s->db, insertSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(s->db));
    
    size_t replayLen = 0;
    uint8_t *replayData = generate_raw_replay(r, &replayLen);
    
    check_bind(s->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":mode"),       r->mode));
    check_bind(s->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":playerId"),   p->playerId));
    check_bind(s->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":grade"),      r->grade));
    check_bind(s->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":startLevel"), r->starting_level));
    check_bind(s->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":level"),      r->ending_level));
    check_bind(s->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":time"),       r->time));
    check_bind(s->db, sqlite3_bind_blob(sql, sqlite3_bind_parameter_index(sql, ":replay"),     replayData, replayLen, SQLITE_STATIC));

    const int ret = sqlite3_step(sql);
    check(ret == SQLITE_DONE, "Could not insert value into scores table: %s", sqlite3_errmsg(s->db));

    log_info("Wrote replay (%I64u): %s", replayLen, replayDescriptor);
        
 error:
    sqlite3_finalize(sql);
}

int scoredb_get_replay_count(struct scoredb *s, struct player *p)
{
    int replayCount = 0;
    const char getReplayCountSql[] =
        "SELECT COUNT(*) "
        "FROM scores "
        "WHERE playerId = :playerId;";

    sqlite3_stmt *sql;
    check(sqlite3_prepare_v2(s->db, getReplayCountSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(s->db));

    check_bind(s->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":playerId"), p->playerId));

    const int ret = sqlite3_step(sql);
    check(ret == SQLITE_ROW, "Could not get replay count: %s", sqlite3_errmsg(s->db));
    
    replayCount = sqlite3_column_int(sql, 0);
    
 error:
    sqlite3_finalize(sql);
    
    return replayCount;
}

struct replay *scoredb_get_replay_list(struct scoredb *s, struct player *p, int *out_replayCount)
{
    const int replayCount = scoredb_get_replay_count(s, p);
    struct replay *replayList = malloc(sizeof(struct replay) * replayCount);
    
    // TODO: Pagination? Current interface expects a full list of replays
    const char getReplayListSql[] =
        "SELECT scoreId, mode, grade, startLevel, level, time, date "
        "FROM scores "
        "WHERE playerId = :playerId "
        "ORDER BY mode, level DESC, time;";

    sqlite3_stmt *sql;
    check(sqlite3_prepare_v2(s->db, getReplayListSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(s->db));
    
    check_bind(s->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":playerId"), p->playerId));
    
    for (int i = 0; i < replayCount; i++)
    {
        int ret = sqlite3_step(sql);
        check(ret == SQLITE_ROW, "Could not get replay: %s", sqlite3_errmsg(s->db));
        
        replayList[i].index          = sqlite3_column_int(sql, 0);
        replayList[i].mode           = sqlite3_column_int(sql, 1);
        replayList[i].grade          = sqlite3_column_int(sql, 2);
        replayList[i].starting_level = sqlite3_column_int(sql, 3);
        replayList[i].ending_level   = sqlite3_column_int(sql, 4);
        replayList[i].time           = sqlite3_column_int(sql, 5);
        replayList[i].date           = sqlite3_column_int(sql, 6);
    }
    
 error:
    sqlite3_finalize(sql);

    *out_replayCount = replayCount;
    return replayList;
}


void scoredb_get_full_replay(struct scoredb *s, struct replay *out_replay, int replay_id)
{
    const char *getReplaySql =
        "SELECT replay FROM scores "
        "WHERE scoreId = :scoreId;";
    
    sqlite3_stmt *sql;
    check(sqlite3_prepare_v2(s->db, getReplaySql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(s->db));
    
    check_bind(s->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":scoreId"), replay_id));

    const int ret = sqlite3_step(sql);
    check(ret == SQLITE_ROW, "Could not get replay: %s", sqlite3_errmsg(s->db));

    const int replayBufferLength = sqlite3_column_bytes(sql, 0);
    const uint8_t *replayBuffer = sqlite3_column_blob(sql, 0);
    
    read_replay_from_memory(out_replay, replayBuffer, replayBufferLength);
    
 error:
    sqlite3_finalize(sql);
}

void scoredb_get_full_replay_by_condition(struct scoredb *s, struct replay *out_replay, int mode)
{
    const char *getReplaySql =
        "SELECT replay FROM scores "
        "WHERE mode = :mode "
        "ORDER BY grade DESC, level DESC, time, date "
        "LIMIT 1;";
    
    sqlite3_stmt *sql;
    check(sqlite3_prepare_v2(s->db, getReplaySql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(s->db));
    
    check_bind(s->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":mode"), mode));
    
    int ret = sqlite3_step(sql);
    check(ret == SQLITE_ROW, "Could not get replay: %s", sqlite3_errmsg(s->db));
    
    int replayBufferLength = sqlite3_column_bytes(sql, 0);
    const uint8_t *replayBuffer = sqlite3_column_blob(sql, 0);
    
    read_replay_from_memory(out_replay, replayBuffer, replayBufferLength);
    
 error:
    sqlite3_finalize(sql);
}
