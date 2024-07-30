/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configstorage.h"

// KF
#include <KSharedConfig>

ConfigStorage::ConfigStorage(QObject *parent)
    : QObject(parent)
{
    auto config = KSharedConfig::openConfig(QStringLiteral("plasma_calendar_astronomicalevents"));
    m_generalConfigGroup = config->group("General");

    m_lunarPhaseShown = m_generalConfigGroup.readEntry("showLunarPhase", true);
    m_seasonShown = m_generalConfigGroup.readEntry("showSeason", true);
}

void ConfigStorage::save()
{
    m_generalConfigGroup.writeEntry("showLunarPhase", m_lunarPhaseShown);
    m_generalConfigGroup.writeEntry("showSeason", m_seasonShown);

    m_generalConfigGroup.sync();
}
