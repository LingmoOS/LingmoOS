// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGENERICDEVICEINTERFACE_H
#define DGENERICDEVICEINTERFACE_H

#include "ddeviceInterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DGenericDeviceInterface : public DDeviceInterface
{
    Q_OBJECT
public:
    explicit DGenericDeviceInterface(const QByteArray &path, QObject *parent = nullptr);
    ~DGenericDeviceInterface() override = default;

    Q_PROPERTY(QString HwAddress READ HwAddress NOTIFY HwAddressChanged)
    Q_PROPERTY(QString typeDescription READ typeDescription NOTIFY TypeDescriptionChanged)

    QString HwAddress() const;
    QString typeDescription() const;

Q_SIGNALS:
    void HwAddressChanged(const QString &address);
    void TypeDescriptionChanged(const QString &desc);

private:
    DDBusInterface *m_genericInter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
