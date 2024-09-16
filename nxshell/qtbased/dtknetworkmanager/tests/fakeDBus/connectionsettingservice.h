// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONNECTIONSETTINGSERVICE_H
#define CONNECTIONSETTINGSERVICE_H

#include "dnetworkmanagertypes.h"
#include <QDBusObjectPath>

DNETWORKMANAGER_USE_NAMESPACE

class FakeConnectionSettingService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.Settings.Connection")
public:
    explicit FakeConnectionSettingService(QObject *parent = nullptr);
    ~FakeConnectionSettingService() override;

    Q_PROPERTY(bool Unsaved READ unsaved)
    Q_PROPERTY(QString Filename READ filename)
    Q_PROPERTY(quint32 Flags READ flags)

    bool m_unsaved{true};
    QString m_filename{"/etc/NetworkManager/system-connections/uniontech.nmconnection"};
    quint32 m_flags{3};

    bool unsaved() const { return m_unsaved; }
    QString filename() const { return m_filename; };
    quint32 flags() const { return m_flags; };

    bool m_updateSettingTrigger{false};
    bool m_deleteSettingTrigger{false};
    bool m_settingsTrigger{false};
public Q_SLOTS:
    Q_SCRIPTABLE void Update(SettingDesc) { m_updateSettingTrigger = true; }

    Q_SCRIPTABLE void Delete() { m_deleteSettingTrigger = true; }

    Q_SCRIPTABLE SettingDesc GetSettings()
    {
        m_settingsTrigger = true;
        return {};
    }

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/Settings/3"};
};

#endif
