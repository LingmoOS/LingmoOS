/* nm-fortisslvpn-service - SSLVPN integration with NetworkManager

    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2008 Red Hat Inc.
    SPDX-FileCopyrightText: Dan Williams <dcbw@redhat.com>
    SPDX-FileCopyrightText: 2015 Lubomir Rintel <lkundrak@v3.sk>
*/

#ifndef __NM_FORTISSLVPN_SERVICE_H__
#define __NM_FORTISSLVPN_SERVICE_H__

/* For the NM <-> VPN plugin service */
#define NM_DBUS_SERVICE_FORTISSLVPN "org.freedesktop.NetworkManager.fortisslvpn"
#define NM_DBUS_INTERFACE_FORTISSLVPN "org.freedesktop.NetworkManager.fortisslvpn"
#define NM_DBUS_PATH_FORTISSLVPN "/org/freedesktop/NetworkManager/fortisslvpn"

#define NM_FORTISSLVPN_KEY_GATEWAY "gateway"
#define NM_FORTISSLVPN_KEY_USER "user"
#define NM_FORTISSLVPN_KEY_PASSWORD "password"
#define NM_FORTISSLVPN_KEY_OTP "otp"
#define NM_FORTISSLVPN_KEY_2FA "2fa"
#define NM_FORTISSLVPN_KEY_CA "ca"
#define NM_FORTISSLVPN_KEY_CERT "cert"
#define NM_FORTISSLVPN_KEY_KEY "key"
#define NM_FORTISSLVPN_KEY_TRUSTED_CERT "trusted-cert"
#define NM_FORTISSLVPN_KEY_REALM "realm"

#endif /* __NM_FORTISSLVPN_SERVICE_H__ */
