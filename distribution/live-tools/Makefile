# Makefile

# live-tools(7) - System Support Scripts
# Copyright (C) 2016-2020 The Debian Live team
# Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
#
# This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
# This is free software, and you are welcome to redistribute it
# under certain conditions; see COPYING for details.

SHELL := sh -e

LANGUAGES = $(shell cd manpages/po && ls)

SCRIPTS = bin/* live-tools.shutdown

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
	# Installing scripts
	mkdir -p $(DESTDIR)/bin
	cp -r bin/* $(DESTDIR)/bin

	# Installing systemd-shutdown script
	mkdir -p $(DESTDIR)/lib/systemd/system-shutdown
	cp live-tools.shutdown $(DESTDIR)/lib/systemd/system-shutdown/

	# Installing docs
	mkdir -p $(DESTDIR)/usr/share/doc/live-tools
	cp -r COPYING $(DESTDIR)/usr/share/doc/live-tools

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

uninstall:
	# Uninstalling scripts
	for SCRIPT in bin/*; \
	do \
		rm -f $(DESTDIR)/bin/$$(basename $${SCRIPT}); \
	done

	rmdir --ignore-fail-on-non-empty $(DESTDIR)/bin > /dev/null 2>&1 || true
	rmdir --ignore-fail-on-non-empty $(DESTDIR) > /dev/null 2>&1 || true

	# Uninstalling docs
	rm -rf $(DESTDIR)/usr/share/doc/live-tools

	rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr/share/doc > /dev/null 2>&1 || true
	rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr/share > /dev/null 2>&1 || true
	rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr > /dev/null 2>&1 || true
	rmdir --ignore-fail-on-non-empty $(DESTDIR) > /dev/null 2>&1 || true

	# Uninstalling manpages
	for MANPAGE in manpages/en/*; \
	do \
		SECTION="$$(basename $${MANPAGE} | awk -F. '{ print $$2 }')"; \
		rm -f $(DESTDIR)/usr/share/man/man$${SECTION}/$$(basename $${MANPAGE} .en.$${SECTION}).$${SECTION}; \
		rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr/share/man/man$${SECTION} > /dev/null 2>&1 || true; \
		rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr/share/man > /dev/null 2>&1 || true; \
		rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr/share > /dev/null 2>&1 || true; \
		rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr > /dev/null 2>&1 || true; \
		rmdir --ignore-fail-on-non-empty $(DESTDIR) > /dev/null 2>&1 || true; \
	done

	for LANGUAGE in $(LANGUAGES); \
	do \
		for MANPAGE in manpages/$${LANGUAGE}/*; \
		do \
			SECTION="$$(basename $${MANPAGE} | awk -F. '{ print $$3 }')"; \
			rm -f $(DESTDIR)/usr/share/man/$${LANGUAGE}/man$${SECTION}/$$(basename $${MANPAGE} .$${LANGUAGE}.$${SECTION}).$${SECTION}; \
			rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr/share/man/$${LANGUAGE}/man$${SECTION} > /dev/null 2>&1 || true; \
			rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr/share/man/$${LANGUAGE} > /dev/null 2>&1 || true; \
			rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr/share/man > /dev/null 2>&1 || true; \
			rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr/share > /dev/null 2>&1 || true; \
			rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr > /dev/null 2>&1 || true; \
			rmdir --ignore-fail-on-non-empty $(DESTDIR) > /dev/null 2>&1 || true; \
		done; \
	done

clean:

distclean: clean

reinstall: uninstall install
