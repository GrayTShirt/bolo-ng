#include <ring.h>
#include <stdlib.h>


struct __ring {
	unsigned int   head;      /* where the data starts               */
	unsigned int   used;      /* how much data is stored in the ring */
	unsigned int   capacity;  /* how much data can be stored (total) */
	unsigned char  buf[];     /* the buffer (variable length)        */
};

ring_t
ring_alloc(unsigned int cap)
{
	ring_t r;

	r = malloc(sizeof(struct __ring) + cap);
	if (!r) {
		return INVALID_RING;
	}

	r->capacity = cap;
	r->head     = 0;
	r->used     = 0;
	return r;
}

void
ring_free(ring_t r)
{
	free(r);
}

int
ring_full(ring_t r) {
	return r->used == r->capacity;
}

int
ring_empty(ring_t r)
{
	return r->used == 0;
}

ssize_t
ring_copy(void *dst, ring_t src, size_t n)
{
	ssize_t ncopied;

	n = (n > src->used ? src->used : n);
	for (ncopied = 0; ncopied < n; ncopied++) {
		((unsigned char*)dst)[ncopied] = src->buf[(src->head + ncopied) % src->capacity];
	}
	return ncopied;
}

ssize_t
ring_move(void *dst, ring_t src, size_t n)
{
	ssize_t l = ring_copy(dst, src, n);
	if (l > 0) {
		src->head += l;
		src->used -= l;
	}
	return l;
}

ssize_t
ring_fill(ring_t dst, void *src, size_t n)
{
	const char *p = src;
	ssize_t nwrit = 0;

	for (p = src; n > 0; p++, n--) {
		nwrit++;
		dst->buf[(dst->head + dst->used++) % dst->capacity] = *p;
	}

	return nwrit;
}

void
ring_fdump(FILE *io, ring_t r)
{
	int i;

	fprintf(io, "ring [%d] - [%d] (capacity %d/%d b)",
			r->head, (r->head + r->used + 1) % r->capacity,
			r->used, r->capacity);
	fprintf(io, " [");
	for (i = 0; i < r->used; i++) {
		fprintf(io, "%c", r->buf[(r->head + i) % r->capacity]);
	}
	fprintf(io, "]\n");
}
