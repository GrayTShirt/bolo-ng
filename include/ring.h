#ifndef RING_H
#define RING_H

#include <sys/types.h>
#include <stdio.h>

typedef struct __ring *ring_t;
#define INVALID_RING ((ring_t)(0))

#define RING_DEFAULT 8192

ring_t
ring_alloc(unsigned int cap);

void
ring_free(ring_t r);

int
ring_full(ring_t r);

int
ring_empty(ring_t r);

ssize_t
ring_copy(void *dst, ring_t src, size_t n);

ssize_t
ring_move(void *dst, ring_t src, size_t n);

ssize_t
ring_fill(ring_t dst, void *src, size_t n);

void
ring_fdump(FILE *io, ring_t r);

#endif
