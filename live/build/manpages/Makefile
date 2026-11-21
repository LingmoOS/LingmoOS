# Makefile

SHELL := sh -e

LANGUAGES = $(shell cd po && ls)

all: build

po4a.cfg:
	echo "[po4a_langs] $(LANGUAGES)" > po4a.cfg
	echo "[po4a_paths] pot/\$$master.pot \$$lang:po/\$$lang/\$$master.po" >> po4a.cfg

	for MANPAGE in en/*; \
	do \
		SECTION="$$(basename $${MANPAGE} | sed -e 's|\.|\n|g' | tail -n1)"; \
		echo "[type: man] $${MANPAGE} \$$lang:\$$lang/$$(basename $${MANPAGE} .$${SECTION}).\$$lang.$${SECTION}" >> po4a.cfg; \
	done

update:
	./bin/update-version.sh

check:
	@echo -n "Checking the integrity of .po files "

	@if [ -x /usr/bin/msgfmt ]; \
	then \
		for POFILE in po/*/*; \
		do \
			msgfmt --check --output-file=/dev/null $${POFILE}; \
			echo -n "."; \
		done; \
	else \
		echo "E: msgfmt - command not found"; \
		echo "I: gettext can be obtained from http://www.gnu.org/software/gettext/"; \
		echo "I: On Debian based systems, gettext can be installed with 'apt-get install gettext'."; \
		exit 1; \
	fi

	@echo " done!"

build: check po4a.cfg
	@if [ ! -x "$$(which po4a 2>/dev/null)" ]; \
	then \
		echo "E: po4a - command not found"; \
		echo "I: po4a can be obtained from https://po4a.org"; \
		echo "I: On Debian based systems, po4a can be installed with 'apt-get install po4a'."; \
		exit 1; \
	fi

	po4a --copyright-holder "Debian Live Project" --keep 0 --package-name live-build --package-version $(shell cd .. && dpkg-parsechangelog -S Version) po4a.cfg

clean:
	rm -f po/*/*.po~

distclean: clean
	rm -f po4a.cfg
	rm -rf $(LANGUAGES)

rebuild: distclean update build

.PHONY: all update check build clean distclean rebuild
