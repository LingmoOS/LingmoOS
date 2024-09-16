// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <qdebug.h>
#include <qobject.h>
#include <qscopedpointer.h>

class DaemonPowerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakePower")
public:
    explicit DaemonPowerService(QObject *parent = nullptr);
    virtual ~DaemonPowerService();

    Q_PROPERTY(qint32 BatteryLidClosedAction READ batteryLidClosedAction WRITE setBatteryLidClosedAction);
    Q_PROPERTY(qint32 BatteryLockDelay READ batteryLockDelay WRITE setBatteryLockDelay);
    Q_PROPERTY(qint32 BatteryPressPowerBtnAction READ batteryPressPowerBtnAction WRITE setBatteryPressPowerBtnAction);
    Q_PROPERTY(qint32 BatteryScreenBlackDelay READ batteryScreenBlackDelay WRITE setBatteryScreenBlackDelay);
    Q_PROPERTY(qint32 BatteryScreensaverDelay READ batteryScreensaverDelay WRITE setBatteryScreensaverDelay);
    Q_PROPERTY(qint32 BatterySleepDelay READ batterySleepDelay WRITE setBatterySleepDelay);
    Q_PROPERTY(qint32 LinePowerLidClosedAction READ linePowerLidClosedAction WRITE setLinePowerLidClosedAction);
    Q_PROPERTY(qint32 LinePowerLockDelay READ linePowerLockDelay WRITE setLinePowerLockDelay);
    Q_PROPERTY(qint32 LinePowerPressPowerBtnAction READ linePowerPressPowerBtnAction WRITE setLinePowerPressPowerBtnAction);
    Q_PROPERTY(qint32 LinePowerScreenBlackDelay READ linePowerScreenBlackDelay WRITE setLinePowerScreenBlackDelay);
    Q_PROPERTY(qint32 LinePowerScreensaverDelay READ linePowerScreensaverDelay WRITE setLinePowerScreensaverDelay);
    Q_PROPERTY(qint32 LinePowerSleepDelay READ linePowerSleepDelay WRITE setLinePowerSleepDelay);
    Q_PROPERTY(qint32 LowPowerAutoSleepThreshold READ lowPowerAutoSleepThreshold WRITE setLowPowerAutoSleepThreshold);
    Q_PROPERTY(bool LowPowerNotifyEnable READ lowPowerNotifyEnable WRITE setLowPowerNotifyEnable);
    Q_PROPERTY(qint32 LowPowerNotifyThreshold READ lowPowerNotifyThreshold WRITE setLowPowerNotifyThreshold);
    Q_PROPERTY(bool ScreenBlackLock READ screenBlackLock WRITE setScreenBlackLock);
    Q_PROPERTY(bool SleepLock READ sleepLock WRITE setSleepLock);

    inline qint32 batteryLidClosedAction() const { return m_batteryLidClosedAction; }

    inline void setBatteryLidClosedAction(const qint32 value) { m_batteryLidClosedAction = value; }

    inline qint32 batteryLockDelay() const { return m_batteryLockDelay; }

    inline void setBatteryLockDelay(const qint32 value) { m_batteryLockDelay = value; }

    inline qint32 batteryPressPowerBtnAction() const { return m_batteryPressPowerBtnAction; }

    inline void setBatteryPressPowerBtnAction(const qint32 value)
    {
        m_batteryPressPowerBtnAction = value;
    }

    inline qint32 batteryScreenBlackDelay() const { return m_batteryScreenBlackDelay; }

    inline void setBatteryScreenBlackDelay(const qint32 value)
    {
        m_batteryScreenBlackDelay = value;
    }

    inline qint32 batteryScreensaverDelay() const { return m_batteryScreensaverDelay; }

    inline void setBatteryScreensaverDelay(const qint32 value)
    {
        m_batteryScreensaverDelay = value;
    }

    inline qint32 batterySleepDelay() const { return m_batterySleepDelay; }

    inline void setBatterySleepDelay(const qint32 value) { m_batterySleepDelay = value; }

    inline qint32 linePowerLidClosedAction() const { return m_linePowerLidClosedAction; }

    inline void setLinePowerLidClosedAction(const qint32 value)
    {
        m_linePowerLidClosedAction = value;
    }

    inline qint32 linePowerLockDelay() const { return m_linePowerLockDelay; }

    inline void setLinePowerLockDelay(const qint32 value) { m_linePowerLockDelay = value; }

    inline qint32 linePowerPressPowerBtnAction() const { return m_linePowerPressPowerBtnAction; }

    inline void setLinePowerPressPowerBtnAction(const qint32 value)
    {
        m_linePowerPressPowerBtnAction = value;
    }

    inline qint32 linePowerScreenBlackDelay() const { return m_linePowerScreenBlackDelay; }

    inline void setLinePowerScreenBlackDelay(const qint32 value)
    {
        m_linePowerScreenBlackDelay = value;
    }

    inline qint32 linePowerScreensaverDelay() const { return m_linePowerScreensaverDelay; }

    inline void setLinePowerScreensaverDelay(const qint32 value)
    {
        m_linePowerScreensaverDelay = value;
    }

    inline qint32 linePowerSleepDelay() const { return m_linePowerSleepDelay; }

    inline void setLinePowerSleepDelay(const qint32 value) { m_linePowerSleepDelay = value; }

    inline qint32 lowPowerAutoSleepThreshold() const { return m_lowPowerAutoSleepThreshold; }

    inline void setLowPowerAutoSleepThreshold(const qint32 value)
    {
        m_lowPowerAutoSleepThreshold = value;
    }

    inline bool lowPowerNotifyEnable() const { return m_lowPowerNotifyEnable; }

    inline void setLowPowerNotifyEnable(const bool value) { m_lowPowerNotifyEnable = value; }

    inline qint32 lowPowerNotifyThreshold() const { return m_lowPowerNotifyThreshold; }

    inline void setLowPowerNotifyThreshold(const qint32 value)
    {
        m_lowPowerNotifyThreshold = value;
    }

    inline bool screenBlackLock() const { return m_screenBlackLock; }

    inline void setScreenBlackLock(const bool value) { m_screenBlackLock = value; }

    inline bool sleepLock() const { return m_sleepLock; }

    inline void setSleepLock(const bool value) { m_sleepLock = value; }

signals:
    Q_SCRIPTABLE void PropertiesChanged(const QVariantMap &properties);

public slots:

    Q_SCRIPTABLE void Reset() { m_reset = true; }

public:
    bool m_reset;
    qint32 m_batteryLidClosedAction;
    qint32 m_batteryLockDelay;
    qint32 m_batteryPressPowerBtnAction;
    qint32 m_batteryScreenBlackDelay;
    qint32 m_batteryScreensaverDelay;
    qint32 m_batterySleepDelay;
    qint32 m_linePowerLidClosedAction;
    qint32 m_linePowerLockDelay;
    qint32 m_linePowerPressPowerBtnAction;
    qint32 m_linePowerScreenBlackDelay;
    qint32 m_linePowerScreensaverDelay;
    qint32 m_linePowerSleepDelay;
    qint32 m_lowPowerAutoSleepThreshold;
    bool m_lowPowerNotifyEnable;
    qint32 m_lowPowerNotifyThreshold;
    bool m_screenBlackLock;
    bool m_sleepLock;

private:
    void registerService();
    void unRegisterService();
};
