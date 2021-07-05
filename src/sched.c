// Scheduler

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

#include <vector.h>
#include <pthread.h>

#ifdef _RUN_THREADED 
inline static size_t num_cpus() {
	return sysconf( _SC_NPROCESSORS_ONLN );
}

struct taskarg {
	vec_t li;
	sched_cb cb;
};

static void* _spawn(void* _arg)
{
	struct taskarg* restrict arg = _arg;
	if(arg->li.len>0)
		arg->cb(&arg->li);
	vec_free(arg->li);
	return NULL;
}

bool sched_should(size_t ntasks)
{
	static size_t num = 0;
	// XXX: This is not thread-safe, but this function is only ever called by the main thread, so...
	if(!num) num = num_cpus();
	return (num > 1 && ntasks > 1);
}

bool sched_spawn(vec_t full, sched_cb callback, struct tasklist *restrict t_list)
{
	register size_t spn = num_cpus() + 1;

	if (spn > full.len) spn = full.len;

	dprintf("Spawning %lu worker threads", spn);
	// Split tasks
	*t_list = (struct tasklist){
		.argc = spn,
		.argv = calloc(sizeof(struct taskarg), spn),
		.tasks = calloc(sizeof(pthread_t), spn),
	};
	struct taskarg* tasklist = t_list->argv;

	for(register int i=0;i<spn;i++) tasklist[i] = (struct taskarg){.li = vec_new_with_cap(full.element, full.len), .cb = callback };
	
	for (register int i=0;i<full.len;i++)
	{
		vec_push(&tasklist[i%spn].li, vec_index(&full, i));
	}

	for(register int i=0;i<spn;i++)
	{
		if(pthread_create(&t_list->tasks[i], NULL, &_spawn, &tasklist[i]))
		{
			perror("Failed to spawn thread");
			return false;
		}
		dprintf("Worker thead %d of %lu OK", i, spn);
	}

	return true;
}

void sched_wait(struct tasklist* restrict t_list)
{
	dprintf("Waiting on %lu worker threads", t_list->argc);
	for (size_t i=0;i<t_list->argc;i++) {
		if(pthread_join(t_list->tasks[i], NULL)) {
			perror("Failed to join thread");
			continue;
		}
		dprintf("Joined thread %lu of %lu okay", i, t_list->argc);
	}
	free(t_list->tasks);
	free(t_list->argv);
	dprintf("Freed args and thread handles okay");
}
#endif
