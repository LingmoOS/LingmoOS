// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIREDDEVICEINTERFACE_H
#define DWIREDDEVICEINTERFACE_H

#include "ddeviceInterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DWiredDeviceInterface : public DDeviceInterface
{
    Q_OBJECT
public:
    explicit DWiredDeviceInterface(const QByteArray &path, QObject *parent = nullptr);
    ~DWiredDeviceInterface() override = default;

    Q_PROPERTY(QString HwAddress READ HwAddress NOTIFY HwAddressChanged)
    Q_PROPERTY(QString permHwAddress READ permHwAddress NOTIFY PermHwAddressChanged)
    Q_PROPERTY(quint32 speed READ speed NOTIFY SpeedChanged)
    Q_PROPERTY(QStringList S390Subchannels READ S390Subchannels NOTIFY S390SubchannelsChanged)
    Q_PROPERTY(bool carrier READ carrier NOTIFY CarrierChanged)

    QString HwAddress() const;
    QString permHwAddress() const;
    quint32 speed() const;
    QStringList S390Subchannels() const;
    bool carrier() const;

Q_SIGNALS:
    void HwAddressChanged(const QString &address);
    void PermHwAddressChanged(const QString &address);
    void SpeedChanged(const quint32 speed);
    void S390SubchannelsChanged(const QStringList &channels);
    void CarrierChanged(const bool carrier);

private:
    DDBusInterface *m_wiredInter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
