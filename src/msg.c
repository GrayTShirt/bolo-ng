#include <bolo.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
	int nframes;             /* how many frames do we have?   */
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
		memmove(f->data, buf + 2, len);

		m->nframes++;
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

#define s_frame_valid(f,t,l) ((f) && (f)->type == (t) && ((t) == BOLO_FRAME_STRING || (f)->length == l))

static bolo_frame_t
s_nth_frame(bolo_message_t m, int n)
{
	bolo_frame_t f;

	f = m->frames;
	while (n > 0 && f) {
		f = f->next;
		n--;
	}
	return f;
}

/* hamming weight algorithm, in 8-bit */
static unsigned char
bits8(unsigned char b)
{
	b = b - ((b >> 1) & 0x55);
	b = (b & 0x33) + ((b >> 2) & 0x33);
	return ((b + (b >> 4)) & 0x0f);
}

int
bolo_message_valid(bolo_message_t m)
{
	int i;
	unsigned int type, len;

	if (!m) {
		return 0;
	}

	if (m->version != BOLO_PROTOCOL_V1) {
		return 0;
	}

#define requires_single_payload() \
	do { if (bits8(m->payload) != 1) return 0; } while (0)
#define requires_empty_payload() \
	do { if (m->payload != 0) return 0; } while (0)
#define requires_one_or_more_payloads() \
	do { if (m->payload == 0) return 0; } while (0)
#define requires_payloads(p) \
	do { if (m->payload & p != m->payload) return 0; } while (0)

#define requires_exact_frame_count(n) \
	do { if (m->nframes != (n)) return 0; } while (0)
#define requires_min_frame_count(n) \
	do { if (m->nframes < (n)) return 0; } while (0)
#define requires_min_max_frame_count(a,b) \
	do { if (m->nframes < (a) || m->nframes > (b)) return 0; } while (0)
#define requires_frame(n,t,l) \
	do { if (!s_frame_valid(s_nth_frame(m, (n)), (t), (l))) return 0; } while (0)

	switch (m->opcode) {
	case BOLO_OPCODE_HEARTBEAT:
		requires_empty_payload();      /* per RFC-TSDP $4.3.1.1 */
		requires_exact_frame_count(2); /* per RFC-TSDP $4.3.1.2 */
		requires_frame(0, BOLO_FRAME_TSTAMP, 64);
		requires_frame(1, BOLO_FRAME_UINT,   64);
		return 1;

	case BOLO_OPCODE_SUBMIT:
		requires_single_payload();
		switch (m->payload) {
		case BOLO_PAYLOAD_SAMPLE:
			requires_min_frame_count(3);
			requires_frame(0, BOLO_FRAME_STRING, 0);
			requires_frame(1, BOLO_FRAME_TSTAMP, 64);
			for (i = 2; i < m->nframes; i++) {
				requires_frame(i, BOLO_FRAME_FLOAT, 64);
			}
			return 1;

		case BOLO_PAYLOAD_TALLY:
			requires_min_max_frame_count(2, 3);
			requires_frame(0, BOLO_FRAME_STRING, 0);
			requires_frame(1, BOLO_FRAME_TSTAMP, 64);
			if (m->nframes == 3) {
				requires_frame(2, BOLO_FRAME_UINT, 64);
			}
			return 1;

		case BOLO_PAYLOAD_DELTA:
			requires_exact_frame_count(3);
			requires_frame(0, BOLO_FRAME_STRING, 0);
			requires_frame(1, BOLO_FRAME_TSTAMP, 64);
			requires_frame(2, BOLO_FRAME_FLOAT,  64);
			return 1;

		case BOLO_PAYLOAD_STATE:
			requires_min_max_frame_count(3, 4);
			requires_frame(0, BOLO_FRAME_STRING, 0);
			requires_frame(1, BOLO_FRAME_TSTAMP, 64);
			requires_frame(2, BOLO_FRAME_UINT,   32);
			if (m->nframes == 4) {
				requires_frame(3, BOLO_FRAME_STRING, 0);
			}
			return 1;

		case BOLO_PAYLOAD_EVENT:
			requires_exact_frame_count(3);
			requires_frame(0, BOLO_FRAME_STRING, 0);
			requires_frame(1, BOLO_FRAME_TSTAMP, 64);
			requires_frame(2, BOLO_FRAME_STRING, 0);
			return 1;

		case BOLO_PAYLOAD_FACT:
			requires_exact_frame_count(2);
			requires_frame(0, BOLO_FRAME_STRING, 0);
			requires_frame(1, BOLO_FRAME_STRING, 0);
			return 1;
		}
		return 0;

	case BOLO_OPCODE_BROADCAST:
		requires_single_payload();
		switch (m->payload) {
		case BOLO_PAYLOAD_SAMPLE:
			requires_exact_frame_count(9);
			requires_frame(0, BOLO_FRAME_STRING, 0);
			requires_frame(1, BOLO_FRAME_TSTAMP, 64);
			requires_frame(2, BOLO_FRAME_UINT,   32);
			requires_frame(3, BOLO_FRAME_UINT,   16);
			requires_frame(4, BOLO_FRAME_FLOAT,  64);
			requires_frame(5, BOLO_FRAME_FLOAT,  64);
			requires_frame(6, BOLO_FRAME_FLOAT,  64);
			requires_frame(7, BOLO_FRAME_FLOAT,  64);
			requires_frame(8, BOLO_FRAME_FLOAT,  64);
			return 1;

		case BOLO_PAYLOAD_TALLY:
			requires_exact_frame_count(4);
			requires_frame(0, BOLO_FRAME_STRING, 0);
			requires_frame(1, BOLO_FRAME_TSTAMP, 64);
			requires_frame(2, BOLO_FRAME_UINT,   32);
			requires_frame(3, BOLO_FRAME_UINT,   64);
			return 1;

		case BOLO_PAYLOAD_DELTA:
			requires_exact_frame_count(4);
			requires_frame(0, BOLO_FRAME_STRING, 0);
			requires_frame(1, BOLO_FRAME_TSTAMP, 64);
			requires_frame(2, BOLO_FRAME_UINT,   32);
			requires_frame(3, BOLO_FRAME_FLOAT,  64);
			return 1;

		case BOLO_PAYLOAD_STATE:
			/* FIXME: has some flag-based stuff to check */
			return 0;

		case BOLO_PAYLOAD_EVENT:
			requires_exact_frame_count(3);
			requires_frame(0, BOLO_FRAME_STRING, 0);
			requires_frame(1, BOLO_FRAME_TSTAMP, 64);
			requires_frame(2, BOLO_FRAME_STRING, 0);
			return 1;

		case BOLO_PAYLOAD_FACT:
			requires_exact_frame_count(3);
			requires_frame(0, BOLO_FRAME_STRING, 0);
			requires_frame(1, BOLO_FRAME_STRING, 0);
			return 1;

		}
		return 0;

	case BOLO_OPCODE_FORGET:
		requires_payloads(BOLO_PAYLOAD_SAMPLE
		                | BOLO_PAYLOAD_TALLY
		                | BOLO_PAYLOAD_DELTA
		                | BOLO_PAYLOAD_STATE);
		requires_exact_frame_count(1);
		requires_frame(0, BOLO_FRAME_STRING, 0);
		return 1;

	case BOLO_OPCODE_REPLAY:
		requires_one_or_more_payloads();
		requires_exact_frame_count(0);
		return 1;

	case BOLO_OPCODE_SUBSCRIBE:
		requires_one_or_more_payloads();
		requires_exact_frame_count(1);
		requires_frame(0, BOLO_FRAME_STRING, 0);
		return 1;

	default:
		/* unrecognized opcode! */
		return 0;
	}

	return 1;
}

static const char* OPCODE_NAMES[] = {
	"HEARTBEAT",
	"SUBMIT",
	"BROADCAST",
	"FORGET",
	"REPLAY",
	"SUBSCRIBE",
	NULL
};
static const char*
s_opcode_name(unsigned int opcode)
{
	if (opcode > BOLO_OPCODE_SUBSCRIBE) {
		return "<unknown>";
	}
	return OPCODE_NAMES[opcode];
}

static char binfmt[65];
static const char*
s_in_binary(unsigned long i, int size)
{
	int off = 24;

	if (size > 4) {
		memcpy(binfmt, "<bad size>", 11);
		return binfmt;
	}

	binfmt[64] = '\0';
	while (size-- > 0) {
		binfmt[off + 0] = (i & 0x01) ? '1' : '0';
		binfmt[off + 1] = (i & 0x02) ? '1' : '0';
		binfmt[off + 2] = (i & 0x04) ? '1' : '0';
		binfmt[off + 3] = (i & 0x08) ? '1' : '0';
		binfmt[off + 4] = (i & 0x10) ? '1' : '0';
		binfmt[off + 5] = (i & 0x20) ? '1' : '0';
		binfmt[off + 6] = (i & 0x40) ? '1' : '0';
		binfmt[off + 7] = (i & 0x80) ? '1' : '0';
		off -= 8;
		i = i >> 8;
	}
	return binfmt + off;
}

void
bolo_message_fdump(FILE *io, bolo_message_t m)
{
	int i;
	bolo_frame_t f;

	fprintf(io, "version: %d\n"
	            "opcode:  %d [%s]\n"
	            "flags:   %02x (%sb)\n"
	            "payload: %04x (%sb)\n",
			m->version,
			m->opcode,  s_opcode_name(m->opcode),
			m->flags,   s_in_binary(m->flags,   sizeof(m->flags)),
			m->payload, s_in_binary(m->payload, sizeof(m->payload)));

	if (m->payload & BOLO_PAYLOAD_SAMPLE) fprintf(io, "          - SAMPLE (%04x)\n", BOLO_PAYLOAD_SAMPLE);
	if (m->payload & BOLO_PAYLOAD_TALLY)  fprintf(io, "          - TALLY  (%04x)\n", BOLO_PAYLOAD_TALLY);
	if (m->payload & BOLO_PAYLOAD_DELTA)  fprintf(io, "          - DELTA  (%04x)\n", BOLO_PAYLOAD_DELTA);
	if (m->payload & BOLO_PAYLOAD_STATE)  fprintf(io, "          - STATE  (%04x)\n", BOLO_PAYLOAD_STATE);
	if (m->payload & BOLO_PAYLOAD_EVENT)  fprintf(io, "          - EVENT  (%04x)\n", BOLO_PAYLOAD_EVENT);
	if (m->payload & BOLO_PAYLOAD_FACT)   fprintf(io, "          - FACT   (%04x)\n", BOLO_PAYLOAD_FACT);

	fprintf(io, "frames:  %d\n", m->nframes);
	for (f = m->frames; f; f = f->next, i++) {
		fprintf(io, "  [% 2d] ", i);
		switch (f->type) {
		case BOLO_FRAME_UINT:    fprintf(io, "UINT/%d\n",   f->length);
		case BOLO_FRAME_FLOAT:   fprintf(io, "FLOAT/%d\n",  f->length);
		case BOLO_FRAME_STRING:  fprintf(io, "STRING/%d\n", f->length);
		case BOLO_FRAME_TSTAMP:  fprintf(io, "TSTAMP/%d\n", f->length);
		case BOLO_FRAME_NIL:     fprintf(io, "NIL/%d\n",    f->length);
		default: fprintf(io, "\?\?\?(%02x)/%d\n", f->type, f->length);
		}
	}
}
