/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_UNINDEXEDFILEINDEXER
#define BALOO_UNINDEXEDFILEINDEXER

#include <QRunnable>
#include <QObject>

namespace Baloo {

class Database;
class FileIndexerConfig;

class UnindexedFileIndexer : public QObject, public QRunnable
{
    Q_OBJECT
public:
    UnindexedFileIndexer(Database* db, const FileIndexerConfig* config);

    void run() override;

Q_SIGNALS:
    void done();

private:
    Database* m_db;
    const FileIndexerConfig* m_config;
};
}

#endif //BALOO_UNINDEXEDFILEINDEXER
