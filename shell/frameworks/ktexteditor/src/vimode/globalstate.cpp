/*
    SPDX-FileCopyrightText: 2008-2011 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012-2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "globalstate.h"
#include "history.h"
#include "macros.h"
#include "mappings.h"
#include "registers.h"

#include <KConfigGroup>

using namespace KateVi;

GlobalState::GlobalState()
{
    m_macros = new Macros();
    m_mappings = new Mappings();
    m_registers = new Registers();
    m_searchHistory = new History();
    m_replaceHistory = new History();
    m_commandHistory = new History();

    readConfig(config().data());
}

GlobalState::~GlobalState()
{
    writeConfig(config().data());
    config().data()->sync();

    delete m_searchHistory;
    delete m_replaceHistory;
    delete m_commandHistory;
    delete m_macros;
    delete m_mappings;
    delete m_registers;
}

void GlobalState::writeConfig(KConfig *configFile) const
{
    // FIXME: use own groups instead of one big group!
    KConfigGroup config(configFile, QStringLiteral("Kate Vi Input Mode Settings"));
    m_macros->writeConfig(config);
    m_mappings->writeConfig(config);
    m_registers->writeConfig(config);
}

void GlobalState::readConfig(const KConfig *configFile)
{
    // FIXME: use own groups instead of one big group!
    const KConfigGroup config(configFile, QStringLiteral("Kate Vi Input Mode Settings"));

    m_macros->readConfig(config);
    m_mappings->readConfig(config);
    m_registers->readConfig(config);
}

KSharedConfigPtr GlobalState::config()
{
    // use dummy config for unit tests!
    return QStandardPaths::isTestModeEnabled()
        ? KSharedConfig::openConfig(QStringLiteral("katevirc-unittest"), KConfig::SimpleConfig, QStandardPaths::TempLocation)
        : KSharedConfig::openConfig(QStringLiteral("katevirc"));
}
