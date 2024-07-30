/*
    This file is part of the KDE Project
    SPDX-FileCopyrightText: 2009-2011 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2013-2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BALOO_METADATA_MOVER_H_
#define BALOO_METADATA_MOVER_H_

#include <QObject>

namespace Baloo
{

class Database;
class Transaction;

class MetadataMover : public QObject
{
    Q_OBJECT

public:
    explicit MetadataMover(Database* db, QObject* parent = nullptr);
    ~MetadataMover() override;

public Q_SLOTS:
    void moveFileMetadata(const QString& from, const QString& to);
    void removeFileMetadata(const QString& file);

Q_SIGNALS:
    /**
     * Emitted for files (and folders) that have been moved but
     * do not have metadata to be moved. This allows the file indexer
     * service to pick them up in case they are of interest. The
     * typical example would be moving a file from a non-indexed into
     * an indexed folder.
     */
    void movedWithoutData(const QString& path);

    void fileRemoved(const QString& path);

private:
    /**
     * Remove the metadata for file \p url
     */
    void removeMetadata(Transaction* tr, const QString& url);

    /**
     * Recursively update the nie:url and nie:isPartOf properties
     * of the resource describing \p from.
     */
    void updateMetadata(Transaction* tr, const QString& from, const QString& to);

    Database* m_db;
};
}

#endif
