/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIMAXNSP_P_H
#define NETWORKMANAGERQT_WIMAXNSP_P_H

#include "dbus/wimaxnspinterface.h"
#include "wimaxnsp.h"

namespace NetworkManager
{
class WimaxNspPrivate : public QObject
{
    Q_OBJECT
public:
    WimaxNspPrivate(const QString &path, WimaxNsp *q);

    OrgFreedesktopNetworkManagerWiMaxNspInterface iface;
    QString uni;
    WimaxNsp::NetworkType networkType;
    QString name;
    uint signalQuality;

    Q_DECLARE_PUBLIC(WimaxNsp)
    WimaxNsp *q_ptr;
private Q_SLOTS:
    void propertiesChanged(const QVariantMap &properties);
};
}

#endif // NETWORKMANAGERQT_WIMAXNSP_P_H
