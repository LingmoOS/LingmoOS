/*
    This file is part of the KFileMetaData project
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>
    SPDX-FileCopyrightText: 2016 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KFILEMETADTA_WRITER_H
#define KFILEMETADTA_WRITER_H

#include "kfilemetadata_export.h"
#include <QStringList>

#include <memory>

namespace KFileMetaData
{

class WriteData;
class WriterCollection;
class WriterPrivate;

/**
 * \class Writer writer.h <KFileMetaData/Writer>
 *
 * \brief The Writer class is used to write data to a file.
 */
class KFILEMETADATA_EXPORT Writer
{
    enum WriterPluginOwnership {
        AutoDeletePlugin,
        DoNotDeletePlugin,
    };

public:
    Writer(Writer&&);
    virtual ~Writer();

    void write(const WriteData& data);
    QStringList mimetypes() const;

private:
    KFILEMETADATA_NO_EXPORT Writer();

    Writer(const Writer&) = delete;
    void operator =(const Writer&) = delete;

    KFILEMETADATA_NO_EXPORT void setAutoDeletePlugin(WriterPluginOwnership autoDelete);

    std::unique_ptr<WriterPrivate> d;
    friend class WriterPrivate;
    friend class WriterCollectionPrivate;
};
}

#endif // KFILEMETADTA_WRITER_H
