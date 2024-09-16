// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DADSLDEVICE_H
#define DADSLDEVICE_H

#include "ddevice.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DAdslDevicePrivate;

class DAdslDevice : public DDevice
{
    Q_OBJECT
    Q_PROPERTY(bool carrier READ carrier NOTIFY carrierChanged)
public:
    explicit DAdslDevice(const quint64 id, QObject *parent = nullptr);
    ~DAdslDevice() override = default;

    bool carrier() const;

Q_SIGNALS:
    void carrierChanged(const bool found);

private:
    Q_DECLARE_PRIVATE(DAdslDevice)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
