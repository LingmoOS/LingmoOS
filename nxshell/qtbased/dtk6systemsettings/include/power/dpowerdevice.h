// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dpowertypes.h"

#include <dexpected.h>
#include <qdatetime.h>
#include <qscopedpointer.h>
using PowerHistoryList = QList<DTK_POWER_NAMESPACE::History>;
using PowerStatisticList = QList<DTK_POWER_NAMESPACE::Statistic>;

DPOWER_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;

class DPowerDevicePrivate;

class DPowerDevice : public QObject
{
    Q_OBJECT
public:
    virtual ~DPowerDevice();

    Q_PROPERTY(bool hasHistory READ hasHistory);
    Q_PROPERTY(bool hasStatistics READ hasStatistics);
    Q_PROPERTY(bool isPresent READ isPresent);
    Q_PROPERTY(bool isRechargeable READ isRechargeable);
    Q_PROPERTY(bool online READ online);
    Q_PROPERTY(bool powerSupply READ powerSupply);
    Q_PROPERTY(double capacity READ capacity);
    Q_PROPERTY(double energy READ energy NOTIFY energyChanged);
    Q_PROPERTY(double energyEmpty READ energyEmpty);
    Q_PROPERTY(double energyFull READ energyFull);
    Q_PROPERTY(double energyFullDesign READ energyFullDesign);
    Q_PROPERTY(double energyRate READ energyRate NOTIFY energyRateChanged);
    Q_PROPERTY(double luminosity READ luminosity);
    Q_PROPERTY(double percentage READ percentage NOTIFY percentageChanged);
    Q_PROPERTY(double temperature READ temperature);
    Q_PROPERTY(double voltage READ voltage);
    Q_PROPERTY(uint chargeCycles READ chargeCycles);
    Q_PROPERTY(qint64 timeToEmpty READ timeToEmpty NOTIFY timeToEmptyChanged);
    Q_PROPERTY(qint64 timeToFull READ timeToFull NOTIFY timeToFullChanged);
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged);
    Q_PROPERTY(QString model READ model);
    Q_PROPERTY(QString nativePath READ nativePath);
    Q_PROPERTY(QString serial READ serial);
    Q_PROPERTY(QString vendor READ vendor);
    Q_PROPERTY(uint batteryLevel READ batteryLevel);
    Q_PROPERTY(uint state READ state);
    Q_PROPERTY(uint technology READ technology);
    Q_PROPERTY(uint type READ type);
    Q_PROPERTY(uint warningLevel READ warningLevel);
    Q_PROPERTY(QDateTime updateTime READ updateTime NOTIFY updateTimeChanged);
    Q_PROPERTY(QString deviceName READ deviceName);

    bool hasHistory() const;
    bool hasStatistics() const;
    bool isPresent() const;
    bool isRechargeable() const;
    bool online() const;
    bool powerSupply() const;
    double capacity() const;
    double energy() const;
    double energyEmpty() const;
    double energyFull() const;
    double energyFullDesign() const;
    double energyRate() const;
    double luminosity() const;
    double percentage() const;
    double temperature() const;
    double voltage() const;
    qint32 chargeCycles() const;
    qint64 timeToEmpty() const;
    qint64 timeToFull() const;
    QString iconName() const;
    QString model() const;
    QString nativePath() const;
    QString serial() const;
    QString vendor() const;
    quint32 batteryLevel() const;
    quint32 state() const;
    quint32 technology() const;
    quint32 type() const;
    quint32 warningLevel() const;
    QDateTime updateTime() const;
    QString deviceName() const;

signals:
    void updateTimeChanged(const QDateTime &value);
    void percentageChanged(const double value);
    void timeToEmptyChanged(const qint64 value);
    void timeToFullChanged(const qint64 value);
    void energyRateChanged(const double value);
    void energyChanged(const double value);
    void iconNameChanged(const QString &value);

public slots:
    DExpected<PowerHistoryList> history(const QString &type,
                                        const uint timespan,
                                        const uint resolution) const;
    DExpected<PowerStatisticList> statistics(const QString &type) const;
    DExpected<void> refresh();

private:
    QScopedPointer<DPowerDevicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DPowerDevice)

    explicit DPowerDevice(const QString &name, QObject *parent = nullptr);
    Q_DISABLE_COPY(DPowerDevice)

    friend class DPowerManager;
};

DPOWER_END_NAMESPACE
