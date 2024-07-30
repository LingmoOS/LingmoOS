/*
    This file is part of KFileMetaData
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KFILEMETADATA_TYPES
#define KFILEMETADATA_TYPES

#include <QVariant>

/**
 * The KFileMetaData namespace
 */
namespace KFileMetaData {

/**
 * The Type namespace
 */
namespace Type {

/**
 * A Type represents a way to represent a way to group files based on
 * a higher level view, which the user generally expects.
 *
 * Every extractor provides a list of types applicable for each file.
 */
enum Type {
    FirstType = 0,
    Empty = 0,

    /**
     * A file which contains a compressed collection of other files
     * eg - tar, zip, rar, gz
     * File types which use e.g. zip as an implementation detail -
     * Java JAR files, OpenDocument formats, etc. - are not treated
     * as Archive.
     */
    Archive,

    /**
     * Used to mark any file which just contains audio. Do not use this
     * type if the file also contains Video
     */
    Audio,

    /**
     * Any file which contains Video. It may also contain Audio
     */
    Video,

    /**
     * Any Image file. This includes both raster and vector formats.
     */
    Image,

    /**
     * Any file which counts as a document. Documents are generally
     * files which contain rich text, formatting and maybe images,
     * but also just plain text.
     */
    Document,

    /**
     * A SpreadSheet file. This is a specialization of the Document type
     * Any file which has this type should also have the Document type
     */
    Spreadsheet,

    /**
     * A Presentation file. This is a specialization of the Document type.
     * Any file which has this type should also have the Document type
     */
    Presentation,

    /**
     * Any file which contains text data (i.e. not binary data) counts as
     * a Text file. As this also includes various file types which contain
     * data collections (e.g. csv, json) or code (e.g. QML, Makefile),
     * "Document" is typically the more appropriate type.
     */
    Text,

    /**
     * A directory or folder
     */
    Folder,

    LastType = Folder
};

} // namespace Type
} // namespace KFileMetaData

Q_DECLARE_METATYPE(KFileMetaData::Type::Type)

#endif
