/*
 *   SPDX-FileCopyrightText: 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

// Self
#include "Database.h"
#include <kactivities-features.h>

// Qt
#include <QDir>
#include <QStandardPaths>

// Utils
#include <utils/d_ptr_implementation.h>
#include <utils/qsqlquery_iterator.h>

// System
#include <array>
#include <cmath>
#include <memory>

// Local
#include "DebugResources.h"
#include "Utils.h"

#include <common/database/schema/ResourcesDatabaseSchema.h>

class ResourcesDatabaseInitializer::Private
{
public:
    Common::Database::Ptr database;
};

Common::Database::Ptr resourcesDatabase()
{
    static ResourcesDatabaseInitializer instance;
    return instance.d->database;
}

void ResourcesDatabaseInitializer::initDatabase(bool retryOnFail)
{
    //
    // There are three situations we want to handle:
    // 1. The database can not be opened at all.
    //    This means that the current database files have
    //    been corrupted and that we need to replace them
    //    with the last working backup.
    // 2. The database was opened, but an error appeared
    //    somewhere at runtime.
    // 3. The database was successfully opened and no errors
    //    appeared during runtime.
    //
    // To achieve this, we will have three locations for
    // database files:
    //
    // 1. `resources` - the current database files
    // 2. `resources-test-backup` - at each KAMD start,
    //    we copy the current database files here.
    //    If an error appears during execution, the files
    //    will be removed and the error will be added to
    //    the log file `resources/errors.log`
    // 3. `resources-working-backup` - on each KAMD start,
    //    if there are files in `resources-test-backup`
    //    (meaning no error appeared at runtime), they
    //    will be copied to `resources-working-backup`.
    //
    // This means that the `working` backup will be a bit
    // older, but it will be the last database that produced
    // no errors at runtime.
    //

    const QString databaseDirectoryPath =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kactivitymanagerd/resources/");

    const QString databaseTestBackupDirectoryPath =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kactivitymanagerd/resources/test-backup/");

    const QString databaseWorkingBackupDirectoryPath =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kactivitymanagerd/resources/working-backup/");

    static const std::array<QString, 3> databaseFiles{"database", "database-wal", "database-shm"};

    {
        QDir dir;
        dir.mkpath(databaseDirectoryPath);
        dir.mkpath(databaseTestBackupDirectoryPath);
        dir.mkpath(databaseWorkingBackupDirectoryPath);

        if (!dir.exists(databaseDirectoryPath) || !dir.exists(databaseTestBackupDirectoryPath) || !dir.exists(databaseWorkingBackupDirectoryPath)) {
            qCWarning(KAMD_LOG_RESOURCES) << "Database directory can not be created!";
            return;
        }
    }

    const QDir databaseDirectory(databaseDirectoryPath);
    const QDir databaseTestBackupDirectory(databaseTestBackupDirectoryPath);
    const QDir databaseWorkingBackupDirectory(databaseWorkingBackupDirectoryPath);

    auto removeDatabaseFiles = [](const QDir &dir) {
        return std::all_of(databaseFiles.begin(), databaseFiles.cend(), [&dir](const QString &fileName) {
            const auto filePath = dir.filePath(fileName);
            return !QFile::exists(filePath) || QFile::remove(filePath);
        });
    };

    auto copyDatabaseFiles = [removeDatabaseFiles](const QDir &fromDir, const QDir &toDir) {
        return removeDatabaseFiles(toDir) && std::all_of(databaseFiles.begin(), databaseFiles.cend(), [&fromDir, &toDir](const QString &fileName) {
                   const auto fromFilePath = fromDir.filePath(fileName);
                   const auto toFilePath = toDir.filePath(fileName);
                   return QFile::copy(fromFilePath, toFilePath);
               });
    };

    auto databaseFilesExistIn = [](const QDir &dir) {
        return dir.exists() && std::all_of(databaseFiles.begin(), databaseFiles.cend(), [&dir](const QString &fileName) {
                   const auto filePath = dir.filePath(fileName);
                   return QFile::exists(filePath);
               });
    };

    // First, let's move the files from `resources-test-backup` to
    // `resources-working-backup` (if they exist)
    if (databaseFilesExistIn(databaseTestBackupDirectory)) {
        qCDebug(KAMD_LOG_RESOURCES) << "Marking the test backup as working...";
        if (copyDatabaseFiles(databaseTestBackupDirectory, databaseWorkingBackupDirectory)) {
            removeDatabaseFiles(databaseTestBackupDirectory);
        } else {
            qCWarning(KAMD_LOG_RESOURCES) << "Marking the test backup as working failed!";
            removeDatabaseFiles(databaseWorkingBackupDirectory);
        }
    }

    // Next, copy the current database files to `resources-test-backup`
    if (databaseFilesExistIn(databaseDirectory)) {
        qCDebug(KAMD_LOG_RESOURCES) << "Creating the backup of the current database...";
        if (!copyDatabaseFiles(databaseDirectory, databaseTestBackupDirectory)) {
            qCWarning(KAMD_LOG_RESOURCES) << "Creating the backup of the current database failed!";
            removeDatabaseFiles(databaseTestBackupDirectory);
        }
    }

    // Now we can try to open the database
    d->database = Common::Database::instance(Common::Database::ResourcesDatabase, Common::Database::ReadWrite);

    if (d->database) {
        qCDebug(KAMD_LOG_RESOURCES) << "Database opened successfully";
        QObject::connect(d->database.get(), &Common::Database::error, d->database.get(), [databaseTestBackupDirectory, removeDatabaseFiles](const QSqlError &error) {
            const QString errorLog =
                QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kactivitymanagerd/resources/errors.log");
            QFile file(errorLog);
            if (file.open(QIODevice::Append)) {
                QTextStream out(&file);
                out << QDateTime::currentDateTime().toString(Qt::ISODate) << " error: " << error.text() << "\n";
            } else {
                qCWarning(KAMD_LOG_RESOURCES) << QDateTime::currentDateTime().toString(Qt::ISODate) << " error: " << error.text();
            }

            removeDatabaseFiles(databaseTestBackupDirectory);
        });
        Common::ResourcesDatabaseSchema::initSchema(*d->database);

    } else {
        // The current database can not be opened, delete the
        // backup we just created
        removeDatabaseFiles(databaseTestBackupDirectory);

        if (databaseFilesExistIn(databaseWorkingBackupDirectoryPath)) {
            qCWarning(KAMD_LOG_RESOURCES) << "The database seems to be corrupted, trying to load the latest working version";

            const auto success = copyDatabaseFiles(databaseWorkingBackupDirectory, databaseDirectory);

            if (success && retryOnFail) {
                // Avoid infinite recursion
                initDatabase(false);
            }

        } else {
            qCWarning(KAMD_LOG_RESOURCES) << "The database might be corrupted and there is no working backup";
        }
    }
}

ResourcesDatabaseInitializer::ResourcesDatabaseInitializer()
{
    initDatabase(true);
}

ResourcesDatabaseInitializer::~ResourcesDatabaseInitializer()
{
}
