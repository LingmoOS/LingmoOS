dir = usr/share/graphics

files = \
	logo_lingmo.png \
	logo_lingmo_dark.png \
	logo_lingmo.ini \
	note_icon.png \
	warning_icon.png

build-local:: $(files)

clean-local::
	rm -f logo_lingmo.png logo_lingmo_dark.png

install-local:: $(outdir)
	ln -fs logo_lingmo.png $(outdir)/logo_installer.png
	ln -fs logo_lingmo_dark.png $(outdir)/logo_installer_dark.png
	ln -fs logo_lingmo.ini $(outdir)/logo_installer.ini

logo_lingmo.png: logo_lingmo_hydrogen.svg
	rsvg-convert $< > $@

logo_lingmo_dark.png: logo_lingmo_hydrogen_dark.svg
	rsvg-convert $< > $@

note_icon.png: note_icon.svg
	rsvg-convert $< > $@

warning_icon.png: warning_icon.svg
	rsvg-convert $< > $@

include ../../../../Makefile.inc
