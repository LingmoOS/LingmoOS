/*
    SPDX-FileCopyrightText: 2012-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_TEAM_DEVICE_P_H
#define NETWORKMANAGERQT_TEAM_DEVICE_P_H

#include "device_p.h"
#include "manager.h"
#include "manager_p.h"
#include "teamdevice.h"

#include "teamdeviceinterface.h"

namespace NetworkManager
{
class TeamDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    TeamDevicePrivate(const QString &path, TeamDevice *q);
    ~TeamDevicePrivate() override;

    OrgFreedesktopNetworkManagerDeviceTeamInterface iface;
    bool carrier;
    QString config;
    QString hwAddress;
    QStringList slaves;

    Q_DECLARE_PUBLIC(TeamDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

}

#endif
