/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "indexerconfig.h"
#include "../file/fileindexerconfig.h"
#include "../file/fileexcludefilters.h"
#include "../file/regexpcache.h"

#include <QDBusConnection>
#include "maininterface.h"
#include "baloosettings.h"

using namespace Baloo;

class BALOO_CORE_NO_EXPORT IndexerConfig::Private {
public:
    FileIndexerConfig m_config;
    BalooSettings m_settings;
};

IndexerConfig::IndexerConfig()
    : d(new Private)
{
}

IndexerConfig::~IndexerConfig()
{
    d->m_settings.save();
}

bool IndexerConfig::fileIndexingEnabled() const
{
    return d->m_settings.indexingEnabled();
}

void IndexerConfig::setFileIndexingEnabled(bool enabled) const
{
    d->m_settings.setIndexingEnabled(enabled);
}

bool IndexerConfig::shouldBeIndexed(const QString& path) const
{
    return d->m_config.shouldBeIndexed(path);
}

bool IndexerConfig::canBeSearched(const QString& folder) const
{
    return d->m_config.canBeSearched(folder);
}

QStringList IndexerConfig::excludeFolders() const
{
    return d->m_config.excludeFolders();
}

QStringList IndexerConfig::includeFolders() const
{
    return d->m_config.includeFolders();
}

QStringList IndexerConfig::excludeFilters() const
{
    return d->m_config.excludeFilters();
}

QStringList IndexerConfig::excludeMimetypes() const
{
    return d->m_config.excludeMimetypes();
}

void IndexerConfig::setExcludeFolders(const QStringList& excludeFolders)
{
    d->m_settings.setExcludedFolders(excludeFolders);
}

void IndexerConfig::setIncludeFolders(const QStringList& includeFolders)
{
    d->m_settings.setFolders(includeFolders);
}

void IndexerConfig::setExcludeFilters(const QStringList& excludeFilters)
{
    d->m_settings.setExcludedFilters(excludeFilters);
}

void IndexerConfig::setExcludeMimetypes(const QStringList& excludeMimetypes)
{
    d->m_settings.setExcludedMimetypes(excludeMimetypes);
}

bool IndexerConfig::indexHidden() const
{
    return d->m_settings.indexHiddenFolders();
}

void IndexerConfig::setIndexHidden(bool value) const
{
    d->m_settings.setIndexHiddenFolders(value);
}

bool IndexerConfig::onlyBasicIndexing() const
{
    return d->m_settings.onlyBasicIndexing();
}

void IndexerConfig::setOnlyBasicIndexing(bool value)
{
    d->m_settings.setOnlyBasicIndexing(value);
}

void IndexerConfig::refresh() const
{
    org::kde::baloo::main mainInterface(QStringLiteral("org.kde.baloo"),
                                                QStringLiteral("/"),
                                                QDBusConnection::sessionBus());
    mainInterface.updateConfig();
}
