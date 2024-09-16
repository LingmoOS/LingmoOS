// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLOCKDEVICEMONITOR_P_H
#define DBLOCKDEVICEMONITOR_P_H

#include <DBlockDeviceMonitor>

#include <QDBusObjectPath>
#include <QDBusMessage>

DMOUNT_BEGIN_NAMESPACE
class DBlockDeviceMonitorPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DBlockDeviceMonitor)

public:
    explicit DBlockDeviceMonitorPrivate(DBlockDeviceMonitor *qq)
        : QObject { qq }, q_ptr { qq }
    {
    }
    ~DBlockDeviceMonitorPrivate() override { }

private Q_SLOTS:
    void onInterfacesAdded(const QDBusObjectPath &objPath, const QMap<QString, QVariantMap> &ifaces);
    void onInterfacesRemoved(const QDBusObjectPath &objPath, const QStringList &ifaces);
    void onPropertiesChanged(const QString &iface, const QVariantMap &changedProperties, const QDBusMessage &message);

private:
    DBlockDeviceMonitor *q_ptr { nullptr };
    bool isWatching { false };
};

DMOUNT_END_NAMESPACE

#endif   // DBLOCKDEVICEMONITOR_P_H
