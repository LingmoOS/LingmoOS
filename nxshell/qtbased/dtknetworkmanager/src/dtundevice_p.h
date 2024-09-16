// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTUNDEVICE_P_H
#define DTUNDEVICE_P_H

#include "dbus/dtundeviceinterface.h"
#include "dtundevice.h"
#include "ddevice_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DTunDevicePrivate : public DDevicePrivate
{
    Q_OBJECT
public:
    explicit DTunDevicePrivate(const quint64, DTunDevice *parent = nullptr);
    ~DTunDevicePrivate() override = default;

    DTunDeviceInterface *m_tun{nullptr};
    Q_DECLARE_PUBLIC(DTunDevice)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
