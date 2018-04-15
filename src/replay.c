#include "replay.h"

#include "game_qs.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

struct packed_input pack_input(struct keyflags *k)
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

void unpack_input(struct packed_input p, struct keyflags *out_keys)
{
    out_keys->left  = p.data & pi_left;
    out_keys->right = p.data & pi_right;
    out_keys->up    = p.data & pi_up;
    out_keys->down  = p.data & pi_down;
    out_keys->a     = p.data & pi_a;
    out_keys->b     = p.data & pi_b;
    out_keys->c     = p.data & pi_c;
    out_keys->d     = p.data & pi_d;
}

#define REPLAY_DESCRIPTOR_BUF_SIZE 32
void get_replay_descriptor(struct replay *r, char *buffer, size_t bufferLength)
{
    char modeStringBuffer[REPLAY_DESCRIPTOR_BUF_SIZE];

    switch(r->mode) {
        case MODE_PENTOMINO:
            strncpy(modeStringBuffer, "PENTOMINO", REPLAY_DESCRIPTOR_BUF_SIZE);
            break;
        case MODE_G2_DEATH:
            strncpy(modeStringBuffer, "G2 DEATH", REPLAY_DESCRIPTOR_BUF_SIZE);
            break;
        case MODE_G3_TERROR:
            strncpy(modeStringBuffer, "G3 TERROR", REPLAY_DESCRIPTOR_BUF_SIZE);
            break;
        case MODE_G1_20G:
            strncpy(modeStringBuffer, "G1 20G", REPLAY_DESCRIPTOR_BUF_SIZE);
            break;
        case MODE_G1_MASTER:
            strncpy(modeStringBuffer, "G1 MASTER", REPLAY_DESCRIPTOR_BUF_SIZE);
            break;
        case MODE_G2_MASTER:
            strncpy(modeStringBuffer, "G2 MASTER", REPLAY_DESCRIPTOR_BUF_SIZE);
            break;
        default:
            break;
    }

    nz_timer *t = nz_timer_create(60);
    t->time = r->time;

    char dateBuffer[REPLAY_DESCRIPTOR_BUF_SIZE];
    struct tm *ts = localtime(&r->date);
    strftime(dateBuffer, REPLAY_DESCRIPTOR_BUF_SIZE, "%Y.%m.%d", ts);

    snprintf(buffer, bufferLength, "%s  %-10s %4d-%-4d  %02d:%02d:%02d   %s",
             get_grade_name(r->grade),
             modeStringBuffer,
             r->starting_level,
             r->ending_level,
             timegetmin(t),
             timegetsec(t) % 60,
             timegetmsec(t) / 10,
             dateBuffer);
}

void read_replay_from_memory(struct replay *out_replay, const uint8_t *buffer, size_t bufferLength)
{
    // Keep the same existing format
    const uint8_t *scanner = buffer;

    out_replay->mode = ((int*)scanner)[0];
    scanner += sizeof(int);

    out_replay->mode_flags = ((int*)scanner)[0];
    scanner += sizeof(int);

    out_replay->seed = ((long*)scanner)[0];
    scanner += sizeof(long);

    out_replay->grade = ((int*)scanner)[0];
    scanner += sizeof(int);

    out_replay->time = ((long*)scanner)[0];
    scanner += sizeof(long);

    out_replay->starting_level = ((int*)scanner)[0];
    scanner += sizeof(int);

    out_replay->ending_level = ((int*)scanner)[0];
    scanner += sizeof(int);

    out_replay->date = ((long*)scanner)[0];
    scanner += sizeof(long);

    out_replay->len = ((int*)scanner)[0];
    scanner += sizeof(int);

    memcpy(&out_replay->pinputs[0], scanner, out_replay->len * sizeof(struct packed_input));
}

uint8_t* generate_raw_replay(struct replay *r, size_t *out_replayLength)
{
    // TODO: Endianness?
    uint8_t *buffer = malloc(sizeof(struct replay));
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

void dispose_raw_replay(void* data)
{
    free(data);
}
