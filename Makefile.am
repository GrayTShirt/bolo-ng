#
# Makefile.am - bolo automake file
#

AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS  = -I build
EXTRA_DIST       =
CLEANFILES       =

AM_CPPFLAGS = -I$(top_srcdir)/include

############################################################

TESTS = t/qname

check_PROGRAMS = qname-string \
                 qname-equiv \
                 qname-match

qname_string_SOURCES = src/qname.c \
                       t/bin/qname-string.c

qname_equiv_SOURCES  = src/qname.c \
                       t/bin/qname-equiv.c

qname_match_SOURCES  = src/qname.c \
                       t/bin/qname-match.c

############################################################

src/qname.c: src/qname_chars.inc

CLEANFILES += src/qname_chars.inc
src/qname_chars.inc: src/qname_chars.tbl
	$(srcdir)/util/tablegen < $< > $@
