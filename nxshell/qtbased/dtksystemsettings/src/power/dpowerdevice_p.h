// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dpowerdevice.h"

DPOWER_BEGIN_NAMESPACE

class UPowerDeviceInterface;

class DPowerDevicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit DPowerDevicePrivate(DPowerDevice *parent)
        : q_ptr(parent)
    {
    }

    virtual ~DPowerDevicePrivate() { }

public:
    QString devicename;
    UPowerDeviceInterface *m_device_inter;
    DPowerDevice *q_ptr;
    Q_DECLARE_PUBLIC(DPowerDevice)
};

DPOWER_END_NAMESPACE
