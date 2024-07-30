/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_FILE_H
#define BALOO_FILE_H

#include "core_export.h"
#include <KFileMetaData/Properties>

#include <memory>

namespace Baloo {

/**
 * @class File file.h <Baloo/File>
 *
 * @short Provides access to all File Metadata
 *
 * The File class acts as a temporary container for all the file metadata.
 */
class BALOO_CORE_EXPORT File
{
public:
    File();
    File(const File& f);

    /**
     * Constructor
     *
     * \p url the local url of the file
     */
    File(const QString& url);
    ~File();

    const File& operator =(const File& f);

    /**
     * The local url of the file
     */
    QString path() const;

    /**
     * Gives a variant map of the properties that have been extracted
     * from the file by the indexer
     */
    KFileMetaData::PropertyMultiMap properties() const;
    QVariant property(KFileMetaData::Property::Property property) const;

    // FIXME: More descriptive error?
    bool load();
    bool load(const QString& url);

private:
    class Private;
    std::unique_ptr<Private> const d;
};

}

#endif // BALOO_FILE_H
