################################################################################
#
# httping
#
################################################################################

HTTPING_VERSION = 2.5
HTTPING_SOURCE = httping-$(HTTPING_VERSION).tgz
HTTPING_SITE = http://www.vanheusden.com/httping
HTTPING_LICENSE = GPL-2.0
HTTPING_LICENSE_FILES = license.txt
HTTPING_LDFLAGS = $(TARGET_LDFLAGS) \
	$(TARGET_NLS_LIBS) \
	$(if $(LINGMO_PACKAGE_LIBICONV),-liconv)
HTTPING_DEPENDENCIES = \
	$(TARGET_NLS_DEPENDENCIES) \
	$(if $(LINGMO_PACKAGE_LIBICONV),libiconv) \
	$(if $(LINGMO_PACKAGE_NCURSES_WCHAR),ncurses) \
	$(if $(LINGMO_PACKAGE_OPENSSL),openssl) \
	$(if $(LINGMO_PACKAGE_FFTW_DOUBLE),fftw-double)
HTTPING_MAKE_OPTS = $(TARGET_CONFIGURE_OPTS) \
	FW=$(if $(LINGMO_PACKAGE_FFTW_DOUBLE),yes,no) \
	NC=$(if $(LINGMO_PACKAGE_NCURSES_WCHAR),yes,no) \
	SSL=$(if $(LINGMO_PACKAGE_OPENSSL),yes,no) \
	TFO=$(if $(LINGMO_PACKAGE_HTTPING_TFO),yes,no) \
	NO_GETTEXT=$(if $(LINGMO_SYSTEM_ENABLE_NLS),no,yes)

define HTTPING_BUILD_CMDS
	$(HTTPING_MAKE_OPTS) LDFLAGS="$(HTTPING_LDFLAGS)" \
		$(MAKE) DEBUG=no -C $(@D)
endef

define HTTPING_INSTALL_TARGET_CMDS
	$(HTTPING_MAKE_OPTS) $(MAKE) DESTDIR=$(TARGET_DIR) -C $(@D) install
endef

$(eval $(generic-package))
