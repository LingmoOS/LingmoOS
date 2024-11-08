#!/usr/bin/make -f
CRONTAB = $(CURDIR)/crontab

install:
	@if [ `whoami` = d-i ]; then \
	    crontab $(CRONTAB); \
	else \
	    echo >&2 'Error: must run as "d-i" user.'; \
	    exit 1; \
	fi

check:
	@-crontab -l | diff -u $(CRONTAB) -
