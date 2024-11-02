/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "devicestate.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

namespace {

const char *upower_service = "org.freedesktop.UPower";
const char *upower_path = "/org/freedesktop/UPower";
const char *upower_path_properties_interface = "org.freedesktop.DBus.Properties";
const char *properties_changed_signal = "PropertiesChanged";
const char *on_battery_property = "OnBattery";

const char *schema_id = "org.lingmo.power-manager";
const char *ac_key = "powerPolicyAc";
const char *battery_key = "powerPolicyBattery";

const char *status_manager_service = "com.lingmo.statusmanager.interface";
const char *status_manager_path = "/";
const char *status_manager_interface = "com.lingmo.statusmanager.interface";

}

PowerManager::PowerManager()
{
    initGSettings();
    initDbusConnection();
    initPowerMode();
}

sched_policy::PowerMode PowerManager::currentPowerMode() const
{
    return m_currentPowerMode;
}

sched_policy::PowerType PowerManager::currentPowerType() const
{
    return m_currentPowerType;
}

void PowerManager::onPropertiesChanged(QString name, QMap<QString, QVariant> value, QStringList)
{
    if (value.contains(on_battery_property)) {
        if (value.value(on_battery_property).toBool()) {
            m_currentPowerType = sched_policy::PowerType::Battery;
            handlePowerModeChanged(m_gsettings->get(battery_key).toInt());
        } else {
            m_currentPowerType = sched_policy::PowerType::Ac;
            handlePowerModeChanged(m_gsettings->get(ac_key).toInt());
        }

        Q_EMIT powerTypeChanged(m_currentPowerType);
    }
}

void PowerManager::initGSettings()
{
    if (!QGSettings::isSchemaInstalled(schema_id)) {
        qWarning() << schema_id << "not installed.";
        return;
    }
    m_gsettings.reset(new QGSettings(schema_id));
    connect(m_gsettings.get(), &QGSettings::changed, this, [this](const QString &key) {
        if ((key == ac_key && m_currentPowerType == sched_policy::PowerType::Ac) ||
            (key == battery_key && m_currentPowerType == sched_policy::PowerType::Battery)) {
            handlePowerModeChanged(m_gsettings->get(key).toInt());
        }
    });
}

void PowerManager::initDbusConnection()
{
    QDBusConnection::systemBus().connect(
        upower_service, upower_path,
        upower_path_properties_interface,
        properties_changed_signal,
        this, SLOT(onPropertiesChanged(QString,QMap<QString,QVariant>,QStringList)));
}

void PowerManager::initPowerMode()
{
    QDBusInterface upowerInterface(
        upower_service, upower_path,
        upower_path_properties_interface, QDBusConnection::systemBus());

    QDBusReply<QDBusVariant> reply = upowerInterface.call("Get", upower_service, on_battery_property);
    if (!reply.isValid()) {
        qWarning() << "Get pwoer mode failed, " << reply.error();
        return;
    }

    // OnBattery
    if (reply.value().variant().toBool()) {
        qDebug() << "init pwoer mode: onBattery";
        m_currentPowerType = sched_policy::PowerType::Battery;
        if (m_gsettings) {
            handlePowerModeChanged(m_gsettings->get(battery_key).toInt());
        }
    } else {
        qDebug() << "init pwoer mode: Ac";
        m_currentPowerType = sched_policy::PowerType::Ac;
        if (m_gsettings) {
            handlePowerModeChanged(m_gsettings->get(ac_key).toInt());
        }
    }
}

void PowerManager::handlePowerModeChanged(int value)
{
    switch (value) {
    case 0:
        m_currentPowerMode = sched_policy::PowerMode::Performance;
        break;

    case 1:
        m_currentPowerMode = sched_policy::PowerMode::Balance;
        break;

    case 2:
        m_currentPowerMode = sched_policy::PowerMode::Save;
        break;

    default:
        m_currentPowerMode = sched_policy::PowerMode::Unknown;
        qWarning() << QString("Power mode changed to unkwon mode with the value: %1.").arg(value);
        return;
    }

    qDebug() << "power mode changed " << value;
    Q_EMIT powerModeChanged(m_currentPowerMode);
}

DeviceModeManager::DeviceModeManager(bool softFreezeEnabled)
    : m_softFreezeModeEnabled(softFreezeEnabled)
{
    resetDeviceMode();
    initConnections();
}

sched_policy::DeviceMode DeviceModeManager::currentDeviceMode() const
{
    return m_currentDeviceMode;
}

void DeviceModeManager::handleSoftFreezeModeEnabledChanged(bool enabled)
{
    m_softFreezeModeEnabled = enabled;
    updateDeviceMode();
}

void DeviceModeManager::resetDeviceMode()
{
    if (isTabletMode()) {
        m_currentDeviceMode = sched_policy::DeviceMode::Tablet;
    } else if (m_softFreezeModeEnabled) {
        m_currentDeviceMode = sched_policy::DeviceMode::SoftFreeze;
    } else {
        m_currentDeviceMode = sched_policy::DeviceMode::PC;
    }
}

void DeviceModeManager::initConnections()
{
    QDBusConnection::sessionBus().connect(
        status_manager_service, status_manager_path,
        status_manager_interface, "modeChangeSignal",
        this, SLOT(updateDeviceMode()));
}

void DeviceModeManager::updateDeviceMode()
{
    sched_policy::DeviceMode oldMode = m_currentDeviceMode;

    resetDeviceMode();

    if (m_currentDeviceMode != oldMode)
        Q_EMIT deviceModeChanged(m_currentDeviceMode);
}

bool DeviceModeManager::isTabletMode()
{
    QDBusInterface dbusInterface(
        status_manager_service, status_manager_path,
        status_manager_interface, QDBusConnection::sessionBus());
    if (!dbusInterface.isValid()) {
        qWarning() << dbusInterface.lastError().message();
        return false;
    }

    QDBusReply<bool> reply = dbusInterface.call("get_current_tabletmode");
    if (!reply.isValid()) {
        qWarning() << reply.error().message();
        return false;
    }

    return reply.value();
}
