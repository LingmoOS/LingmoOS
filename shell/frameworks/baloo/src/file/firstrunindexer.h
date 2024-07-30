/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_FIRSTRUNINDEXER_H
#define BALOO_FIRSTRUNINDEXER_H

#include <QRunnable>
#include <QObject>
#include <QStringList>

namespace Baloo {

class Database;
class FileIndexerConfig;

class FirstRunIndexer : public QObject, public QRunnable
{
    Q_OBJECT
public:
    FirstRunIndexer(Database* db, FileIndexerConfig* config, const QStringList& folders);

    void run() override;

Q_SIGNALS:
    void done();

private:
    Database* m_db;
    FileIndexerConfig* m_config;

    QStringList m_folders;
};
}

#endif // BALOO_FIRSTRUNINDEXER_H
