################################################################################
#
# collectd
#
################################################################################

COLLECTD_VERSION = 5.12.0
COLLECTD_SITE = \
	https://github.com/collectd/collectd/releases/download/collectd-$(COLLECTD_VERSION)
COLLECTD_SOURCE = collectd-$(COLLECTD_VERSION).tar.bz2
COLLECTD_CONF_ENV = ac_cv_lib_yajl_yajl_alloc=yes
COLLECTD_INSTALL_STAGING = YES
COLLECTD_LICENSE = MIT (daemon, plugins), GPL-2.0 (plugins), LGPL-2.1 (plugins)
COLLECTD_LICENSE_FILES = COPYING
COLLECTD_CPE_ID_VENDOR = collectd
COLLECTD_SELINUX_MODULES = apache collectd
# We're patching configure.ac
COLLECTD_AUTORECONF = YES

# These require unmet dependencies, are fringe, pointless or deprecated
COLLECTD_PLUGINS_DISABLE = \
	apple_sensors aquaero ascent barometer dpdkstat email \
	gmond hddtemp intel_rdt java lpar \
	madwifi mbmon mic multimeter netapp notify_desktop numa \
	oracle perl pf pinba powerdns python routeros \
	sigrok tape target_v5upgrade teamspeak2 ted \
	tokyotyrant turbostat uuid varnish vserver write_kafka \
	write_mongodb xencpu xmms zfs_arc zone

COLLECTD_CONF_ENV += LIBS="-lm"

COLLECTD_CFLAGS = $(TARGET_CFLAGS)

ifeq ($(LINGMO_TOOLCHAIN_HAS_GCC_BUG_68485),y)
COLLECTD_CFLAGS += -O0
endif

COLLECTD_CONF_ENV += CFLAGS="$(COLLECTD_CFLAGS)"

#
# NOTE: There's also a third availible setting "intswap", which might
# be needed on some old ARM hardware (see [2]), but is not being
# accounted for as per discussion [1]
#
# [1] http://lists.busybox.net/pipermail/buildroot/2017-November/206100.html
# [2] http://lists.busybox.net/pipermail/buildroot/2017-November/206251.html
#
ifeq ($(LINGMO_ENDIAN),"BIG")
COLLECTD_FP_LAYOUT=endianflip
else
COLLECTD_FP_LAYOUT=nothing
endif

COLLECTD_CONF_OPTS += \
	--with-nan-emulation \
	--with-fp-layout=$(COLLECTD_FP_LAYOUT) \
	--with-perl-bindings=no \
	--disable-werror \
	$(foreach p, $(COLLECTD_PLUGINS_DISABLE), --disable-$(p)) \
	$(if $(LINGMO_PACKAGE_COLLECTD_AGGREGATION),--enable-aggregation,--disable-aggregation) \
	$(if $(LINGMO_PACKAGE_COLLECTD_AMQP),--enable-amqp,--disable-amqp) \
	$(if $(LINGMO_PACKAGE_COLLECTD_APACHE),--enable-apache,--disable-apache) \
	$(if $(LINGMO_PACKAGE_COLLECTD_APCUPS),--enable-apcups,--disable-apcups) \
	$(if $(LINGMO_PACKAGE_COLLECTD_BATTERY),--enable-battery,--disable-battery) \
	$(if $(LINGMO_PACKAGE_COLLECTD_BIND),--enable-bind,--disable-bind) \
	$(if $(LINGMO_PACKAGE_COLLECTD_BUDDYINFO),--enable-buddyinfo,--disable-buddyinfo) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CAPABILITIES),--enable-capabilities,--disable-capabilities) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CEPH),--enable-ceph,--disable-ceph) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CHRONY),--enable-chrony,--disable-chrony) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CGROUPS),--enable-cgroups,--disable-cgroups) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CONNTRACK),--enable-conntrack,--disable-conntrack) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CONTEXTSWITCH),--enable-contextswitch,--disable-contextswitch) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CPU),--enable-cpu,--disable-cpu) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CPUFREQ),--enable-cpufreq,--disable-cpufreq) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CPUSLEEP),--enable-cpusleep,--disable-cpusleep) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CSV),--enable-csv,--disable-csv) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CURL),--enable-curl,--disable-curl) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CURL_JSON),--enable-curl_json,--disable-curl_json) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CURL_XML),--enable-curl_xml,--disable-curl_xml) \
	$(if $(LINGMO_PACKAGE_COLLECTD_DBI),--enable-dbi,--disable-dbi) \
	$(if $(LINGMO_PACKAGE_COLLECTD_DF),--enable-df,--disable-df) \
	$(if $(LINGMO_PACKAGE_COLLECTD_DISK),--enable-disk,--disable-disk) \
	$(if $(LINGMO_PACKAGE_COLLECTD_DNS),--enable-dns,--disable-dns) \
	$(if $(LINGMO_PACKAGE_COLLECTD_DPDK_TELEMETRY),--enable-dpdk_telemetry,--disable-dpdk_telemetry) \
	$(if $(LINGMO_PACKAGE_COLLECTD_DRBD),--enable-drbd,--disable-drbd) \
	$(if $(LINGMO_PACKAGE_COLLECTD_EMPTY_COUNTER),--enable-match_empty_counter,--disable-match_empty_counter) \
	$(if $(LINGMO_PACKAGE_COLLECTD_ENTROPY),--enable-entropy,--disable-entropy) \
	$(if $(LINGMO_PACKAGE_COLLECTD_ETHSTAT),--enable-ethstat,--disable-ethstat) \
	$(if $(LINGMO_PACKAGE_COLLECTD_EXEC),--enable-exec,--disable-exec) \
	$(if $(LINGMO_PACKAGE_COLLECTD_FHCOUNT),--enable-fhcount,--disable-fhcount) \
	$(if $(LINGMO_PACKAGE_COLLECTD_FILECOUNT),--enable-filecount,--disable-filecount) \
	$(if $(LINGMO_PACKAGE_COLLECTD_FSCACHE),--enable-fscache,--disable-fscache) \
	$(if $(LINGMO_PACKAGE_COLLECTD_GPS),--enable-gps,--disable-gps) \
	$(if $(LINGMO_PACKAGE_COLLECTD_GRAPHITE),--enable-write_graphite,--disable-write_graphite) \
	$(if $(LINGMO_PACKAGE_COLLECTD_GRPC),--enable-grpc,--disable-grpc) \
	$(if $(LINGMO_PACKAGE_COLLECTD_HASHED),--enable-match_hashed,--disable-match_hashed) \
	$(if $(LINGMO_PACKAGE_COLLECTD_HUGEPAGES),--enable-hugepages,--disable-hugepages) \
	$(if $(LINGMO_PACKAGE_COLLECTD_INFINIBAND),--enable-infiniband,--disable-infiniband) \
	$(if $(LINGMO_PACKAGE_COLLECTD_INTERFACE),--enable-interface,--disable-interface) \
	$(if $(LINGMO_PACKAGE_COLLECTD_IPC),--enable-ipc,--disable-ipc) \
	$(if $(LINGMO_PACKAGE_COLLECTD_IPMI),--enable-ipmi,--disable-ipmi) \
	$(if $(LINGMO_PACKAGE_COLLECTD_IPTABLES),--enable-iptables,--disable-iptables) \
	$(if $(LINGMO_PACKAGE_COLLECTD_IPVS),--enable-ipvs,--disable-ipvs) \
	$(if $(LINGMO_PACKAGE_COLLECTD_IRQ),--enable-irq,--disable-irq) \
	$(if $(LINGMO_PACKAGE_COLLECTD_LOAD),--enable-load,--disable-load) \
	$(if $(LINGMO_PACKAGE_COLLECTD_LOGFILE),--enable-logfile,--disable-logfile) \
	$(if $(LINGMO_PACKAGE_COLLECTD_LOGPARSER),--enable-logparser,--disable-logparser) \
	$(if $(LINGMO_PACKAGE_COLLECTD_LOGSTASH),--enable-log_logstash,--disable-log_logstash) \
	$(if $(LINGMO_PACKAGE_COLLECTD_LUA),--enable-lua,--disable-lua) \
	$(if $(LINGMO_PACKAGE_COLLECTD_MD),--enable-md,--disable-md) \
	$(if $(LINGMO_PACKAGE_COLLECTD_MDEVENTS),--enable-mdevents,--disable-mdevents) \
	$(if $(LINGMO_PACKAGE_COLLECTD_MEMCACHEC),--enable-memcachec,--disable-memcachec) \
	$(if $(LINGMO_PACKAGE_COLLECTD_MEMCACHED),--enable-memcached,--disable-memcached) \
	$(if $(LINGMO_PACKAGE_COLLECTD_MEMORY),--enable-memory,--disable-memory) \
	$(if $(LINGMO_PACKAGE_COLLECTD_MODBUS),--enable-modbus,--disable-modbus) \
	$(if $(LINGMO_PACKAGE_COLLECTD_MQTT),--enable-mqtt,--disable-mqtt) \
	$(if $(LINGMO_PACKAGE_COLLECTD_MYSQL),--enable-mysql,--disable-mysql) \
	$(if $(LINGMO_PACKAGE_COLLECTD_NETLINK),--enable-netlink,--disable-netlink) \
	$(if $(LINGMO_PACKAGE_COLLECTD_NETWORK),--enable-network,--disable-network) \
	$(if $(LINGMO_PACKAGE_COLLECTD_NFS),--enable-nfs,--disable-nfs) \
	$(if $(LINGMO_PACKAGE_COLLECTD_NGINX),--enable-nginx,--disable-nginx) \
	$(if $(LINGMO_PACKAGE_COLLECTD_NOTIFICATION),--enable-target_notification,--disable-target_notification) \
	$(if $(LINGMO_PACKAGE_COLLECTD_NOTIFY_EMAIL),--enable-notify_email,--disable-notify_email) \
	$(if $(LINGMO_PACKAGE_COLLECTD_NOTIFY_NAGIOS),--enable-notify_nagios,--disable-notify_nagios) \
	$(if $(LINGMO_PACKAGE_COLLECTD_NTPD),--enable-ntpd,--disable-ntpd) \
	$(if $(LINGMO_PACKAGE_COLLECTD_NUT),--enable-nut,--disable-nut) \
	$(if $(LINGMO_PACKAGE_COLLECTD_OLSRD),--enable-olsrd,--disable-olsrd) \
	$(if $(LINGMO_PACKAGE_COLLECTD_ONEWIRE),--enable-onewire,--disable-onewire) \
	$(if $(LINGMO_PACKAGE_COLLECTD_OPENLDAP),--enable-openldap,--disable-openldap) \
	$(if $(LINGMO_PACKAGE_COLLECTD_OPENVPN),--enable-openvpn,--disable-openvpn) \
	$(if $(LINGMO_PACKAGE_COLLECTD_PING),--enable-ping,--disable-ping) \
	$(if $(LINGMO_PACKAGE_COLLECTD_POSTGRESQL),--enable-postgresql,--disable-postgresql) \
	$(if $(LINGMO_PACKAGE_COLLECTD_PROCESSES),--enable-processes,--disable-processes) \
	$(if $(LINGMO_PACKAGE_COLLECTD_PROTOCOLS),--enable-protocols,--disable-protocols) \
	$(if $(LINGMO_PACKAGE_COLLECTD_REDIS),--enable-redis,--disable-redis) \
	$(if $(LINGMO_PACKAGE_COLLECTD_REGEX),--enable-match_regex,--disable-match-regex) \
	$(if $(LINGMO_PACKAGE_COLLECTD_REPLACE),--enable-target_replace,--disable-target_replace) \
	$(if $(LINGMO_PACKAGE_COLLECTD_RIEMANN),--enable-write_riemann,--disable-write_riemann) \
	$(if $(LINGMO_PACKAGE_COLLECTD_RRDCACHED),--enable-rrdcached,--disable-rrdcached) \
	$(if $(LINGMO_PACKAGE_COLLECTD_RRDTOOL),--enable-rrdtool,--disable-rrdtool) \
	$(if $(LINGMO_PACKAGE_COLLECTD_SCALE),--enable-target_scale,--disable-target_scale) \
	$(if $(LINGMO_PACKAGE_COLLECTD_SENSORS),--enable-sensors,--disable-sensors) \
	$(if $(LINGMO_PACKAGE_COLLECTD_SERIAL),--enable-serial,--disable-serial) \
	$(if $(LINGMO_PACKAGE_COLLECTD_STATSD),--enable-statsd,--disable-statsd) \
	$(if $(LINGMO_PACKAGE_COLLECTD_SET),--enable-target_set,--disable-target_set) \
	$(if $(LINGMO_PACKAGE_COLLECTD_SMART),--enable-smart,--disable-smart) \
	$(if $(LINGMO_PACKAGE_COLLECTD_SNMP),--enable-snmp,--disable-snmp) \
	$(if $(LINGMO_PACKAGE_COLLECTD_SWAP),--enable-swap,--disable-swap) \
	$(if $(LINGMO_PACKAGE_COLLECTD_SYNPROXY),--enable-synproxy,--disable-synproxy) \
	$(if $(LINGMO_PACKAGE_COLLECTD_SYSLOG),--enable-syslog,--disable-syslog) \
	$(if $(LINGMO_PACKAGE_COLLECTD_TABLE),--enable-table,--disable-table) \
	$(if $(LINGMO_PACKAGE_COLLECTD_TAIL),--enable-tail,--disable-tail) \
	$(if $(LINGMO_PACKAGE_COLLECTD_TAIL_CSV),--enable-tail_csv,--disable-tail_csv) \
	$(if $(LINGMO_PACKAGE_COLLECTD_TCPCONNS),--enable-tcpconns,--disable-tcpconns) \
	$(if $(LINGMO_PACKAGE_COLLECTD_THERMAL),--enable-thermal,--disable-thermal) \
	$(if $(LINGMO_PACKAGE_COLLECTD_THRESHOLD),--enable-threshold,--disable-threshold) \
	$(if $(LINGMO_PACKAGE_COLLECTD_TIMEDIFF),--enable-match_timediff,--disable-match_timediff) \
	$(if $(LINGMO_PACKAGE_COLLECTD_UBI),--enable-ubi,--disable-ubi) \
	$(if $(LINGMO_PACKAGE_COLLECTD_UNIXSOCK),--enable-unixsock,--disable-unixsock) \
	$(if $(LINGMO_PACKAGE_COLLECTD_UPTIME),--enable-uptime,--disable-uptime) \
	$(if $(LINGMO_PACKAGE_COLLECTD_USERS),--enable-users,--disable-users) \
	$(if $(LINGMO_PACKAGE_COLLECTD_VALUE),--enable-match_value,--disable-match_value) \
	$(if $(LINGMO_PACKAGE_COLLECTD_VIRT),--enable-virt,--disable-virt) \
	$(if $(LINGMO_PACKAGE_COLLECTD_VMEM),--enable-vmem,--disable-vmem) \
	$(if $(LINGMO_PACKAGE_COLLECTD_WIRELESS),--enable-wireless,--disable-wireless) \
	$(if $(LINGMO_PACKAGE_COLLECTD_WRITEHTTP),--enable-write_http,--disable-write_http) \
	$(if $(LINGMO_PACKAGE_COLLECTD_WRITEINFLUXDBUDP),--enable-write_influxdb_udp,--disable-write_influxdb_udp) \
	$(if $(LINGMO_PACKAGE_COLLECTD_WRITELOG),--enable-write_log,--disable-write_log) \
	$(if $(LINGMO_PACKAGE_COLLECTD_WRITEPROMETHEUS),--enable-write_prometheus,--disable-write_prometheus) \
	$(if $(LINGMO_PACKAGE_COLLECTD_WRITEREDIS),--enable-write_redis,--disable-write_redis) \
	$(if $(LINGMO_PACKAGE_COLLECTD_WRITESENSU),--enable-write_sensu,--disable-write_sensu) \
	$(if $(LINGMO_PACKAGE_COLLECTD_WRITETSDB),--enable-write_tsdb,--disable-write_tsdb) \
	$(if $(LINGMO_PACKAGE_COLLECTD_ZOOKEEPER),--enable-zookeeper,--disable-zookeeper)

COLLECTD_DEPENDENCIES = \
	host-pkgconf \
	$(if $(LINGMO_PACKAGE_COLLECTD_AMQP),rabbitmq-c) \
	$(if $(LINGMO_PACKAGE_COLLECTD_APACHE),libcurl) \
	$(if $(LINGMO_PACKAGE_COLLECTD_BIND),libcurl libxml2) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CAPABILITIES),jansson libmicrohttpd) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CEPH),yajl) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CURL),libcurl) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CURL_JSON),libcurl yajl) \
	$(if $(LINGMO_PACKAGE_COLLECTD_CURL_XML),libcurl libxml2) \
	$(if $(LINGMO_PACKAGE_COLLECTD_DBI),libdbi) \
	$(if $(LINGMO_PACKAGE_COLLECTD_DNS),libpcap) \
	$(if $(LINGMO_PACKAGE_COLLECTD_DPDK_TELEMETRY),jansson) \
	$(if $(LINGMO_PACKAGE_COLLECTD_GPS),gpsd) \
	$(if $(LINGMO_PACKAGE_COLLECTD_GRPC),grpc) \
	$(if $(LINGMO_PACKAGE_COLLECTD_IPMI),openipmi) \
	$(if $(LINGMO_PACKAGE_COLLECTD_IPTABLES),iptables) \
	$(if $(LINGMO_PACKAGE_COLLECTD_LOGSTASH),yajl) \
	$(if $(LINGMO_PACKAGE_COLLECTD_LUA),luainterpreter) \
	$(if $(LINGMO_PACKAGE_COLLECTD_MEMCACHEC),libmemcached) \
	$(if $(LINGMO_PACKAGE_COLLECTD_MODBUS),libmodbus) \
	$(if $(LINGMO_PACKAGE_COLLECTD_MQTT),mosquitto) \
	$(if $(LINGMO_PACKAGE_COLLECTD_MYSQL),mariadb) \
	$(if $(LINGMO_PACKAGE_COLLECTD_NETLINK),libmnl) \
	$(if $(LINGMO_PACKAGE_COLLECTD_NGINX),libcurl) \
	$(if $(LINGMO_PACKAGE_COLLECTD_NOTIFY_EMAIL),libesmtp) \
	$(if $(LINGMO_PACKAGE_COLLECTD_NUT),nut) \
	$(if $(LINGMO_PACKAGE_COLLECTD_ONEWIRE),owfs) \
	$(if $(LINGMO_PACKAGE_COLLECTD_OPENLDAP),openldap) \
	$(if $(LINGMO_PACKAGE_COLLECTD_PING),liboping) \
	$(if $(LINGMO_PACKAGE_COLLECTD_POSTGRESQL),postgresql) \
	$(if $(LINGMO_PACKAGE_COLLECTD_REDIS),hiredis) \
	$(if $(LINGMO_PACKAGE_COLLECTD_RIEMANN),libtool riemann-c-client) \
	$(if $(LINGMO_PACKAGE_COLLECTD_RRDTOOL),rrdtool) \
	$(if $(LINGMO_PACKAGE_COLLECTD_SENSORS),lm-sensors) \
	$(if $(LINGMO_PACKAGE_COLLECTD_SMART),libatasmart) \
	$(if $(LINGMO_PACKAGE_COLLECTD_SNMP),netsnmp) \
	$(if $(LINGMO_PACKAGE_COLLECTD_VIRT),libvirt libxml2) \
	$(if $(LINGMO_PACKAGE_COLLECTD_WRITEHTTP),libcurl) \
	$(if $(LINGMO_PACKAGE_COLLECTD_WRITEPROMETHEUS),libmicrohttpd protobuf-c) \
	$(if $(LINGMO_PACKAGE_COLLECTD_WRITEREDIS),hiredis)

# include/library fixups
ifeq ($(LINGMO_PACKAGE_JANSSON),y)
COLLECTD_CONF_OPTS += --with-libjansson=$(STAGING_DIR)/usr
endif
ifeq ($(LINGMO_PACKAGE_LIBCURL),y)
COLLECTD_CONF_OPTS += --with-libcurl=$(STAGING_DIR)/usr
endif
ifeq ($(LINGMO_PACKAGE_LUAJIT),y)
COLLECTD_CONF_ENV += LIBLUA_PKG_CONFIG_NAME=luajit
endif
ifeq ($(LINGMO_PACKAGE_MARIADB),y)
COLLECTD_CONF_OPTS += --with-libmysql=$(STAGING_DIR)/usr
endif
ifeq ($(LINGMO_PACKAGE_NETSNMP),y)
COLLECTD_CONF_OPTS += --with-libnetsnmp=$(STAGING_DIR)/usr
endif
ifeq ($(LINGMO_PACKAGE_POSTGRESQL),y)
COLLECTD_CONF_OPTS += --with-libpq=$(STAGING_DIR)/usr/bin/pg_config
COLLECTD_CONF_ENV += LIBS="-lpthread -lm"
endif
ifeq ($(LINGMO_PACKAGE_YAJL),y)
COLLECTD_CONF_OPTS += --with-libyajl=$(STAGING_DIR)/usr
endif

# network can use libgcrypt
ifeq ($(LINGMO_PACKAGE_LIBGCRYPT),y)
COLLECTD_DEPENDENCIES += libgcrypt
COLLECTD_CONF_OPTS += --with-libgcrypt=$(STAGING_DIR)/usr/bin/libgcrypt-config
else
COLLECTD_CONF_OPTS += --with-libgcrypt=no
endif

define COLLECTD_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) DESTDIR=$(TARGET_DIR) -C $(@D) install
	rm -f $(TARGET_DIR)/usr/bin/collectd-nagios
endef

ifeq ($(LINGMO_PACKAGE_COLLECTD_POSTGRESQL),)
define COLLECTD_REMOVE_UNNEEDED_POSTGRESQL_DEFAULT_CONF
	rm -f $(TARGET_DIR)/usr/share/collectd/postgresql_default.conf
endef
COLLECTD_POST_INSTALL_TARGET_HOOKS += COLLECTD_REMOVE_UNNEEDED_POSTGRESQL_DEFAULT_CONF
endif

define COLLECTD_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 644 package/collectd/collectd.service \
		$(TARGET_DIR)/usr/lib/systemd/system/collectd.service
endef

define COLLECTD_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 package/collectd/S90collectd \
		$(TARGET_DIR)/etc/init.d/S90collectd
endef

$(eval $(autotools-package))
