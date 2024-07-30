/*
 *  SPDX-FileCopyrightText: 2008-2010 Dario Freddi <drf@kde.org>
 *  SPDX-FileCopyrightText: 2023 Jakob Petsovits <jpetso@petsovits.com>
 *  SPDX-FileCopyrightText: 2024 Fabian Arndt <fabian.arndt@root-core.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ExternalServiceSettings.h"

// KDE
#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KLocalizedString>
#include <Solid/Battery>
#include <Solid/Device>

// Qt
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusServiceWatcher>
#include <QPointer>

// debug category for qCInfo()
#include <powerdevil_debug.h>

using namespace Qt::StringLiterals;

namespace
{
constexpr int ChargeThresholdUnsupported = -1;
}

namespace PowerDevil
{

ExternalServiceSettings::ExternalServiceSettings(QObject *parent)
    : QObject(parent)
    , m_chargeStartThreshold(ChargeThresholdUnsupported)
    , m_chargeStopThreshold(ChargeThresholdUnsupported)
    , m_savedChargeStartThreshold(ChargeThresholdUnsupported)
    , m_savedChargeStopThreshold(ChargeThresholdUnsupported)
    , m_chargeStopThresholdMightNeedReconnect(false)
    , m_isBatteryConservationModeSupported(false)
    , m_batteryConservationMode(false)
    , m_savedBatteryConservationMode(false)
{
}

void ExternalServiceSettings::executeChargeThresholdHelperAction(const QString &actionName,
                                                                 QWindow *parentWindowForKAuth,
                                                                 const QVariantMap &arguments,
                                                                 const std::function<void(KAuth::ExecuteJob *job)> callback)
{
    KAuth::Action action(QStringLiteral("org.kde.powerdevil.chargethresholdhelper.%1").arg(actionName));
    action.setHelperId(QStringLiteral("org.kde.powerdevil.chargethresholdhelper"));
    action.setParentWindow(parentWindowForKAuth);
    action.setArguments(arguments);

    KAuth::ExecuteJob *job = action.execute();
    QPointer thisAlive(this);
    QPointer jobAlive(job);
    job->exec();

    if (!thisAlive || !jobAlive) {
        qCInfo(POWERDEVIL) << action.name() << "failed: was deleted during job execution";
        return;
    }

    if (job->error()) {
        qCInfo(POWERDEVIL) << "KAuth action" << action.name() << "failed:" << job->errorText();
    }
    callback(job);
}

void ExternalServiceSettings::load(QWindow *parentWindowForKAuth)
{
    // Battery thresholds (start / stop)
    executeChargeThresholdHelperAction(u"getthreshold"_s, parentWindowForKAuth, {}, [&](KAuth::ExecuteJob *job) {
        if (job->error()) {
            setSavedChargeStartThreshold(ChargeThresholdUnsupported);
            setSavedChargeStopThreshold(ChargeThresholdUnsupported);
            return;
        }

        const auto data = job->data();
        setSavedChargeStartThreshold(data.value(QStringLiteral("chargeStartThreshold")).toInt());
        setSavedChargeStopThreshold(data.value(QStringLiteral("chargeStopThreshold")).toInt());
        setChargeStopThreshold(m_savedChargeStopThreshold);
        setChargeStartThreshold(m_savedChargeStartThreshold);
    });

    // Battery Conservation Mode (fixed)
    executeChargeThresholdHelperAction(u"getconservationmode"_s, parentWindowForKAuth, {}, [&](KAuth::ExecuteJob *job) {
        if (job->error()) {
            setBatteryConservationModeSupported(false);
            setSavedBatteryConservationMode(false);
            return;
        }

        const auto data = job->data();
        setSavedBatteryConservationMode(data.value(QStringLiteral("batteryConservationModeEnabled")).toBool());
        setBatteryConservationMode(m_savedBatteryConservationMode);
        setBatteryConservationModeSupported(true);
    });
}

void ExternalServiceSettings::save(QWindow *parentWindowForKAuth)
{
    // Battery threshold (start / stop)
    if ((isChargeStartThresholdSupported() && m_chargeStartThreshold != m_savedChargeStartThreshold)
        || (isChargeStopThresholdSupported() && m_chargeStopThreshold != m_savedChargeStopThreshold)) {
        int newChargeStartThreshold = isChargeStartThresholdSupported() ? m_chargeStartThreshold : ChargeThresholdUnsupported;
        int newChargeStopThreshold = isChargeStopThresholdSupported() ? m_chargeStopThreshold : ChargeThresholdUnsupported;

        executeChargeThresholdHelperAction(u"setthreshold"_s,
                                           parentWindowForKAuth,
                                           {
                                               {QStringLiteral("chargeStartThreshold"), newChargeStartThreshold},
                                               {QStringLiteral("chargeStopThreshold"), newChargeStopThreshold},
                                           },
                                           [&](KAuth::ExecuteJob *job) {
                                               if (job->error()) {
                                                   setChargeStopThreshold(m_savedChargeStopThreshold);
                                                   setChargeStartThreshold(m_savedChargeStartThreshold);
                                                   return;
                                               }

                                               setSavedChargeStartThreshold(newChargeStartThreshold);
                                               setSavedChargeStopThreshold(newChargeStopThreshold);
                                           });
    }

    // Battery Conservation Mode (fixed)
    if (isBatteryConservationModeSupported() && m_batteryConservationMode != m_savedBatteryConservationMode) {
        executeChargeThresholdHelperAction(u"setconservationmode"_s,
                                           parentWindowForKAuth,
                                           {
                                               {QStringLiteral("batteryConservationModeEnabled"), m_batteryConservationMode},
                                           },
                                           [&](KAuth::ExecuteJob *job) {
                                               if (job->error()) {
                                                   setBatteryConservationMode(m_savedBatteryConservationMode);
                                                   return;
                                               }

                                               setSavedBatteryConservationMode(m_batteryConservationMode);
                                           });
    }
}

bool ExternalServiceSettings::isSaveNeeded() const
{
    return (isChargeStartThresholdSupported() && m_chargeStartThreshold != m_savedChargeStartThreshold)
        || (isChargeStopThresholdSupported() && m_chargeStopThreshold != m_savedChargeStopThreshold)
        || (isBatteryConservationModeSupported() && m_batteryConservationMode != m_savedBatteryConservationMode);
}

void ExternalServiceSettings::setBatteryConservationModeSupported(bool supported)
{
    if (m_isBatteryConservationModeSupported != supported) {
        m_isBatteryConservationModeSupported = supported;
        Q_EMIT isBatteryConservationModeSupportedChanged();
    }
}

bool ExternalServiceSettings::isBatteryConservationModeSupported() const
{
    return m_isBatteryConservationModeSupported;
}

void ExternalServiceSettings::setSavedBatteryConservationMode(bool enabled)
{
    m_savedBatteryConservationMode = enabled;
}

bool ExternalServiceSettings::isChargeStartThresholdSupported() const
{
    return m_savedChargeStartThreshold != ChargeThresholdUnsupported;
}

bool ExternalServiceSettings::isChargeStopThresholdSupported() const
{
    return m_savedChargeStopThreshold != ChargeThresholdUnsupported;
}

void ExternalServiceSettings::setSavedChargeStartThreshold(int threshold)
{
    bool wasChargeStartThresholdSupported = isChargeStartThresholdSupported();
    m_savedChargeStartThreshold = threshold;
    if (wasChargeStartThresholdSupported != isChargeStartThresholdSupported()) {
        Q_EMIT isChargeStartThresholdSupportedChanged();
    }
}

void ExternalServiceSettings::setSavedChargeStopThreshold(int threshold)
{
    bool wasChargeStopThresholdSupported = isChargeStopThresholdSupported();
    m_savedChargeStopThreshold = threshold;
    if (wasChargeStopThresholdSupported != isChargeStopThresholdSupported()) {
        Q_EMIT isChargeStopThresholdSupportedChanged();
    }
}

bool ExternalServiceSettings::batteryConservationMode() const
{
    return m_batteryConservationMode;
}

void ExternalServiceSettings::setBatteryConservationMode(bool enabled)
{
    if (enabled == m_batteryConservationMode) {
        return;
    }
    m_batteryConservationMode = enabled;
    Q_EMIT batteryConservationModeChanged();
    Q_EMIT settingsChanged();
}

int ExternalServiceSettings::chargeStartThreshold() const
{
    return m_chargeStartThreshold;
}

int ExternalServiceSettings::chargeStopThreshold() const
{
    return m_chargeStopThreshold;
}

void ExternalServiceSettings::setChargeStartThreshold(int threshold)
{
    if (threshold == m_chargeStartThreshold) {
        return;
    }
    m_chargeStartThreshold = threshold;
    Q_EMIT chargeStartThresholdChanged();
    Q_EMIT settingsChanged();
}

void ExternalServiceSettings::setChargeStopThreshold(int threshold)
{
    if (threshold == m_chargeStopThreshold) {
        return;
    }
    m_chargeStopThreshold = threshold;
    Q_EMIT chargeStopThresholdChanged();

    if (m_chargeStopThreshold > m_savedChargeStopThreshold) {
        // Only show message if there is actually a charging or fully charged battery
        const auto devices = Solid::Device::listFromType(Solid::DeviceInterface::Battery, QString());
        for (const Solid::Device &device : devices) {
            const Solid::Battery *b = qobject_cast<const Solid::Battery *>(device.asDeviceInterface(Solid::DeviceInterface::Battery));
            if (b->chargeState() == Solid::Battery::Charging || b->chargeState() == Solid::Battery::FullyCharged) {
                setChargeStopThresholdMightNeedReconnect(true);
                break;
            }
        }
    } else {
        setChargeStopThresholdMightNeedReconnect(false);
    }

    Q_EMIT settingsChanged();
}

bool ExternalServiceSettings::chargeStopThresholdMightNeedReconnect() const
{
    return m_chargeStopThresholdMightNeedReconnect;
}

void ExternalServiceSettings::setChargeStopThresholdMightNeedReconnect(bool mightNeedReconnect)
{
    if (mightNeedReconnect == m_chargeStopThresholdMightNeedReconnect) {
        return;
    }
    m_chargeStopThresholdMightNeedReconnect = mightNeedReconnect;
    Q_EMIT chargeStopThresholdMightNeedReconnectChanged();
}

} // namespace PowerDevil

#include "moc_ExternalServiceSettings.cpp"
