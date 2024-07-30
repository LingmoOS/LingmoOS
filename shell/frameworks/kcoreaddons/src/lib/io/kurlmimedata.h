/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2005-2012 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KURLMIMEDATA_H
#define KURLMIMEDATA_H

#include "kcoreaddons_export.h"
#include <QFlags>
#include <QMap>
#include <QUrl>
QT_BEGIN_NAMESPACE
class QMimeData;
QT_END_NAMESPACE

/**
 * Utility functions for using URLs in QMimeData.
 * In addition to QMimeData::setUrls() and QMimeData::urls(), these functions allow to:
 *
 * - Store two sets of URLs, the KDE-specific URLs and the equivalent local-file URLs
 *     for compatibility with non-KDE applications
 * - Store KIO metadata, such as the HTTP referrer for a given URL (some websites
 *     require it for downloading e.g. an image)
 *
 * @since 5.0
 */
namespace KUrlMimeData
{
typedef QMap<QString, QString> MetaDataMap;

/**
 * Adds URLs and KIO metadata into the given QMimeData.
 *
 * WARNING: do not call this method multiple times on the same mimedata object,
 * you can add urls only once. But you can add other things, e.g. images, XML...
 *
 * @param mimeData the QMimeData instance used to drag or copy this URL
 */
KCOREADDONS_EXPORT void setUrls(const QList<QUrl> &urls, const QList<QUrl> &mostLocalUrls, QMimeData *mimeData);

/**
 * Export URLs through the XDG Documents Portal to allow interaction from/with sandbox code.
 * This implements the application/vnd.portal.filetransfer mimetype extension.
 * When built without dbus support or the portal isn't installed on the target system, then this
 * is no-op and returns false.
 * Remote URLs are automatically mounted into the file system using kio-fuse
 * @returns whether all URLS were exported through the portal
 */
KCOREADDONS_EXPORT bool exportUrlsToPortal(QMimeData *mimeData);

/**
 * @param metaData KIO metadata shipped in the mime data, which is used for instance to
 * set a correct HTTP referrer (some websites require it for downloading e.g. an image)
 */
KCOREADDONS_EXPORT void setMetaData(const MetaDataMap &metaData, QMimeData *mimeData);

/**
 * Return the list of mimeTypes that can be decoded by urlsFromMimeData
 */
KCOREADDONS_EXPORT QStringList mimeDataTypes();

/**
 * Flags to be used in urlsFromMimeData.
 */
enum DecodeOption {
    /**
     * When the mimedata contains both KDE-style URLs (eg: desktop:/foo) and
     * the "most local" version of the URLs (eg: file:///home/dfaure/Desktop/foo),
     * decode it as the KDE-style URL. Useful in DnD code e.g. when moving icons,
     * and the kde-style url is used as identifier for the icons.
     */
    PreferKdeUrls = 0,
    /**
     * When the mimedata contains both KDE-style URLs (eg: desktop:/foo) and
     * the "most local" version of the URLs (eg: file:///home/dfaure/Desktop/foo),
     * decode it as local urls. Useful in paste/drop operations that end up calling KIO,
     * so that urls from other users work as well.
     */
    PreferLocalUrls = 1,
};
Q_DECLARE_FLAGS(DecodeOptions, DecodeOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(DecodeOptions)

/**
 * Extract a list of urls from the contents of @p mimeData.
 *
 * Compared to QMimeData::urls(), this method has support for retrieving KDE-specific URLs
 * when urls() would retrieve "most local URLs" instead as well as support for the XDG Documents Portal
 * via the application/vnd.portal.filetransfer mimedata extension.
 *
 * Decoding will fail if @p mimeData does not contain any URLs, or if at
 * least one extracted URL is not valid.
 *
 * When application/vnd.portal.filetransfer is set you'll only receive URLs retrieved from the XDG Documents Portal.
 * When the portal is not available application/vnd.portal.filetransfer gets ignored.
 *
 * @param mimeData the mime data to extract from; cannot be 0
 * @param decodeOptions options for decoding
 * @param metaData optional pointer to a map which will hold the metadata after this call
 * @return the list of urls
 */
KCOREADDONS_EXPORT QList<QUrl> urlsFromMimeData(const QMimeData *mimeData, DecodeOptions decodeOptions = PreferKdeUrls, MetaDataMap *metaData = nullptr);
}

#endif /* KURLMIMEDATA_H */
