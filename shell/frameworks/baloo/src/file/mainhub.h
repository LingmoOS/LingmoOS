/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_MAINHUB_H
#define BALOO_MAINHUB_H

#include <QObject>

#include "filewatch.h"
#include "fileindexscheduler.h"

namespace Baloo {

class Database;
class FileIndexerConfig;

class MainHub : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.baloo.main")
public:
    MainHub(Database* db, FileIndexerConfig* config, bool firstRun);

public Q_SLOTS:
    Q_SCRIPTABLE void quit() const;
    Q_SCRIPTABLE void updateConfig();

private:
    Database* m_db;
    FileIndexerConfig* m_config;

    FileWatch m_fileWatcher;
    FileIndexScheduler m_fileIndexScheduler;

};
}

#endif // BALOO_MAINHUB_H
