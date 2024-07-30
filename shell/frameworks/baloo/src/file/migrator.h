/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_MIGRATOR_H
#define BALOO_MIGRATOR_H

#include <QString>

namespace Baloo {

class FileIndexerConfig;

class Migrator
{
public:
    Migrator(const QString& dbPath, FileIndexerConfig* config);

    bool migrationRequired() const;
    void migrate();

private:
    QString m_dbPath;
    FileIndexerConfig* m_config;
};
}

#endif // BALOO_MIGRATOR_H
