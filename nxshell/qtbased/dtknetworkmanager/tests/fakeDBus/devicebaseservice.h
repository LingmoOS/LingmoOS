// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef STATISTICSSERVICE_H
#define STATISTICSSERVICE_H

#include <QDBusObjectPath>
#include "deviceservice.h"

class FakeDeviceBaseService : public FakeDeviceService
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.Device.Statistics")
public:
    explicit FakeDeviceBaseService(QObject *parent = nullptr)
        : FakeDeviceService(parent){};
    virtual ~FakeDeviceBaseService() = default;

    Q_PROPERTY(quint32 RefreshRateMs READ refreshRateMs WRITE setRefreshRateMs)
    Q_PROPERTY(quint32 RxBytes READ rxBytes)
    Q_PROPERTY(quint32 TxBytes READ txBytes)

    quint32 m_refreshRateMs{10};
    quint64 m_rxBytes{3718};
    quint64 m_txBytes{910};

    quint32 refreshRateMs() { return m_refreshRateMs; }
    void setRefreshRateMs(const quint32 newRate) { m_refreshRateMs = newRate; }
    quint32 rxBytes() { return m_rxBytes; }
    quint32 txBytes() { return m_txBytes; }
};

#endif
