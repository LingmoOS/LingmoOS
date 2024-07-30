/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_UNINDEXEDFILEITERATOR_H
#define BALOO_UNINDEXEDFILEITERATOR_H

#include "filtereddiriterator.h"

#include <QMimeDatabase>

namespace Baloo {

class Transaction;

/**
 * Iterate over all the files (and directories) under a specific directory which require
 * indexing. This checks the following -
 * - Config include / exclude path
 * - Config filters
 * - Config mimetype filters
 * - Database for mtime differences
 */
class UnIndexedFileIterator
{
public:
    UnIndexedFileIterator(const FileIndexerConfig* config, Transaction* transaction, const QString& folder);
    ~UnIndexedFileIterator();

    QString next();
    QString filePath() const;
    QString mimetype() const;
    bool mTimeChanged() const;
    bool cTimeChanged() const;

private:
    bool shouldIndex(const QString& filePath);

    const FileIndexerConfig* m_config;
    Transaction* m_transaction;
    FilteredDirIterator m_iter;

    QMimeDatabase m_mimeDb;
    QString m_mimetype;

    bool m_mTimeChanged;
    bool m_cTimeChanged;
};

}

#endif // BALOO_UNINDEXEDFILEITERATOR_H
