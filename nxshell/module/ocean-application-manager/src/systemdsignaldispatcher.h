// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYSTEMDSIGNALDISPATCHER_H
#define SYSTEMDSIGNALDISPATCHER_H

#include "global.h"
#include <QDBusMessage>

class SystemdSignalDispatcher : public QObject
{
    Q_OBJECT
public:
    ~SystemdSignalDispatcher() override = default;
    static SystemdSignalDispatcher &instance()
    {
        static SystemdSignalDispatcher dispatcher;
        return dispatcher;
    }
Q_SIGNALS:
    void SystemdUnitNew(QString unitName, QDBusObjectPath systemdUnitPath);
    void SystemdJobNew(QString unitName, QDBusObjectPath systemdUnitPath);
    void SystemdUnitRemoved(QString unitName, QDBusObjectPath systemdUnitPath);
    void SystemdEnvironmentChanged(QStringList envs);

private Q_SLOTS:
    void onUnitNew(QString unitName, QDBusObjectPath systemdUnitPath);
    void onJobNew(uint32_t id, QDBusObjectPath systemdUnitPath, QString unitName);
    void onUnitRemoved(QString unitName, QDBusObjectPath systemdUnitPath);
    void onPropertiesChanged(QString interface, QVariantMap props, QStringList invalid);

private:
    explicit SystemdSignalDispatcher(QObject *parent = nullptr)
        : QObject(parent)
    {
        auto &con = ApplicationManager1DBus::instance().globalDestBus();
        auto ret = con.call(QDBusMessage::createMethodCall(SystemdService, SystemdObjectPath, SystemdInterfaceName, "Subscribe"));
        if (ret.type() == QDBusMessage::ErrorMessage) {
            qFatal("%s", ret.errorMessage().toLocal8Bit().data());
        }
        if (!connectToSignals()) {
            std::terminate();
        }
    }
    bool connectToSignals() noexcept;
};

#endif
