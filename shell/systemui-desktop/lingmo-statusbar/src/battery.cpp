/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     lingmoos <lingmoos@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "battery.h"
#include <QSettings>
#include <QDBusPendingCall>

static const QString s_sServer = "com.lingmo.Settings";
static const QString s_sPath = "/PrimaryBattery";
static const QString s_sInterface = "com.lingmo.PrimaryBattery";

static Battery *SELF = nullptr;

Battery *Battery::self()
{
    if (!SELF)
        SELF = new Battery;

    return SELF;
}

Battery::Battery(QObject *parent)
    : QObject(parent)
    , m_upowerInterface("org.freedesktop.UPower",
                        "/org/freedesktop/UPower",
                        "org.freedesktop.UPower",
                        QDBusConnection::systemBus())
    , m_interface("com.lingmo.Settings",
                  "/PrimaryBattery",
                  "com.lingmo.PrimaryBattery",
                  QDBusConnection::sessionBus())
    , m_available(false)
    , m_onBattery(false)
    , m_showPercentage(false)
{
    m_available = m_interface.isValid() && !m_interface.lastError().isValid();

    if (m_available) {
        QSettings settings("lingmoos", "statusbar");
        settings.setDefaultFormat(QSettings::IniFormat);
        m_showPercentage = settings.value("BatteryPercentage", false).toBool();

        QDBusConnection::sessionBus().connect(s_sServer, s_sPath, s_sInterface, "chargeStateChanged", this, SLOT(chargeStateChanged(int)));
        QDBusConnection::sessionBus().connect(s_sServer, s_sPath, s_sInterface, "chargePercentChanged", this, SLOT(chargePercentChanged(int)));
        QDBusConnection::sessionBus().connect(s_sServer, s_sPath, s_sInterface, "lastChargedPercentChanged", this, SLOT(lastChargedPercentChanged()));
        QDBusConnection::sessionBus().connect(s_sServer, s_sPath, s_sInterface, "capacityChanged", this, SLOT(capacityChanged(int)));
        QDBusConnection::sessionBus().connect(s_sServer, s_sPath, s_sInterface, "remainingTimeChanged", this, SLOT(remainingTimeChanged(qlonglong)));

        // Update Icon
        QDBusConnection::sessionBus().connect(s_sServer, s_sPath, s_sInterface, "chargePercentChanged", this, SLOT(iconSourceChanged()));

        QDBusInterface interface("org.freedesktop.UPower", "/org/freedesktop/UPower",
                                 "org.freedesktop.UPower", QDBusConnection::systemBus());

        QDBusConnection::systemBus().connect("org.freedesktop.UPower", "/org/freedesktop/UPower",
                                             "org.freedesktop.DBus.Properties",
                                             "PropertiesChanged", this,
                                             SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

        if (interface.isValid()) {
            m_onBattery = interface.property("OnBattery").toBool();
        }

        emit validChanged();
    }
}

bool Battery::available() const
{
    return m_available;
}

bool Battery::onBattery() const
{
    return m_onBattery;
}

bool Battery::showPercentage() const
{
    return m_showPercentage;
}

void Battery::setShowPercentage(bool enabled)
{
    if (enabled == m_showPercentage)
        return;

    m_showPercentage = enabled;

    QSettings settings("lingmoos", "statusbar");
    settings.setDefaultFormat(QSettings::IniFormat);
    settings.setValue("BatteryPercentage", m_showPercentage);

    emit showPercentageChanged();
}

int Battery::chargeState() const
{
    return m_interface.property("chargeState").toInt();
}

int Battery::chargePercent() const
{
    return m_interface.property("chargePercent").toInt();
}

int Battery::lastChargedPercent() const
{
    return m_interface.property("lastChargedPercent").toInt();
}

int Battery::capacity() const
{
    return m_interface.property("capacity").toInt();
}

QString Battery::statusString() const
{
    return m_interface.property("statusString").toString();
}

QString Battery::iconSource() const
{
    int percent = this->chargePercent();
    int range = 0;

    if (percent >= 95)
        range = 100;
    else if (percent >= 85)
        range = 90;
    else if (percent>= 75)
        range = 80;
    else if (percent >= 65)
        range = 70;
    else if (percent >= 55)
        range = 60;
    else if (percent >= 45)
        range = 50;
    else if (percent >= 35)
        range = 40;
    else if (percent >= 25)
        range = 30;
    else if (percent >= 15)
        range = 20;
    else if (percent >= 5)
        range = 10;
    else
        range = 0;

    if (m_onBattery)
        return QString("battery-level-%1-symbolic.svg").arg(range);

    return QString("battery-level-%1-charging-symbolic.svg").arg(range);
}

void Battery::onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps)
{
    Q_UNUSED(ifaceName);
    Q_UNUSED(changedProps);
    Q_UNUSED(invalidatedProps);

    bool onBattery = m_upowerInterface.property("OnBattery").toBool();
    if (onBattery != m_onBattery) {
        m_onBattery = onBattery;
        m_interface.call("refresh");
        emit onBatteryChanged();
        emit iconSourceChanged();
    }
}
