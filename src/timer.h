#ifndef _timer_h
#define _timer_h

#define TIMEMAX		0xFFFFFFFFu

typedef struct {
	unsigned long time;
	int fps;
} nz_timer;

nz_timer *nz_timer_create(int fps);
void nz_timer_destroy(nz_timer *t);

int timeinc(nz_timer *t);
int timedec(nz_timer *t);
/*
int timeset(nz_timer *t, long timenew);
*/
int timegethr(nz_timer *t);
int timegetmin(nz_timer *t);
int timegetsec(nz_timer *t);
int timegetmsec(nz_timer *t);

#endif
