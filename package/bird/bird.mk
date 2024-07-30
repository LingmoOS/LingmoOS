################################################################################
#
# bird
#
################################################################################

BIRD_VERSION = 2.0.12
BIRD_SITE = https://bird.network.cz/download
BIRD_LICENSE = GPL-2.0+
BIRD_LICENSE_FILES = README
BIRD_CPE_ID_VENDOR = nic
BIRD_SELINUX_MODULES = bird
BIRD_DEPENDENCIES = host-flex host-bison

# 0001-configure.ac-fix-build-with-autoconf-2.70.patch
BIRD_AUTORECONF = YES

ifeq ($(LINGMO_PACKAGE_BIRD_CLIENT),y)
BIRD_CONF_OPTS += --enable-client
BIRD_DEPENDENCIES += ncurses readline
else
BIRD_CONF_OPTS += --disable-client
endif

ifeq ($(LINGMO_PACKAGE_LIBSSH),y)
BIRD_CONF_OPTS += --enable-libssh
BIRD_DEPENDENCIES += libssh
else
BIRD_CONF_OPTS += --disable-libssh
endif

BIRD_PROTOCOLS = \
	$(if $(LINGMO_PACKAGE_BIRD_BFD),bfd) \
	$(if $(LINGMO_PACKAGE_BIRD_BABEL),babel) \
	$(if $(LINGMO_PACKAGE_BIRD_BGP),bgp) \
	$(if $(LINGMO_PACKAGE_BIRD_MRT),mrt) \
	$(if $(LINGMO_PACKAGE_BIRD_OSPF),ospf) \
	$(if $(LINGMO_PACKAGE_BIRD_PERF),perf) \
	$(if $(LINGMO_PACKAGE_BIRD_PIPE),pipe) \
	$(if $(LINGMO_PACKAGE_BIRD_RADV),radv) \
	$(if $(LINGMO_PACKAGE_BIRD_RIP),rip) \
	$(if $(LINGMO_PACKAGE_BIRD_RPKI),rpki) \
	$(if $(LINGMO_PACKAGE_BIRD_STATIC),static)

BIRD_CONF_OPTS += --with-protocols=$(subst $(space),$(comma),$(strip $(BIRD_PROTOCOLS)))

$(eval $(autotools-package))
