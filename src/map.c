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

#include <map.h>

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

	register struct mmap map = { .fd = fd, .ptr = NULL, .len = st.st_size };

	if ((map.ptr = mmap(NULL, map.len, PROT_READ, MAP_SHARED,fd, 0)) == MAP_FAILED) {
		perror("mmap() failed");
		close(fd);
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
