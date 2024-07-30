/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2016 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DATABASE_H
#define BALOO_DATABASE_H

#include <QMutex>

#include "document.h"
#include "databasedbis.h"

namespace Baloo {

class DatabaseTest;

class BALOO_ENGINE_EXPORT Database
{
public:
    /**
     * Init database for given DB path, will not open it.
     * @param path db path
     */
    explicit Database(const QString& path);

    /**
     * Destruct db, might close it, if opened.
     */
    ~Database();

    /**
     * Database open mode
     */
    enum OpenMode {
        /**
         * Create + open read-write database.
         */
        CreateDatabase,

        /**
         * Read-Write Database, only works if database exists.
         */
        ReadWriteDatabase,

        /**
         * Read-Only Database, only works if database exists.
         */
        ReadOnlyDatabase,
    };

    /**
     * Open database in given mode.
     * Nop after open was done (even if mode differs).
     * There is no close as this would invalidate the database for all threads using it.
     * @param mode create or open only?
     * @return success?
     */
    bool open(OpenMode mode);

    /**
     * Is database open?
     * @return database open?
     */
    bool isOpen() const;

    /**
     * Path to database.
     * @return database path
     */
    QString path() const;

    /**
     * Is the database available for use?
     * For example if indexing is disabled or the indexer did never run this is false.
     * @return database available
     */
    bool isAvailable() const;

private:
    /**
     * serialize access, as open might be called from multiple threads
     */
    mutable QMutex m_mutex;

    /**
     * database path
     */
    const QString m_path;

    MDB_env* m_env;
    DatabaseDbis m_dbis;

    friend class Transaction;
    friend class DatabaseTest;

};
}

#endif // BALOO_DATABASE_H
