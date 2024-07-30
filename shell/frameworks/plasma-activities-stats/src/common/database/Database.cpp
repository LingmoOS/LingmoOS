/*
    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "Database.h"

#include <common/database/schema/ResourcesDatabaseSchema.h>

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QThread>

#include <map>
#include <mutex>

#include "plasma-activities-stats-logsettings.h"

namespace Common
{
namespace
{
std::mutex databases_mutex;

struct DatabaseInfo {
    Qt::HANDLE thread;
    Database::OpenMode openMode;
};

bool operator<(const DatabaseInfo &left, const DatabaseInfo &right)
{
    return left.thread < right.thread ? true : left.thread > right.thread ? false : left.openMode < right.openMode;
}

std::map<DatabaseInfo, std::weak_ptr<Database>> databases;
}

class QSqlDatabaseWrapper
{
private:
    QSqlDatabase m_database;
    bool m_open;
    QString m_connectionName;

public:
    QSqlDatabaseWrapper(const DatabaseInfo &info)
        : m_open(false)
    {
        m_connectionName = QStringLiteral("kactivities_db_resources_")
            // Adding the thread number to the database name
            + QString::number((quintptr)info.thread)
            // And whether it is read-only or read-write
            + (info.openMode == Database::ReadOnly ? QStringLiteral("_readonly") : QStringLiteral("_readwrite"));

        m_database = QSqlDatabase::contains(m_connectionName) ? QSqlDatabase::database(m_connectionName)
                                                              : QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);

        if (info.openMode == Database::ReadOnly) {
            m_database.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
        }

        // We are allowing the database file to be overridden mostly for testing purposes
        m_database.setDatabaseName(ResourcesDatabaseSchema::path());

        m_open = m_database.open();

        if (!m_open) {
            qCWarning(PLASMA_ACTIVITIES_STATS_LOG) << "PlasmaActivities: Database is not open: " << m_database.connectionName() << m_database.databaseName()
                                             << m_database.lastError();

            if (info.openMode == Database::ReadWrite) {
                qFatal("PlasmaActivities: Opening the database in RW mode should always succeed");
            }
        }
    }

    ~QSqlDatabaseWrapper()
    {
        qCDebug(PLASMA_ACTIVITIES_STATS_LOG) << "Closing SQL connection: " << m_connectionName;
    }

    QSqlDatabase &get()
    {
        return m_database;
    }

    bool isOpen() const
    {
        return m_open;
    }

    QString connectionName() const
    {
        return m_connectionName;
    }
};

class Database::Private
{
public:
    Private()
    {
    }

    QSqlQuery query(const QString &query)
    {
        return database ? QSqlQuery(query, database->get()) : QSqlQuery();
    }

    QSqlQuery query()
    {
        return database ? QSqlQuery(database->get()) : QSqlQuery();
    }

    std::unique_ptr<QSqlDatabaseWrapper> database;
};

Database::Locker::Locker(Database &database)
    : m_database(database.d->database->get())
{
    m_database.transaction();
}

Database::Locker::~Locker()
{
    m_database.commit();
}

Database::Ptr Database::instance(Source source, OpenMode openMode)
{
    Q_UNUSED(source) // for the time being

    std::lock_guard<std::mutex> lock(databases_mutex);

    // We are saving instances per thread and per read/write mode
    DatabaseInfo info;
    info.thread = QThread::currentThreadId();
    info.openMode = openMode;

    // Do we have an instance matching the request?
    auto search = databases.find(info);
    if (search != databases.end()) {
        auto ptr = search->second.lock();

        if (ptr) {
            return ptr;
        }
    }

    // Creating a new database instance
    auto ptr = std::make_shared<Database>();

    ptr->d->database = std::make_unique<QSqlDatabaseWrapper>(info);

    if (!ptr->d->database->isOpen()) {
        return nullptr;
    }

    databases[info] = ptr;

    if (info.openMode == ReadOnly) {
        // From now on, only SELECT queries will work
        ptr->setPragma(QStringLiteral("query_only = 1"));

        // These should not make any difference
        ptr->setPragma(QStringLiteral("synchronous = 0"));

    } else {
        // Using the write-ahead log and sync = NORMAL for faster writes
        ptr->setPragma(QStringLiteral("synchronous = 1"));
    }

    // Maybe we should use the write-ahead log
    auto walResult = ptr->pragma(QStringLiteral("journal_mode = WAL"));

    if (walResult != QLatin1String("wal")) {
        qCWarning(PLASMA_ACTIVITIES_STATS_LOG) << "PlasmaActivities: Database can not be opened in WAL mode. Check the "
                                            "SQLite version (required >3.7.0). And whether your filesystem "
                                            "supports shared memory";

        return nullptr;
    }

    // We don't have a big database, lets flush the WAL when
    // it reaches 400k, not 4M as is default
    ptr->setPragma(QStringLiteral("wal_autocheckpoint = 100"));

    qCDebug(PLASMA_ACTIVITIES_STATS_LOG) << "PlasmaActivities: Database connection: " << ptr->d->database->connectionName()
                                   << "\n    query_only:         " << ptr->pragma(QStringLiteral("query_only"))
                                   << "\n    journal_mode:       " << ptr->pragma(QStringLiteral("journal_mode"))
                                   << "\n    wal_autocheckpoint: " << ptr->pragma(QStringLiteral("wal_autocheckpoint"))
                                   << "\n    synchronous:        " << ptr->pragma(QStringLiteral("synchronous"));

    return ptr;
}

Database::Database()
    : d(new Database::Private())
{
}

Database::~Database()
{
}

QSqlQuery Database::createQuery() const
{
    return d->query();
}

QSqlQuery Database::execQuery(const QString &query) const
{
    return d->query(query);
}

QSqlQuery Database::execQueries(const QStringList &queries) const
{
    QSqlQuery result;

    for (const auto &query : queries) {
        result = execQuery(query);
    }

    return result;
}

void Database::setPragma(const QString &pragma)
{
    execQuery(QStringLiteral("PRAGMA ") + pragma);
}

QVariant Database::pragma(const QString &pragma) const
{
    return value(QStringLiteral("PRAGMA ") + pragma);
}

QVariant Database::value(const QString &query) const
{
    auto result = execQuery(query);
    return result.next() ? result.value(0) : QVariant();
}

} // namespace Common
