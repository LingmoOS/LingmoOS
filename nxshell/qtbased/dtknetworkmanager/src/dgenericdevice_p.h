// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGENERICDEVICE_P_H
#define DGENERICDEVICE_P_H

#include "dbus/dgenericdeviceinterface.h"
#include "dgenericdevice.h"
#include "ddevice_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DGenericDevicePrivate : public DDevicePrivate
{
    Q_OBJECT
public:
    explicit DGenericDevicePrivate(const quint64, DGenericDevice *parent = nullptr);
    ~DGenericDevicePrivate() override = default;

    DGenericDeviceInterface *m_generic{nullptr};
    Q_DECLARE_PUBLIC(DGenericDevice)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
