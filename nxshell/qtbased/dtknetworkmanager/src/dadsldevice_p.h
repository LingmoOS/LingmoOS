// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DADSLDEVICE_P_H
#define DADSLDEVICE_P_H

#include "dbus/dadsldeviceinterface.h"
#include "dadsldevice.h"
#include "ddevice_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DAdslDevicePrivate : public DDevicePrivate
{
    Q_OBJECT
public:
    explicit DAdslDevicePrivate(const quint64 id, DAdslDevice *parent = nullptr);
    ~DAdslDevicePrivate() override = default;

    DAdslDeviceInterface *m_adsl{nullptr};
    Q_DECLARE_PUBLIC(DAdslDevice)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
