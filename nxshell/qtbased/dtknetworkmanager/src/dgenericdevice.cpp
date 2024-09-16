// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dgenericdevice_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DGenericDevicePrivate::DGenericDevicePrivate(const quint64 id, DGenericDevice *parent)
    : DDevicePrivate(id, parent)
#ifdef USE_FAKE_INTERFACE
    , m_generic(new DGenericDeviceInterface("/com/deepin/FakeNetworkManager/Devices/" + QByteArray::number(id), this))
#else
    , m_generic(new DGenericDeviceInterface("/org/freedesktop/NetworkManager/Devices/" + QByteArray::number(id), this))
#endif
{
}

DGenericDevice::DGenericDevice(const quint64 id, QObject *parent)
    : DDevice(*new DGenericDevicePrivate(id, this), parent)
{
    Q_D(const DGenericDevice);
    connect(d->m_generic, &DGenericDeviceInterface::HwAddressChanged, this, [this](const QString &addr) {
        Q_EMIT this->HwAddressChanged(addr.toUtf8());
    });
    connect(d->m_generic, &DGenericDeviceInterface::TypeDescriptionChanged, this, &DGenericDevice::typeDescriptionChanged);
}

QByteArray DGenericDevice::HwAddress() const
{
    Q_D(const DGenericDevice);
    return d->m_generic->HwAddress().toUtf8();
}

QString DGenericDevice::typeDescription() const
{
    Q_D(const DGenericDevice);
    return d->m_generic->typeDescription();
}

DNETWORKMANAGER_END_NAMESPACE
