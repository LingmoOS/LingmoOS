PROGS = \
	extract-cert

include $(top_rulesdir)/Makefile.inc

CPPFLAGS += -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
extract-cert: LDLIBS += -lcrypto
