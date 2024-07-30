/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_KIO_TAGS_H_
#define BALOO_KIO_TAGS_H_

#include <KFileMetaData/UserMetaData>
#include <KIO/ForwardingWorkerBase>

#include "query.h"

namespace Baloo
{

class TagsProtocol : public KIO::ForwardingWorkerBase
{
    Q_OBJECT
public:
    TagsProtocol(const QByteArray& pool_socket, const QByteArray& app_socket);
    ~TagsProtocol() override;

    enum UrlType {
        InvalidUrl,
        FileUrl,
        TagUrl,
    };
    Q_ENUM(UrlType)

    /**
     * List all files and folders tagged with the corresponding tag, along with
     * additional tags that can be used to filter the results
     */
    KIO::WorkerResult listDir(const QUrl& url) override;

    /**
     * Will be forwarded for files.
     */
    KIO::WorkerResult get(const QUrl& url) override;

    /**
     * Files and folders can be copied to the virtual folders resulting
     * is assignment of the corresponding tag.
     */
    KIO::WorkerResult copy(const QUrl& src, const QUrl& dest, int permissions, KIO::JobFlags flags) override;

    /**
     * File renaming will be forwarded.
     * Folder renaming results in renaming of the tag.
     */
    KIO::WorkerResult rename(const QUrl& src, const QUrl& dest, KIO::JobFlags flags) override;

    /**
     * File deletion means remocing the tag
     * Folder deletion will result in deletion of the tag.
     */
    KIO::WorkerResult del(const QUrl& url, bool isfile) override;

    /**
     * Files will be forwarded.
     * Tags will be created as virtual folders.
     */
    KIO::WorkerResult mimetype(const QUrl& url) override;

    /**
     * Virtual folders will be created.
     */
    KIO::WorkerResult mkdir(const QUrl& url, int permissions) override;

    /**
     * Files will be forwarded.
     * Tags will be created as virtual folders.
     */
    KIO::WorkerResult stat(const QUrl& url) override;

protected:
    bool rewriteUrl(const QUrl& url, QUrl& newURL) override;

private:
    enum ParseFlags {
        ChopLastSection,
        LazyValidation,
    };

    struct ParseResult {
        UrlType urlType = InvalidUrl;
        QString decodedUrl;
        QString tag;
        QUrl fileUrl;
        KFileMetaData::UserMetaData metaData = KFileMetaData::UserMetaData(QString());
        Query query;
        KIO::UDSEntryList pathUDSResults;
    };

    ParseResult parseUrl(const QUrl& url, const QList<ParseFlags> &flags = QList<ParseFlags>());
    QStringList m_unassignedTags;
};
}

#endif // BALOO_KIO_TAGS_H_
