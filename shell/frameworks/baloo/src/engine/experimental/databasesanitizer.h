/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2018 Michael Heidelbach <ottwolt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BALOODATABASESANITIZER_H
#define BALOODATABASESANITIZER_H

#include "transaction.h"

namespace Baloo
{
class DatabaseSanitizerImpl;
/**
* \brief Provide methods to show database problems and
* sanitize them.
*/
class BALOO_ENGINE_EXPORT DatabaseSanitizer
{
public:
    enum ItemAccessFilterFlags {
        IgnoreNone = 0,
        IgnoreAvailable = 1,
        IgnoreUnavailable = 2,
        IgnoreMounted = 8,
        IgnoreUnmounted = 0x10,
    };
    Q_DECLARE_FLAGS(ItemAccessFilters, ItemAccessFilterFlags)

public:
    DatabaseSanitizer(const Database& db, Transaction::TransactionType type);
    DatabaseSanitizer(Database* db, Transaction::TransactionType type);
    ~DatabaseSanitizer();

    /**
    * Print database content to stdout
    *
    * \p deviceIDs filter by device ids. Negative numbers list everything but...
    * with empty \p deviceIDs(default) everything is printed.
    *
    * \p missingOnly Simulate purging operation. Only inaccessible items are printed.
    *
    * \p urlFilter Filter result urls. Default is null = Print everything.
    */
    void printList(const QVector<qint64>& deviceIds,
        const ItemAccessFilters accessFilter = IgnoreNone,
        const QSharedPointer<QRegularExpression>& urlFilter = nullptr
    );
    /**
    * Print info about known devices to stdout
    *
    * \p deviceIDs filter by device ids. Negative numbers list everything but...
    * with empty \p deviceIDs(default) everything is printed.
    *
    * \p accessFilter filter by accessibility. E.g IgnoreAvailable|IgnoreUnmounted
    * prints only mounted devices with inaccessible files.
    */
    void printDevices(const QVector<qint64>& deviceIds, const ItemAccessFilters accessFilter = IgnoreNone);

    void removeStaleEntries(const QVector<qint64>& deviceIds,
        const DatabaseSanitizer::ItemAccessFilters accessFilter = DatabaseSanitizer::IgnoreNone,
        const bool dryRun = false,
        const QSharedPointer<QRegularExpression>& urlFilter = nullptr
    );

private:
    DatabaseSanitizer(const DatabaseSanitizer& rhs) = delete;
    DatabaseSanitizerImpl* m_pimpl;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DatabaseSanitizer::ItemAccessFilters)

}
#endif // BALOODATABASESANITIZER_H
