// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dpowersettings.h"

#include "dbus/daemonpowerinterface.h"
#include "dbus/systempowerinterface.h"
#include "dpowersettings_p.h"
#include "dpowertypes.h"

#include <qdebug.h>
#include <qsharedpointer.h>

DPOWER_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DError;
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DUnexpected;

void DPowerSettingsPrivate::connectDBusSignal()
{
    Q_Q(DPowerSettings);
    connect(m_systemPowerInter,
            &SystemPowerInterface::CpuGovernorChanged,
            q,
            &DPowerSettings::cpuGovernorChanged);
    connect(m_systemPowerInter,
            &SystemPowerInterface::PowerSavingModeAutoChanged,
            q,
            &DPowerSettings::autoPowerSavingOnBatteryChanged);
    connect(m_systemPowerInter,
            &SystemPowerInterface::PowerSavingModeAutoWhenBatteryLowChanged,
            q,
            &DPowerSettings::autoPowerSavingWhenBatteryLowChanged);
    connect(m_systemPowerInter,
            &SystemPowerInterface::PowerSavingModeBrightnessDropPercentChanged,
            q,
            &DPowerSettings::powerSavingBrightnessDropPercentChanged);
    connect(m_systemPowerInter,
            &SystemPowerInterface::PowerSavingModeEnabledChanged,
            q,
            &DPowerSettings::powerSavingModeEnabledChanged);
    connect(m_systemPowerInter,
            &SystemPowerInterface::PowerSavingModeBrightnessDataChanged,
            q,
            &DPowerSettings::powerSavingModeBrightnessDataChanged);
    connect(m_systemPowerInter,
            &SystemPowerInterface::CpuBoostChanged,
            q,
            &DPowerSettings::cpuBoostChanged);
    connect(m_systemPowerInter, &SystemPowerInterface::ModeChanged, q, [q](const QString &value) {
        if (value == "powsersave")
            emit q->powerModeChanged(PowerMode::PowerSave);
        else if (value == "performance")
            emit q->powerModeChanged(PowerMode::Performance);
        else if (value == "balance")
            emit q->powerModeChanged(PowerMode::Balance);
        else
            emit q->powerModeChanged(PowerMode::Unknown);
    });
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::BatteryLidClosedActionChanged,
            q,
            [q](const qint32 value) {
                if (value < 1 || value > 4)
                    return;
                emit q->batteryLidClosedActionChanged(static_cast<LidClosedAction>(value));
            });
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::BatteryLockDelayChanged,
            q,
            &DPowerSettings::batteryLockDelayChanged);
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::BatteryPressPowerBtnActionChanged,
            q,
            [q](const qint32 value) {
                if (value < 0 || value > 4)
                    return;
                emit q->batteryPressPowerBtnActionChanged(static_cast<PowerBtnAction>(value));
            });
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::BatteryScreenBlackDelayChanged,
            q,
            &DPowerSettings::batteryScreenBlackDelayChanged);
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::BatteryScreensaverDelayChanged,
            q,
            &DPowerSettings::batteryScreensaverDelayChanged);
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::BatterySleepDelayChanged,
            q,
            &DPowerSettings::batterySleepDelayChanged);
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::LinePowerLidClosedActionChanged,
            q,
            [q](const qint32 value) {
                if (value < 1 || value > 4)
                    return;
                emit q->linePowerLidClosedActionChanged(static_cast<LidClosedAction>(value));
            });
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::LinePowerLockDelayChanged,
            q,
            &DPowerSettings::linePowerLockDelayChanged);
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::LinePowerPressPowerBtnActionChanged,
            q,
            [q](const qint32 value) {
                if (value < 0 || value > 4)
                    return;
                emit q->linePowerPressPowerBtnActionChanged(static_cast<PowerBtnAction>(value));
            });
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::LinePowerScreenBlackDelayChanged,
            q,
            &DPowerSettings::linePowerScreenBlackDelayChanged);
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::LinePowerScreensaverDelayChanged,
            q,
            &DPowerSettings::linePowerScreensaverDelayChanged);
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::LinePowerSleepDelayChanged,
            q,
            &DPowerSettings::linePowerSleepDelayChanged);
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::LowPowerAutoSleepThresholdChanged,
            q,
            &DPowerSettings::lowPowerAutoSleepThresholdChanged);
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::LowPowerNotifyEnableChanged,
            q,
            &DPowerSettings::lowPowerNotifyEnableChanged);
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::LowPowerNotifyThresholdChanged,
            q,
            &DPowerSettings::lowPowerNotifyThresholdChanged);
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::ScreenBlackLockChanged,
            q,
            &DPowerSettings::screenBlackLockChanged);
    connect(m_daemonPowerInter,
            &DaemonPowerInterface::SleepLockChanged,
            q,
            &DPowerSettings::sleepLockChanged);
}

DPowerSettings::DPowerSettings(QObject *parent)
    : QObject(parent)
    , d_ptr(new DPowerSettingsPrivate(this))
{
    Q_D(DPowerSettings);
    d->m_systemPowerInter = new SystemPowerInterface(this);
    d->m_daemonPowerInter = new DaemonPowerInterface(this);
    d->connectDBusSignal();
}

DPowerSettings::~DPowerSettings() { }

QString DPowerSettings::cpuGovernor() const
{
    Q_D(const DPowerSettings);
    return d->m_systemPowerInter->cpuGovernor();
}

PowerMode DPowerSettings::powerMode() const
{
    Q_D(const DPowerSettings);
    QMap<QString, PowerMode> modeMap;
    modeMap["performance"] = PowerMode::Performance;
    modeMap["balance"] = PowerMode::Balance;
    modeMap["powersave"] = PowerMode::PowerSave;
    auto mode = d->m_systemPowerInter->mode();
    PowerMode realMode;
    if (modeMap.contains(mode))
        realMode = modeMap[mode];
    else
        realMode = PowerMode::Unknown;

    return realMode;
}

void DPowerSettings::setPowerMode(const PowerMode &mode)
{
    Q_D(DPowerSettings);
    if (mode == PowerMode::Unknown)
        return;
    QMap<PowerMode, QString> modeMap;
    modeMap[PowerMode::Performance] = "performance";
    modeMap[PowerMode::Balance] = "balance";
    modeMap[PowerMode::PowerSave] = "powersave";
    d->m_systemPowerInter->setMode(modeMap[mode]);
}

bool DPowerSettings::autoPowerSavingOnBattery() const
{
    Q_D(const DPowerSettings);
    return d->m_systemPowerInter->powerSavingModeAuto();
}

void DPowerSettings::setAutoPowerSavingOnBattery(const bool enabled)
{
    Q_D(DPowerSettings);
    d->m_systemPowerInter->setPowerSavingModeAuto(enabled);
}

bool DPowerSettings::autoPowerSavingWhenBatteryLow() const
{
    Q_D(const DPowerSettings);
    return d->m_systemPowerInter->powerSavingModeAutoWhenBatteryLow();
}

void DPowerSettings::setAutoPowerSavingWhenBatteryLow(const bool enabled)
{
    Q_D(DPowerSettings);
    d->m_systemPowerInter->setPowerSavingModeAutoWhenBatteryLow(enabled);
}

quint32 DPowerSettings::powerSavingBrightnessDropPercent() const
{
    Q_D(const DPowerSettings);
    return d->m_systemPowerInter->powerSavingModeBrightnessDropPercent();
}

void DPowerSettings::setPowerSavingBrightnessDropPercent(const quint32 value)
{
    Q_D(DPowerSettings);
    d->m_systemPowerInter->setPowerSavingModeBrightnessDropPercent(value);
}

LidClosedAction DPowerSettings::batteryLidClosedAction() const
{
    Q_D(const DPowerSettings);
    auto action = d->m_daemonPowerInter->batteryLidClosedAction();
    if (action < 1 || action > 4)
        return LidClosedAction::Unknown;
    return static_cast<LidClosedAction>(action);
}

void DPowerSettings::setBatteryLidClosedAction(const LidClosedAction &value)
{
    Q_D(DPowerSettings);
    if (value == LidClosedAction::Unknown)
        return;

    d->m_daemonPowerInter->setBatteryLidClosedAction(static_cast<qint32>(value));
}

qint32 DPowerSettings::batteryLockDelay() const
{
    Q_D(const DPowerSettings);
    return d->m_daemonPowerInter->batteryLockDelay();
}

void DPowerSettings::setBatteryLockDelay(const qint32 value)
{
    Q_D(DPowerSettings);
    d->m_daemonPowerInter->setBatteryLockDelay(value);
}

PowerBtnAction DPowerSettings::batteryPressPowerBtnAction() const
{
    Q_D(const DPowerSettings);
    auto action = d->m_daemonPowerInter->batteryPressPowerBtnAction();
    if (action < 0 || action > 4)
        return PowerBtnAction::Unknown;
    return static_cast<PowerBtnAction>(action);
}

void DPowerSettings::setBatteryPressPowerBtnAction(const PowerBtnAction &value)
{
    Q_D(DPowerSettings);
    if (value == PowerBtnAction::Unknown)
        return;
    d->m_daemonPowerInter->setBatteryPressPowerBtnAction(static_cast<qint32>(value));
}

qint32 DPowerSettings::batteryScreenBlackDelay() const
{
    Q_D(const DPowerSettings);
    return d->m_daemonPowerInter->batteryScreenBlackDelay();
}

void DPowerSettings::setBatteryScreenBlackDelay(const qint32 value)
{
    Q_D(DPowerSettings);
    d->m_daemonPowerInter->setBatteryScreenBlackDelay(value);
}

qint32 DPowerSettings::batteryScreensaverDelay() const
{
    Q_D(const DPowerSettings);
    return d->m_daemonPowerInter->batteryScreensaverDelay();
}

void DPowerSettings::setBatteryScreensaverDelay(const qint32 value)
{
    Q_D(DPowerSettings);
    d->m_daemonPowerInter->setBatteryScreensaverDelay(value);
}

qint32 DPowerSettings::batterySleepDelay() const
{
    Q_D(const DPowerSettings);
    return d->m_daemonPowerInter->batterySleepDelay();
}

void DPowerSettings::setBatterySleepDelay(const qint32 value)
{
    Q_D(DPowerSettings);
    d->m_daemonPowerInter->setBatterySleepDelay(value);
}

LidClosedAction DPowerSettings::linePowerLidClosedAction() const
{
    Q_D(const DPowerSettings);
    auto action = d->m_daemonPowerInter->linePowerLidClosedAction();
    if (action < 1 || action > 4)
        return LidClosedAction::Unknown;
    return static_cast<LidClosedAction>(action);
}

void DPowerSettings::setLinePowerLidClosedAction(const LidClosedAction &value)
{
    Q_D(DPowerSettings);
    if (value == LidClosedAction::Unknown)
        return;
    d->m_daemonPowerInter->setLinePowerLidClosedAction(static_cast<qint32>(value));
}

qint32 DPowerSettings::linePowerLockDelay() const
{
    Q_D(const DPowerSettings);
    return d->m_daemonPowerInter->linePowerLockDelay();
}

void DPowerSettings::setLinePowerLockDelay(const qint32 value)
{
    Q_D(DPowerSettings);
    d->m_daemonPowerInter->setLinePowerLockDelay(value);
}

PowerBtnAction DPowerSettings::linePowerPressPowerBtnAction() const
{
    Q_D(const DPowerSettings);
    auto action = d->m_daemonPowerInter->linePowerPressPowerBtnAction();
    if (action < 0 || action > 4)
        return PowerBtnAction::Unknown;
    return static_cast<PowerBtnAction>(action);
}

void DPowerSettings::setLinePowerPressPowerBtnAction(const PowerBtnAction &value)
{
    Q_D(DPowerSettings);
    if (value == PowerBtnAction::Unknown)
        return;
    d->m_daemonPowerInter->setLinePowerPressPowerBtnAction(static_cast<qint32>(value));
}

qint32 DPowerSettings::linePowerScreenBlackDelay() const
{
    Q_D(const DPowerSettings);
    return d->m_daemonPowerInter->linePowerScreenBlackDelay();
}

void DPowerSettings::setLinePowerScreenBlackDelay(const qint32 value)
{
    Q_D(DPowerSettings);
    d->m_daemonPowerInter->setLinePowerScreenBlackDelay(value);
}

qint32 DPowerSettings::linePowerScreensaverDelay() const
{
    Q_D(const DPowerSettings);
    return d->m_daemonPowerInter->linePowerScreensaverDelay();
}

void DPowerSettings::setLinePowerScreensaverDelay(const qint32 value)
{
    Q_D(DPowerSettings);
    d->m_daemonPowerInter->setLinePowerScreensaverDelay(value);
}

qint32 DPowerSettings::linePowerSleepDelay() const
{
    Q_D(const DPowerSettings);
    return d->m_daemonPowerInter->linePowerSleepDelay();
}

void DPowerSettings::setLinePowerSleepDelay(const qint32 value)
{
    Q_D(DPowerSettings);
    d->m_daemonPowerInter->setLinePowerSleepDelay(value);
}

qint32 DPowerSettings::lowPowerAutoSleepThreshold() const
{
    Q_D(const DPowerSettings);
    return d->m_daemonPowerInter->lowPowerAutoSleepThreshold();
}

void DPowerSettings::setLowPowerAutoSleepThreshold(const qint32 value)
{
    Q_D(DPowerSettings);
    d->m_daemonPowerInter->setLowPowerAutoSleepThreshold(value);
}

bool DPowerSettings::lowPowerNotifyEnable() const
{
    Q_D(const DPowerSettings);
    return d->m_daemonPowerInter->lowPowerNotifyEnable();
}

void DPowerSettings::setLowPowerNotifyEnable(const bool value)
{
    Q_D(const DPowerSettings);
    d->m_daemonPowerInter->setLowPowerNotifyEnable(value);
}

qint32 DPowerSettings::lowPowerNotifyThreshold() const
{
    Q_D(const DPowerSettings);
    return d->m_daemonPowerInter->lowPowerNotifyThreshold();
}

void DPowerSettings::setLowPowerNotifyThreshold(const qint32 value)
{
    Q_D(DPowerSettings);
    d->m_daemonPowerInter->setLowPowerNotifyThreshold(value);
}

bool DPowerSettings::screenBlackLock() const
{
    Q_D(const DPowerSettings);
    return d->m_daemonPowerInter->screenBlackLock();
}

void DPowerSettings::setScreenBlackLock(const bool value)
{
    Q_D(DPowerSettings);
    d->m_daemonPowerInter->setScreenBlackLock(value);
}

bool DPowerSettings::sleepLock() const
{
    Q_D(const DPowerSettings);
    return d->m_daemonPowerInter->sleepLock();
}

void DPowerSettings::setSleepLock(const bool value)
{
    Q_D(DPowerSettings);
    d->m_daemonPowerInter->setSleepLock(value);
}

DExpected<void> DPowerSettings::reset()
{
    Q_D(DPowerSettings);
    auto reply = d->m_daemonPowerInter->reset();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DPOWER_END_NAMESPACE
