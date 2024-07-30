/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_INDEXCLEANER_H
#define BALOO_INDEXCLEANER_H

#include <QRunnable>
#include <QObject>

namespace Baloo {

class Database;
class FileIndexerConfig;

class IndexCleaner : public QObject, public QRunnable
{
    Q_OBJECT
public:
    IndexCleaner(Database* db, FileIndexerConfig* config);
    void run() override;

Q_SIGNALS:
    void done();

private:
    Database* m_db;
    FileIndexerConfig* m_config;
};
}

#endif // BALOO_INDEXCLEANER_H
