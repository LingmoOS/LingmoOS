// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBLOCKDEVICE_P_H
#define DBLOCKDEVICE_P_H

#include "dblockdevice.h"

QT_BEGIN_NAMESPACE
class QDBusObjectPath;
QT_END_NAMESPACE

class OrgFreedesktopUDisks2BlockInterface;

class DBlockDevicePrivate
{
public:
    explicit DBlockDevicePrivate(DBlockDevice *qq);

    OrgFreedesktopUDisks2BlockInterface *dbus;
    bool watchChanges = false;
    DBlockDevice *q_ptr;
    QDBusError err;

    void _q_onInterfacesAdded(const QDBusObjectPath &object_path, const QMap<QString, QVariantMap> &interfaces_and_properties);
    void _q_onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);
    void _q_onPropertiesChanged(const QString &interface, const QVariantMap &changed_properties);

    Q_DECLARE_PUBLIC(DBlockDevice)
};

#endif // DBLOCKDEVICE_P_H
