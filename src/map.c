#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/mman.h>
#include<fcntl.h>

#define FILEMODE S_IRWXU | S_IRGRP | S_IROTH

#define DEFAULT_ADVICE MADV_SEQUENTIAL

#define ADVICE DEFAULT_ADVICE | MADV_WILLNEED

#include <map.h>

static inline int _map_advise(const mmap_t* restrict map, int adv)
{
	return madvise(map->ptr, map->len, adv);
}

int set_preload_map(mmap_t* restrict map)
{
	if(_map_advise(map, ADVICE) != 0) {
		perror("failed to advise kernel about mapped page(s)");
		return 0;
	}
	return 1;
}

int open_and_map(const char* file, mmap_t* restrict ptr)
{
	int fd;
	struct stat st;
	if ((fd = open(file, O_RDONLY, FILEMODE)) < 0) {
		perror("Failed to open file");
		return 0;
	}

	if (fstat(fd, &st) < 0) {
		perror("Failed to stat file");
		close(fd);
		return 0;
	}

	struct mmap map = { .fd = fd, .ptr = NULL, .len = st.st_size };

	if ((map.ptr = mmap(NULL, map.len, PROT_READ, MAP_SHARED,fd, 0)) == MAP_FAILED) {
		perror("mmap() failed");
		close(fd);
		return 0;
	}

	if(_map_advise(&map, DEFAULT_ADVICE) != 0) {
		perror("madvise(): failed to set default advice");
		//XXX: Should this be a hard error, or should we return the map if this fails anyway?
		unmap_and_close(map);
		return 0;
	}
	
	*ptr = map;
	
	return 1;
}

int unmap_and_close(mmap_t map)
{
	if (munmap(map.ptr, map.len) < 0) {
		perror("munmap() failed");
		return 0;
	}
	if (close(map.fd) <0) {
		perror("Failed to close fd");
		return 0;
	}

	return 1;
}
