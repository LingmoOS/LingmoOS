/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "migrator.h"
#include "fileindexerconfig.h"

#include <QFile>
#include <QDir>

using namespace Baloo;

Migrator::Migrator(const QString& dbPath, FileIndexerConfig* config)
    : m_dbPath(dbPath)
    , m_config(config)
{
    Q_ASSERT(!dbPath.isEmpty());
    Q_ASSERT(!dbPath.endsWith(QLatin1Char('/')));
    Q_ASSERT(config);
}

/*
 * Changing this version number indicates that the old index should be deleted
 * and the indexing should be started from scratch.
 */
static int s_dbVersion = 2;

bool Migrator::migrationRequired() const
{
    return m_config->databaseVersion() != s_dbVersion;
}

void Migrator::migrate()
{
    Q_ASSERT(migrationRequired());

    int dbVersion = m_config->databaseVersion();
    if (dbVersion == 0 && QFile::exists(m_dbPath + QStringLiteral("/file"))) {
        QDir dir(m_dbPath + QStringLiteral("/file"));
        dir.removeRecursively();
    }
    else if (QFile::exists(m_dbPath + QStringLiteral("/index"))) {
        QFile::remove(m_dbPath + QStringLiteral("/index"));
        QFile::remove(m_dbPath + QStringLiteral("/index-lock"));
    }

    m_config->setDatabaseVersion(s_dbVersion);
}
