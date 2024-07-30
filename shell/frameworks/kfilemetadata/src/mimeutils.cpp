/*
    This file is part of KFileMetaData
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "mimeutils.h"

namespace KFileMetaData {
namespace MimeUtils {

QMimeType strictMimeType(const QString& filePath, const QMimeDatabase& db)
{
    auto extensionMimes = db.mimeTypesForFileName(filePath);
    auto contentMime    = db.mimeTypeForFile(filePath, QMimeDatabase::MatchContent);

    if (extensionMimes.contains(contentMime)) {
        // content based mime type is one of the types for the file extension, e.g.:
        // *.ogg -> [ audio/ogg, audio/x-vorbis+ogg, ...]
        // content -> audio/x-vorbis+ogg
        return contentMime;
    }

    for (const auto &mime : extensionMimes) {
        // check if the content is generic and the extension is more specific, e.g.:
        // *.mkv -> [ video/matroska ]
        // content -> application/matroska
        if (mime.inherits(contentMime.name())) {
            return mime;
        }
    }
    // content mime type does not match the extension, trust the content
    return contentMime;
}

}} // namespace KFileMetaData::MimeUtils
