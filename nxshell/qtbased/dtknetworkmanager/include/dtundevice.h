// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTUNDEVICE_H
#define DTUNDEVICE_H

#include "ddevice.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DTunDevicePrivate;

class DTunDevice : public DDevice
{
    Q_OBJECT
    Q_PROPERTY(QByteArray HwAddress READ HwAddress NOTIFY HwAddressChanged)
public:
    explicit DTunDevice(const quint64 id, QObject *parent = nullptr);
    ~DTunDevice() override = default;

    QByteArray HwAddress() const;

Q_SIGNALS:
    void HwAddressChanged(const QByteArray &addr);

private:
    Q_DECLARE_PRIVATE(DTunDevice)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
