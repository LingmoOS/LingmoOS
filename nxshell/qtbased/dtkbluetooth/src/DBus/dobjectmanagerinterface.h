// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOBJECTMANAGERINTERFACE_H
#define DOBJECTMANAGERINTERFACE_H

#include "dtkbluetooth_global.h"
#include "dbluetoothdbustypes.h"
#include <DDBusInterface>
#include <QDBusPendingReply>

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DObjectManagerInterface : public QObject
{
    Q_OBJECT
public:
    explicit DObjectManagerInterface(const QString &service, QObject *parent = nullptr);
    ~DObjectManagerInterface() override = default;

public Q_SLOTS:

    QDBusPendingReply<ObjectMap> getManagedObjects();

Q_SIGNALS:
    void InterfacesAdded(const QDBusObjectPath &path, const MapVariantMap &interfaces);
    void InterfacesRemoved(const QDBusObjectPath &path, const QStringList &args);

private Q_SLOTS:
    void InterfacesAdd(const QDBusObjectPath &path, const MapVariantMap &interfaces);
    void InterfacesRemove(const QDBusObjectPath &path, const QStringList &args);

private:
    DDBusInterface *m_inter{nullptr};
};

DBLUETOOTH_END_NAMESPACE

#endif
