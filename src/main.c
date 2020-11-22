#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include <fcmp.h>

#include <map.h>
#include <vector.h>

#ifdef _RUN_THREADED
#include <sched.h>
#endif

const char* _prog_name = "fcmp";

__attribute__((noreturn, noinline)) void usage() 
{
	fprintf(stderr, "fcmp: compare files for identity\n");
	fprintf(stderr, "usage: %s <files...>\n", _prog_name);
	exit(-1);
}

_FORCE_INLINE const void* die_with_usage_if_null(const void* ptr)
{
	if (!ptr) usage();
	else return ptr;
}

static int unmap_all(mmap_t ptrs[], size_t len)
{
	register int rval=1;
	dprintf("Unmapping %lu entries", len);
	for (register size_t i=0;i<len;i++)
	{
		if(!unmap_and_close(ptrs[i])) {
			fprintf(stderr, "Failed to unmap and close fd %d", ptrs[i].fd);
			rval = -1;
		}
	}
	return rval;
}

static int compare_then_close(const mmap_t * restrict map1, mmap_t map2)
{
	register int rval=0;

	if (map1->len != map2.len) rval = 2;
	else if (memcmp(map1->ptr, map2.ptr, map1->len) != 0) rval = 1;

	if(!unmap_and_close(map2)) {
		fprintf(stderr, "Failed to unmap and close");
		rval=-1;
	}

	return rval;
}

#ifdef _RUN_THREADED
struct t_task {
	_Atomic int* _ALIAS othis;
	_Atomic bool* _ALIAS ocontinue;

	int ithis;
	const char* fthis;
	mmap_t mthis;
	const mmap_t* map1;
};

void proc_thread(vec_t* restrict v_tasks)
{
	struct t_task * tasks = v_tasks->ptr;
	mmap_t mrest[v_tasks->len];
#ifdef DEBUG
	const char* frest[v_tasks->len];
#endif
	int nrest = v_tasks->len;

	const mmap_t* map1;

	{
		for(register int i=0;i<v_tasks->len;i++)
		{
			mrest[i] = tasks[i].mthis;
#ifdef DEBUG
			frest[i] = tasks[i].fthis;
#endif
		}
		map1 = tasks[0].map1;
	}
	
	register int rval=0;
	for(register int i=0;i<nrest;i++)
	{
		if (! *tasks[0].ocontinue) {
			dprintf("Signalled to drop rest of tasks");
			unmap_all(mrest+i, nrest-i);
			break;
		}
		dprintf("Checking %d \"%s\"", tasks[i].ithis, frest[i]);
		switch ((rval=compare_then_close(map1, mrest[i]))) {
			case 0: break;
			default:
				// Close the rest
				dprintf("Unmapping mrest from %d (len %d) while max of nrest is %d", (i+1), nrest-(i+1), nrest);
				if(i<nrest-1) unmap_all(mrest+ (i+1), nrest- (i+1));
				*tasks[0].ocontinue = false;
				goto end;
		}
		dprintf("Ident %d OK", tasks[i].ithis);
	}
end:
	*tasks[0].othis = rval;
}
#endif

int main(int argc, char** argv)
{
	_prog_name = argv[0];

	const int nrest = argc-2;
	if (nrest==0) usage();
	dprintf("There are %d extra files to chk", nrest);
	const char* f1 = die_with_usage_if_null(argv[1]);
	const char* frest[nrest];

	for (register int i=0;i<nrest;i++) {
		frest[i] = die_with_usage_if_null(argv[2+i]);
		dprintf("frest[%d] = \"%s\"", i, frest[i]);
	}

	mmap_t map1;
	mmap_t mrest[nrest];

	if (!open_and_map(f1, &map1)) {
		fprintf(stderr, "Failed to open or map %s\n", f1);
		return -1;
	}

	for(register int i=0;i<nrest;i++) {
		const char* f2 = frest[i];
		dprintf("Attempting to map %d (%s)", i, f2);
		if (!open_and_map(f2, &mrest[i])) {
			fprintf(stderr, "Failed to open or map arg %d, `%s`\n", i+2, f2);
			unmap_and_close(map1);
			unmap_all(mrest, i);
			return -1;
		}
	}
	dprintf("All map okay");
	register int rval=0;
#ifdef _RUN_THREADED	
	if(sched_should(nrest) || _RUN_THREADED) {
		dprintf("Running multi-threaded");
		_Atomic int rvals[nrest];
		_Atomic bool sync_cont = true;
		vec_t vtask_args = vec_new_with_cap(sizeof(struct t_task), nrest);
		struct t_task* task_args = vtask_args.ptr;
		for (int i=0;i<nrest;i++) {
			rvals[i] = 0;
			task_args[i] = (struct t_task){
				.ithis = i,
				.fthis = frest[i],
				.mthis = mrest[i],
				.map1 = &map1,
				.othis = &rvals[i],
				.ocontinue = &sync_cont,
			};
		}
		vtask_args.len = (size_t)nrest;

		tasklist_t threads;
		if(!sched_spawn(vtask_args, &proc_thread, &threads)) {
			fprintf(stderr, "Failed to spawn tasks\n");
			abort(); //no clear way to exit gracefully from this...
		}
		vec_free(vtask_args);

		dprintf("Children spawned");

		sched_wait(&threads);

		for (register int i=0;i<nrest;i++) {
			if(rvals[i]) {
				rval = rvals[i];
				break;
			}
		}

		goto end;
	} else {
#endif
		dprintf("Running single threaded");
	for(register int i=0;i<nrest;i++) {
		dprintf("Checking %d \"%s\"", i, frest[i]);
		switch ((rval=compare_then_close(&map1, mrest[i]))) {
			case 0: break;
			default:
				// Close the rest
				dprintf("Unmapping mrest from %d (len %d) while max of nrest is %d", (i+1), nrest-(i+1), nrest);
				if(i<nrest-1) unmap_all(mrest+ (i+1), nrest- (i+1));
				goto end;
		}
		dprintf("Ident %d OK", i); 
	}
#ifdef _RUN_THREADED
	}
#endif
end:
	dprintf("Unmapping `map1`");
	if(!unmap_and_close(map1)) {
		fprintf(stderr, "Failed to unmap and close %s", f1);
		rval=-1;
	}

	dprintf("Final rval is %d", rval);

	return rval;
}
