#ifndef _MAP_H
#define _MAP_H

#ifdef __cplusplus
extern "C" {
#define restrict __restrict__
#endif

#include <stddef.h>

typedef struct mmap {
	int fd;

	void* ptr;
	size_t len;
} mmap_t;

int open_and_map(const char* file, mmap_t* restrict ptr);
int unmap_and_close(mmap_t map);

#ifdef _cplusplus
}
#undef restrict
#endif

#endif /* _MAP_H */
