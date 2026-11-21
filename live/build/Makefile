# Makefile

SHELL := sh -e

LANGUAGES = $(shell cd manpages/po && ls)

SCRIPTS = frontend/* functions/* examples/auto/* examples/hooks/*.chroot examples/hooks/reproducible/*.chroot scripts/*.sh scripts/*/* share/bin/* share/hooks/*/*

all: build

test:
	@echo -n "Checking for syntax errors"

	@for SCRIPT in $(SCRIPTS); \
	do \
		sh -n $${SCRIPT}; \
		echo -n "."; \
	done

	@echo " done."

	@echo -n "Checking for bashisms"

	@if [ -x /usr/bin/checkbashisms ]; \
	then \
		for SCRIPT in $(SCRIPTS); \
		do \
			checkbashisms -f -x $${SCRIPT}; \
			echo -n "."; \
		done; \
	else \
		echo "WARNING: Skipping bashism test - you need to install devscripts."; \
	fi

	@echo " done."

build:
	@echo "Nothing to build."

install:
	# Installing shared data
	mkdir -p $(DESTDIR)/usr/share/live/build
	cp -r data functions $(DESTDIR)/usr/share/live/build
	sed -e 's/.*(\(.*\)).*/\1/; s/^[0-9]://; q' debian/changelog >$(DESTDIR)/usr/share/live/build/VERSION
	cp -r share/* $(DESTDIR)/usr/share/live/build

	# Installing executables
	mkdir -p $(DESTDIR)/usr/bin
	cp -a frontend/* $(DESTDIR)/usr/bin

	mkdir -p $(DESTDIR)/usr/lib/live
	cp -a scripts/* $(DESTDIR)/usr/lib/live

	# Installing documentation
	mkdir -p $(DESTDIR)/usr/share/doc/live-build
	cp -r COPYING examples $(DESTDIR)/usr/share/doc/live-build

	# Installing manpages
	for MANPAGE in manpages/en/*; \
	do \
		SECTION="$$(basename $${MANPAGE} | awk -F. '{ print $$2 }')"; \
		install -D -m 0644 $${MANPAGE} $(DESTDIR)/usr/share/man/man$${SECTION}/$$(basename $${MANPAGE}); \
	done

	make -C manpages
	for LANGUAGE in $(LANGUAGES); \
	do \
		for MANPAGE in manpages/$${LANGUAGE}/*; \
		do \
			SECTION="$$(basename $${MANPAGE} | awk -F. '{ print $$3 }')"; \
			install -D -m 0644 $${MANPAGE} $(DESTDIR)/usr/share/man/$${LANGUAGE}/man$${SECTION}/$$(basename $${MANPAGE} .$${LANGUAGE}.$${SECTION}).$${SECTION}; \
		done; \
	done

uninstall:
	# Uninstalling shared data
	rm -rf $(DESTDIR)/usr/share/live/build
	rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr/share/live > /dev/null 2>&1 || true

	# Uninstalling executables
	rm -f $(DESTDIR)/usr/bin/lb $(DESTDIR)/usr/bin/live-build

	# Uninstalling documentation
	rm -rf $(DESTDIR)/usr/share/doc/live-build

	# Uninstalling manpages
	for MANPAGE in manpages/en/*; \
	do \
		SECTION="$$(basename $${MANPAGE} | awk -F. '{ print $$2 }')"; \
		rm -f $(DESTDIR)/usr/share/man/man$${SECTION}/$$(basename $${MANPAGE} .en.$${SECTION}).$${SECTION}; \
	done

	for LANGUAGE in $(LANGUAGES); \
	do \
		for MANPAGE in manpages/$${LANGUAGE}/*; \
		do \
			SECTION="$$(basename $${MANPAGE} | awk -F. '{ print $$3 }')"; \
			rm -f $(DESTDIR)/usr/share/man/$${LANGUAGE}/man$${SECTION}/$$(basename $${MANPAGE} .$${LANGUAGE}.$${SECTION}).$${SECTION}; \
		done; \
	done

clean:

distclean:

reinstall: uninstall install

.PHONY: all test build install uninstall clean distclean reinstall
