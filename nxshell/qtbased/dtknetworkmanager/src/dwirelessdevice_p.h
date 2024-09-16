// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIRELESSDEVICE_P_H
#define DWIRELESSDEVICE_P_H

#include "dbus/dwirelessdeviceinterface.h"
#include "dwirelessdevice.h"
#include "ddevice_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DWirelessDevicePrivate : public DDevicePrivate
{
    Q_OBJECT
public:
    explicit DWirelessDevicePrivate(const quint64, DWirelessDevice *parent = nullptr);
    ~DWirelessDevicePrivate() override = default;

    DWirelessDeviceInterface *m_wireless{nullptr};
    Q_DECLARE_PUBLIC(DWirelessDevice)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
