/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KCrash>

#include <iostream>

#include "global.h"
#include "database.h"
#include "fileindexerconfig.h"
#include "priority.h"
#include "migrator.h"
#include "mainhub.h"

#include <QDBusConnection>
#include <QCoreApplication>
#include <QFile>

int main(int argc, char** argv)
{
    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    QCoreApplication app(argc, argv);

    Baloo::FileIndexerConfig indexerConfig;
    if (!indexerConfig.indexingEnabled()) {
        std::cout << "Baloo File Indexing has been disabled" << std::endl;
        return 0;
    }

    if (!QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.baloo"))) {
        qWarning() << "Failed to register via dbus. Another instance is running";
        return 1;
    }

    // Crash Handling
    KCrash::setFlags(KCrash::AutoRestart);

    const QString path = Baloo::fileIndexDbPath();

    Baloo::Migrator migrator(path, &indexerConfig);
    if (migrator.migrationRequired()) {
        migrator.migrate();
    }

    bool firstRun = !QFile::exists(path + QStringLiteral("/index"));

    Baloo::Database *db = Baloo::globalDatabaseInstance();

    /**
     * try to open, if that fails, try to unlink the index db and retry
     */
    if (!db->open(Baloo::Database::CreateDatabase)) {
        // delete old stuff, set to initial run!
        qWarning() << "Failed to create database, removing corrupted database.";
        QFile::remove(path + QStringLiteral("/index"));
        QFile::remove(path + QStringLiteral("/index-lock"));
        firstRun = true;

        // try to create now after cleanup, if still no works => fail
        if (!db->open(Baloo::Database::CreateDatabase)) {
            qWarning() << "Failed to create database after deleting corrupted one.";
            return 1;
        }
    }

    Baloo::MainHub hub(db, &indexerConfig, firstRun);
    return app.exec();
}
