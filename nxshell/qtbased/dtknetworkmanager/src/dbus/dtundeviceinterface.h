// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTUNDEVICEINTERFACE_H
#define DTUNDEVICEINTERFACE_H

#include "ddeviceInterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DTunDeviceInterface : public DDeviceInterface
{
    Q_OBJECT
public:
    explicit DTunDeviceInterface(const QByteArray &path, QObject *parent = nullptr);
    ~DTunDeviceInterface() override = default;

    Q_PROPERTY(QString HwAddress READ HwAddress NOTIFY HwAddressChanged)

    QString HwAddress() const;

Q_SIGNALS:
    void HwAddressChanged(const QString &address);

private:
    DDBusInterface *m_tunInter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
