/*
 * SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kcolorschemewatcher_xdg.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QDebug>

KColorSchemeWatcherXDG::KColorSchemeWatcherXDG()
    : KColorSchemeWatcherBackend()
{
    QDBusConnection::sessionBus().connect(QStringLiteral("org.freedesktop.portal.Desktop"),
                                          QStringLiteral("/org/freedesktop/portal/desktop"),
                                          QStringLiteral("org.freedesktop.portal.Settings"),
                                          QStringLiteral("SettingChanged"),
                                          this,
                                          SLOT(slotSettingChanged(QString, QString, QDBusVariant)));

    QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                    QStringLiteral("/org/freedesktop/portal/desktop"),
                                                    QStringLiteral("org.freedesktop.portal.Settings"),
                                                    QStringLiteral("Read"));
    m.setArguments({QStringLiteral("org.freedesktop.appearance"), QStringLiteral("color-scheme")});

    auto reply = QDBusConnection::sessionBus().call(m);

    const uint result = reply.arguments().first().value<QDBusVariant>().variant().value<QDBusVariant>().variant().toUInt();
    m_preference = fdoToInternal(result);
}

KColorSchemeWatcher::ColorPreference KColorSchemeWatcherXDG::systemPreference() const
{
    return m_preference;
}

void KColorSchemeWatcherXDG::slotSettingChanged(QString nameSpace, QString key, QDBusVariant value)
{
    if (nameSpace == QLatin1String("org.freedesktop.appearance") && key == QLatin1String("color-scheme")) {
        const uint result = value.variant().toUInt();
        auto newValue = fdoToInternal(result);

        if (m_preference != newValue) {
            m_preference = fdoToInternal(result);
            Q_EMIT systemPreferenceChanged();
        }
    }
}

KColorSchemeWatcher::ColorPreference KColorSchemeWatcherXDG::fdoToInternal(uint value) const
{
    if (value == 0) {
        return KColorSchemeWatcher::NoPreference;
    } else if (value == 1) {
        return KColorSchemeWatcher::PreferDark;
    } else if (value == 2) {
        return KColorSchemeWatcher::PreferLight;
    } else {
        qWarning() << "Unhandled org.freedesktop.appearance color-scheme value" << value;
        return KColorSchemeWatcher::NoPreference;
    }
}

#include "moc_kcolorschemewatcher_xdg.cpp"
