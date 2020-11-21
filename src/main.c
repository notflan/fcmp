#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <map.h>

static const char* _prog_name = "fcmp";

__attribute__((noreturn)) void usage() 
{
	fprintf(stderr, "fcmp: compare files for identity\n");
	fprintf(stderr, "usage: %s <file1> <file2>\n", _prog_name);
	exit(-1);
}

__attribute__((always_inline)) static inline const void* die_if_null(const void* ptr)
{
	if (!ptr) usage();
	else return ptr;
}

int main(int argc, char** argv)
{
	_prog_name = argv[0];

	const char* f1 = die_if_null(argv[1]);
	const char* f2 = die_if_null(argv[2]);

	mmap_t map1, map2;
	if (!open_and_map(f1, &map1)) {
		fprintf(stderr, "Failed to open or map %s\n", f1);
		return -1;
	}
	if (!open_and_map(f2, &map2)) {
		fprintf(stderr, "Failed to open or map %s\n", f2);
		unmap_and_close(map1);
		return -1;
	}

	register int rval=0;
	if (map1.len != map2.len) rval = 2;
	else if (memcmp(map1.ptr, map2.ptr, map1.len) != 0) rval = 1;

	if(!unmap_and_close(map1)) {
		fprintf(stderr, "Failed to unmap and close %s", f1);
		rval=-1;
	}

	if(!unmap_and_close(map2)) {
		fprintf(stderr, "Failed to unmap and close %s", f2);
		rval=-1;
	}

	return rval;
}
