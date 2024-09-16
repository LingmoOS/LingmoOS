// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DADAPTERINTERFACE_H
#define DADAPTERINTERFACE_H

#include "dtkbluetooth_global.h"
#include "dbluetoothdbustypes.h"
#include <QObject>
#include <QDBusObjectPath>
#include <DDBusInterface>
#include <QDBusPendingReply>

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DAdapterInterface : public QObject
{
    Q_OBJECT
public:
    explicit DAdapterInterface(const QString &path, QObject *parent = nullptr);
    ~DAdapterInterface() override = default;

    Q_PROPERTY(QString address READ address CONSTANT)
    Q_PROPERTY(QString addressType READ addressType NOTIFY addressTypeChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString alias READ alias WRITE setAlias NOTIFY aliasChanged)
    Q_PROPERTY(bool powered READ powered WRITE setPowered NOTIFY poweredChanged)
    Q_PROPERTY(bool discoverable READ discoverable WRITE setDiscoverable NOTIFY discoverableChanged)
    Q_PROPERTY(
        quint32 discoverableTimeout READ discoverableTimeout WRITE setDiscoverableTimeout NOTIFY discoverableTimeoutChanged)
    Q_PROPERTY(bool discovering READ discovering NOTIFY discoveringChanged)

    QString address() const;
    QString addressType() const;
    QString name() const;
    QString alias() const;
    void setAlias(const QString &alias);
    bool powered() const;
    void setPowered(const bool powered);
    bool discoverable() const;
    void setDiscoverable(const bool discoverable);
    quint32 discoverableTimeout() const;
    void setDiscoverableTimeout(const quint32 discoverableTimeout);
    bool discovering() const;
    QString adapterPath() const;

public Q_SLOTS:
    QDBusPendingReply<ObjectMap> devices() const;
    QDBusPendingReply<void> removeDevice(const QDBusObjectPath &device) const;
    QDBusPendingReply<void> startDiscovery() const;
    QDBusPendingReply<void> stopDiscovery() const;

Q_SIGNALS:
    void addressTypeChanged(const QString &type);
    void nameChanged(const QString &name);
    void aliasChanged(const QString &alias);
    void poweredChanged(const bool &powered);
    void discoverableChanged(const bool &discoverable);
    void discoverableTimeoutChanged(const quint32 discoverableTimeout);
    void discoveringChanged(const bool discovering);

    void removed();
    void deviceAdded(const QString &device);
    void deviceRemoved(const QString &device);

private:
    DDBusInterface *m_inter{nullptr};
};

DBLUETOOTH_END_NAMESPACE

#endif
