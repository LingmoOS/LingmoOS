// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ADSLDEVICESERVICE_H
#define ADSLDEVICESERVICE_H

#include "devicebaseservice.h"

class FakeAdslDeviceService : public FakeDeviceBaseService
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.Device.Adsl")
public:
    FakeAdslDeviceService(QObject *parent = nullptr)
        : FakeDeviceBaseService(parent)
    {
    }
    ~FakeAdslDeviceService() override = default;

    Q_PROPERTY(bool Carrier READ carrier)

    bool m_carrier{true};

    bool carrier() { return m_carrier; }
};

#endif
