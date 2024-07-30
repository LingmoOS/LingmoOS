/*
    SPDX-FileCopyrightText: 2009, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIRELESSNETWORK_P_H
#define NETWORKMANAGERQT_WIRELESSNETWORK_P_H

#include "wirelessdevice.h"

#include <QPointer>

namespace NetworkManager
{
class WirelessNetworkPrivate
{
public:
    Q_DECLARE_PUBLIC(WirelessNetwork)
    WirelessNetwork *q_ptr;

    WirelessNetworkPrivate(WirelessNetwork *q, WirelessDevice *);
    ~WirelessNetworkPrivate();

    void addAccessPointInternal(const AccessPoint::Ptr &accessPoint);

    QString ssid;
    int strength;
    QPointer<WirelessDevice> wirelessNetworkInterface;
    QHash<QString, AccessPoint::Ptr> aps;
    AccessPoint::Ptr referenceAp;

private Q_SLOTS:
    void accessPointAppeared(const QString &uni);
    void accessPointDisappeared(const QString &uni);
    void updateStrength();
};

}

#endif
