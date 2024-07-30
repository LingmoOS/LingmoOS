/*
 *  SPDX-FileCopyrightText: 2022 Devin Lin <devin@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "quicksettingsconfig.h"

#include <QDebug>

const QString CONFIG_FILE = QStringLiteral("lingmomobilerc");
const QString QUICKSETTINGS_CONFIG_GROUP = QStringLiteral("QuickSettings");

QuickSettingsConfig::QuickSettingsConfig(QObject *parent)
    : QObject{parent}
    , m_config{KSharedConfig::openConfig(CONFIG_FILE, KConfig::SimpleConfig)}
{
    m_configWatcher = KConfigWatcher::create(m_config);

    connect(m_configWatcher.data(), &KConfigWatcher::configChanged, this, [this](const KConfigGroup &group) -> void {
        if (group.name() == QUICKSETTINGS_CONFIG_GROUP) {
            Q_EMIT enabledQuickSettingsChanged();
            Q_EMIT disabledQuickSettingsChanged();
        }
    });
}

QList<QString> QuickSettingsConfig::enabledQuickSettings() const
{
    auto group = KConfigGroup{m_config, QUICKSETTINGS_CONFIG_GROUP};
    // TODO move defaults to file
    // we aren't worried about quicksettings not showing up though, any that are not specified will be automatically added to the end
    return group.readEntry("enabledQuickSettings",
                           QList<QString>{QStringLiteral("org.kde.lingmo.quicksetting.wifi"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.mobiledata"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.bluetooth"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.flashlight"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.screenrotation"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.settingsapp"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.airplanemode"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.audio"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.battery"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.record"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.nightcolor"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.screenshot"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.powermenu"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.donotdisturb"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.caffeine"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.keyboardtoggle"),
                                          QStringLiteral("org.kde.lingmo.quicksetting.hotspot")});
}

void QuickSettingsConfig::setEnabledQuickSettings(QList<QString> &list)
{
    auto group = KConfigGroup{m_config, QUICKSETTINGS_CONFIG_GROUP};
    group.writeEntry("enabledQuickSettings", list, KConfigGroup::Notify);
    m_config->sync();
}

QList<QString> QuickSettingsConfig::disabledQuickSettings() const
{
    auto group = KConfigGroup{m_config, QUICKSETTINGS_CONFIG_GROUP};
    return group.readEntry("disabledQuickSettings", QList<QString>{});
}

void QuickSettingsConfig::setDisabledQuickSettings(QList<QString> &list)
{
    auto group = KConfigGroup{m_config, QUICKSETTINGS_CONFIG_GROUP};
    group.writeEntry("disabledQuickSettings", list, KConfigGroup::Notify);
    m_config->sync();
}
