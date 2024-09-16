// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHADAPTER_H
#define DBLUETOOTHADAPTER_H

#include "dbluetoothdevice.h"
#include "dbluetoothtypes.h"
#include <QScopedPointer>
#include <DExpected>
#include <DObject>

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DObject;

class DManager;

class DAdapterPrivate;

class DAdapter : public QObject, public DObject
{
    Q_OBJECT
    explicit DAdapter(quint64 adapter, QObject *parent = nullptr);
    friend class DManager;

public:
    ~DAdapter() override;

    Q_PROPERTY(QString address READ address CONSTANT)
    Q_PROPERTY(DDevice::AddressType addressType READ addressType NOTIFY addressTypeChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString alias READ alias WRITE setAlias NOTIFY aliasChanged)
    Q_PROPERTY(bool powered READ powered WRITE setPowered NOTIFY poweredChanged)
    Q_PROPERTY(bool discoverable READ discoverable WRITE setDiscoverable NOTIFY discoverableChanged)
    Q_PROPERTY(
        quint32 discoverableTimeout READ discoverableTimeout WRITE setDiscoverableTimeout NOTIFY discoverableTimeoutChanged)
    Q_PROPERTY(bool discovering READ discovering NOTIFY discoveringChanged)

    QString address() const;
    DDevice::AddressType addressType() const;
    QString name() const;
    QString alias() const;
    void setAlias(const QString &alias);
    bool powered() const;
    void setPowered(bool powered);
    bool discoverable() const;
    void setDiscoverable(bool discoverable);
    quint32 discoverableTimeout() const;
    void setDiscoverableTimeout(quint32 discoverableTimeout);
    bool discovering() const;

    DExpected<QSharedPointer<DDevice>> deviceFromAddress(const QString &deviceAddress) const;
    DExpected<QStringList> devices() const;
public Q_SLOTS:
    DExpected<void> removeDevice(const QString &device) const;
    DExpected<void> startDiscovery() const;
    DExpected<void> stopDiscovery() const;

Q_SIGNALS:
    void addressTypeChanged(DDevice::AddressType type);
    void nameChanged(const QString &name);
    void aliasChanged(const QString &alias);
    void poweredChanged(bool powered);
    void discoverableChanged(bool discoverable);
    void discoverableTimeoutChanged(quint32 discoverableTimeout);
    void discoveringChanged(bool discovering);

    void removed();
    void deviceAdded(const QString &deviceAddress);
    void deviceRemoved(const QString &deviceAddress);

private:
    D_DECLARE_PRIVATE(DAdapter);
};

DBLUETOOTH_END_NAMESPACE
#endif
