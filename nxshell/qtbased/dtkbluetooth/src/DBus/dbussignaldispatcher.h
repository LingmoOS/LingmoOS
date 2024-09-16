// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkbluetooth_global.h"
#include "dobjectmanagerinterface.h"
#include "dbluetoothdbustypes.h"
#include <QObject>
#include <QDBusMetaType>
#include <QMetaType>

#ifndef DBUSSIGNALDISPATCHER_H
#define DBUSSIGNALDISPATCHER_H

DBLUETOOTH_BEGIN_NAMESPACE

class DBusSignalDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit DBusSignalDispatcher(const QString &Service, QObject *parent = nullptr)
        : QObject(parent)
        , source(new DObjectManagerInterface(Service, this))
    {
    }
    ~DBusSignalDispatcher() override = default;

public Q_SLOTS:

    virtual void dispatchAdded(const QDBusObjectPath &path, const MapVariantMap &interfaces) = 0;
    virtual void dispatchRemoved(const QDBusObjectPath &path, const QStringList &args) = 0;

    QDBusPendingReply<ObjectMap> getManagedObjects() const;

protected:
    DObjectManagerInterface *source{nullptr};
};

DBLUETOOTH_END_NAMESPACE

#endif
