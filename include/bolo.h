#ifndef BOLO_H
#define BOLO_H

#include <sys/types.h>

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


typedef struct __bolo_message *bolo_message_t;
typedef struct __bolo_frame   *bolo_frame_t;


#define BOLO_PROTOCOL_V1       1


#define BOLO_OPCODE_HEARTBEAT  0
#define BOLO_OPCODE_SUBMIT     1
#define BOLO_OPCODE_BROADCAST  2
#define BOLO_OPCODE_FORGET     3
#define BOLO_OPCODE_REPLAY     4
#define BOLO_OPCODE_SUBSCRIBE  5


#define BOLO_PAYLOAD_SAMPLE    0x0001
#define BOLO_PAYLOAD_TALLY     0x0002
#define BOLO_PAYLOAD_DELTA     0x0004
#define BOLO_PAYLOAD_STATE     0x0008
#define BOLO_PAYLOAD_EVENT     0x0010
#define BOLO_PAYLOAD_FACT      0x0020
// ......................      ......
#define BOLO_PAYLOAD_RSVP      0xffc0


#define BOLO_FRAME_UINT        0
#define BOLO_FRAME_FLOAT       1
#define BOLO_FRAME_STRING      2
// ......................      .
#define BOLO_FRAME_TSTAMP      6
#define BOLO_FRAME_NIL         7

bolo_message_t
bolo_message_unpack(const void *buf, size_t n, size_t *left);

int
bolo_message_valid(bolo_message_t m);

unsigned int
bolo_message_version(bolo_message_t m);

unsigned int
bolo_message_opcode(bolo_message_t m);

unsigned int
bolo_message_flags(bolo_message_t m);

unsigned int
bolo_message_payload(bolo_message_t m);

unsigned int
bolo_message_nframes(bolo_message_t m);

unsigned int
bolo_message_length(bolo_message_t m);


unsigned int
bolo_frame_type(bolo_frame_t f);

unsigned int
bolo_frame_length(bolo_frame_t f);

unsigned int
bolo_frame_isfinal(bolo_frame_t f);


#endif
