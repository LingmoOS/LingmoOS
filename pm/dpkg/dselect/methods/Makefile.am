## Process this file with automake to produce Makefile.in

nobase_dist_perllib_DATA = \
	Dselect/Method.pm \
	Dselect/Method/Ftp.pm \
	# EOL

nobase_dist_methods_DATA = \
	file/names \
	file/desc.file \
	ftp/names \
	ftp/desc.ftp \
	media/names \
	media/desc.media \
	# EOL

nobase_methods_SCRIPTS = \
	file/install \
	file/setup \
	file/update \
	ftp/install \
	ftp/setup \
	ftp/update \
	media/install \
	media/setup \
	media/update \
	# EOL

EXTRA_DIST = \
	file/setup.sh \
	file/update.sh \
	file/install.sh \
	ftp/setup.pl \
	ftp/update.pl \
	ftp/install.pl \
	media/setup.sh \
	media/update.sh \
	media/install.sh \
	media/README.media \
	# EOL

CLEANFILES = \
	$(nobase_methods_SCRIPTS) \
	# EOL

SUFFIXES =

include $(top_srcdir)/build-aux/subst.am

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(admindir)/methods/mnt
	$(MKDIR_P) $(DESTDIR)$(admindir)/methods/file
	$(MKDIR_P) $(DESTDIR)$(admindir)/methods/ftp
	$(MKDIR_P) $(DESTDIR)$(admindir)/methods/media
