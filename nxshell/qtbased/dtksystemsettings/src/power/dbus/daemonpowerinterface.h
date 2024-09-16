// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtkpower_global.h"

#include <ddbusinterface.h>
#include <qdbuspendingreply.h>
#include <qscopedpointer.h>

DPOWER_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DDBusInterface;

class DaemonPowerInterface : public QObject
{
    Q_OBJECT
public:
    explicit DaemonPowerInterface(QObject *parent = nullptr);

    virtual ~DaemonPowerInterface() { }

    Q_PROPERTY(qint32 BatteryLidClosedAction READ batteryLidClosedAction WRITE setBatteryLidClosedAction NOTIFY
                   BatteryLidClosedActionChanged);
    Q_PROPERTY(qint32 BatteryLockDelay READ batteryLockDelay WRITE setBatteryLockDelay NOTIFY BatteryLockDelayChanged);
    Q_PROPERTY(qint32 BatteryPressPowerBtnAction READ batteryPressPowerBtnAction WRITE setBatteryPressPowerBtnAction NOTIFY
                   BatteryPressPowerBtnActionChanged);
    Q_PROPERTY(qint32 BatteryScreenBlackDelay READ batteryScreenBlackDelay WRITE setBatteryScreenBlackDelay NOTIFY
                   BatteryScreenBlackDelayChanged);
    Q_PROPERTY(qint32 BatteryScreensaverDelay READ batteryScreensaverDelay WRITE setBatteryScreensaverDelay NOTIFY
                   BatteryScreensaverDelayChanged);
    Q_PROPERTY(qint32 BatterySleepDelay READ batterySleepDelay WRITE setBatterySleepDelay NOTIFY BatterySleepDelayChanged);
    Q_PROPERTY(qint32 LinePowerLidClosedAction READ linePowerLidClosedAction WRITE setLinePowerLidClosedAction NOTIFY
                   LinePowerLidClosedActionChanged);
    Q_PROPERTY(qint32 LinePowerLockDelay READ linePowerLockDelay WRITE setLinePowerLockDelay NOTIFY LinePowerLockDelayChanged);
    Q_PROPERTY(qint32 LinePowerPressPowerBtnAction READ linePowerPressPowerBtnAction WRITE setLinePowerPressPowerBtnAction NOTIFY
                   LinePowerPressPowerBtnActionChanged);
    Q_PROPERTY(qint32 LinePowerScreenBlackDelay READ linePowerScreenBlackDelay WRITE setLinePowerScreenBlackDelay NOTIFY
                   LinePowerScreenBlackDelayChanged);
    Q_PROPERTY(qint32 LinePowerScreensaverDelay READ linePowerScreensaverDelay WRITE setLinePowerScreensaverDelay NOTIFY
                   LinePowerScreensaverDelayChanged);
    Q_PROPERTY(
        qint32 LinePowerSleepDelay READ linePowerSleepDelay WRITE setLinePowerSleepDelay NOTIFY LinePowerSleepDelayChanged);
    Q_PROPERTY(qint32 LowPowerAutoSleepThreshold READ lowPowerAutoSleepThreshold WRITE setLowPowerAutoSleepThreshold NOTIFY
                   LowPowerAutoSleepThresholdChanged);
    Q_PROPERTY(
        bool LowPowerNotifyEnable READ lowPowerNotifyEnable WRITE setLowPowerNotifyEnable NOTIFY LowPowerNotifyEnableChanged);
    Q_PROPERTY(qint32 LowPowerNotifyThreshold READ lowPowerNotifyThreshold WRITE setLowPowerNotifyThreshold NOTIFY
                   LowPowerNotifyThresholdChanged);
    Q_PROPERTY(bool ScreenBlackLock READ screenBlackLock WRITE setScreenBlackLock NOTIFY ScreenBlackLockChanged);
    Q_PROPERTY(bool SleepLock READ sleepLock WRITE setSleepLock NOTIFY SleepLockChanged);

    inline qint32 batteryLidClosedAction() const
    {
        return qdbus_cast<qint32>(m_inter->property("BatteryLidClosedAction"));
    }

    inline void setBatteryLidClosedAction(const qint32 value)
    {
        m_inter->setProperty("BatteryLidClosedAction", QVariant::fromValue(value));
    }

    inline qint32 batteryLockDelay() const
    {
        return qdbus_cast<qint32>(m_inter->property("BatteryLockDelay"));
    }

    inline void setBatteryLockDelay(const qint32 value)
    {
        m_inter->setProperty("BatteryLockDelay", QVariant::fromValue(value));
    }

    inline qint32 batteryPressPowerBtnAction() const
    {
        return qdbus_cast<qint32>(m_inter->property("BatteryPressPowerBtnAction"));
    }

    inline void setBatteryPressPowerBtnAction(const qint32 value)
    {
        m_inter->setProperty("BatteryPressPowerBtnAction", QVariant::fromValue(value));
    }

    inline qint32 batteryScreenBlackDelay() const
    {
        return qdbus_cast<qint32>(m_inter->property("BatteryScreenBlackDelay"));
    }

    inline void setBatteryScreenBlackDelay(const qint32 value)
    {
        m_inter->setProperty("BatteryScreenBlackDelay", QVariant::fromValue(value));
    }

    inline qint32 batteryScreensaverDelay() const
    {
        return qdbus_cast<qint32>(m_inter->property("BatteryScreensaverDelay"));
    }

    inline void setBatteryScreensaverDelay(const qint32 value)
    {
        m_inter->setProperty("BatteryScreensaverDelay", QVariant::fromValue(value));
    }

    inline qint32 batterySleepDelay() const
    {
        return qdbus_cast<qint32>(m_inter->property("BatterySleepDelay"));
    }

    inline void setBatterySleepDelay(const qint32 value)
    {
        m_inter->setProperty("BatterySleepDelay", QVariant::fromValue(value));
    }

    inline qint32 linePowerLidClosedAction() const
    {
        return qdbus_cast<qint32>(m_inter->property("LinePowerLidClosedAction"));
    }

    inline void setLinePowerLidClosedAction(const qint32 value)
    {
        m_inter->setProperty("LinePowerLidClosedAction", QVariant::fromValue(value));
    }

    inline qint32 linePowerLockDelay() const
    {
        return qdbus_cast<qint32>(m_inter->property("LinePowerLockDelay"));
    }

    inline void setLinePowerLockDelay(const qint32 value)
    {
        m_inter->setProperty("LinePowerLockDelay", QVariant::fromValue(value));
    }

    inline qint32 linePowerPressPowerBtnAction() const
    {
        return qdbus_cast<qint32>(m_inter->property("LinePowerPressPowerBtnAction"));
    }

    inline void setLinePowerPressPowerBtnAction(const qint32 value)
    {
        m_inter->setProperty("LinePowerPressPowerBtnAction", QVariant::fromValue(value));
    }

    inline qint32 linePowerScreenBlackDelay() const
    {
        return qdbus_cast<qint32>(m_inter->property("LinePowerScreenBlackDelay"));
    }

    inline void setLinePowerScreenBlackDelay(const qint32 value)
    {
        m_inter->setProperty("LinePowerScreenBlackDelay", QVariant::fromValue(value));
    }

    inline qint32 linePowerScreensaverDelay() const
    {
        return qdbus_cast<qint32>(m_inter->property("LinePowerScreensaverDelay"));
    }

    inline void setLinePowerScreensaverDelay(const qint32 value)
    {
        m_inter->setProperty("LinePowerScreensaverDelay", QVariant::fromValue(value));
    }

    inline qint32 linePowerSleepDelay() const
    {
        return qdbus_cast<qint32>(m_inter->property("LinePowerSleepDelay"));
    }

    inline void setLinePowerSleepDelay(const qint32 value)
    {
        m_inter->setProperty("LinePowerSleepDelay", QVariant::fromValue(value));
    }

    inline qint32 lowPowerAutoSleepThreshold() const
    {
        return qdbus_cast<qint32>(m_inter->property("LowPowerAutoSleepThreshold"));
    }

    inline void setLowPowerAutoSleepThreshold(const qint32 value)
    {
        m_inter->setProperty("LowPowerAutoSleepThreshold", QVariant::fromValue(value));
    }

    inline bool lowPowerNotifyEnable() const
    {
        return qdbus_cast<bool>(m_inter->property("LowPowerNotifyEnable"));
    }

    inline void setLowPowerNotifyEnable(const bool value)
    {
        m_inter->setProperty("LowPowerNotifyEnable", QVariant::fromValue(value));
    }

    inline qint32 lowPowerNotifyThreshold() const
    {
        return qdbus_cast<qint32>(m_inter->property("LowPowerNotifyThreshold"));
    }

    inline void setLowPowerNotifyThreshold(const qint32 value)
    {
        m_inter->setProperty("LowPowerNotifyThreshold", QVariant::fromValue(value));
    }

    inline bool screenBlackLock() const
    {
        return qdbus_cast<bool>(m_inter->property("ScreenBlackLock"));
    }

    inline void setScreenBlackLock(const bool value)
    {
        m_inter->setProperty("ScreenBlackLock", QVariant::fromValue(value));
    }

    inline bool sleepLock() const { return qdbus_cast<bool>(m_inter->property("SleepLock")); }

    inline void setSleepLock(const bool value)
    {
        m_inter->setProperty("SleepLock", QVariant::fromValue(value));
    }

signals:
    void BatteryLidClosedActionChanged(const qint32 value);
    void BatteryLockDelayChanged(const qint32 value);
    void BatteryPressPowerBtnActionChanged(const qint32 value);
    void BatteryScreenBlackDelayChanged(const qint32 value);
    void BatteryScreensaverDelayChanged(const qint32 value);
    void BatterySleepDelayChanged(const qint32 value);
    void LinePowerLidClosedActionChanged(const qint32 value);
    void LinePowerLockDelayChanged(const qint32 value);
    void LinePowerPressPowerBtnActionChanged(const qint32 value);
    void LinePowerScreenBlackDelayChanged(const qint32 value);
    void LinePowerScreensaverDelayChanged(const qint32 value);
    void LinePowerSleepDelayChanged(const qint32 value);
    void LowPowerAutoSleepThresholdChanged(const qint32 value);
    void LowPowerNotifyEnableChanged(const bool value);
    void LowPowerNotifyThresholdChanged(const qint32 value);
    void ScreenBlackLockChanged(const bool value);
    void SleepLockChanged(const bool value);

public slots:

    QDBusPendingReply<> reset() { return m_inter->asyncCall("Reset"); }

private:
    QScopedPointer<DDBusInterface> m_inter;
};

DPOWER_END_NAMESPACE
