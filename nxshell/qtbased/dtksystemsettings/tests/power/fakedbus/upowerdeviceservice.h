// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "upowertypes_p.h"

#include <qdatetime.h>
#include <qdbuspendingreply.h>
#include <qdebug.h>
#include <qobject.h>
#include <qscopedpointer.h>

class UPowerDeviceService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakePower")
public:
    explicit UPowerDeviceService(QObject *parent = nullptr);
    virtual ~UPowerDeviceService();
    // properties
    Q_PROPERTY(bool HasHistory READ hasHistory);
    Q_PROPERTY(bool HasStatistics READ hasStatistics);
    Q_PROPERTY(bool IsPresent READ isPresent);
    Q_PROPERTY(bool IsRechargeable READ isRechargeable);
    Q_PROPERTY(bool Online READ online);
    Q_PROPERTY(bool PowerSupply READ powerSupply);
    Q_PROPERTY(double Capacity READ capacity);
    Q_PROPERTY(double Energy READ energy);
    Q_PROPERTY(double EnergyEmpty READ energyEmpty);
    Q_PROPERTY(double EnergyFull READ energyFull);
    Q_PROPERTY(double EnergyFullDesign READ energyFullDesign);
    Q_PROPERTY(double EnergyRate READ energyRate);
    Q_PROPERTY(double Luminosity READ luminosity);
    Q_PROPERTY(double Percentage READ percentage);
    Q_PROPERTY(double Temperature READ temperature);
    Q_PROPERTY(double Voltage READ voltage);
    Q_PROPERTY(quint32 ChargeCycles READ chargeCycles);
    Q_PROPERTY(qint64 TimeToEmpty READ timeToEmpty);
    Q_PROPERTY(qint64 TimeToFull READ timeToFull);
    Q_PROPERTY(QString IconName READ iconName);
    Q_PROPERTY(QString Model READ model);
    Q_PROPERTY(QString NativePath READ nativePath);
    Q_PROPERTY(QString Serial READ serial);
    Q_PROPERTY(QString Vendor READ vendor);
    Q_PROPERTY(quint32 BatteryLevel READ batteryLevel);
    Q_PROPERTY(quint32 State READ state);
    Q_PROPERTY(quint32 Technology READ technology);
    Q_PROPERTY(quint32 Type READ type);
    Q_PROPERTY(quint32 WarningLevel READ warningLevel);
    Q_PROPERTY(quint64 UpdateTime READ updateTime);
    Q_PROPERTY(QString DeviceName READ deviceName);

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
    quint64 updateTime() const;
    QString deviceName() const;

public slots:
    Q_SCRIPTABLE QList<DTK_POWER_NAMESPACE::History_p> GetHistory(const QString &type,
                                                                  const uint timespan,
                                                                  const uint resolution) const;
    Q_SCRIPTABLE QList<DTK_POWER_NAMESPACE::Statistic_p> GetStatistics(const QString &type) const;
    // void refresh();
private:
    void registerService();
    void unRegisterService();
    bool m_reset;
    bool m_refesh;
    QString name = "battery";
};
