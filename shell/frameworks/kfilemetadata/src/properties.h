/*
    This file is part of KFileMetaData
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KFILEMETADATA_PROPERTIES
#define KFILEMETADATA_PROPERTIES

#include "kfilemetadata_export.h"

#include <QMap>
#include <QVariant>

namespace KFileMetaData {

/**
 * The Property namespace
 */
namespace Property {

/**
 * @brief The Property enum contains all files property types that KFileMetaData manipulates
 */
enum Property {
    Empty = 0,

    /**
     * The Bit Rate of the Audio in the File. Represented as an integer
     * in kbit/sec
     */
    BitRate,

    /**
     * The number of channels of the Audio in the File. Represented as an
     * integer.
     */
    Channels,

    /**
     * The duration of the media in the file. Represented as an integer
     * in seconds.
     */
    Duration,

    /**
     * The Genre of an Audio file. This s represented as a string
     * of genres and not integers. The IDv1 standard provides a list of
     * commonly excepted genres.
     */
    Genre,

    /**
     * The same rate or frequency of the Audio in the file. This is represented
     * as an integer in Hz. So a file with "44.1KHz" will have a frequency
     * of 44100
     */
    SampleRate,

    /**
     * Represents the track number in a set. Typically maps to the "TRCK" tag
     * in IDv3
     */
    TrackNumber,

    /**
     * Indicates the year a track was released. Represented as an integer.
     * Typically mapped to the "TYE (Year)" tag in IDv1
     */
    ReleaseYear,

    /**
     * Represents a comment stored in the file. This can map
     * to e.g. the "COMM" field from IDv3
     */
    Comment,

    /**
     * Represents the artist of a media file. This generally corresponds
     * to the IDv1 ARTIST tag. Many extractors often split this string
     * into a number of artists.
     */
    Artist,

    /**
     * Represents the album of a media file. This generally corresponds
     * to the IDv1 ALBUM tag.
     */
    Album,

    /**
     * Represents the album artist of a media file. This generally corresponds
     * to the IDv3 TPE2 ("Band/Orchestra/Accompaniment") tag.
     */
    AlbumArtist,

    /**
     * Represents the Composer of a media file. This generally corresponds
     * to the IDv2 COMPOSER tag.
     */
    Composer,

    /**
     * Represents the Lyricist of a media file. This generally corresponds
     * to the IDv2 "Lyricist/text writer" tag.
     */
    Lyricist,

    /**
     * The Author field indicated the primary creator of a document.
     * This often corresponds directly to dc:creator
     */
    Author,

    /**
     * Refers to the Title of the content of the file. This can represented
     * by the IDv1 tag TT2 (Title/songname/content description) or the TITLE
     * in a PDF file or the 'dc:title' tag in DublinCore.
     */
    Title,

    /**
     * Refers to the subject of the file. This directly corresponds to the
     * 'dc:subject' tag from DublinCore.
     */
    Subject,

    /**
     * Refers to the Application used to create this file. In the ODF standard
     * this maps to the 'meta:generator' tag. In PDFs its mapped to the
     * "Producer" tag.
     */
    Generator,

    /**
     * The number of pages in a document
     */
    PageCount,

    /**
     * The number of words in a document. This is often only provided for
     * documents where the word count is available in the metadata.
     */
    WordCount,

    /**
     * The number of lines in a document. This is often only provided for
     * documents where the line count is available in the metadata.
     */
    LineCount,

    /**
     * The language the document is written in. This directly maps to the
     * 'dc:language' tag from DublinCore. We do NOT employ any language
     * detection schemes on the text.
     * @since 5.50
     */
    Language,

    /**
     * The copyright of the file. Represented as a string.
     */
    Copyright,

    /**
     * The publisher of the content. Represented as a string.
     */
    Publisher,

    /**
     * The date the content of the file was created. This is extracted
     * from the File MetaData and not from the file system.
     * In ODF, it corresponds to "meta:creation-date", in PDF to the
     * "CreationDate" tag, and otherwise the "dcterms:created" tag.
     */
    CreationDate,

    /**
     * The keywords used to represent the document. This is mostly a string list
     * of all the keywords.
     */
    Keywords,

    /**
     * Represents the width of the Media in pixels. This is generally
     * only applicable for Images and Videos.
     */
    Width,

    /**
     * Represents the height of the Media in pixels. This is generally
     * only applicable for Images and Videos.
     */
    Height,

    /**
     * The Aspect Ratio of the visual image or video.
     * It is the width of a pixel divided by the height of the pixel.
     */
    AspectRatio,

    /**
     * Number of frames per second
     */
    FrameRate,

    /**
     * The manufacturer of the equipment used for generating the file
     * and metadata. Typically maps to the 'Exif.Image.Make' tag.
     * @since 5.60
     */
    Manufacturer,

    /**
     * The model name of the equipment used for generating the file
     * and metadata. Typically maps to the 'Exif.Image.Model' tag.
     * @since 5.60
     */
    Model,

    ImageDateTime,
    ImageOrientation,
    PhotoFlash,
    PhotoPixelXDimension,
    PhotoPixelYDimension,
    PhotoDateTimeOriginal,
    PhotoFocalLength,
    PhotoFocalLengthIn35mmFilm,
    PhotoExposureTime,
    PhotoFNumber,
    PhotoApertureValue,
    PhotoExposureBiasValue,
    PhotoWhiteBalance,
    PhotoMeteringMode,
    PhotoISOSpeedRatings,
    PhotoSaturation,
    PhotoSharpness,
    PhotoGpsLatitude,
    PhotoGpsLongitude,
    PhotoGpsAltitude,

    TranslationUnitsTotal,
    TranslationUnitsWithTranslation,
    TranslationUnitsWithDraftTranslation,
    TranslationLastAuthor,
    TranslationLastUpDate,
    TranslationTemplateDate,

    /**
     * The URL this file has originally been downloaded from.
     */
    OriginUrl,

    /**
     * The subject of the email this file was originally attached to.
     */
    OriginEmailSubject,

    /**
     * The sender of the email this file was originally attached to.
     */
    OriginEmailSender,

    /**
     * The message ID of the email this file was originally attached to.
     */
    OriginEmailMessageId,

    /**
     * Represents the disc number in a multi-disc set. Typically maps to the "TPOS" tag for mp3
     */
    DiscNumber,

    /**
     * Represents the location where an audio file was recorded.
     */
    Location,

    /**
     * Represents the (lead) performer of an audio file.
     */
    Performer,

    /**
     * Represents the ensemble of an audio file.
     */
    Ensemble,

    /**
     * Represents the arranger of an audio file.
     */
    Arranger,

    /**
     * Represents the conductor of an audio file.
     */
    Conductor,

    /**
     * Represents the opus of an audio file mostly used for classical music.
     */
    Opus,

    /**
     * Represents the label of the content.
     */
    Label,

    /**
     * Contains the name of the compilation of an audio file.
     */
    Compilation,

    /**
     * Contains the license information of the file
     */
    License,

    /**
     * For ratings stored in Metadata tags
     */
    Rating,
    /**
     * Contains the lyrics of a song embedded in the file
     */
    Lyrics,
    /**
      * Contains ReplayGain information for audio files
      */
    ReplayGainAlbumPeak,
    /**
      * Contains ReplayGain information for audio files
      * The album gain is given in "dB"
      */
    ReplayGainAlbumGain,
    /**
      * Contains ReplayGain information for audio files
      */
    ReplayGainTrackPeak,
    /**
      * Contains ReplayGain information for audio files
      * The track gain is given in "dB"
      */
    ReplayGainTrackGain,

    /**
     * Represents the description stored in the file. This maps
     * to the 'dc:description' tag from DublinCore
     */
    Description,
};

} // namespace Property

using PropertyMultiMap = QMultiMap<Property::Property, QVariant>;

} // namespace KFileMetaData

Q_DECLARE_METATYPE(KFileMetaData::Property::Property)
Q_DECLARE_METATYPE(KFileMetaData::PropertyMultiMap)

#endif
