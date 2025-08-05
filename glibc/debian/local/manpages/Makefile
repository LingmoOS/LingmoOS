#! /usr/bin/make -f

DIRS := de es fr id pl pt_BR

SGML_MAN = locale-gen.8

all: $(patsubst %.pod,%.1,$(wildcard *.pod)) $(SGML_MAN) po4a-man
%.1: %.pod
	pod2man --center="Debian GNU/Linux" --release="etch" $< > $@

locale-gen.8: locale-gen.8.sgml
	docbook-to-man $< > $@

po4a-man:
	po4a -q po4a.cfg

clean:
	po4a -q po4a.cfg
	$(foreach dir, $(DIRS), rm -rf $(dir)/*.8)
	rm -f po/*~
