#include "replay.h"
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <sstream>
#include "Timer.h"
#include "game_qs.h"
#include "input/KeyFlags.h"

struct tm;

// clang-format off
enum packed_input_mask {
    pi_left   = 1 << 0,
    pi_right  = 1 << 1,
    pi_up     = 1 << 2,
    pi_down   = 1 << 3,
    pi_a      = 1 << 4,
    pi_b      = 1 << 5,
    pi_c      = 1 << 6,
    pi_d      = 1 << 7,
};

struct packed_input pack_input(Shiro::KeyFlags *k)
{
    struct packed_input i = { 0 };

    if (k->left)  i.data |= pi_left;
    if (k->right) i.data |= pi_right;
    if (k->up)    i.data |= pi_up;
    if (k->down)  i.data |= pi_down;
    if (k->a)     i.data |= pi_a;
    if (k->b)     i.data |= pi_b;
    if (k->c)     i.data |= pi_c;
    if (k->d)     i.data |= pi_d;

    return i;
}

void unpack_input(struct packed_input p, Shiro::KeyFlags *out_keys)
{
    out_keys->left  = (p.data & pi_left) ? 1 : 0;
    out_keys->right = (p.data & pi_right) ? 1 : 0;
    out_keys->up    = (p.data & pi_up) ? 1 : 0;
    out_keys->down  = (p.data & pi_down) ? 1 : 0;
    out_keys->a     = (p.data & pi_a) ? 1 : 0;
    out_keys->b     = (p.data & pi_b) ? 1 : 0;
    out_keys->c     = (p.data & pi_c) ? 1 : 0;
    out_keys->d     = (p.data & pi_d) ? 1 : 0;
}
// clang-format on

std::string get_replay_descriptor(struct replay *r)
{
    std::string modeString;

    switch(r->mode)
    {
        case MODE_PENTOMINO:
            modeString = "PENTOMINO";
            break;
        case MODE_G2_DEATH:
            modeString = "G2 DEATH";
            break;
        case MODE_G3_TERROR:
            modeString = "G3 TERROR";
            break;
        case MODE_G1_20G:
            modeString = "G1 20G";
            break;
        case MODE_G1_MASTER:
            modeString = "G1 MASTER";
            break;
        case MODE_G2_MASTER:
            modeString = "G2 MASTER";
            break;
        default:
            break;
    }

    Shiro::Timer t(60.0, r->time);

    std::string dateString;
    tm *ts = localtime(&r->date);
    std::stringstream dateSS;
    dateSS << std::put_time(ts, "%Y.%m.%d");
    dateString = dateSS.str();

    std::stringstream returnSS;
    returnSS <<
        get_grade_name(r->grade) << "  " <<
        std::setfill(' ') << std::left << std::setw(10) << modeString << " " <<
        std::right << std::setw(4) << r->starting_level << "-" <<
        std::left << std::setw(4) << r->ending_level << "  " <<
        std::setfill('0') << std::right <<
        std::setw(2) << t.min() << ":" <<
        std::setw(2) << t.sec() % 60 << ":" <<
        std::setw(2) << t.csec() % 100 << "   " <<
        dateString;
    return returnSS.str();

    /*
    snprintf(buffer,
             bufferLength,
             "%s  %-10s %4d-%-4d  %02d:%02d:%02d   %s",
             get_grade_name(r->grade),
             modeStringBuffer,
             r->starting_level,
             r->ending_level,
             t.min(),//timegetmin(t),
             t.sec() % 60,//timegetsec(t) % 60,
             t.csec(),//timegetmsec(t) / 10,
             dateBuffer);
    */
}

// TODO: Use SDL endianness functions to ensure replay data is stored little
// endian. The majority of players play on x86/x64 in older versions, so it'd
// maintain compatibilty with old replays at first.

void read_replay_from_memory(struct replay *out_replay, const uint8_t *buffer)
{
    // Keep the same existing format
    const uint8_t *scanner = buffer;

    out_replay->mode = ((int *)scanner)[0];
    scanner += sizeof(int);

    out_replay->mode_flags = ((int *)scanner)[0];
    scanner += sizeof(int);

    out_replay->seed = ((long *)scanner)[0];
    scanner += sizeof(long);

    out_replay->grade = ((int *)scanner)[0];
    scanner += sizeof(int);

    out_replay->time = ((long *)scanner)[0];
    scanner += sizeof(long);

    out_replay->starting_level = ((int *)scanner)[0];
    scanner += sizeof(int);

    out_replay->ending_level = ((int *)scanner)[0];
    scanner += sizeof(int);

    out_replay->date = ((long *)scanner)[0];
    scanner += sizeof(long);

    out_replay->len = ((int *)scanner)[0];
    scanner += sizeof(int);

    memcpy(&out_replay->pinputs[0], scanner, out_replay->len * sizeof(struct packed_input));
}

uint8_t *generate_raw_replay(struct replay *r, size_t *out_replayLength)
{
    uint8_t *buffer = (uint8_t *)malloc(sizeof(struct replay));
    size_t bufferOffset = 0;

    memcpy(buffer + bufferOffset, &r->mode, sizeof(int));
    bufferOffset += sizeof(int);

    memcpy(buffer + bufferOffset, &r->mode_flags, sizeof(int));
    bufferOffset += sizeof(int);

    memcpy(buffer + bufferOffset, &r->seed, sizeof(long));
    bufferOffset += sizeof(long);

    memcpy(buffer + bufferOffset, &r->grade, sizeof(int));
    bufferOffset += sizeof(int);

    memcpy(buffer + bufferOffset, &r->time, sizeof(long));
    bufferOffset += sizeof(long);

    memcpy(buffer + bufferOffset, &r->starting_level, sizeof(int));
    bufferOffset += sizeof(int);

    memcpy(buffer + bufferOffset, &r->ending_level, sizeof(int));
    bufferOffset += sizeof(int);

    memcpy(buffer + bufferOffset, &r->date, sizeof(long));
    bufferOffset += sizeof(long);

    memcpy(buffer + bufferOffset, &r->len, sizeof(int));
    bufferOffset += sizeof(int);

    memcpy(buffer + bufferOffset, &r->pinputs, sizeof(struct packed_input) * r->len);
    bufferOffset += sizeof(struct packed_input) * r->len;

    *out_replayLength = bufferOffset;

    return buffer;
}

void dispose_raw_replay(uint8_t *buffer) { free(buffer); }