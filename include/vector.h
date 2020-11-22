#ifndef _VECTOR_H
#define _VECTOR_H

#include <stdlib.h>
#include <stdbool.h>

#include "fcmp.h"

typedef struct {
	size_t len, cap;

	size_t element, scap;

	void* ptr;
} vec_t;

#define VEC_DEFAULT_CAP 16

vec_t vec_new_with_cap(size_t elem, size_t cap);
void vec_push(vec_t* restrict self, const void* restrict item);
bool vec_pop(vec_t* restrict self, void* restrict item);
void* vec_index(const vec_t* restrict self, size_t i);
vec_t vec_clone(const vec_t* restrict self);

_FORCE_INLINE vec_t vec_new(size_t elem) { return vec_new_with_cap(elem, VEC_DEFAULT_CAP); }
_FORCE_INLINE void vec_free(vec_t v) { free(v.ptr); }

#endif /* _VECTOR_H */
