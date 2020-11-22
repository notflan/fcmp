#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <vector.h>

_FORCE_INLINE void* die_if_null(void* ptr)
{
	if (!ptr) abort();
	return ptr;
}

vec_t vec_new_with_cap(size_t elem, size_t cap) {
	return (vec_t){
		.len =0,
		.cap = cap,
		.scap = cap,
		.element = elem,
		.ptr = die_if_null(calloc(elem, cap)),
	};
}

static inline void vec_extend_one(vec_t* restrict self)
{
	self->ptr = die_if_null(reallocarray(self->ptr, self->element, (self->cap+=self->scap)));
}

void* vec_index(const vec_t* restrict self, size_t i)
{
	if (i >= self->len) return NULL;
	return (void*)(((uintptr_t)self->ptr)+ (self->element*i));
}

void vec_push(vec_t* restrict self, const void* restrict item)
{
	if (self->len >= self->cap) {
		vec_extend_one(self);
	}
	memcpy(die_if_null(vec_index(self, self->len++)), item, self->element);
}

bool vec_pop(vec_t* restrict self, void* restrict item)
{
	if (self->len>0) {
		memcpy(item, die_if_null(vec_index(self, self->len--)), self->element);
		return true;
	} else return false;	
}

vec_t vec_clone(const vec_t* restrict self)
{
	register vec_t new = {
		.len = self->len,
		.cap = self->cap,
		.element = self->element,
		.scap = self->scap,
		.ptr = die_if_null(calloc(self->element, self->cap)),
	};
	memcpy(self->ptr, new.ptr, new.len * new.element);
	return new;
}
