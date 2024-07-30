################################################################################
#
# busybox
#
################################################################################

BUSYBOX_VERSION = 1.36.1
BUSYBOX_SITE = https://www.busybox.net/downloads
BUSYBOX_SOURCE = busybox-$(BUSYBOX_VERSION).tar.bz2
BUSYBOX_LICENSE = GPL-2.0, bzip2-1.0.4
BUSYBOX_LICENSE_FILES = LICENSE archival/libarchive/bz/LICENSE
BUSYBOX_CPE_ID_VENDOR = busybox

# 0003-libbb-sockaddr2str-ensure-only-printable-characters-.patch
# 0004-nslookup-sanitize-all-printed-strings-with-printable.patch
BUSYBOX_IGNORE_CVES += CVE-2022-28391

BUSYBOX_CFLAGS = \
	$(TARGET_CFLAGS)

BUSYBOX_LDFLAGS = \
	$(TARGET_LDFLAGS)

# Packages that provide commands that may also be busybox applets:
BUSYBOX_DEPENDENCIES = \
	$(if $(LINGMO_PACKAGE_ATTR),attr) \
	$(if $(LINGMO_PACKAGE_BASH),bash) \
	$(if $(LINGMO_PACKAGE_BC),bc) \
	$(if $(LINGMO_PACKAGE_BINUTILS),binutils) \
	$(if $(LINGMO_PACKAGE_COREUTILS),coreutils) \
	$(if $(LINGMO_PACKAGE_CPIO),cpio) \
	$(if $(LINGMO_PACKAGE_DCRON),dcron) \
	$(if $(LINGMO_PACKAGE_DEBIANUTILS),debianutils) \
	$(if $(LINGMO_PACKAGE_DIFFUTILS),diffutils) \
	$(if $(LINGMO_PACKAGE_DOS2UNIX),dos2unix) \
	$(if $(LINGMO_PACKAGE_DOSFSTOOLS),dosfstools) \
	$(if $(LINGMO_PACKAGE_E2FSPROGS),e2fsprogs) \
	$(if $(LINGMO_PACKAGE_FBSET),fbset) \
	$(if $(LINGMO_PACKAGE_GAWK),gawk) \
	$(if $(LINGMO_PACKAGE_GREP),grep) \
	$(if $(LINGMO_PACKAGE_GZIP),gzip) \
	$(if $(LINGMO_PACKAGE_I2C_TOOLS),i2c-tools) \
	$(if $(LINGMO_PACKAGE_IFENSLAVE),ifenslave) \
	$(if $(LINGMO_PACKAGE_IFPLUGD),ifplugd) \
	$(if $(LINGMO_PACKAGE_IFUPDOWN),ifupdown) \
	$(if $(LINGMO_PACKAGE_IPCALC),ipcalc) \
	$(if $(LINGMO_PACKAGE_IPROUTE2),iproute2) \
	$(if $(LINGMO_PACKAGE_IPUTILS),iputils) \
	$(if $(LINGMO_PACKAGE_KMOD),kmod) \
	$(if $(LINGMO_PACKAGE_LESS),less) \
	$(if $(LINGMO_PACKAGE_LSOF),lsof) \
	$(if $(LINGMO_PACKAGE_MTD),mtd) \
	$(if $(LINGMO_PACKAGE_NET_TOOLS),net-tools) \
	$(if $(LINGMO_PACKAGE_NETCAT),netcat) \
	$(if $(LINGMO_PACKAGE_NETCAT_OPENBSD),netcat-openbsd) \
	$(if $(LINGMO_PACKAGE_NMAP),nmap) \
	$(if $(LINGMO_PACKAGE_NTP),ntp) \
	$(if $(LINGMO_PACKAGE_PCIUTILS),pciutils) \
	$(if $(LINGMO_PACKAGE_PROCPS_NG),procps-ng) \
	$(if $(LINGMO_PACKAGE_PSMISC),psmisc) \
	$(if $(LINGMO_PACKAGE_START_STOP_DAEMON),start-stop-daemon) \
	$(if $(LINGMO_PACKAGE_SYSKLOGD),sysklogd) \
	$(if $(LINGMO_PACKAGE_SYSTEMD),systemd) \
	$(if $(LINGMO_PACKAGE_SYSVINIT),sysvinit) \
	$(if $(LINGMO_PACKAGE_TAR),tar) \
	$(if $(LINGMO_PACKAGE_TFTPD),tftpd) \
	$(if $(LINGMO_PACKAGE_TRACEROUTE),traceroute) \
	$(if $(LINGMO_PACKAGE_UNZIP),unzip) \
	$(if $(LINGMO_PACKAGE_USBUTILS),usbutils) \
	$(if $(LINGMO_PACKAGE_UTIL_LINUX),util-linux) \
	$(if $(LINGMO_PACKAGE_VIM),vim) \
	$(if $(LINGMO_PACKAGE_WATCHDOG),watchdog) \
	$(if $(LINGMO_PACKAGE_WGET),wget) \
	$(if $(LINGMO_PACKAGE_WHOIS),whois)

# Link against libtirpc if available so that we can leverage its RPC
# support for NFS mounting with BusyBox
ifeq ($(LINGMO_PACKAGE_LIBTIRPC),y)
BUSYBOX_DEPENDENCIES += libtirpc host-pkgconf
BUSYBOX_CFLAGS += "`$(PKG_CONFIG_HOST_BINARY) --cflags libtirpc`"
# Don't use LDFLAGS for -ltirpc, because LDFLAGS is used for
# the non-final link of modules as well.
BUSYBOX_CFLAGS_busybox += "`$(PKG_CONFIG_HOST_BINARY) --libs libtirpc`"
endif

# Allows the build system to tweak CFLAGS
BUSYBOX_MAKE_ENV = \
	$(TARGET_MAKE_ENV) \
	CFLAGS="$(BUSYBOX_CFLAGS)" \
	CFLAGS_busybox="$(BUSYBOX_CFLAGS_busybox)"

ifeq ($(LINGMO_REPRODUCIBLE),y)
BUSYBOX_MAKE_ENV += \
	KCONFIG_NOTIMESTAMP=1
endif

BUSYBOX_MAKE_OPTS = \
	AR="$(TARGET_AR)" \
	NM="$(TARGET_NM)" \
	RANLIB="$(TARGET_RANLIB)" \
	CC="$(TARGET_CC)" \
	ARCH=$(NORMALIZED_ARCH) \
	PREFIX="$(TARGET_DIR)" \
	EXTRA_LDFLAGS="$(BUSYBOX_LDFLAGS)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	CONFIG_PREFIX="$(TARGET_DIR)" \
	SKIP_STRIP=y

# specifying BUSYBOX_CONFIG_FILE on the command-line overrides the .config
# setting.
# check-package disable Ifdef
ifndef BUSYBOX_CONFIG_FILE
BUSYBOX_CONFIG_FILE = $(call qstrip,$(LINGMO_PACKAGE_BUSYBOX_CONFIG))
endif

BUSYBOX_KCONFIG_SUPPORTS_DEFCONFIG = NO
BUSYBOX_KCONFIG_FILE = $(BUSYBOX_CONFIG_FILE)
BUSYBOX_KCONFIG_FRAGMENT_FILES = $(call qstrip,$(LINGMO_PACKAGE_BUSYBOX_CONFIG_FRAGMENT_FILES))
BUSYBOX_KCONFIG_EDITORS = menuconfig xconfig gconfig
BUSYBOX_KCONFIG_OPTS = $(BUSYBOX_MAKE_OPTS)

ifeq ($(LINGMO_PACKAGE_BUSYBOX_INDIVIDUAL_BINARIES),y)
define BUSYBOX_PERMISSIONS
# Set permissions on all applets with BB_SUID_REQUIRE and BB_SUID_MAYBE.
# 12 Applets are pulled from applets.h using grep command :
#  grep -r -e "APPLET.*BB_SUID_REQUIRE\|APPLET.*BB_SUID_MAYBE" \
#  $(@D)/include/applets.h
# These applets are added to the device table and the makedev file
# ignores the files with type 'F' ( optional files).
	/usr/bin/wall 			 F 4755 0  0 - - - - -
	/bin/ping 			 F 4755 0  0 - - - - -
	/bin/ping6 			 F 4755 0  0 - - - - -
	/usr/bin/crontab 		 F 4755 0  0 - - - - -
	/sbin/findfs 			 F 4755 0  0 - - - - -
	/bin/login 			 F 4755 0  0 - - - - -
	/bin/mount 			 F 4755 0  0 - - - - -
	/usr/bin/passwd 		 F 4755 0  0 - - - - -
	/bin/su 			 F 4755 0  0 - - - - -
	/usr/bin/traceroute 		 F 4755 0  0 - - - - -
	/usr/bin/traceroute6 		 F 4755 0  0 - - - - -
	/usr/bin/vlock 			 F 4755 0  0 - - - - -
endef
else
define BUSYBOX_PERMISSIONS
	/bin/busybox                     f 4755 0  0 - - - - -
endef
endif

# If mdev will be used for device creation enable it and copy S10mdev to /etc/init.d
ifeq ($(LINGMO_ROOTFS_DEVICE_CREATION_DYNAMIC_MDEV),y)
define BUSYBOX_INSTALL_MDEV_SCRIPT
	$(INSTALL) -D -m 0755 package/busybox/S10mdev \
		$(TARGET_DIR)/etc/init.d/S10mdev
endef
define BUSYBOX_INSTALL_MDEV_CONF
	$(INSTALL) -D -m 0644 package/busybox/mdev.conf \
		$(TARGET_DIR)/etc/mdev.conf
endef
define BUSYBOX_SET_MDEV
	$(call KCONFIG_ENABLE_OPT,CONFIG_MDEV)
	$(call KCONFIG_ENABLE_OPT,CONFIG_FEATURE_MDEV_CONF)
	$(call KCONFIG_ENABLE_OPT,CONFIG_FEATURE_MDEV_EXEC)
	$(call KCONFIG_ENABLE_OPT,CONFIG_FEATURE_MDEV_LOAD_FIRMWARE)
endef
endif

# sha passwords need USE_BB_CRYPT_SHA
ifeq ($(LINGMO_TARGET_GENERIC_PASSWD_SHA256)$(LINGMO_TARGET_GENERIC_PASSWD_SHA512),y)
define BUSYBOX_SET_CRYPT_SHA
	$(call KCONFIG_ENABLE_OPT,CONFIG_USE_BB_CRYPT_SHA)
endef
endif

ifeq ($(LINGMO_USE_MMU),y)
define BUSYBOX_SET_MMU
	$(call KCONFIG_DISABLE_OPT,CONFIG_NOMMU)
endef
else
define BUSYBOX_SET_MMU
	$(call KCONFIG_ENABLE_OPT,CONFIG_NOMMU)
	$(call KCONFIG_DISABLE_OPT,CONFIG_SWAPON)
	$(call KCONFIG_DISABLE_OPT,CONFIG_SWAPOFF)
	$(call KCONFIG_DISABLE_OPT,CONFIG_ASH)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_BASH_COMPAT)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_BRACE_EXPANSION)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_HELP)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_INTERACTIVE)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_SAVEHISTORY)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_JOB)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_TICK)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_IF)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_LOOPS)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_CASE)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_FUNCTIONS)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_LOCAL)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_RANDOM_SUPPORT)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_EXPORT_N)
	$(call KCONFIG_ENABLE_OPT,CONFIG_HUSH_MODE_X)
endef
endif

# If we're using static libs do the same for busybox
ifeq ($(LINGMO_STATIC_LIBS),y)
define BUSYBOX_PREFER_STATIC
	$(call KCONFIG_ENABLE_OPT,CONFIG_STATIC)
endef
endif

define BUSYBOX_INSTALL_UDHCPC_SCRIPT
	if grep -q CONFIG_UDHCPC=y $(@D)/.config; then \
		$(INSTALL) -m 0755 -D package/busybox/udhcpc.script \
			$(TARGET_DIR)/usr/share/udhcpc/default.script; \
		$(INSTALL) -m 0755 -d \
			$(TARGET_DIR)/usr/share/udhcpc/default.script.d; \
	fi
endef

define BUSYBOX_INSTALL_ZCIP_SCRIPT
	if grep -q CONFIG_ZCIP=y $(@D)/.config; then \
		$(INSTALL) -m 0755 -D $(@D)/examples/zcip.script \
			$(TARGET_DIR)/usr/share/zcip/default.script; \
	fi
endef

ifeq ($(LINGMO_INIT_BUSYBOX),y)

define BUSYBOX_SET_INIT
	$(call KCONFIG_ENABLE_OPT,CONFIG_INIT)
endef

ifeq ($(LINGMO_TARGET_GENERIC_GETTY),y)
define BUSYBOX_SET_GETTY
	$(SED) '/# GENERIC_SERIAL$$/s~^.*#~$(SYSTEM_GETTY_PORT)::respawn:/sbin/getty -L $(SYSTEM_GETTY_OPTIONS) $(SYSTEM_GETTY_PORT) $(SYSTEM_GETTY_BAUDRATE) $(SYSTEM_GETTY_TERM) #~' \
		$(TARGET_DIR)/etc/inittab
endef
else
define BUSYBOX_SET_GETTY
	$(SED) '/# GENERIC_SERIAL$$/s~^.*#~#ttyS0::respawn:/sbin/getty -L ttyS0 115200 vt100 #~' $(TARGET_DIR)/etc/inittab
endef
endif # LINGMO_TARGET_GENERIC_GETTY
BUSYBOX_TARGET_FINALIZE_HOOKS += BUSYBOX_SET_GETTY

BUSYBOX_TARGET_FINALIZE_HOOKS += SYSTEM_REMOUNT_ROOT_INITTAB

endif # LINGMO_INIT_BUSYBOX

ifeq ($(LINGMO_PACKAGE_BUSYBOX_SELINUX),y)
BUSYBOX_DEPENDENCIES += host-pkgconf libselinux libsepol
define BUSYBOX_SET_SELINUX
	$(call KCONFIG_ENABLE_OPT,CONFIG_SELINUX)
	$(call KCONFIG_ENABLE_OPT,CONFIG_SELINUXENABLED)
endef
endif

# enable relevant options to allow the Busybox less applet to be used
# as a systemd pager
ifeq ($(LINGMO_PACKAGE_SYSTEMD):$(LINGMO_PACKAGE_LESS),y:)
define BUSYBOX_SET_LESS_FLAGS
	$(call KCONFIG_ENABLE_OPT,CONFIG_FEATURE_LESS_DASHCMD)
	$(call KCONFIG_ENABLE_OPT,CONFIG_FEATURE_LESS_RAW)
	$(call KCONFIG_ENABLE_OPT,CONFIG_FEATURE_LESS_TRUNCATE)
	$(call KCONFIG_ENABLE_OPT,CONFIG_FEATURE_LESS_FLAGS)
	$(call KCONFIG_ENABLE_OPT,CONFIG_FEATURE_LESS_ENV)
endef
endif

ifeq ($(LINGMO_PACKAGE_BUSYBOX_INDIVIDUAL_BINARIES),y)
define BUSYBOX_SET_INDIVIDUAL_BINARIES
	$(call KCONFIG_ENABLE_OPT,CONFIG_BUILD_LIBBUSYBOX)
	$(call KCONFIG_ENABLE_OPT,CONFIG_FEATURE_INDIVIDUAL)
endef

define BUSYBOX_INSTALL_INDIVIDUAL_BINARIES
	rm -f $(TARGET_DIR)/bin/busybox
endef
endif

# Disable SHA1 and SHA256 HWACCEL to avoid segfault in init
# with some x86 toolchains (mostly musl?).
ifeq ($(LINGMO_i386),y)
define BUSYBOX_MUSL_DISABLE_SHA_HWACCEL
	$(call KCONFIG_DISABLE_OPT,CONFIG_SHA1_HWACCEL)
	$(call KCONFIG_DISABLE_OPT,CONFIG_SHA256_HWACCEL)
endef
endif

# Only install our logging scripts if no other package does it.
ifeq ($(LINGMO_PACKAGE_SYSKLOGD)$(LINGMO_PACKAGE_RSYSLOG)$(LINGMO_PACKAGE_SYSLOG_NG),)
define BUSYBOX_INSTALL_LOGGING_SCRIPT
	if grep -q CONFIG_SYSLOGD=y $(@D)/.config; \
	then \
		$(INSTALL) -m 0755 -D package/busybox/S01syslogd \
			$(TARGET_DIR)/etc/init.d/S01syslogd; \
	fi; \
	if grep -q CONFIG_KLOGD=y $(@D)/.config; \
	then \
		$(INSTALL) -m 0755 -D package/busybox/S02klogd \
			$(TARGET_DIR)/etc/init.d/S02klogd; \
	fi
endef
endif

# Only install our sysctl scripts if no other package does it.
ifeq ($(LINGMO_PACKAGE_PROCPS_NG),)
define BUSYBOX_INSTALL_SYSCTL_SCRIPT
	if grep -q CONFIG_BB_SYSCTL=y $(@D)/.config; \
	then \
		$(INSTALL) -m 0755 -D package/busybox/S02sysctl \
			$(TARGET_DIR)/etc/init.d/S02sysctl ; \
	fi
endef
endif

ifeq ($(LINGMO_INIT_BUSYBOX),y)
define BUSYBOX_INSTALL_INITTAB
	if test ! -e $(TARGET_DIR)/etc/inittab; then \
		$(INSTALL) -D -m 0644 package/busybox/inittab $(TARGET_DIR)/etc/inittab; \
	fi
endef
endif

ifeq ($(LINGMO_PACKAGE_BUSYBOX_WATCHDOG),y)
define BUSYBOX_SET_WATCHDOG
	$(call KCONFIG_ENABLE_OPT,CONFIG_WATCHDOG)
endef
define BUSYBOX_INSTALL_WATCHDOG_SCRIPT
	$(INSTALL) -D -m 0755 package/busybox/S15watchdog \
		$(TARGET_DIR)/etc/init.d/S15watchdog
	$(SED) s/PERIOD/$(call qstrip,$(LINGMO_PACKAGE_BUSYBOX_WATCHDOG_PERIOD))/ \
		$(TARGET_DIR)/etc/init.d/S15watchdog
endef
endif

# PAM support requires thread support in the toolchain
ifeq ($(LINGMO_PACKAGE_LINUX_PAM)$(LINGMO_TOOLCHAIN_HAS_THREADS),yy)
define BUSYBOX_LINUX_PAM
	$(call KCONFIG_ENABLE_OPT,CONFIG_PAM)
endef
BUSYBOX_DEPENDENCIES += linux-pam
else
define BUSYBOX_LINUX_PAM
	$(call KCONFIG_DISABLE_OPT,CONFIG_PAM)
endef
endif

# Telnet support
define BUSYBOX_INSTALL_TELNET_SCRIPT
	if grep -q CONFIG_FEATURE_TELNETD_STANDALONE=y $(@D)/.config; then \
		$(INSTALL) -m 0755 -D package/busybox/S50telnet \
			$(TARGET_DIR)/etc/init.d/S50telnet ; \
	fi
endef
define BUSYBOX_INSTALL_TELNET_SERVICE
	if grep -q CONFIG_FEATURE_TELNETD_STANDALONE=y $(@D)/.config; then \
		$(INSTALL) -D -m 0644 package/busybox/telnetd.service \
			$(TARGET_DIR)/usr/lib/systemd/system/telnetd.service ; \
	fi
endef

# Add /bin/{a,hu}sh to /etc/shells otherwise some login tools like dropbear
# can reject the user connection. See man shells.
define BUSYBOX_INSTALL_ADD_TO_SHELLS
	if grep -q CONFIG_ASH=y $(BUSYBOX_DIR)/.config; then \
		grep -qsE '^/bin/ash$$' $(TARGET_DIR)/etc/shells \
		|| echo "/bin/ash" >> $(TARGET_DIR)/etc/shells; \
	fi
	if grep -q CONFIG_HUSH=y $(BUSYBOX_DIR)/.config; then \
		grep -qsE '^/bin/hush$$' $(TARGET_DIR)/etc/shells \
		|| echo "/bin/hush" >> $(TARGET_DIR)/etc/shells; \
	fi
endef
BUSYBOX_TARGET_FINALIZE_HOOKS += BUSYBOX_INSTALL_ADD_TO_SHELLS

define BUSYBOX_KCONFIG_FIXUP_CMDS
	$(BUSYBOX_MUSL_DISABLE_SHA_HWACCEL)
	$(BUSYBOX_SET_MMU)
	$(BUSYBOX_PREFER_STATIC)
	$(BUSYBOX_SET_MDEV)
	$(BUSYBOX_SET_CRYPT_SHA)
	$(BUSYBOX_LINUX_PAM)
	$(BUSYBOX_SET_INIT)
	$(BUSYBOX_SET_WATCHDOG)
	$(BUSYBOX_SET_SELINUX)
	$(BUSYBOX_SET_LESS_FLAGS)
	$(BUSYBOX_SET_INDIVIDUAL_BINARIES)
endef

define BUSYBOX_BUILD_CMDS
	$(BUSYBOX_MAKE_ENV) $(MAKE) $(BUSYBOX_MAKE_OPTS) -C $(@D)
endef

define BUSYBOX_INSTALL_TARGET_CMDS
	# Use the 'noclobber' install rule, to prevent BusyBox from overwriting
	# any full-blown versions of apps installed by other packages.
	$(BUSYBOX_MAKE_ENV) $(MAKE) $(BUSYBOX_MAKE_OPTS) -C $(@D) install-noclobber
	$(BUSYBOX_INSTALL_INDIVIDUAL_BINARIES)
	$(BUSYBOX_INSTALL_INITTAB)
	$(BUSYBOX_INSTALL_UDHCPC_SCRIPT)
	$(BUSYBOX_INSTALL_ZCIP_SCRIPT)
	$(BUSYBOX_INSTALL_MDEV_CONF)
endef

# Install the sysvinit scripts, for the moment, but not those that already
# have a corresponding one in openrc.
define BUSYBOX_INSTALL_INIT_OPENRC
	$(BUSYBOX_INSTALL_MDEV_SCRIPT)
	$(BUSYBOX_INSTALL_LOGGING_SCRIPT)
	$(BUSYBOX_INSTALL_WATCHDOG_SCRIPT)
	$(BUSYBOX_INSTALL_TELNET_SCRIPT)
endef

define BUSYBOX_INSTALL_INIT_SYSTEMD
	$(BUSYBOX_INSTALL_TELNET_SERVICE)
endef

define BUSYBOX_INSTALL_INIT_SYSV
	$(BUSYBOX_INSTALL_MDEV_SCRIPT)
	$(BUSYBOX_INSTALL_LOGGING_SCRIPT)
	$(BUSYBOX_INSTALL_WATCHDOG_SCRIPT)
	$(BUSYBOX_INSTALL_SYSCTL_SCRIPT)
	$(BUSYBOX_INSTALL_TELNET_SCRIPT)
endef

# Checks to give errors that the user can understand
# Must be before we call to kconfig-package
ifeq ($(LINGMO_PACKAGE_BUSYBOX)$(BR_BUILDING),yy)
ifeq ($(call qstrip,$(LINGMO_PACKAGE_BUSYBOX_CONFIG)),)
$(error No BusyBox configuration file specified, check your LINGMO_PACKAGE_BUSYBOX_CONFIG setting)
endif
endif

$(eval $(kconfig-package))
