/*
    This file is part of KFileMetaData
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef LINGMOFILEMETADATA_TYPES
#define LINGMOFILEMETADATA_TYPES

namespace LingmoUIFileMetadata {
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
     * Any file which contains a compressed collection of other files
     * eg - tar, zip, rar, gz
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
     * files which contain rich text, formatting and maybe images
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
     * Any file which just contains plain text data counts
     * as a Text file
     */
    Text,

    /**
     * A directory or folder
     */
    Folder,

    LastType = Folder
};

}
}

#endif
