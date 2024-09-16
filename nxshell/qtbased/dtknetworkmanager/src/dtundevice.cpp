// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtundevice_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DTunDevicePrivate::DTunDevicePrivate(const quint64 id, DTunDevice *parent)
    : DDevicePrivate(id, parent)
#ifdef USE_FAKE_INTERFACE
    , m_tun(new DTunDeviceInterface("/com/deepin/FakeNetworkManager/Devices/" + QByteArray::number(id), this))
#else
    , m_tun(new DTunDeviceInterface("/org/freedesktop/NetworkManager/Devices/" + QByteArray::number(id), this))
#endif
{
}

DTunDevice::DTunDevice(const quint64 id, QObject *parent)
    : DDevice(*new DTunDevicePrivate(id, this), parent)
{
    Q_D(const DTunDevice);
    connect(d->m_tun, &DTunDeviceInterface::HwAddressChanged, this, [this](const QString &addr) {
        Q_EMIT this->HwAddressChanged(addr.toUtf8());
    });
}

QByteArray DTunDevice::HwAddress() const
{
    Q_D(const DTunDevice);
    return d->m_tun->HwAddress().toUtf8();
}

DNETWORKMANAGER_END_NAMESPACE
