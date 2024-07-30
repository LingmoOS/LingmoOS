/*
    SPDX-FileCopyrightText: 2023 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "upowerdevice.h"
#include <powerdevil_debug.h>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>

inline constexpr QLatin1StringView UPOWER_SERVICE("org.freedesktop.UPower");
inline constexpr QLatin1StringView UPOWER_IFACE_DEVICE("org.freedesktop.UPower.Device");

UPowerDevice::UPowerDevice(const QString &dbusObjectPath)
{
    qCDebug(POWERDEVIL) << "Creating UPowerDevice for" << dbusObjectPath;
    QDBusConnection::systemBus().connect(UPOWER_SERVICE,
                                         dbusObjectPath,
                                         QStringLiteral("org.freedesktop.DBus.Properties"),
                                         QStringLiteral("PropertiesChanged"),
                                         this,
                                         SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

    auto call = QDBusMessage::createMethodCall(UPOWER_SERVICE, dbusObjectPath, QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("GetAll"));
    call << UPOWER_IFACE_DEVICE;

    QDBusPendingReply<QVariantMap> reply = QDBusConnection::systemBus().asyncCall(call);
    reply.waitForFinished();

    if (reply.isValid()) {
        updateProperties(reply.value());
    }
}

void UPowerDevice::onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps)
{
    Q_UNUSED(invalidatedProps);
    if (ifaceName != UPOWER_IFACE_DEVICE) {
        return;
    }

    if (updateProperties(changedProps)) {
        Q_EMIT propertiesChanged();
    }
}

bool UPowerDevice::updateProperties(const QVariantMap &properties)
{
    qCDebug(POWERDEVIL) << "Update:" << properties;
    // Set whan any properties affecting charging state are updated
    bool updated = false;

    for (auto it = properties.begin(); it != properties.end(); it++) {
        if (it.key() == u"UpdateTime") {
            m_updateTime = it.value().toULongLong();
        } else if (it.key() == u"Energy") {
            m_energy = it.value().toDouble();
            updated = true;
        } else if (it.key() == u"EnergyRate") {
            m_energyRate = it.value().toDouble();
            updated = true;
        } else if (it.key() == u"EnergyFull") {
            m_energyFull = it.value().toDouble();
            updated = true;
        } else if (it.key() == u"State") {
            auto state = it.value().toInt();
            m_state = (state == 1) ? State::Charging : (state == 2) ? State::Discharging : (state == 4) ? State::FullyCharged : State::Unknown;
            updated = true;
        } else if (it.key() == u"Type") {
            auto type = it.value().toInt();
            m_type = (type == 2) ? Type::Battery : (type == 3) ? Type::Ups : Type::Unknown;
            updated = true;
        } else if (it.key() == u"PowerSupply") {
            m_isPowerSupply = it.value().toBool();
            updated = true;
        } else if (it.key() == u"IsPresent") {
            m_isPresent = it.value().toBool();
            updated = true;
        }
    }

    return updated;
}

#include "moc_upowerdevice.cpp"
