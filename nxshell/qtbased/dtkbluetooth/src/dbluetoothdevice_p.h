// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHDEVICE_P_H
#define DBLUETOOTHDEVICE_P_H

#include "ddeviceinterface.h"
#include "dbluetoothdevice.h"
#include <DObjectPrivate>
#include <DObject>

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DObjectPrivate;

class DDevicePrivate : public DObjectPrivate
{
public:
    explicit DDevicePrivate(const QString &path, DDevice *parent = nullptr);
    ~DDevicePrivate() override;
    bool isValid() const;

    DDeviceInterface *m_device{nullptr};
    D_DECLARE_PUBLIC(DDevice);
};

DBLUETOOTH_END_NAMESPACE

#endif
