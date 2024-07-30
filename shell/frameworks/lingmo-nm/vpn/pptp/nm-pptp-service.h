/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* nm-pptp-service - PPTP VPN integration with NetworkManager


    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: Dan Williams <dcbw@redhat.com>
    SPDX-FileCopyrightText: 2008 Red Hat Inc. //krazy:exclude=copyright
*/

#ifndef NM_PPTP_SERVICE_H
#define NM_PPTP_SERVICE_H

#define NM_DBUS_SERVICE_PPTP_PPP "org.freedesktop.NetworkManager.pptp-ppp"
#define NM_DBUS_PATH_PPTP_PPP "/org/freedesktop/NetworkManager/pptp/ppp"
#define NM_DBUS_INTERFACE_PPTP_PPP "org.freedesktop.NetworkManager.pptp.ppp"

/* For the NM <-> VPN plugin service */
#define NM_DBUS_SERVICE_PPTP "org.freedesktop.NetworkManager.pptp"
#define NM_DBUS_INTERFACE_PPTP "org.freedesktop.NetworkManager.pptp"
#define NM_DBUS_PATH_PPTP "/org/freedesktop/NetworkManager/pptp"

#define NM_PPTP_KEY_GATEWAY "gateway"
#define NM_PPTP_KEY_USER "user"
#define NM_PPTP_KEY_PASSWORD "password"
#define NM_PPTP_KEY_DOMAIN "domain"
#define NM_PPTP_KEY_REFUSE_EAP "refuse-eap"
#define NM_PPTP_KEY_REFUSE_PAP "refuse-pap"
#define NM_PPTP_KEY_REFUSE_CHAP "refuse-chap"
#define NM_PPTP_KEY_REFUSE_MSCHAP "refuse-mschap"
#define NM_PPTP_KEY_REFUSE_MSCHAPV2 "refuse-mschapv2"
#define NM_PPTP_KEY_REQUIRE_MPPE "require-mppe"
#define NM_PPTP_KEY_REQUIRE_MPPE_40 "require-mppe-40"
#define NM_PPTP_KEY_REQUIRE_MPPE_128 "require-mppe-128"
#define NM_PPTP_KEY_MPPE_STATEFUL "mppe-stateful"
#define NM_PPTP_KEY_NOBSDCOMP "nobsdcomp"
#define NM_PPTP_KEY_NODEFLATE "nodeflate"
#define NM_PPTP_KEY_NO_VJ_COMP "no-vj-comp"
#define NM_PPTP_KEY_LCP_ECHO_FAILURE "lcp-echo-failure"
#define NM_PPTP_KEY_LCP_ECHO_INTERVAL "lcp-echo-interval"

#endif /* NM_PPTP_PLUGIN_H */
