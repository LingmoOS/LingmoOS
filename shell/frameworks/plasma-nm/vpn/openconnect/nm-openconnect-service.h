/* -*- Mode: C; tab-width: 5; indent-tabs-mode: t; c-basic-offset: 5 -*- */
/* NetworkManager -- Network link manager

    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2008-2009 Intel Corporation. //krazy:exclude=copyright

    Based on nm-openconnect-vpnc.h:
    SPDX-FileCopyrightText: 2005-2008 Red Hat Inc. //krazy:exclude=copyright
    SPDX-FileCopyrightText: 2007-2008 Novell Inc. //krazy:exclude=copyright
*/

#ifndef NM_OPENCONNECT_SERVICE_H
#define NM_OPENCONNECT_SERVICE_H

#define NM_DBUS_SERVICE_OPENCONNECT "org.freedesktop.NetworkManager.openconnect"
#define NM_DBUS_INTERFACE_OPENCONNECT "org.freedesktop.NetworkManager.openconnect"
#define NM_DBUS_PATH_OPENCONNECT "/org/freedesktop/NetworkManager/openconnect"

#define NM_OPENCONNECT_KEY_GATEWAY "gateway"
#define NM_OPENCONNECT_KEY_COOKIE "cookie"
#define NM_OPENCONNECT_KEY_GWCERT "gwcert"
#define NM_OPENCONNECT_KEY_AUTHTYPE "authtype"
#define NM_OPENCONNECT_KEY_USERCERT "usercert"
#define NM_OPENCONNECT_KEY_CACERT "cacert"
#define NM_OPENCONNECT_KEY_PRIVKEY "userkey"
#define NM_OPENCONNECT_KEY_MTU "mtu"
#define NM_OPENCONNECT_KEY_PEM_PASSPHRASE_FSID "pem_passphrase_fsid"
#define NM_OPENCONNECT_KEY_PREVENT_INVALID_CERT "prevent_invalid_cert"
#define NM_OPENCONNECT_KEY_PROTOCOL "protocol"
#define NM_OPENCONNECT_KEY_PROXY "proxy"
#define NM_OPENCONNECT_KEY_USERAGENT "useragent"
#define NM_OPENCONNECT_KEY_VERSION_STRING "version_string"
#define NM_OPENCONNECT_KEY_CSD_ENABLE "enable_csd_trojan"
#define NM_OPENCONNECT_KEY_CSD_WRAPPER "csd_wrapper"
#define NM_OPENCONNECT_KEY_TOKEN_MODE "stoken_source"
#define NM_OPENCONNECT_KEY_TOKEN_SECRET "stoken_string"
#define NM_OPENCONNECT_KEY_REPORTED_OS "reported_os"
#define NM_OPENCONNECT_KEY_MCACERT "mcacert"
#define NM_OPENCONNECT_KEY_MCAKEY "mcakey"
#define NM_OPENCONNECT_KEY_MCA_PASS "mca_key_pass"

#define NM_OPENCONNECT_USER "nm-openconnect"

#endif /* NM_OPENCONNECT_PLUGIN_H */
