/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2008-2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2010 Alejandro Fiestas <alex@eyeos.org>
    SPDX-FileCopyrightText: 2010-2013 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-only

*/

#include "lidcontroller.h"

using namespace Qt::StringLiterals;

inline constexpr QLatin1StringView UPOWER_SERVICE("org.freedesktop.UPower");
inline constexpr QLatin1StringView UPOWER_PATH("/org/freedesktop/UPower");
inline constexpr QLatin1StringView UPOWER_IFACE("org.freedesktop.UPower");

LidController::LidController()
    : QObject()
{
    if (!QDBusConnection::systemBus().interface()->isServiceRegistered(UPOWER_SERVICE)) {
        // Activate it.
        QDBusConnection::systemBus().interface()->startService(UPOWER_SERVICE);
    }

    m_upowerInterface = new OrgFreedesktopUPowerInterface(UPOWER_SERVICE, u"/org/freedesktop/UPower"_s, QDBusConnection::systemBus(), this);

    QDBusConnection::systemBus().connect(UPOWER_SERVICE,
                                         UPOWER_PATH,
                                         u"org.freedesktop.DBus.Properties"_s,
                                         u"PropertiesChanged"_s,
                                         this,
                                         SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

    m_isLidPresent = m_upowerInterface->lidIsPresent();
    m_isLidClosed = m_upowerInterface->lidIsClosed();
    Q_EMIT lidClosedChanged(m_isLidClosed);
}

void LidController::onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps)
{
    if (ifaceName != UPOWER_IFACE) {
        return;
    }

    if (m_isLidPresent) {
        bool lidIsClosed = m_isLidClosed;
        if (changedProps.contains(QStringLiteral("LidIsClosed"))) {
            lidIsClosed = changedProps[QStringLiteral("LidIsClosed")].toBool();
        } else if (invalidatedProps.contains(QStringLiteral("LidIsClosed"))) {
            lidIsClosed = m_upowerInterface->lidIsClosed();
        }
        if (lidIsClosed != m_isLidClosed) {
            m_isLidClosed = lidIsClosed;
            Q_EMIT lidClosedChanged(m_isLidClosed);
        }
    }
}

bool LidController::isLidClosed() const
{
    return m_isLidClosed;
}

bool LidController::isLidPresent() const
{
    return m_isLidPresent;
}

#include "moc_lidcontroller.cpp"