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
int set_preload_map(mmap_t* restrict map);
/// Undo a previous call to `set_preload_map()`.
/// *freeing* (1 - yes, 0, no) - If the next operation on the map will be `unmap_and_close()`, advise the kernel to drop the mapped pages whenever it wants. Do not specify this if you will use the map again after this call.
int unset_preload_map(mmap_t* restrict map, int freeing);

#ifdef _cplusplus
}
#undef restrict
#endif

#endif /* _MAP_H */
