#ifndef __SCOREDB_H_
#define __SCOREDB_H_

struct sqlite3;
typedef struct sqlite3 sqlite3;

struct replay;
struct player;

struct scoredb {
    sqlite3 *db;
};

void scoredb_init(struct scoredb *s, const char *filename);
void scoredb_terminate(struct scoredb *s);

struct scoredb *scoredb_create(const char *filename);
void scoredb_destroy(struct scoredb *s);

void scoredb_create_player(struct scoredb *s, struct player *out_player, const char *playerName);
void scoredb_update_player(struct scoredb *s, struct player *p);

void scoredb_add(struct scoredb *s, struct player* p, struct replay *r);

int scoredb_get_replay_count(struct scoredb *s, struct player* p);

// Get list of replay descriptors (no replay data)
struct replay *scoredb_get_replay_list(struct scoredb *s, struct player *p, int *out_replayCount);

void scoredb_get_full_replay(struct scoredb *s, struct replay *out_replay, int replay_id);
void scoredb_get_full_replay_by_condition(struct scoredb *s, struct replay *out_replay, int mode);

#endif // __SCOREDB_H_
