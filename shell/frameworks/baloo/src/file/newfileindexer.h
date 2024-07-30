/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_NEWFILEINDEXER_H
#define BALOO_NEWFILEINDEXER_H

#include <QRunnable>
#include <QStringList>
#include <QObject>

namespace Baloo {

class Database;
class FileIndexerConfig;

/**
 * Does not check the folder path or the mtime of the file
 */
class NewFileIndexer : public QObject, public QRunnable
{
    Q_OBJECT
public:
    NewFileIndexer(Database* db, const FileIndexerConfig* config, const QStringList& newFiles);

    void run() override;

Q_SIGNALS:
    void done();

private:
    Database* m_db;
    const FileIndexerConfig* m_config;
    QStringList m_files;
};

}

#endif // BALOO_NEWFILEINDEXER_H
