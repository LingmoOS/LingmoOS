# We have to do some funny stuff here with the manpages. In order to ensure
# a dist tarball doesn't get put out there without manpages, we keep those
# files listed in EXTRA_DIST no matter what. However, we only add them to
# man_MANS if --enable-asciidoc and/or --enable-doxygen are used.

ASCIIDOC_MANS = \
	checkupdates.8 \
	paccache.8 \
	pacdiff.8 \
	paclist.8 \
	paclog-pkglist.8 \
	pacscripts.8 \
	pacsearch.8 \
	pacsort.8 \
	pactree.8 \
	rankmirrors.8 \
	updpkgsums.8

HTML_MANPAGES = \
	checkupdates.8.html \
	paccache.8.html \
	pacdiff.8.html \
	paclist.8.html \
	paclog-pkglist.8.html \
	pacscripts.8.html \
	pacsearch.8.html \
	pacsort.8.html \
	pactree.8.html \
	rankmirrors.8.html \
	updpkgsums.8.html

HTML_OTHER = \
	contributing.html \
	release-procedure.html

HTML_DOCS = \
	$(HTML_MANPAGES) \
	$(HTML_OTHER)

EXTRA_DIST = \
	asciidoc.conf \
	asciidoc-override.css \
	checkupdates.8.adoc \
	paccache.8.adoc \
	pacdiff.8.adoc \
	paclist.8.adoc \
	paclog-pkglist.8.adoc \
	pacscripts.8.adoc \
	pacsearch.8.adoc \
	pacsort.8.adoc \
	pactree.8.adoc \
	rankmirrors.8.adoc \
	updpkgsums.8.adoc \
	footer.adoc \
	$(ASCIIDOC_MANS)

# Files that should be removed, but which Automake does not know.
MOSTLYCLEANFILES = *.xml $(ASCIIDOC_MANS) $(HTML_DOCS) website.tar.gz

# Ensure manpages are fresh when building a dist tarball
dist-hook:
	$(MAKE) $(AM_MAKEFLAGS) clean
	$(MAKE) $(AM_MAKEFLAGS) all

if USE_GIT_VERSION
GIT_VERSION := $(shell sh -c 'git describe --long --abbrev=4 --dirty | sed s/^v//')
REAL_PACKAGE_VERSION = $(GIT_VERSION)
else
REAL_PACKAGE_VERSION = $(PACKAGE_VERSION)
endif

man_MANS =
dist_man_MANS = $(ASCIIDOC_MANS)

html: $(HTML_DOCS)

website: website.tar.gz

.PHONY: html website

website.tar.gz: html
	$(AM_V_GEN)bsdtar czf $@ $(HTML_DOCS) \
		asciidoc-override.css \
		-C /etc/asciidoc/stylesheets/ \
		asciidoc.css \
		-C /etc/asciidoc/javascripts/ \
		asciidoc.js \
		-C /etc/asciidoc/ \
		images

pkgdatadir = ${datadir}/${PACKAGE}

ASCIIDOC_OPTS = \
	-f $(srcdir)/asciidoc.conf \
	-a lpm_contrib_version="$(REAL_PACKAGE_VERSION)" \
	-a pkgdatadir=$(pkgdatadir) \
	-a localstatedir=$(localstatedir) \
	-a sysconfdir=$(sysconfdir)

A2X_OPTS = \
	--no-xmllint \
	-d manpage \
	-f manpage \
	--xsltproc-opts='-param man.endnotes.list.enabled 0 -param man.endnotes.are.numbered 0'

# These rules are due to the includes and files of the asciidoc text
$(ASCIIDOC_MANS): asciidoc.conf footer.adoc Makefile.am
	$(AM_V_GEN)a2x $(A2X_OPTS) --asciidoc-opts="$(ASCIIDOC_OPTS) --out-file=./$@.xml" $(srcdir)/$@.adoc

%.html: %.adoc
	$(AM_V_GEN)asciidoc $(ASCIIDOC_OPTS) -o - $*.adoc | \
		sed -e 's/\r$$//' > $@

HACKING.html: ../HACKING
	$(AM_V_GEN)asciidoc $(ASCIIDOC_OPTS) -o - ../HACKING | \
		sed -e 's/\r$$//' > $@

# Customizations for certain HTML docs
$(HTML_MANPAGES): asciidoc.conf footer.adoc Makefile.am
$(HTML_OTHER): asciidoc.conf Makefile.am
%.html: ASCIIDOC_OPTS += -a linkcss -a toc -a icons -a max-width=960px -a stylesheet=asciidoc-override.css
%.8.html: ASCIIDOC_OPTS += -d manpage
%.5.html: ASCIIDOC_OPTS += -d manpage
%.3.html: ASCIIDOC_OPTS += -d manpage

# Dependency rules
checkupdates.8 checkupdates.8.html: checkupdates.8.adoc
paccache.8 paccache.8.html: paccache.8.adoc
pacdiff.8 pacdiff.8.html: pacdiff.8.adoc
paclist.8 paclist.8.html: paclist.8.adoc
paclog-pkglist.8 paclog-pkglist.8.html: paclog-pkglist.8.adoc
pacscripts.8 pacscripts.8.html: pacscripts.8.adoc
pacsearch.8 pacsearch.8.html: pacsearch.8.adoc
pacsort.8 pacsort.8.html: pacsort.8.adoc
pactree.8 pactree.8.html: pactree.8.adoc
rankmirrors.8 rankmirrors.8.html: rankmirrors.8.adoc
updpkgsums.8 updpkgsums.8.html: updpkgsums.8.adoc
