#pragma once
#define NO_REPLAY -1
#define MAX_KEYFLAGS 72000 // 20 minutes of inputs (@ 60 fps)
#include <cstdint>
#include <ctime>
#include <string>

namespace Shiro { struct KeyFlags; }

struct packed_input {
    uint8_t data;
};

struct packed_input pack_input(Shiro::KeyFlags *k);
void unpack_input(struct packed_input p, Shiro::KeyFlags *out_keys);

struct replay {
    unsigned int len;
    unsigned int mlen;

    int mode;
    unsigned int mode_flags;
    long seed;
    int grade;
    uint64_t time;
    int starting_level;
    int ending_level;

    time_t date;

    int index;

    struct packed_input pinputs[MAX_KEYFLAGS];
};

std::string get_replay_descriptor(struct replay *r);

void read_replay_from_memory(struct replay *out_replay, const uint8_t *buffer);

uint8_t* generate_raw_replay(struct replay *r, size_t *out_replayLength);
void dispose_raw_replay(uint8_t* buffer);