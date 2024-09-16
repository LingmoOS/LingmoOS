// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbluetoothadapter_p.h"
#include "dbluetoothdevice.h"
#include "dbluetoothutils.h"

DBLUETOOTH_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DUnexpected;
using DTK_CORE_NAMESPACE::emplace_tag;

DAdapterPrivate::DAdapterPrivate(quint64 adapterId, DAdapter *parent)
    : DObjectPrivate(parent)
    , m_adapter(new DAdapterInterface("/org/bluez/hci" + QString::number(adapterId)))
{
}

DAdapterPrivate::~DAdapterPrivate()
{
    delete m_adapter;
}

DAdapter::DAdapter(quint64 adapter, QObject *parent)
    : QObject(parent)
    , DObject(*new DAdapterPrivate(adapter, this))
{
    D_DC(DAdapter);
    connect(d->m_adapter, &DAdapterInterface::nameChanged, this, &DAdapter::nameChanged);
    connect(d->m_adapter, &DAdapterInterface::addressTypeChanged, this, [this](const QString &type) {
        Q_EMIT addressTypeChanged(stringToAddressType(type));
    });
    connect(d->m_adapter, &DAdapterInterface::aliasChanged, this, &DAdapter::aliasChanged);
    connect(d->m_adapter, &DAdapterInterface::poweredChanged, this, &DAdapter::poweredChanged);
    connect(d->m_adapter, &DAdapterInterface::discoverableChanged, this, &DAdapter::discoverableChanged);
    connect(d->m_adapter, &DAdapterInterface::discoverableTimeoutChanged, this, &DAdapter::discoverableTimeoutChanged);
    connect(d->m_adapter, &DAdapterInterface::discoveringChanged, this, &DAdapter::discoveringChanged);
    connect(d->m_adapter, &DAdapterInterface::removed, this, &DAdapter::removed);
    connect(d->m_adapter, &DAdapterInterface::deviceAdded, this, &DAdapter::deviceAdded);
    connect(d->m_adapter, &DAdapterInterface::deviceRemoved, this, &DAdapter::deviceRemoved);
}

DAdapter::~DAdapter() = default;

QString DAdapter::address() const
{
    D_DC(DAdapter);
    return d->m_adapter->address();
}

DDevice::AddressType DAdapter::addressType() const
{
    D_DC(DAdapter);
    return stringToAddressType(d->m_adapter->addressType());
}

QString DAdapter::name() const
{
    D_DC(DAdapter);
    return d->m_adapter->name();
}

bool DAdapter::powered() const
{
    D_DC(DAdapter);
    return d->m_adapter->powered();
}

QString DAdapter::alias() const
{
    D_DC(DAdapter);
    return d->m_adapter->alias();
}

void DAdapter::setAlias(const QString &alias)
{
    D_DC(DAdapter);
    d->m_adapter->setAlias(alias);
}

void DAdapter::setPowered(const bool powered)
{
    D_DC(DAdapter);
    d->m_adapter->setPowered(powered);
}

bool DAdapter::discoverable() const
{
    D_DC(DAdapter);
    return d->m_adapter->discoverable();
}

void DAdapter::setDiscoverable(const bool discoverable)
{
    D_DC(DAdapter);
    d->m_adapter->setDiscoverable(discoverable);
}

quint32 DAdapter::discoverableTimeout() const
{
    D_DC(DAdapter);
    return d->m_adapter->discoverableTimeout();
}

void DAdapter::setDiscoverableTimeout(const quint32 discoverableTimeout)
{
    D_DC(DAdapter);
    d->m_adapter->setDiscoverableTimeout(discoverableTimeout);
}

bool DAdapter::discovering() const
{
    D_DC(DAdapter);
    return d->m_adapter->discovering();
}

DExpected<QSharedPointer<DDevice>> DAdapter::deviceFromAddress(const QString &deviceAddress) const
{
    D_DC(DAdapter);
    const auto &reply = devices();
    if (!reply)
        return DUnexpected(reply.error());
    const auto &deviceList = reply.value();
    const auto &adapterPath = d->m_adapter->adapterPath();
    if (!deviceList.contains(deviceAddrToDBusPath(adapterPath, deviceAddress)))
        return DUnexpected{emplace_tag::USE_EMPLACE, -1, "no such device in current adapter"};
    return QSharedPointer<DDevice>(new DDevice(adapterPath, deviceAddress));
}

DExpected<QStringList> DAdapter::devices() const
{
    D_DC(DAdapter);
    auto reply = d->m_adapter->devices();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    auto DeviceList = getSpecificObject(
        reply.value(),
        {QString(BlueZDeviceInterface)},
        MapVariantMap{{QString(BlueZDeviceInterface),
                       QVariantMap{{QString("Adapter"),
                                    QVariant::fromValue<QDBusObjectPath>(QDBusObjectPath(d->m_adapter->adapterPath()))}}}});
    QStringList ret;
    for (const auto &device : DeviceList)
        ret.append(dBusPathToDeviceAddr(device));
    return ret;
}

DExpected<void> DAdapter::removeDevice(const QString &device) const
{
    D_DC(DAdapter);
    auto reply = d->m_adapter->removeDevice(QDBusObjectPath{deviceAddrToDBusPath(d->m_adapter->adapterPath(), device)});
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DExpected<void> DAdapter::startDiscovery() const
{
    D_DC(DAdapter);
    auto reply = d->m_adapter->startDiscovery();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DExpected<void> DAdapter::stopDiscovery() const
{
    D_DC(DAdapter);
    auto reply = d->m_adapter->stopDiscovery();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DBLUETOOTH_END_NAMESPACE
