/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_ENUMS_H
#define PLASMA_NM_ENUMS_H

#include <QObject>

#include <qqmlregistration.h>

class Enums : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    Q_ENUMS(ConnectionStatus)
    Q_ENUMS(ConnectionType)
    Q_ENUMS(SecurityType)

public:
    explicit Enums(QObject *parent = nullptr);
    ~Enums() override;

    enum ConnectionStatus {
        UnknownState = 0,
        Activating,
        Activated,
        Deactivating,
        Deactivated,
    };

    enum ConnectionType {
        UnknownConnectionType = 0,
        Adsl,
        Bluetooth,
        Bond,
        Bridge,
        Cdma,
        Gsm,
        Infiniband,
        OLPCMesh,
        Pppoe,
        Vlan,
        Vpn,
        Wimax,
        Wired,
        Wireless,
    };

    enum SecurityType {
        UnknownSecurity = -1,
        NoneSecurity = 0,
        StaticWep,
        DynamicWep,
        Leap,
        WpaPsk,
        WpaEap,
        Wpa2Psk,
        Wpa2Eap,
        SAE,
        Wpa3SuiteB192,
        OWE,
    };
};

#endif // PLASMA_NM_ENUMS_H
