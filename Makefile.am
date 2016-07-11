#
# Makefile.am - bolo automake file
#

AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS  = -I build
EXTRA_DIST       =
CLEANFILES       =

AM_CPPFLAGS = -I$(top_srcdir)/include

############################################################

bin_PROGRAMS = qname
qname_SOURCES = src/qname.c \
                src/bin/qname.c

############################################################

src/qname.c: src/qname_chars.inc

CLEANFILES += src/qname_chars.inc
src/qname_chars.inc: src/qname_chars.tbl
	$(srcdir)/util/tablegen < $< > $@
