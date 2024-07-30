/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_FILECONTENTINDEXERPROVIDER_H
#define BALOO_FILECONTENTINDEXERPROVIDER_H

#include <QVector>

namespace Baloo {

class Database;

class FileContentIndexerProvider
{
public:
    explicit FileContentIndexerProvider(Database* db);

    uint size();
    QVector<quint64> fetch(uint size);
    bool markFailed(quint64 id);

private:
    Database* m_db;
};
}

#endif // BALOO_FILECONTENTINDEXERPROVIDER_H
