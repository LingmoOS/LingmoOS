################################################################################
#
# heirloom-mailx
#
################################################################################

HEIRLOOM_MAILX_VERSION = 12.5
HEIRLOOM_MAILX_SOURCE = heirloom-mailx_$(HEIRLOOM_MAILX_VERSION).orig.tar.gz
HEIRLOOM_MAILX_SITE = http://snapshot.debian.org/archive/debian/20150815T155609Z/pool/main/h/heirloom-mailx
HEIRLOOM_MAILX_PATCH = heirloom-mailx_$(HEIRLOOM_MAILX_VERSION)-5.debian.tar.xz
HEIRLOOM_MAILX_LICENSE = BSD-4-Clause, Bellcore (base64), OpenVision (imap_gssapi), RSA Data Security (md5), Network Working Group (hmac), MPL-1.1 (nss)
HEIRLOOM_MAILX_LICENSE_FILES = COPYING
HEIRLOOM_MAILX_CPE_ID_VENDOR = heirloom
HEIRLOOM_MAILX_CPE_ID_PRODUCT = mailx
# 0011-outof-Introduce-expandaddr-flag.patch in the Debian patches
HEIRLOOM_MAILX_IGNORE_CVES += CVE-2014-7844
# 0014-globname-Invoke-wordexp-with-WRDE_NOCMD.patch in the Debian patches
HEIRLOOM_MAILX_IGNORE_CVES += CVE-2004-2771

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
HEIRLOOM_MAILX_DEPENDENCIES += openssl
endif

define HEIRLOOM_MAILX_CONFIGURE_CMDS
	(cd $(@D); $(TARGET_CONFIGURE_OPTS) $(SHELL) ./makeconfig)
endef

HEIRLOOM_MAILX_CFLAGS = $(TARGET_CFLAGS)

# -fPIC is needed to build with NIOS2 toolchains.
HEIRLOOM_MAILX_CFLAGS += -fPIC

ifeq ($(LINGMO_TOOLCHAIN_HAS_GCC_BUG_101916),y)
HEIRLOOM_MAILX_CFLAGS += -O0
endif

define HEIRLOOM_MAILX_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) \
		CFLAGS="$(HEIRLOOM_MAILX_CFLAGS)" \
		-C $(@D)
endef

define HEIRLOOM_MAILX_INSTALL_TARGET_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		PREFIX=/usr \
		UCBINSTALL=$(INSTALL) \
		STRIP=/bin/true \
		DESTDIR=$(TARGET_DIR) \
		install
endef

$(eval $(generic-package))
