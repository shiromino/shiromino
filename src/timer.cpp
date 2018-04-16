#include <stdio.h>
#include <stdlib.h>

#include "timer.h"

nz_timer *nz_timer_create(int fps)
{
    if(fps < 0 || fps > 1000)
        return NULL;

    nz_timer *t = (nz_timer *) malloc(sizeof(nz_timer));

    t->time = 0;
    t->fps = fps;

    return t;
}

void nz_timer_destroy(nz_timer *t)
{
    free(t);
}

int timeinc(nz_timer *t)
{
    if(t->time >= TIMEMAX)
        return 1;

    t->time++;
    return 0;
}

int timedec(nz_timer *t)
{
    if(t->time <= (long)(0l - TIMEMAX))
        return 1;

    t->time--;
    return 0;
}
/*
int timeset(nz_timer *t, long timenew)
{
    if(timenew > TIMEMAX || timenew < (long)(0l - TIMEMAX))
        return 1;

    t->time = timenew;

    return 0;
}
*/
int timegethr(nz_timer *t)
{
    int hr = (t->time / t->fps) / 3600;

    return hr;
}

int timegetmin(nz_timer *t)
{
    int min = (t->time / t->fps) / 60;

    return min;
}

int timegetsec(nz_timer *t)
{
    int sec = t->time / t->fps;

    return sec;
}

int timegetmsec(nz_timer *t)
{
    int f = 0;
    if(t->fps == 60) {
        f = t->time % 60;
        return ((f / 6) * 100) + ( ( ((f % 6) * 5) / 3) * 10);
    }

    int msec = (int)(( (double)(t->time) / (double)(t->fps) ) * 1000);

    return msec;
}
