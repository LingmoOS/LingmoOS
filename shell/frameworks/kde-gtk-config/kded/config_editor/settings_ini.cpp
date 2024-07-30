// SPDX-FileCopyrightText: 2019, 2022 Mikhail Zolotukhin <zomial@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "settings_ini.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QStandardPaths>

#include "config_editor/utils.h"
#include <gio/gio.h>

namespace
{
constinit unsigned s_timerId = 0;
KConfigGroup s_configGroup3;
KConfigGroup s_configGroup4;

#if GLIB_CHECK_VERSION(2, 74, 0)
void
#else
int
#endif
syncConfig(void *)
{
    if (s_configGroup3.isValid()) {
        s_configGroup3.sync();
        s_configGroup3 = KConfigGroup();
    }
    if (s_configGroup4.isValid()) {
        s_configGroup4.sync();
        s_configGroup4 = KConfigGroup();
    }
    s_timerId = 0;
#if !GLIB_CHECK_VERSION(2, 74, 0)
    return G_SOURCE_REMOVE;
#endif
}

KConfigGroup &gtkConfigGroup(int gtkVersion)
{
    static_assert(!std::is_trivially_copy_assignable_v<KConfigGroup>);
    if (gtkVersion == 3 && s_configGroup3.isValid()) {
        return s_configGroup3;
    } else if (gtkVersion == 4 && s_configGroup4.isValid()) {
        return s_configGroup4;
    }

    QString configLocation = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    auto gtkConfigPath = QStringLiteral("%1/gtk-%2.0/settings.ini").arg(configLocation, QString::number(gtkVersion));

    KSharedConfig::Ptr gtkConfig = KSharedConfig::openConfig(gtkConfigPath, KConfig::NoGlobals);
    if (gtkVersion == 4) {
        s_configGroup4 = gtkConfig->group(QStringLiteral("Settings"));
        return s_configGroup4;
    }
    s_configGroup3 = gtkConfig->group(QStringLiteral("Settings"));
    return s_configGroup3;
}
}

namespace SettingsIniEditor
{
void setValue(const QString &paramName, const QVariant &paramValue, int gtkVersion)
{
    auto setValueForVersion = [&](int version) {
        KConfigGroup &group = gtkConfigGroup(version);
        group.writeEntry(paramName, paramValue);
        if (s_timerId == 0) {
#if GLIB_CHECK_VERSION(2, 74, 0)
            s_timerId = g_timeout_add_once(100, syncConfig, nullptr);
#else
            s_timerId = g_timeout_add(100, syncConfig, nullptr);
#endif
        }
    };

    if (gtkVersion != -1) {
        setValueForVersion(gtkVersion);
        return;
    }

    for (auto version : Utils::s_gtkVersions) {
        setValueForVersion(version);
    }
}

void unsetValue(const QString &paramName, int gtkVersion)
{
    auto unsetValueForVersion = [&](int version) {
        KConfigGroup &group = gtkConfigGroup(version);
        group.deleteEntry(paramName);
        if (s_timerId == 0) {
#if GLIB_CHECK_VERSION(2, 74, 0)
            s_timerId = g_timeout_add_once(100, syncConfig, nullptr);
#else
            s_timerId = g_timeout_add(100, syncConfig, nullptr);
#endif
        }
    };

    if (gtkVersion != -1) {
        unsetValueForVersion(gtkVersion);
        return;
    }

    for (auto version : Utils::s_gtkVersions) {
        unsetValueForVersion(version);
    }
}

QString value(const QString &paramName, int gtkVersion)
{
    if (gtkVersion == -1) {
        gtkVersion = *Utils::s_gtkVersions.begin();
    }

    KConfigGroup &group = gtkConfigGroup(gtkVersion);
    return group.readEntry(paramName);
}

}
