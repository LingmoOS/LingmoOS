/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "soundthemeconfig.h"

const QString DEFAULT_SOUND_THEME = QStringLiteral("ocean");

SoundThemeConfig::SoundThemeConfig(QObject *parent)
    : QObject(parent)
    , m_soundTheme(DEFAULT_SOUND_THEME)
{
    m_configWatcher = KConfigWatcher::create(KSharedConfig::openConfig(QStringLiteral("kdeglobals")));
    connect(m_configWatcher.get(), &KConfigWatcher::configChanged, this, &SoundThemeConfig::kdeglobalsChanged);

    const KConfigGroup soundGroup = m_configWatcher->config()->group("Sounds");
    m_soundTheme = soundGroup.readEntry("Theme", DEFAULT_SOUND_THEME);
}

QString SoundThemeConfig::soundTheme() const
{
    return m_soundTheme;
}

void SoundThemeConfig::kdeglobalsChanged(const KConfigGroup &group, const QByteArrayList &names)
{
    if (group.name() != QLatin1String("Sounds") || !names.contains(QByteArrayLiteral("Theme"))) {
        return;
    }

    m_soundTheme = group.readEntry("Theme", DEFAULT_SOUND_THEME);
    Q_EMIT soundThemeChanged(m_soundTheme);
}

#include "moc_soundthemeconfig.cpp"
