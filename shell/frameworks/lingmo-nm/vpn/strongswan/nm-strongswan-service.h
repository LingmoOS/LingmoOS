/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* NetworkManager -- Network link manager

    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005-2008 Red Hat Inc. //krazy:exclude=copyright
    SPDX-FileCopyrightText: 2007-2008 Novell Inc. //krazy:exclude=copyright
*/

#ifndef NM_STRONGSWAN_SERVICE_H
#define NM_STRONGSWAN_SERVICE_H

#define NM_DBUS_SERVICE_STRONGSWAN "org.freedesktop.NetworkManager.strongswan"
#define NM_DBUS_INTERFACE_STRONGSWAN "org.freedesktop.NetworkManager.strongswan"
#define NM_DBUS_PATH_STRONGSWAN "/org/freedesktop/NetworkManager/strongswan"

#define NM_STRONGSWAN_GATEWAY "address"
#define NM_STRONGSWAN_CERTIFICATE "certificate"
#define NM_STRONGSWAN_USER "user"
#define NM_STRONGSWAN_METHOD "method"
#define NM_STRONGSWAN_USERKEY "userkey"
#define NM_STRONGSWAN_USERCERT "usercert"
#define NM_STRONGSWAN_SECRET "password"
#define NM_STRONGSWAN_SECRET_TYPE "secret_type"
#define NM_STRONGSWAN_INNERIP "virtual"
#define NM_STRONGSWAN_ENCAP "encap"
#define NM_STRONGSWAN_IPCOMP "ipcomp"
#define NM_STRONGSWAN_PROPOSAL "proposal"
#define NM_STRONGSWAN_IKE "ike"
#define NM_STRONGSWAN_ESP "esp"
#define NM_STRONGSWAN_RIDENTITY "remote-identity"
#define NM_STRONGSWAN_POPTION "password-flags"

#define NM_STRONGSWAN_AUTH_KEY "key"
#define NM_STRONGSWAN_AUTH_AGENT "agent"
#define NM_STRONGSWAN_AUTH_SMARTCARD "smartcard"
#define NM_STRONGSWAN_AUTH_EAP "eap"

#define NM_STRONGSWAN_PW_TYPE_SAVE "save"
#define NM_STRONGSWAN_PW_TYPE_ASK "ask"
#define NM_STRONGSWAN_PW_TYPE_UNUSED "unused"

#endif /* NM_Strongswan_PLUGIN_H */
