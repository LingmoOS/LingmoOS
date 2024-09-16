// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLOCKDEVICE_P_H
#define DBLOCKDEVICE_P_H

#include <DBlockDevice>

QT_BEGIN_NAMESPACE
class QDBusObjectPath;
QT_END_NAMESPACE

class OrgFreedesktopUDisks2BlockInterface;

DMOUNT_BEGIN_NAMESPACE

class DBlockDevicePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DBlockDevice)

public:
    explicit DBlockDevicePrivate(DBlockDevice *qq);
    ~DBlockDevicePrivate() override {}

    OrgFreedesktopUDisks2BlockInterface *dbus { nullptr };
    bool watchChanges { false };
    DBlockDevice *q_ptr { nullptr };
    QDBusError err;

private Q_SLOTS:
    void onInterfacesAdded(const QDBusObjectPath &objectPath, const QMap<QString, QVariantMap> &interfacesAndProperties);
    void onInterfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces);
    void onPropertiesChanged(const QString &interface, const QVariantMap &changedProperties);
};

DMOUNT_END_NAMESPACE

#endif   // DBLOCKDEVICE_P_H
