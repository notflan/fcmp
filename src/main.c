#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <map.h>

static const char* _prog_name = "fcmp";

#ifdef DEBUG
#define __name(d) #d
#define dprintf(fmt, ...) printf("[dbg @" __FILE__ "->%s:%d] " fmt "\n", __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#else
#define dprintf(fmt, ...)
#endif

__attribute__((noreturn)) static void usage() 
{
	fprintf(stderr, "fcmp: compare files for identity\n");
	fprintf(stderr, "usage: %s <files...>\n", _prog_name);
	exit(-1);
}

__attribute__((always_inline)) static inline const void* die_if_null(const void* ptr)
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

int main(int argc, char** argv)
{
	_prog_name = argv[0];

	const int nrest = argc-2;
	if (nrest==0) usage();
	dprintf("There are %d extra files to chk", nrest);
	const char* f1 = die_if_null(argv[1]);
	const char* frest[nrest];

	for (register int i=0;i<nrest;i++) {
		frest[i] = die_if_null(argv[2+i]);
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

end:
	if(!unmap_and_close(map1)) {
		fprintf(stderr, "Failed to unmap and close %s", f1);
		rval=-1;
	}

	dprintf("Final rval is %d", rval);
	return rval;
}
