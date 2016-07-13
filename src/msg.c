#include <bolo.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

struct __bolo_frame {
	unsigned char  type;   /* type of payload (BOLO_FRAME_*) */
	unsigned short length; /* length of raw payload (data[]) */
	union {
		uint32_t  uint32;  /* BOLO_FRAME_UINT/32             */
		uint64_t  uint64;  /* BOLO_FRAME_UINT/64             */
		float     float32; /* BOLO_FRAME_FLOAT/32            */
		float     float64; /* BOLO_FRAME_FLOAT/64            */
		char     *string;  /* BOLO_FRAME_STRING              */
		uint64_t  tstamp;  /* BOLO_FRAME_TSTAMP/64           */
		/* nothing */      /* BOLO_FRAME_NIL                 */
	} payload;

	bolo_frame_t next;     /* next sequential frame or NULL  */
	uint8_t data[];        /* variable length data buffer    */
};

struct __bolo_message {
	unsigned char   version; /* TSDP protocol version (1)     */
	unsigned char   opcode;  /* what type of message is this? */
	unsigned char   flags;   /* opcode-specific flags         */
	unsigned short  payload; /* BOLO_PAYLOAD_* constant(s)    */

	int complete;            /* do we have all the frames?    */
	bolo_frame_t frames;     /* constituent MSG FRAMEs        */
	bolo_frame_t last;       /* helper pointer to last frame  */
};


#define BYTE(x,n) (((unsigned char*)(x))[(n)])
#define WORD(x,n) ((BYTE((x),(n)) << 8) | BYTE((x),(n)+1))

#define extract_header_version(h) ((BYTE((h),0) & 0xf0) >> 4)
#define extract_header_opcode(h)   (BYTE((h),0) & 0x0f)
#define extract_header_flags(h)    (BYTE((h),1))
#define extract_header_payload(h) ((BYTE((h),2) >> 8) | (BYTE((h), 3)))

#define extract_frame_final(f)     ((BYTE((f),0) & 0x80) >> 7)
#define extract_frame_type(f)      ((BYTE((f),0) & 0x70) >> 4)
#define extract_frame_length(f)   (((BYTE((f),0) & 0x0f) << 8) | (BYTE((f), 1)))

bolo_message_t
bolo_message_unpack(const void *buf, size_t n, size_t *left)
{
	bolo_message_t m;

	assert(buf);  /* need a buffer to read from... */
	assert(left); /* need a place to store unused part of buf... */

	/* we must have at least enough for a header */
	if (n < 4) {
		return NULL;
	}

	m = calloc(1, sizeof(struct __bolo_message));
	if (!m) {
		return NULL;
	}

	/* extract header fields from (network byte-order) buf */
	m->version = extract_header_version(buf);
	m->opcode  = extract_header_opcode(buf);
	m->flags   = extract_header_flags(buf);
	m->payload = extract_header_payload(buf);

	m->complete = 0;
	*left = n - 4;
	buf += 4;

	/* extract the frames */
	if (m->opcode == BOLO_OPCODE_REPLAY) {
		/* REPLAY has no frames ... */
		return m;
	}

	while (*left > 2) { /* have enough for a header */
		bolo_frame_t f;
		unsigned short len = extract_frame_length(buf);
		if (len > *left - 2) {
			/* not enough in buf[] to read the payload */
			return m;
		}

		f = calloc(1, sizeof(struct __bolo_frame)
		            + len); /* variable frame data */
		if (!f) {
			return NULL;
		}

		f->type   = extract_frame_type(buf);
		f->length = len;
		if (!m->frames) {
			m->frames = f;
			m->last   = f;
		} else {
			assert(m->last);
			m->last->next = f;
			m->last       = f;
		}
		m->complete = extract_frame_final(buf);
		*left -= 2 + len;
		buf   += 2 + len;
	}

	return m;
}
