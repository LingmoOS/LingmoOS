/*
 *   SPDX-FileCopyrightText: 2013 Julien Borderie <frajibe@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "configurationmanager.h"
#include <KSharedConfig>

const QString ConfigurationManager::GENERAL_CONFIG_GROUP = QStringLiteral("General");
const QString ConfigurationManager::SHOW_HIDDEN_ENTRIES_PROPERTY_NAME = QStringLiteral("ShowHidden");
const QString ConfigurationManager::SPLITTER_SIZES_PROPERTY_NAME = QStringLiteral("SplitterSizes");

ConfigurationManager *ConfigurationManager::m_instance = nullptr;

ConfigurationManager::ConfigurationManager()
    : m_configGroup(KSharedConfig::openConfig(), GENERAL_CONFIG_GROUP)
{
}

bool ConfigurationManager::hiddenEntriesVisible() const
{
    return m_configGroup.readEntry(SHOW_HIDDEN_ENTRIES_PROPERTY_NAME, false);
}

void ConfigurationManager::setHiddenEntriesVisible(bool visible)
{
    m_configGroup.writeEntry(SHOW_HIDDEN_ENTRIES_PROPERTY_NAME, visible);
    m_configGroup.sync();
}

QList<int> ConfigurationManager::getSplitterSizes() const
{
    return m_configGroup.readEntry(SPLITTER_SIZES_PROPERTY_NAME, QList<int>());
}

void ConfigurationManager::setSplitterSizes(const QList<int> &sizes)
{
    m_configGroup.writeEntry(SPLITTER_SIZES_PROPERTY_NAME, sizes);
    m_configGroup.sync();
}
