/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2016 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "database.h"
#include "transaction.h"
#include "postingdb.h"
#include "documentdb.h"
#include "documenturldb.h"
#include "documentiddb.h"
#include "positiondb.h"
#include "documenttimedb.h"
#include "documentdatadb.h"
#include "mtimedb.h"

#include "enginequery.h"

#include "andpostingiterator.h"
#include "orpostingiterator.h"
#include "phraseanditerator.h"

#include "writetransaction.h"
#include "idutils.h"
#include "fsutils.h"

#include "enginedebug.h"

// MSVC does not understand the inline assembly in valgrind.h
// Defining NVALGRIND stubs out all definitions, so we can use
// the macros without ifdef'ing these in place
#if defined _MSC_VER && !defined NVALGRIND
#define NVALGRIND 1
#endif
#include "valgrind.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMutexLocker>

using namespace Baloo;

Database::Database(const QString& path)
    : m_path(path)
    , m_env(nullptr)
{
}

Database::~Database()
{
    // try only to close if we did open the DB successfully
    if (m_env) {
        mdb_env_close(m_env);
        m_env = nullptr;
    }
}

bool Database::open(OpenMode mode)
{
    QMutexLocker locker(&m_mutex);

    // nop if already open!
    if (m_env) {
        return true;
    }

    QDir dir(m_path);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
        dir.refresh();
    }
    QFileInfo indexInfo(dir, QStringLiteral("index"));

    if ((mode != CreateDatabase) && !indexInfo.exists()) {
        return false;
    }

    if (mode == CreateDatabase) {
        if (!QFileInfo(dir.absolutePath()).permission(QFile::WriteOwner)) {
            qCCritical(ENGINE) << m_path << "does not have write permissions. Aborting";
            return false;
        }

        if (!indexInfo.exists()) {
            FSUtils::disableCoW(m_path);
        }
    }

    int rc = mdb_env_create(&m_env);
    if (rc) {
        m_env = nullptr;
        return false;
    }

    /**
     * maximal number of allowed named databases, must match number of databases we create below
     * each additional one leads to overhead
     */
    mdb_env_set_maxdbs(m_env, 12);

    /**
     * size limit for database == size limit of mmap
     * use 1 GB on 32-bit, use 256 GB on 64-bit
     * Valgrind by default (without recompiling) limits the mmap size:
     * <= 3.9: 32 GByte, 3.9 to 3.12: 64 GByte, 3.13: 128 GByte
     */
    size_t sizeInGByte = 256;
    if (sizeof(void*) == 4) {
        sizeInGByte = 1;
        qCWarning(ENGINE) << "Running on 32 bit arch, limiting DB mmap to" << sizeInGByte << "GByte";
    } else if (RUNNING_ON_VALGRIND) {
        // valgrind lacks a runtime version check, assume valgrind >= 3.9, and allow for some other mmaps
        sizeInGByte = 40;
        qCWarning(ENGINE) << "Valgrind detected, limiting DB mmap to" << sizeInGByte << "GByte";
    }
    const size_t maximalSizeInBytes = sizeInGByte * size_t(1024) * size_t(1024) * size_t(1024);
    mdb_env_set_mapsize(m_env, maximalSizeInBytes);

    // Set MDB environment flags
    auto mdbEnvFlags = MDB_NOSUBDIR | MDB_NOMEMINIT;
    if (mode == ReadOnlyDatabase) {
        mdbEnvFlags |= MDB_RDONLY;
    }

    // The directory needs to be created before opening the environment
    QByteArray arr = QFile::encodeName(indexInfo.absoluteFilePath());
    rc = mdb_env_open(m_env, arr.constData(), mdbEnvFlags, 0664);
    if (rc) {
        mdb_env_close(m_env);
        m_env = nullptr;
        return false;
    }

    rc = mdb_reader_check(m_env, nullptr);

    if (rc) {
        qCWarning(ENGINE) << "Database::open reader_check" << mdb_strerror(rc);
        mdb_env_close(m_env);
        m_env = nullptr;
        return false;
    }

    //
    // Individual Databases
    //
    MDB_txn* txn;
    if (mode != CreateDatabase) {
        int rc = mdb_txn_begin(m_env, nullptr, MDB_RDONLY, &txn);
        if (rc) {
            qCWarning(ENGINE) << "Database::transaction ro begin" << mdb_strerror(rc);
            mdb_env_close(m_env);
            m_env = nullptr;
            return false;
        }

        m_dbis.postingDbi = PostingDB::open(txn);
        m_dbis.positionDBi = PositionDB::open(txn);

        m_dbis.docTermsDbi = DocumentDB::open("docterms", txn);
        m_dbis.docFilenameTermsDbi = DocumentDB::open("docfilenameterms", txn);
        m_dbis.docXattrTermsDbi = DocumentDB::open("docxatrrterms", txn);

        m_dbis.idTreeDbi = IdTreeDB::open(txn);
        m_dbis.idFilenameDbi = IdFilenameDB::open(txn);

        m_dbis.docTimeDbi = DocumentTimeDB::open(txn);
        m_dbis.docDataDbi = DocumentDataDB::open(txn);

        m_dbis.contentIndexingDbi = DocumentIdDB::open("indexingleveldb", txn);
        m_dbis.failedIdDbi = DocumentIdDB::open("failediddb", txn);

        m_dbis.mtimeDbi = MTimeDB::open(txn);

        if (!m_dbis.isValid()) {
            qCWarning(ENGINE) << "dbis is invalid";
            mdb_txn_abort(txn);
            mdb_env_close(m_env);
            m_env = nullptr;
            return false;
        }

        rc = mdb_txn_commit(txn);
        if (rc) {
            qCWarning(ENGINE) << "Database::transaction ro commit" << mdb_strerror(rc);
            mdb_env_close(m_env);
            m_env = nullptr;
            return false;
        }
    } else {
        int rc = mdb_txn_begin(m_env, nullptr, 0, &txn);
        if (rc) {
            qCWarning(ENGINE) << "Database::transaction begin" << mdb_strerror(rc);
            mdb_env_close(m_env);
            m_env = nullptr;
            return false;
        }

        m_dbis.postingDbi = PostingDB::create(txn);
        m_dbis.positionDBi = PositionDB::create(txn);

        m_dbis.docTermsDbi = DocumentDB::create("docterms", txn);
        m_dbis.docFilenameTermsDbi = DocumentDB::create("docfilenameterms", txn);
        m_dbis.docXattrTermsDbi = DocumentDB::create("docxatrrterms", txn);

        m_dbis.idTreeDbi = IdTreeDB::create(txn);
        m_dbis.idFilenameDbi = IdFilenameDB::create(txn);

        m_dbis.docTimeDbi = DocumentTimeDB::create(txn);
        m_dbis.docDataDbi = DocumentDataDB::create(txn);

        m_dbis.contentIndexingDbi = DocumentIdDB::create("indexingleveldb", txn);
        m_dbis.failedIdDbi = DocumentIdDB::create("failediddb", txn);

        m_dbis.mtimeDbi = MTimeDB::create(txn);

        if (!m_dbis.isValid()) {
            qCWarning(ENGINE) << "dbis is invalid";
            mdb_txn_abort(txn);
            mdb_env_close(m_env);
            m_env = nullptr;
            return false;
        }

        rc = mdb_txn_commit(txn);
        if (rc) {
            qCWarning(ENGINE) << "Database::transaction commit" << mdb_strerror(rc);
            mdb_env_close(m_env);
            m_env = nullptr;
            return false;
        }
    }

    Q_ASSERT(m_env);
    return true;
}

bool Database::isOpen() const
{
    QMutexLocker locker(&m_mutex);
    return m_env != nullptr;
}

QString Database::path() const
{
    QMutexLocker locker(&m_mutex);
    return m_path;
}

bool Database::isAvailable() const
{
    QMutexLocker locker(&m_mutex);
    return QFileInfo::exists(m_path + QStringLiteral("/index"));
}
