#ifndef _SCHED_H
#define _SCHED_H

#include <vector.h>
#include <fcmp.h>

typedef struct tasklist {
	size_t argc;
	struct taskarg* argv;
	pthread_t* tasks;
	
} tasklist_t;

typedef void (*sched_cb)(vec_t* restrict tasklist);

#ifdef _RUN_THREADED
bool sched_spawn(vec_t full, sched_cb callback, struct tasklist *restrict t_list);
void sched_wait(struct tasklist* restrict t_list);
bool sched_should(size_t ntasks);
#endif

#endif /* _SHCED_H */
