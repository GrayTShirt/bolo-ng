# bolo Makefile
#
# author:  James Hunt <james@niftylogic.com>
# created: 2016-07-12
#

CC       ?= clang
AFLCC    ?= afl-clang
TABLEGEN := util/tablegen

CPPFLAGS += -I./include

default: all
all:
clean:
	rm -f $(CLEAN_FILES)

# files to remove on `make clean'
CLEAN_FILES :=

# header files required by all object files.
CORE_H := include/bolo.h

# header files that should be distributed.
DIST_H := include/bolo.h


# source files that comprise the Qualified Name implementation.
QNAME_SRC  := src/qname.c
QNAME_OBJ  := $(QNAME_SRC:.c=.o)
QNAME_FUZZ := $(QNAME_SRC:.c=.fuzz.o)
CLEAN_FILES += $(QNAME_OBJ) $(QNAME_FUZZ)

src/qname_chars.inc: src/qname_chars.tbl $(TABLEGEN)
	$(TABLEGEN) >$@ <$<
src/qname.o: src/qname.c $(CORE_H) src/qname_chars.inc
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

# source files that comprise the Message implementation.
MSG_SRC  := src/msg.c
MSG_OBJ  := $(MSG_SRC:.c=.o)
MSG_FUZZ := $(MSG_SRC:.c=.fuzz.o)
CLEAN_FILES += $(MSG_OBJ) $(MSG_FUZZ)

# source files that comprise the Ring Buffer implementation.
RING_SRC  := src/ring.c
RING_OBJ  := $(RING_SRC:.c=.o)
RING_FUZZ := $(RING_SRC:.c=.fuzz.o)
CLEAN_FILES += $(RING_OBJ) $(RING_FUZZ)


# scripts that perform Contract Testing.
CONTRACT_TEST_SCRIPTS := t/contract/qname \
                         t/contract/msg \
                         t/contract/ring

# binaries that the Contract Tests run.
CONTRACT_TEST_BINS := t/contract/r/qname-string \
                      t/contract/r/qname-equiv \
                      t/contract/r/qname-match \
                      t/contract/r/msg-in \
                      t/contract/r/ring
CLEAN_FILES   += $(CONTRACT_TEST_BINS)

contract-tests: $(CONTRACT_TEST_BINS)
t/contract/r/qname-string: t/contract/r/qname-string.o $(QNAME_OBJ)
t/contract/r/qname-equiv:  t/contract/r/qname-equiv.o  $(QNAME_OBJ)
t/contract/r/qname-match:  t/contract/r/qname-match.o  $(QNAME_OBJ)
t/contract/r/msg-in:       t/contract/r/msg-in.o       $(MSG_OBJ)
t/contract/r/ring:         t/contract/r/ring.o         $(RING_OBJ)


# scripts that perform Memory Testing.
MEM_TEST_SCRIPTS := t/mem/ring

# binaries that the Memory Tests run.
MEM_TEST_BINS := t/mem/r/ring
CLEAN_FILES   += $(MEM_TEST_BINS)

mem-tests: $(MEM_TEST_BINS)
t/mem/r/ring: t/mem/r/ring.c $(RING_OBJ)
mem-test: mem-tests
	for test in $(MEM_TEST_SCRIPTS); do echo $$test; $$test; echo; done


# binaries that the Fuzz Tests run.
FUZZ_TEST_BINS := t/fuzz/r/qname \
                  t/fuzz/r/msg \
                  t/fuzz/r/ring
CLEAN_FILES   += $(FUZZ_TEST_BINS)

fuzz-tests: $(FUZZ_TEST_BINS)
t/fuzz/r/qname: t/fuzz/r/qname.o $(QNAME_FUZZ)
t/fuzz/r/msg:   t/fuzz/r/msg.o   $(MSG_FUZZ)
t/fuzz/r/ring:  t/fuzz/r/ring.o  $(RING_FUZZ)


tests: $(CONTRACT_TEST_BINS)
test: tests
	for test in $(CONTRACT_TEST_SCRIPTS); do echo $$test; $$test; echo; done


%.fuzz.o: %.c
	$(AFLCC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<


.PHONY: all clean
