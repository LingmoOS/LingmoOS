/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "indexcleaner.h"
#include "fileindexerconfig.h"

#include "database.h"
#include "transaction.h"
#include "idutils.h"

#include "baloodebug.h"

#include <QFile>

using namespace Baloo;

IndexCleaner::IndexCleaner(Database* db, FileIndexerConfig* config)
    : m_db(db)
    , m_config(config)
{
    Q_ASSERT(db);
    Q_ASSERT(config);
}

void IndexCleaner::run()
{
    Transaction tr(m_db, Transaction::ReadWrite);

    auto shouldDelete = [&](quint64 id) {
        if (!id) {
            return false;
        }

        const QString url = QFile::decodeName(tr.documentUrl(id));

        if (!QFile::exists(url)) {
            qCDebug(BALOO) << "not exists: " << url;
            return true;
        }

        if (!m_config->shouldBeIndexed(url)) {
            qCDebug(BALOO) << "should not be indexed: " << url;
            return true;
        }

        return false;
    };

    const auto excludeFolders = m_config->excludeFolders();
    for (const QString& folder : excludeFolders) {
        quint64 id = filePathToId(QFile::encodeName(folder));
        if (id > 0 && tr.hasDocument(id)) {
            tr.removeRecursively(id, shouldDelete);
        }
    }
    tr.commit();

    Q_EMIT done();
}

#include "moc_indexcleaner.cpp"
