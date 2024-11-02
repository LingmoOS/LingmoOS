NULL =

SUBDIRS =	\
	icons	\
	$(NULL)

@INTLTOOL_DESKTOP_RULE@
autostartdir = $(sysconfdir)/xdg/autostart
autostart_in_files = lingmo-volume-control-applet.desktop.in
autostart_DATA = $(autostart_in_files:.desktop.in=.desktop)

appsdir = $(datadir)/applications
apps_in_files = lingmo-volume-control.desktop.in
apps_DATA = $(apps_in_files:.desktop.in=.desktop)

gsettingsschema_in_files = org.lingmo.media.gschema.xml.in
gsettings_SCHEMAS = $(gsettingsschema_in_files:.xml.in=.xml)
@GSETTINGS_RULES@
@INTLTOOL_XML_RULE@

EXTRA_DIST =					\
	$(autostart_in_files)			\
	lingmo-volume-control.desktop.in.in	\
	$(gsettingsschema_in_files)		\
	$(NULL)

CLEANFILES =					\
	lingmo-volume-control.desktop		\
	$(gsettings_SCHEMAS)			\
	$(NULL)

DISTCLEANFILES =				\
	lingmo-volume-control-applet.desktop	\
	$(NULL)

MAINTAINERCLEANFILES =			\
	Makefile.in			\
	$(gsettings_SCHEMAS:.xml=.valid)

-include $(top_srcdir)/git.mk
