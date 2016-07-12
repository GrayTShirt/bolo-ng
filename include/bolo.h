#ifndef BOLO_H
#define BOLO_H

typedef struct __bolo_qname *bolo_qname_t;
#define INVALID_QNAME ((bolo_qname_t)(0))

/**
   Parse a qualified name from an input string,
   returning the `bolo_qname_t` that results, or
   the value `INVALID_QNAME` on error, at which
   point `errno` is set appropriately.

   This function allocates memory, and may fail
   if insufficient memory is available.
 **/
bolo_qname_t
bolo_qname_parse(const char *str);

/**
  Frees the memory allocated to the qualified name.
  Handles passing n as `INVALID_QNAME`.
 **/
void
bolo_qname_free(bolo_qname_t n);

/**
  Allocates a fresh null-terminated string which
  contains the canonical representation of the
  given qualified name.

  Returns the empty string for `INVALID_QNAME`.
 **/
char *
bolo_qname_string(bolo_qname_t n);

/**
  Returns non-zero if the two qualified names
  are exactly equivalent, handling wildcard as
  explicit matches (that is `key=*` is equivalent
  to `key=*`, but not `key=value`).

  Returns 0 otherwise.

  The value INVALID_QNAME is never equivalent
  to anything, even another INVALID_QNAME.
 **/
int
bolo_qname_equal(bolo_qname_t a, bolo_qname_t b);

/**
  Returns non-zero if the first qualified name (`qn`) matches the
  second qualified name (`pattern`), honoring wildcard semantics
  in the pattern name.

  Returns 0 if `qn` does not match `pattern`.

  The value INVALID_QNAME will never match any other name,
  even another INVALID_QNAME.
 **/
int
bolo_qname_match(bolo_qname_t qn, bolo_qname_t pattern);

#endif
