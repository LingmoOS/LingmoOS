# Makefile

## live-config(7) - System Configuration Components
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


SHELL := sh -e

LANGUAGES = $(shell cd manpages/po && ls)

SCRIPTS = backend/*/*.init frontend/* components/*

all: build

test:
	@echo -n "Checking for syntax errors"

	@for SCRIPT in $(SCRIPTS); \
	do \
		sh -n $${SCRIPT}; \
		echo -n "."; \
	done

	@echo " done."

	@if [ -x "$$(which checkbashisms 2>/dev/null)" ]; \
	then \
		echo -n "Checking for bashisms"; \
		for SCRIPT in $(SCRIPTS); \
		do \
			checkbashisms -f -x $${SCRIPT}; \
			echo -n "."; \
		done; \
		echo " done."; \
	else \
		echo "W: checkbashisms - command not found"; \
		echo "I: checkbashisms can be obtained from: "; \
		echo "I:   http://git.debian.org/?p=devscripts/devscripts.git"; \
		echo "I: On Debian based systems, checkbashisms can be installed with:"; \
		echo "I:   apt-get install devscripts"; \
	fi

build:
	@echo "Nothing to build."

install:
	# Installing backend
	mkdir -p $(DESTDIR)/etc/init.d
	cp backend/sysvinit/live-config.init $(DESTDIR)/etc/init.d/live-config

	mkdir -p $(DESTDIR)/usr/lib/systemd/system $(DESTDIR)/usr/lib/systemd/system-generators
	cp backend/systemd/live-config.systemd $(DESTDIR)/usr/lib/systemd/system/live-config.service
	cp backend/systemd/live-config-getty-generator $(DESTDIR)/usr/lib/systemd/system-generators/

	# Installing frontend and components
	mkdir -p $(DESTDIR)/usr/bin $(DESTDIR)/usr/lib/live/config
	cp frontend/live-* $(DESTDIR)/usr/bin
	cp frontend/*.sh $(DESTDIR)/usr/lib/live/
	cp components/* $(DESTDIR)/usr/lib/live/config

	mkdir -p $(DESTDIR)/var/lib/live/config

	# Installing shared data
	mkdir -p $(DESTDIR)/usr/share/live/config
	cp -r VERSION share/* $(DESTDIR)/usr/share/live/config

	# Installing docs
	mkdir -p $(DESTDIR)/usr/share/doc/live-config
	cp -r COPYING examples $(DESTDIR)/usr/share/doc/live-config

	# Installing manpages
	for MANPAGE in manpages/en/*; \
	do \
		SECTION="$$(basename $${MANPAGE} | awk -F. '{ print $$2 }')"; \
		install -D -m 0644 $${MANPAGE} $(DESTDIR)/usr/share/man/man$${SECTION}/$$(basename $${MANPAGE}); \
	done

	for LANGUAGE in $(LANGUAGES); \
	do \
		for MANPAGE in manpages/$${LANGUAGE}/*; \
		do \
			SECTION="$$(basename $${MANPAGE} | awk -F. '{ print $$3 }')"; \
			install -D -m 0644 $${MANPAGE} $(DESTDIR)/usr/share/man/$${LANGUAGE}/man$${SECTION}/$$(basename $${MANPAGE} .$${LANGUAGE}.$${SECTION}).$${SECTION}; \
		done; \
	done

clean:
	@echo "Nothing to clean."

distclean: clean
	@echo "Nothing to distclean."

reinstall: uninstall install
