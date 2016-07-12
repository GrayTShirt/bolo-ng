#
# Makefile.am - bolo automake file
#

AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS  = -I build
EXTRA_DIST       =
CLEANFILES       =

AM_CPPFLAGS = -I$(top_srcdir)/include

############################################################

TESTS = t/contract/qname

check_PROGRAMS = t/contract/r/qname-string \
                 t/contract/r/qname-equiv \
                 t/contract/r/qname-match

t_contract_r_qname_string_SOURCES = src/qname.c \
                                    t/contract/r/qname-string.c

t_contract_r_qname_equiv_SOURCES  = src/qname.c \
                                    t/contract/r/qname-equiv.c

t_contract_r_qname_match_SOURCES  = src/qname.c \
                                    t/contract/r/qname-match.c

############################################################

src/qname.c: src/qname_chars.inc

CLEANFILES += src/qname_chars.inc
src/qname_chars.inc: src/qname_chars.tbl
	$(srcdir)/util/tablegen < $< > $@
