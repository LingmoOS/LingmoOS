/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_MODIFIEDFILEINDEXER_H
#define BALOO_MODIFIEDFILEINDEXER_H

#include <QRunnable>
#include <QObject>
#include <QStringList>

namespace Baloo {

class Database;
class FileIndexerConfig;

class ModifiedFileIndexer : public QObject, public QRunnable
{
    Q_OBJECT
public:
    ModifiedFileIndexer(Database* db, const FileIndexerConfig* config, const QStringList& files);

    void run() override;

Q_SIGNALS:
    void done();

private:
    Database* m_db;
    const FileIndexerConfig* m_config;
    QStringList m_files;
};
}

#endif // BALOO_XATTRINDEXER_H
