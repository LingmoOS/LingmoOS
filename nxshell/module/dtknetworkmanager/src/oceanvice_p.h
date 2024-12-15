// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OCEANVICEPRIVATE_P_H
#define OCEANVICEPRIVATE_P_H

#include "oceanvice.h"
#include "dbus/oceanviceInterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DDevicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit DDevicePrivate(const quint64 deviceId, DDevice *parent = nullptr);
    ~DDevicePrivate() override;

    DDeviceInterface *m_device{nullptr};
    DDevice *q_ptr{nullptr};
    Q_DECLARE_PUBLIC(DDevice)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
