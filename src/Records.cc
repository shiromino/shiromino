#include "Records.h"
#include <cassert>
#include <cinttypes>
#include <cstdlib>
#include <iostream>
#include "Debug.h"
#include "Player.h"
#include "replay.h"
#include "sqlite3.h"
#define check_bind(db, bind_call) check((bind_call) == SQLITE_OK, "Could not bind parameter value: %s", sqlite3_errmsg((db)))

static const int MAX_PLAYER_NAME_LENGTH = 64;

/*
void scoredb_checkfill_default_sectiontimes(Shiro::Records::List *records)
{
    sqlite3_stmt *sql;
    const char *getSectionTimeSql = R"(
        SELECT
            time
        FROM
            sectionTimes
        WHERE
            startlevel = :startLevel
        AND
            mode = MODE_G2_DEATH;
    )";

    for(int i = 0; i < 10; i++)
    {
        check(sqlite3_prepare_v2(records->db, getSectionTimeSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":startLevel"), i * 100));

        int returnValue = sqlite3_step(sql);
        if(returnValue != SQLITE_ROW)
        {
            scoredb_add_sectiontime(records, )
        }
    }
}
*/

void scoredb_init(Shiro::Records::List *records, const char *filename) {
        int returnValue = sqlite3_open_v2(filename, &records->db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
        check(returnValue == SQLITE_OK, "Could not open/create sqlite database: %s", sqlite3_errmsg(records->db));

        const char enableForeignKeysSql[] = "PRAGMA foreign_keys = ON;";

        returnValue = sqlite3_exec(records->db, enableForeignKeysSql, NULL, NULL, NULL);
        check(returnValue == 0, "Could not enable foreign key constraints");

        const char createPlayerDbSql[] = R"(
            CREATE TABLE IF NOT EXISTS players (
                playerId INTEGER PRIMARY KEY,
                name VARCHAR(64) UNIQUE NOT NULL COLLATE NOCASE,
                tetroCount INTEGER DEFAULT(0),
                pentoCount INTEGER DEFAULT(0),
                tetrisCount INTEGER DEFAULT(0)
            );
        )";

        returnValue = sqlite3_exec(records->db, createPlayerDbSql, NULL, NULL, NULL);
        check(returnValue == 0, "Could not create players table: %s", sqlite3_errmsg(records->db));

        // TODO: Actually design the database. Replay table? Player table + related columns? Indexes?
        const char createTableSql[] = R"(
            CREATE TABLE IF NOT EXISTS scores (
                scoreId INTEGER PRIMARY KEY,
                playerId INTEGER NOT NULL,
                mode INTEGER,
                grade INTEGER,
                startlevel INTEGER,
                level INTEGER,
                time INTEGER,
                replay BLOB,
                date INTEGER,
                FOREIGN KEY(playerId) REFERENCES players(playerId)
            );
        )";
        returnValue = sqlite3_exec(records->db, createTableSql, NULL, NULL, NULL);
        check(returnValue == 0, "Could not create scores table");

        const char createSectionTimesTableSql[] = R"(
            CREATE TABLE IF NOT EXISTS sectionTimes (
                playerId INTEGER NOT NULL,
                mode INTEGER,
                grade INTEGER,
                startlevel INTEGER,
                endlevel INTEGER,
                time INTEGER,
                date INTEGER,
                FOREIGN KEY(playerId) REFERENCES players(playerId)
            );
        )";
        returnValue = sqlite3_exec(records->db, createSectionTimesTableSql, NULL, NULL, NULL);
        check(returnValue == 0, "Could not create scores table");

        const char createLiveSectionTimesTableSql[] = R"(
            CREATE TABLE IF NOT EXISTS liveSectionTimes (
                playerId INTEGER NOT NULL,
                mode INTEGER,
                startlevel INTEGER,
                endlevel INTEGER,
                time INTEGER,
                date INTEGER,
                FOREIGN KEY(playerId) REFERENCES players(playerId)
            );
        )";
        returnValue = sqlite3_exec(records->db, createLiveSectionTimesTableSql, NULL, NULL, NULL);
        check(returnValue == 0, "Could not create scores table");

        std::cerr << "Opened record list \"" << filename << "\"" << std::endl;

        // TODO: Create a view with human-readable data
}

void scoredb_terminate(Shiro::Records::List *records) {
    sqlite3_close(records->db);
}

Shiro::Records::List *scoredb_create(const char *filename) {
    Shiro::Records::List *records = (Shiro::Records::List *) malloc(sizeof(Shiro::Records::List));
    scoredb_init(records, filename);
    return records;
}

void scoredb_destroy(Shiro::Records::List *records) {
    scoredb_terminate(records);

    free(records);
}

void scoredb_create_player(Shiro::Records::List *records, Shiro::Player *out_player, const char *playerName) {
    sqlite3_stmt *sql;
    const char insertPlayerSql[] = R"(
        INSERT OR IGNORE INTO
            players (name)
        VALUES
            (:playerName);
    )";

    check(sqlite3_prepare_v2(records->db, insertPlayerSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

    size_t playerNameLength = 0;
    while (playerName[playerNameLength] != '\0' && ++playerNameLength < MAX_PLAYER_NAME_LENGTH);

    check(playerName != NULL && playerNameLength > 0, "Player name is invalid");
    check_bind(records->db, sqlite3_bind_text(sql,  sqlite3_bind_parameter_index(sql, ":playerName"), playerName, (int)playerNameLength, SQLITE_STATIC));

    int returnValue = sqlite3_step(sql);
    check(returnValue == SQLITE_DONE, "Could not insert value into players table: %s", sqlite3_errmsg(records->db));

    std::cerr << "Player \"" << playerName << "\" is in players table" << std::endl;

    sqlite3_finalize(sql);

    const char selectPlayerSql[] = R"(
        SELECT
            playerId,
            name,
            tetroCount,
            pentoCount,
            tetrisCount
        FROM
            players
        WHERE
            name = :playerName;
    )";
    check(sqlite3_prepare_v2(records->db, selectPlayerSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

    check_bind(records->db, sqlite3_bind_text(sql,  sqlite3_bind_parameter_index(sql, ":playerName"), playerName, (int)playerNameLength, SQLITE_STATIC));

    returnValue = sqlite3_step(sql);
    check(returnValue == SQLITE_ROW, "Could not get player \"%s\" from players table: %s", playerName, sqlite3_errmsg(records->db));

    out_player->playerId = sqlite3_column_int(sql,  0);
    out_player->name = sqlite3_column_text(sql, 1);
    out_player->tetroCount = sqlite3_column_int(sql,  2);
    out_player->pentoCount = sqlite3_column_int(sql,  3);
    out_player->tetrisCount = sqlite3_column_int(sql,  4);
    sqlite3_finalize(sql);
}

void scoredb_update_player(Shiro::Records::List *records, Shiro::Player *p) {
    sqlite3_stmt *sql;
    const char updatePlayerSql[] = R"(
        UPDATE
            players
        SET
            tetroCount = :tetroCount,
            pentoCount = :pentoCount,
            tetrisCount = :tetrisCount
        WHERE
            playerId = :playerID;
    )";

    check(sqlite3_prepare_v2(records->db, updatePlayerSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":tetroCount"), p->tetroCount));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":pentoCount"), p->pentoCount));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":tetrisCount"), p->tetrisCount));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":playerID"), p->playerId));

    const int returnValue = sqlite3_step(sql);
    check(returnValue == SQLITE_DONE, "Could not update players table for: %s", sqlite3_errmsg(records->db));
    sqlite3_finalize(sql);
}

void scoredb_add_live_sectiontime(Shiro::Records::List *records, Shiro::Player* p, int mode, int startlevel, int endlevel, int time)
{
    sqlite3_stmt *sql;
    const char insertSql[] = R"(
        INSERT INTO liveSectionTimes
            (playerId, mode, startlevel, endlevel, time, date)
        VALUES
            (:playerID, :mode, :startLevel, :endlevel, :time, strftime('%s', 'now'));
    )";

    check(sqlite3_prepare_v2(records->db, insertSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":playerID"), p->playerId));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":mode"), mode));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":startLevel"), startlevel));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":endlevel"), endlevel));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":time"), time));

    const int returnValue = sqlite3_step(sql);
    check(returnValue == SQLITE_DONE, "Could not insert value into section times table: %s", sqlite3_errmsg(records->db));

    sqlite3_finalize(sql);
}

void scoredb_clear_live_sectiontimes(Shiro::Records::List *records)
{
    sqlite3_stmt *sql;
    const char *deleteLiveSectionTimesSql = R"(
        DELETE FROM liveSectionTimes;
    )";

    check(sqlite3_prepare_v2(records->db, deleteLiveSectionTimesSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

    const int returnValue = sqlite3_step(sql);
    check(returnValue == SQLITE_DONE, "Could not clear live section times table: %s", sqlite3_errmsg(records->db));

    sqlite3_finalize(sql);
}

int scoredb_get_sectiontime(Shiro::Records::List *records, int mode, int startlevel)
{
    sqlite3_stmt *sql;
    const char *getSectionTimeSql = R"(
        SELECT
            time
        FROM
            sectionTimes
        WHERE
            startlevel = :startLevel
        AND
            mode = :mode;
    )";

    check(sqlite3_prepare_v2(records->db, getSectionTimeSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":mode"), mode));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":startLevel"), startlevel));

    int returnValue = sqlite3_step(sql);
    if(returnValue == SQLITE_ROW)
    {
        returnValue = sqlite3_column_int(sql, 0);
    }
    else {
        returnValue = -1;
    }

    sqlite3_finalize(sql);

    return returnValue;
}

void scoredb_add_sectiontime(Shiro::Records::List *records, Shiro::Player* p, int mode, int grade, int startlevel, int endlevel, int time)
{
    sqlite3_stmt *sql;
    const char insertSql[] = R"(
        INSERT INTO sectionTimes
            (playerId, mode, grade, startlevel, endlevel, time, date)
        VALUES
            (:playerID, :mode, :grade, :startLevel, :endlevel, :time, strftime('%s', 'now'));
    )";

    const char *getSectionTimeSql = R"(
        SELECT
            mode,
            startlevel,
            time
        FROM
            sectionTimes
        WHERE
            startlevel = :startLevel
        AND
            mode = :mode;
    )";

    check(sqlite3_prepare_v2(records->db, getSectionTimeSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":mode"), mode));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":startLevel"), startlevel));

    bool preexisted = false;

    int returnValue = sqlite3_step(sql);
    if(returnValue == SQLITE_ROW)
    {
        preexisted = true;
    }

    sqlite3_finalize(sql);

    if(preexisted)
    {
        const char *updateSectionTimeSql = R"(
            UPDATE sectionTimes
            SET
                time = :time
            WHERE
                startlevel = :startLevel
            AND
                mode = :mode;
        )";

        check(sqlite3_prepare_v2(records->db, updateSectionTimeSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":time"), time));
        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":mode"), mode));
        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":startLevel"), startlevel));

        const int returnValue = sqlite3_step(sql);
        check(returnValue == SQLITE_DONE, "Could not insert value into section times table: %s", sqlite3_errmsg(records->db));

        sqlite3_finalize(sql);
    }
    else
    {
        check(sqlite3_prepare_v2(records->db, insertSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":playerID"), p->playerId));
        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":mode"), mode));
        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":grade"), grade));
        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":startLevel"), startlevel));
        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":endlevel"), endlevel));
        check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":time"), time));

        const int returnValue = sqlite3_step(sql);
        check(returnValue == SQLITE_DONE, "Could not insert value into section times table: %s", sqlite3_errmsg(records->db));

        sqlite3_finalize(sql);
    }
}

void scoredb_add_replay(Shiro::Records::List *records, Shiro::Player* p, struct replay *r) {
    sqlite3_stmt *sql;
    std::string replayDescriptor = get_replay_descriptor(r);
    const char insertSql[] = R"(
        INSERT INTO scores
            (mode, playerId, grade, startLevel, level, time, replay, date)
        VALUES
            (:mode, :playerID, :grade, :startLevel, :level, :time, :replay, strftime('%s', 'now'));
    )";

    check(sqlite3_prepare_v2(records->db, insertSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

    size_t replayLength = 0;
    uint8_t *replayData = generate_raw_replay(r, &replayLength);

    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":mode"), r->mode));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":playerID"), p->playerId));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":grade"), r->grade));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":startLevel"), r->starting_level));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":level"), r->ending_level));
    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":time"), int(r->time)));
    check_bind(records->db, sqlite3_bind_blob(sql, sqlite3_bind_parameter_index(sql, ":replay"), replayData, (int)replayLength, SQLITE_STATIC));

    const int returnValue = sqlite3_step(sql);
    check(returnValue == SQLITE_DONE, "Could not insert value into scores table: %s", sqlite3_errmsg(records->db));
    dispose_raw_replay(replayData);

    std::cerr << "Wrote replay " << replayLength << ": " << replayDescriptor << std::endl;
    sqlite3_finalize(sql);
}

int scoredb_get_replay_count(Shiro::Records::List *records, Shiro::Player *p) {
    sqlite3_stmt *sql;
    int replayCount = 0;
    const char getReplayCountSql[] = R"(
        SELECT
            COUNT(*)
        FROM
            scores
        WHERE
            playerId = :playerID;
    )";

    check(sqlite3_prepare_v2(records->db, getReplayCountSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

    check_bind(records->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":playerID"), p->playerId));

    const int returnValue = sqlite3_step(sql);
    check(returnValue == SQLITE_ROW, "Could not get replay count: %s", sqlite3_errmsg(records->db));

    replayCount = sqlite3_column_int(sql, 0);
    sqlite3_finalize(sql);
    return replayCount;
}

struct replay *scoredb_get_replay_list(Shiro::Records::List *records, Shiro::Player *p, int *out_replayCount) {
    sqlite3_stmt *sql;
    const std::size_t replayCount = scoredb_get_replay_count(records, p);
    struct replay *replayList = new struct replay[replayCount];
    assert(replayList != nullptr);
    // TODO: Pagination? Current interface expects a full list of replays
    const char getReplayListSql[] = R"(
        SELECT
            scoreId,
            mode,
            grade,
            startLevel,
            level,
            time,
            date
        FROM
            scores
        WHERE
            playerId = :playerID
        ORDER BY
            mode,
            level DESC,
            time
        )";

    check(sqlite3_prepare_v2(records->db, getReplayListSql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

    check_bind(records->db, sqlite3_bind_int(sql,  sqlite3_bind_parameter_index(sql, ":playerID"), p->playerId));

    for (std::size_t i = 0; i < replayCount; i++) {
        int returnValue = sqlite3_step(sql);
        check(returnValue == SQLITE_ROW, "Could not get replay: %s", sqlite3_errmsg(records->db));

        replayList[i].index          = sqlite3_column_int(sql, 0);
        replayList[i].mode           = sqlite3_column_int(sql, 1);
        replayList[i].grade          = sqlite3_column_int(sql, 2);
        replayList[i].starting_level = sqlite3_column_int(sql, 3);
        replayList[i].ending_level   = sqlite3_column_int(sql, 4);
        replayList[i].time           = sqlite3_column_int(sql, 5);
        replayList[i].date           = sqlite3_column_int(sql, 6);
    }
    sqlite3_finalize(sql);

    *out_replayCount = int(replayCount);
    return replayList;
}

void scoredb_delete_replay(Shiro::Records::List *records, int replay_id)
{
    sqlite3_stmt *sql;
    const char *deleteReplaySql = R"(
        DELETE FROM scores
        WHERE scoreId = :scoreID;
    )";

    check(sqlite3_prepare_v2(records->db, deleteReplaySql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":scoreID"), replay_id));

    const int returnValue = sqlite3_step(sql);
    check(returnValue == SQLITE_DONE, "Could not get replay: %s", sqlite3_errmsg(records->db));

    sqlite3_finalize(sql);
}

void scoredb_get_full_replay(Shiro::Records::List *records, struct replay *out_replay, int replay_id) {
    sqlite3_stmt *sql;
    const char *getReplaySql = R"(
        SELECT
            replay
        FROM
            scores
        WHERE
            scoreId = :scoreID;
    )";

    check(sqlite3_prepare_v2(records->db, getReplaySql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":scoreID"), replay_id));

    const int returnValue = sqlite3_step(sql);
    check(returnValue == SQLITE_ROW, "Could not get replay: %s", sqlite3_errmsg(records->db));

    const uint8_t *replayBuffer = (const uint8_t *)sqlite3_column_blob(sql, 0);

    read_replay_from_memory(out_replay, replayBuffer);
    sqlite3_finalize(sql);
}

void scoredb_get_full_replay_by_condition(Shiro::Records::List *records, struct replay *out_replay, int mode) {
    sqlite3_stmt *sql;
    const char *getReplaySql = R"(
        SELECT
            replay
        FROM
            scores
        WHERE
            mode = :mode
        ORDER BY
            grade DESC,
            level DESC,
            time,
            date
        LIMIT 1;
    )";

    check(sqlite3_prepare_v2(records->db, getReplaySql, -1, &sql, NULL) == SQLITE_OK, "Could not prepare sql statement: %s", sqlite3_errmsg(records->db));

    check_bind(records->db, sqlite3_bind_int(sql, sqlite3_bind_parameter_index(sql, ":mode"), mode));

    int returnValue = sqlite3_step(sql);
    check(returnValue == SQLITE_ROW, "Could not get replay: %s", sqlite3_errmsg(records->db));

    const uint8_t *replayBuffer = (const uint8_t *)sqlite3_column_blob(sql, 0);

    read_replay_from_memory(out_replay, replayBuffer);
    sqlite3_finalize(sql);
}
