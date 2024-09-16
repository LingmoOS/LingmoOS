// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIREDDEVICE_H
#define DWIREDDEVICE_H

#include "ddevice.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DWiredDevicePrivate;

class DWiredDevice : public DDevice
{
    Q_OBJECT
    Q_PROPERTY(QByteArray HwAddress READ HwAddress NOTIFY HwAddressChanged)
    Q_PROPERTY(QByteArray permHwAddress READ permHwAddress NOTIFY permHwAddressChanged)
    Q_PROPERTY(quint32 speed READ speed NOTIFY speedChanged)
    Q_PROPERTY(QStringList S390Subchannels READ S390Subchannels NOTIFY S390SubchannelsChanged)
    Q_PROPERTY(bool carrier READ carrier NOTIFY carrierChanged)
public:
    explicit DWiredDevice(const quint64 id, QObject *parent = nullptr);
    ~DWiredDevice() override = default;

    QByteArray HwAddress() const;
    QByteArray permHwAddress() const;
    quint32 speed() const;
    QStringList S390Subchannels() const;
    bool carrier() const;

Q_SIGNALS:
    void HwAddressChanged(const QByteArray &address);
    void permHwAddressChanged(const QByteArray &address);
    void speedChanged(const quint32 speed);
    void S390SubchannelsChanged(const QStringList &channels);
    void carrierChanged(const bool carrier);

private:
    Q_DECLARE_PRIVATE(DWiredDevice)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
