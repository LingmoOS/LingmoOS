// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dadsldevice_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DAdslDevicePrivate::DAdslDevicePrivate(const quint64 id, DAdslDevice *parent)
    : DDevicePrivate(id, parent)
#ifdef USE_FAKE_INTERFACE
    , m_adsl(new DAdslDeviceInterface("/com/deepin/FakeNetworkManager/Devices/" + QByteArray::number(id), this))
#else
    , m_adsl(new DAdslDeviceInterface("/org/freedesktop/NetworkManager/Devices/" + QByteArray::number(id), this))
#endif
{
}

DAdslDevice::DAdslDevice(const quint64 id, QObject *parent)
    : DDevice(*new DAdslDevicePrivate(id, this), parent)
{
    Q_D(const DAdslDevice);
    connect(d->m_adsl, &DAdslDeviceInterface::CarrierChanged, this, &DAdslDevice::carrierChanged);
}

bool DAdslDevice::carrier() const
{
    Q_D(const DAdslDevice);
    return d->m_adsl->carrier();
}

DNETWORKMANAGER_END_NAMESPACE
