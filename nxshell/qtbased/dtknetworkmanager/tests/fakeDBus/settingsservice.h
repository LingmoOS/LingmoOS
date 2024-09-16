// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGMANAGERSERVICE_H
#define SETTINGMANAGERSERVICE_H

#include <QDBusObjectPath>
#include "dnetworkmanagertypes.h"

DNETWORKMANAGER_USE_NAMESPACE

class FakeSettingsService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.Settings")
public:
    explicit FakeSettingsService(QObject *parent = nullptr);
    ~FakeSettingsService() override;

    bool m_listConnectionsTrigger{false};
    bool m_getConnectionByUUIDTrigger{false};
    bool m_addConnectionTrigger{false};

public Q_SLOTS:

    Q_SCRIPTABLE QList<QDBusObjectPath> ListConnections()
    {
        m_listConnectionsTrigger = true;
        return {};
    }

    Q_SCRIPTABLE QDBusObjectPath GetConnectionByUuid(QString)
    {
        m_getConnectionByUUIDTrigger = true;
        return {};
    };

    Q_SCRIPTABLE QDBusObjectPath AddConnection(SettingDesc)
    {
        m_addConnectionTrigger = true;
        return {};
    }

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/Settings"};
};

#endif
