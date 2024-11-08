/*
    EmbeddedImageData extracts binary data of cover art files.
    SPDX-FileCopyrightText: 2018 Alexander Stippich <a.stippich@gmx.net>
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef LINGMOFILEMETADATA_EMBEDDEDIMAGEDATA_H
#define LINGMOFILEMETADATA_EMBEDDEDIMAGEDATA_H

#include "lingmo-file-metadata_global.h"
#include <QByteArray>
#include <QFlags>
#include <QMap>
#include <QMetaType>
#include <memory>

namespace LingmoUIFileMetadata {

/**
 * \class EmbeddedImageData embeddedimagedata.h
 *
 * \brief EmbeddedImageData defines enums for different image types that can
 * be extracted from the metadata of e.g. music files
 */
class LINGMOFILEMETADATA_EXPORT EmbeddedImageData {
public:
    EmbeddedImageData();
    virtual ~EmbeddedImageData();
    /**
     * @see ImageTypes
     */
    enum ImageType {
        FrontCover = 1 << 0x0,
        Other = 1 << 0x01,
        FileIcon = 1 << 0x02,
        OtherFileIcon = 1 << 0x03,
        BackCover = 1 << 0x04,
        LeafletPage = 1 << 0x05,
        Media = 1 << 0x06,
        LeadArtist = 1 << 0x07,
        Artist = 1 << 0x08,
        Conductor = 1 << 0x09,
        Band = 1 << 0x0A,
        Composer = 1 << 0x0B,
        Lyricist = 1 << 0x0C,
        RecordingLocation = 1 << 0x0D,
        DuringRecording = 1 << 0x0E,
        DuringPerformance = 1 << 0x0F,
        MovieScreenCapture = 1 << 0x10,
        ColouredFish = 1 << 0x11,
        Illustration = 1 << 0x12,
        BandLogo = 1 << 0x13,
        PublisherLogo = 1 << 0x14,
        Unknown = 1 << 30,
        AllImages = 0x7fffffff
    };
    /**
     * Stores a combination of #ImageType values.
     */
    Q_DECLARE_FLAGS(ImageTypes, ImageType)


private:
    class EmbeddedImageDataPrivate;
    EmbeddedImageDataPrivate *d; // Placeholder for future binary compatible extensions
    EmbeddedImageData& operator=(const EmbeddedImageData&);
};

}

Q_DECLARE_METATYPE(LingmoUIFileMetadata::EmbeddedImageData::ImageType)
Q_DECLARE_METATYPE(LingmoUIFileMetadata::EmbeddedImageData::ImageTypes)

#endif // LINGMOFILEMETADATA_EMBEDDEDIMAGEDATA_H
