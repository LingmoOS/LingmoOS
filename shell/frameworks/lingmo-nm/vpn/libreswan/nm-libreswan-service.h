/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* NetworkManager-libreswan -- libreswan plugin for Network manager

    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2010 Red Hat Inc.
*/

#ifndef NM_LIBRESWAN_SERVICE_H
#define NM_LIBRESWAN_SERVICE_H

#define NM_DBUS_SERVICE_LIBRESWAN "org.freedesktop.NetworkManager.libreswan"
#define NM_DBUS_INTERFACE_LIBRESWAN "org.freedesktop.NetworkManager.libreswan"
#define NM_DBUS_PATH_LIBRESWAN "/org/freedesktop/NetworkManager/libreswan"

#define NM_LIBRESWAN_RIGHT "right"
#define NM_LIBRESWAN_LEFTID "leftid"
#define NM_LIBRESWAN_PSK_VALUE "pskvalue"
#define NM_LIBRESWAN_PSK_INPUT_MODES "pskinputmodes"
#define NM_LIBRESWAN_LEFTXAUTHUSER "leftxauthusername"
#define NM_LIBRESWAN_XAUTH_PASSWORD "xauthpassword"
#define NM_LIBRESWAN_XAUTH_PASSWORD_INPUT_MODES "xauthpasswordinputmodes"
#define NM_LIBRESWAN_DOMAIN "Domain"
#define NM_LIBRESWAN_DHGROUP "dhgroup"
#define NM_LIBRESWAN_PFSGROUP "pfsgroup"
#define NM_LIBRESWAN_DPDTIMEOUT "dpdtimeout"
#define NM_LIBRESWAN_IKE "ike"
#define NM_LIBRESWAN_ESP "esp"

#define NM_LIBRESWAN_PW_TYPE_SAVE "save"
#define NM_LIBRESWAN_PW_TYPE_ASK "ask"
#define NM_LIBRESWAN_PW_TYPE_UNUSED "unused"

#endif /* NM_LIBRESWAN_SERVICE_H */
