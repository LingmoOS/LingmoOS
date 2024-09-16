// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIREDDEVICE_P_H
#define DWIREDDEVICE_P_H

#include "dbus/dwireddeviceinterface.h"
#include "dwireddevice.h"
#include "ddevice_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DWiredDevicePrivate : public DDevicePrivate
{
    Q_OBJECT
public:
    explicit DWiredDevicePrivate(const quint64 id, DWiredDevice *parent = nullptr);
    ~DWiredDevicePrivate() override = default;

    DWiredDeviceInterface *m_wired{nullptr};
    Q_DECLARE_PUBLIC(DWiredDevice)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
