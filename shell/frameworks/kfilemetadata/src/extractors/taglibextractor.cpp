/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "taglibextractor.h"
#include "embeddedimagedata.h"
#include "kfilemetadata_debug.h"

// Taglib includes
#include <taglib.h>
#include <modfile.h>
#include <s3mfile.h>
#include <xmfile.h>
#include <itfile.h>
#include <tag.h>
#include <tfilestream.h>
#include <tpropertymap.h>
#include <aifffile.h>
#include <apefile.h>
#include <apetag.h>
#include <asffile.h>
#include <flacfile.h>
#include <mp4file.h>
#include <mpcfile.h>
#include <mpegfile.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <opusfile.h>
#include <speexfile.h>
#include <vorbisfile.h>
#include <wavfile.h>
#include <wavpackfile.h>
#include <asftag.h>
#include <asfattribute.h>
#include <asfpicture.h>
#include <id3v2tag.h>
#include <mp4tag.h>
#include <popularimeterframe.h>
#include <attachedpictureframe.h>

using namespace KFileMetaData;

namespace {

const QStringList supportedMimeTypes = {
    QStringLiteral("audio/flac"),
    QStringLiteral("audio/mp4"),
    QStringLiteral("audio/mpeg"),
    QStringLiteral("audio/ogg"),
    QStringLiteral("audio/wav"),
    QStringLiteral("audio/vnd.audible.aax"),
    QStringLiteral("audio/vnd.audible.aaxc"),
    QStringLiteral("audio/vnd.wave"),
    QStringLiteral("audio/x-aiff"),
    QStringLiteral("audio/x-aifc"),
    QStringLiteral("audio/x-ape"),
    QStringLiteral("audio/x-flac+ogg"),
    QStringLiteral("audio/x-ms-wma"),
    QStringLiteral("audio/x-musepack"),
    QStringLiteral("audio/x-opus+ogg"),
    QStringLiteral("audio/x-speex+ogg"),
    QStringLiteral("audio/x-vorbis+ogg"),
    QStringLiteral("audio/x-wav"),
    QStringLiteral("audio/x-wavpack"),
    QStringLiteral("audio/x-mod"),
    QStringLiteral("audio/x-s3m"),
    QStringLiteral("audio/x-xm"),
    QStringLiteral("audio/x-it"),
};

void extractAudioProperties(TagLib::File* file, ExtractionResult* result)
{
    TagLib::AudioProperties* audioProp = file->audioProperties();
    if (audioProp && (result->inputFlags() & ExtractionResult::ExtractMetaData)) {
        if (audioProp->length()) {
            // What about the xml duration?
            result->add(Property::Duration, audioProp->length());
        }

        if (audioProp->bitrate()) {
            result->add(Property::BitRate, audioProp->bitrate() * 1000);
        }

        if (audioProp->channels()) {
            result->add(Property::Channels, audioProp->channels());
        }

        if (audioProp->sampleRate()) {
            result->add(Property::SampleRate, audioProp->sampleRate());
        }
    }
}

void readGenericProperties(const TagLib::PropertyMap &savedProperties, ExtractionResult* result)
{
    if (!(result->inputFlags() & ExtractionResult::ExtractMetaData) || savedProperties.isEmpty()) {
        return;
    }

    if (savedProperties.contains("TITLE")) {
        result->add(Property::Title, TStringToQString(savedProperties["TITLE"].toString()).trimmed());
    }
    if (savedProperties.contains("ALBUM")) {
        result->add(Property::Album, TStringToQString(savedProperties["ALBUM"].toString()).trimmed());
    }
    if (savedProperties.contains("COMMENT")) {
        result->add(Property::Comment, TStringToQString(savedProperties["COMMENT"].toString()).trimmed());
    }
    if (savedProperties.contains("TRACKNUMBER")) {
        result->add(Property::TrackNumber, savedProperties["TRACKNUMBER"].toString().toInt());
    }
    if (savedProperties.contains("DATE")) {
        result->add(Property::ReleaseYear, savedProperties["DATE"].toString().toInt());
    }
    if (savedProperties.contains("OPUS")) {
        result->add(Property::Opus, savedProperties["OPUS"].toString().toInt());
    }
    if (savedProperties.contains("DISCNUMBER")) {
        result->add(Property::DiscNumber, savedProperties["DISCNUMBER"].toString().toInt());
    }
    if (savedProperties.contains("RATING")) {
        /*
         * There is no standard regarding ratings. Mimic MediaMonkey's behavior
         * with a range of 0 to 100 (stored in steps of 10) and make it compatible
         * with baloo rating with a range from 0 to 10
         */
        result->add(Property::Rating, savedProperties["RATING"].toString().toInt() / 10);
    }
    if (savedProperties.contains("LOCATION")) {
        result->add(Property::Location, TStringToQString(savedProperties["LOCATION"].toString()).trimmed());
    }
    if (savedProperties.contains("LANGUAGE")) {
        result->add(Property::Language, TStringToQString(savedProperties["LANGUAGE"].toString()).trimmed());
    }
    if (savedProperties.contains("LICENSE")) {
        result->add(Property::License, TStringToQString(savedProperties["LICENSE"].toString()).trimmed());
    }
    if (savedProperties.contains("PUBLISHER")) {
        result->add(Property::Publisher, TStringToQString(savedProperties["PUBLISHER"].toString()).trimmed());
    }
    if (savedProperties.contains("COPYRIGHT")) {
        result->add(Property::Copyright, TStringToQString(savedProperties["COPYRIGHT"].toString()).trimmed());
    }
    if (savedProperties.contains("LABEL")) {
        result->add(Property::Label, TStringToQString(savedProperties["LABEL"].toString()).trimmed());
    }
    if (savedProperties.contains("ENSEMBLE")) {
        result->add(Property::Ensemble, TStringToQString(savedProperties["ENSEMBLE"].toString()).trimmed());
    }
    if (savedProperties.contains("COMPILATION")) {
        result->add(Property::Compilation, TStringToQString(savedProperties["COMPILATION"].toString()).trimmed());
    }
    if (savedProperties.contains("LYRICS")) {
        result->add(Property::Lyrics, TStringToQString(savedProperties["LYRICS"].toString()).trimmed().replace(QLatin1Char('\r'), QLatin1Char('\n'))); // Convert old Mac line endings
    }
    if (savedProperties.contains("ARTIST")) {
        const auto artists = savedProperties["ARTIST"];
        for (const auto& artist : artists) {
            result->add(Property::Artist, TStringToQString(artist).trimmed());
        }
    }
    if (savedProperties.contains("GENRE")) {
        const auto genres = savedProperties["GENRE"];
        for (const auto& genre : genres) {
            result->add(Property::Genre, TStringToQString(genre).trimmed());
        }
    }
    if (savedProperties.contains("ALBUMARTIST")) {
        const auto albumArtists = savedProperties["ALBUMARTIST"];
        for (const auto& albumArtist : albumArtists) {
            result->add(Property::AlbumArtist, TStringToQString(albumArtist).trimmed());
        }
    }
    if (savedProperties.contains("COMPOSER")) {
        const auto composers = savedProperties["COMPOSER"];
        for (const auto& composer : composers) {
            result->add(Property::Composer, TStringToQString(composer).trimmed());
        }
    }
    if (savedProperties.contains("LYRICIST")) {
        const auto lyricists = savedProperties["LYRICIST"];
        for (const auto& lyricist : lyricists) {
            result->add(Property::Lyricist, TStringToQString(lyricist).trimmed());
        }
    }
    if (savedProperties.contains("CONDUCTOR")) {
        const auto conductors = savedProperties["CONDUCTOR"];
        for (const auto& conductor : conductors) {
            result->add(Property::Conductor, TStringToQString(conductor).trimmed());
        }
    }
    if (savedProperties.contains("ARRANGER")) {
        const auto arrangers = savedProperties["ARRANGER"];
        for (const auto& arranger : arrangers) {
            result->add(Property::Arranger, TStringToQString(arranger).trimmed());
        }
    }
    if (savedProperties.contains("PERFORMER")) {
        const auto performers = savedProperties["PERFORMER"];
        for (const auto& performer : performers) {
            result->add(Property::Performer, TStringToQString(performer).trimmed());
        }
    }
    if (savedProperties.contains("AUTHOR")) {
        const auto authors = savedProperties["AUTHOR"];
        for (const auto& author: authors) {
            result->add(Property::Author, TStringToQString(author).trimmed());
        }
    }

    if (savedProperties.contains("REPLAYGAIN_TRACK_GAIN")) {
        auto trackGainString = TStringToQString(savedProperties["REPLAYGAIN_TRACK_GAIN"].toString(";")).trimmed();
        // remove " dB" suffix
        if (trackGainString.endsWith(QStringLiteral(" dB"), Qt::CaseInsensitive)) {
            trackGainString.chop(3);
        }
        bool success = false;
        double replayGainTrackGain = trackGainString.toDouble(&success);
        if (success) {
            result->add(Property::ReplayGainTrackGain, replayGainTrackGain);
        }
    }
    if (savedProperties.contains("REPLAYGAIN_ALBUM_GAIN")) {
        auto albumGainString = TStringToQString(savedProperties["REPLAYGAIN_ALBUM_GAIN"].toString(";")).trimmed();
        // remove " dB" suffix
        if (albumGainString.endsWith(QStringLiteral(" dB"), Qt::CaseInsensitive)) {
            albumGainString.chop(3);
        }
        bool success = false;
        double replayGainAlbumGain = albumGainString.toDouble(&success);
        if (success) {
            result->add(Property::ReplayGainAlbumGain, replayGainAlbumGain);
        }
    }
    if (savedProperties.contains("REPLAYGAIN_TRACK_PEAK")) {
        auto trackPeakString = TStringToQString(savedProperties["REPLAYGAIN_TRACK_PEAK"].toString(";")).trimmed();
        bool success = false;
        double replayGainTrackPeak = trackPeakString.toDouble(&success);
        if (success) {
            result->add(Property::ReplayGainTrackPeak, replayGainTrackPeak);
        }
    }
    if (savedProperties.contains("REPLAYGAIN_ALBUM_PEAK")) {
        auto albumPeakString = TStringToQString(savedProperties["REPLAYGAIN_ALBUM_PEAK"].toString(";")).trimmed();
        bool success = false;
        double replayGainAlbumPeak = albumPeakString.toDouble(&success);
        if (success) {
            result->add(Property::ReplayGainAlbumPeak, replayGainAlbumPeak);
        }
    }
    if (savedProperties.contains("TRACKERNAME")) {
        result->add(Property::Generator, TStringToQString(savedProperties["TRACKERNAME"].toString()).trimmed());
    }
}

void extractId3Tags(TagLib::ID3v2::Tag* Id3Tags, ExtractionResult* result)
{
    if (!(result->inputFlags() & ExtractionResult::ExtractMetaData) || Id3Tags->isEmpty()) {
        return;
    }

    TagLib::ID3v2::FrameList lstID3v2;

    /*
     * Publisher.
     * Special handling because TagLib::PropertyMap maps "TPUB" to "LABEL"
     * Insert manually for Publisher.
     */
    lstID3v2 = Id3Tags->frameListMap()["TPUB"];
    if (!lstID3v2.isEmpty()) {
        result->add(Property::Publisher, TStringToQString(lstID3v2.front()->toString()));
    }

    // Compilation.
    lstID3v2 = Id3Tags->frameListMap()["TCMP"];
    if (!lstID3v2.isEmpty()) {
        result->add(Property::Compilation, TStringToQString(lstID3v2.front()->toString()));
    }

    /*
     * Rating.
     * There is no standard regarding ratings. Most of the implementations match
     * a 5 stars rating to a range of 0-255 for MP3.
     * Map it to baloo rating with a range of 0 - 10.
     */
    lstID3v2 = Id3Tags->frameListMap()["POPM"];
    if (!lstID3v2.isEmpty()) {
        TagLib::ID3v2::PopularimeterFrame *ratingFrame = static_cast<TagLib::ID3v2::PopularimeterFrame *>(lstID3v2.front());
        int rating = ratingFrame->rating();
        if (rating == 0) {
            rating = 0;
        } else if (rating == 1) {
            TagLib::String ratingProvider = ratingFrame->email();
            if (ratingProvider == "no@email" || ratingProvider == "org.kde.kfilemetadata") {
                rating = 1;
            } else {
                rating = 2;
            }
        } else if (rating >= 1 && rating <= 255) {
            rating = static_cast<int>(0.032 * rating + 2);
        }
        result->add(Property::Rating, rating);
    }
}

template<typename ImageType>
EmbeddedImageData::ImageType mapTaglibType(const ImageType type)
{
    switch (type) {
        case ImageType::FrontCover:
            return EmbeddedImageData::FrontCover;
         case ImageType::Other:
             return EmbeddedImageData::Other;
         case ImageType::FileIcon:
              return EmbeddedImageData::FileIcon;
         case ImageType::OtherFileIcon:
              return EmbeddedImageData::OtherFileIcon;
         case ImageType::BackCover:
              return EmbeddedImageData::BackCover;
         case ImageType::LeafletPage:
              return EmbeddedImageData::LeafletPage;
         case ImageType::Media:
              return EmbeddedImageData::Media;
         case ImageType::LeadArtist:
              return EmbeddedImageData::LeadArtist;
         case ImageType::Artist:
              return EmbeddedImageData::Artist;
         case ImageType::Conductor:
              return EmbeddedImageData::Conductor;
         case ImageType::Band:
              return EmbeddedImageData::Band;
         case ImageType::Composer:
              return EmbeddedImageData::Composer;
         case ImageType::Lyricist:
              return EmbeddedImageData::Lyricist;
         case ImageType::RecordingLocation:
              return EmbeddedImageData::RecordingLocation;
         case ImageType::DuringRecording:
              return EmbeddedImageData::DuringRecording;
         case ImageType::DuringPerformance:
              return EmbeddedImageData::DuringPerformance;
         case ImageType::MovieScreenCapture:
              return EmbeddedImageData::MovieScreenCapture;
         case ImageType::ColouredFish:
              return EmbeddedImageData::ColouredFish;
         case ImageType::Illustration:
              return EmbeddedImageData::Illustration;
         case ImageType::BandLogo:
              return EmbeddedImageData::BandLogo;
         case ImageType::PublisherLogo:
              return EmbeddedImageData::PublisherLogo;
         default:
            return EmbeddedImageData::Unknown;
    }
}

QMap<EmbeddedImageData::ImageType, QByteArray>
extractId3Cover(const TagLib::ID3v2::Tag* Id3Tags,
                const EmbeddedImageData::ImageTypes types)
{
    QMap<EmbeddedImageData::ImageType, QByteArray> images;
    if (!types || Id3Tags->isEmpty()) {
        return images;
    }

    // Attached Picture.
    TagLib::ID3v2::FrameList lstID3v2 = Id3Tags->frameListMap()["APIC"];

    using PictureFrame = TagLib::ID3v2::AttachedPictureFrame;
    for (const auto& frame : std::as_const(lstID3v2)) {
        const auto *coverFrame = static_cast<PictureFrame *>(frame);
        const auto imageType = mapTaglibType<PictureFrame::Type>(coverFrame->type());
        if (types & imageType) {
            const auto& picture = coverFrame->picture();
            images.insert(imageType, QByteArray(picture.data(), picture.size()));
        }
    }
    return images;
}

QMap<EmbeddedImageData::ImageType, QByteArray>
extractFlacCover(const TagLib::List<TagLib::FLAC::Picture *> picList,
                 const EmbeddedImageData::ImageTypes types)
{
    QMap<EmbeddedImageData::ImageType, QByteArray> images;
    if (!types || picList.isEmpty()) {
        return images;
    }

    for (const auto& picture : std::as_const(picList)) {
        const auto imageType = mapTaglibType<TagLib::FLAC::Picture::Type>(picture->type());
        if (types & imageType) {
            images.insert(imageType, QByteArray(picture->data().data(), picture->data().size()));
        }
    }
    return images;
}

void extractMp4Tags(TagLib::MP4::Tag* mp4Tags, ExtractionResult* result)
{
    if (!(result->inputFlags() & ExtractionResult::ExtractMetaData) || mp4Tags->isEmpty()) {
        return;
    }

    auto ratingItem = mp4Tags->item("rate");

    /*
     * There is no standard regarding ratings. Mimic MediaMonkey's behavior
     * with a range of 0 to 100 (stored in steps of 10) and make it compatible
     * with baloo rating with a range from 0 to 10.
     */
    if (ratingItem.isValid()) {
        result->add(Property::Rating, ratingItem.toStringList().toString().toInt() / 10);
    }
}

QMap<EmbeddedImageData::ImageType, QByteArray>
extractMp4Cover(const TagLib::MP4::Tag* mp4Tags,
                const EmbeddedImageData::ImageTypes types)
{
    QMap<EmbeddedImageData::ImageType, QByteArray> images;
    TagLib::MP4::Item coverArtItem = mp4Tags->item("covr");
    if (!(types & EmbeddedImageData::FrontCover) || !coverArtItem.isValid()) {
        return images;
    }

    const TagLib::MP4::CoverArtList coverArtList = coverArtItem.toCoverArtList();
    if (!coverArtList.isEmpty()) {
        const TagLib::MP4::CoverArt& cover = coverArtList.front();
        images.insert(EmbeddedImageData::FrontCover, QByteArray(cover.data().data(), cover.data().size()));
    }
    return images;
}

QMap<EmbeddedImageData::ImageType, QByteArray>
extractApeCover(const TagLib::APE::Tag* apeTags,
                const EmbeddedImageData::ImageTypes types)
{
    QMap<EmbeddedImageData::ImageType, QByteArray> images;
    if (!(types & EmbeddedImageData::FrontCover) || apeTags->isEmpty()) {
        return images;
    }

    TagLib::APE::ItemListMap lstApe = apeTags->itemListMap();
    TagLib::APE::ItemListMap::ConstIterator itApe;

    /* The cover art tag for APEv2 tags starts with the filename as string
     * with zero termination followed by the actual picture data */
    itApe = lstApe.find("COVER ART (FRONT)");
    if (itApe != lstApe.end()) {
        const auto& picture = (*itApe).second.binaryData();
        int position = picture.find('\0');
        if (position >= 0) {
            position += 1;
            images.insert(EmbeddedImageData::FrontCover, QByteArray(picture.data() + position, picture.size() - position));
        }
    }
    return images;
}

void extractAsfTags(TagLib::ASF::Tag* asfTags, ExtractionResult* result)
{
    if (!(result->inputFlags() & ExtractionResult::ExtractMetaData) || asfTags->isEmpty()) {
        return;
    }

    TagLib::ASF::AttributeList lstASF = asfTags->attribute("WM/SharedUserRating");
    if (!lstASF.isEmpty()) {
        int rating = lstASF.front().toString().toInt();
        /*
         * Map the rating values of WMP to Baloo rating.
         * 0->0, 1->2, 25->4, 50->6, 75->8, 99->10
         */
        if (rating == 0) {
            rating = 0;
        } else if (rating == 1) {
            rating = 2;
        } else {
            rating = static_cast<int>(0.09 * rating + 2);
        }
        result->add(Property::Rating, rating);
    }

    lstASF = asfTags->attribute("Author");
    if (!lstASF.isEmpty()) {
        const auto attribute = lstASF.front();
        result->add(Property::Author, TStringToQString(attribute.toString()).trimmed());
    }

    // Lyricist is called "WRITER" for wma/asf files
    lstASF = asfTags->attribute("WM/Writer");
    if (!lstASF.isEmpty()) {
        const auto attribute = lstASF.front();
        result->add(Property::Lyricist, TStringToQString(attribute.toString()).trimmed());
    }

    /*
     * TagLib exports "WM/PUBLISHER" as "LABEL" in the PropertyMap,
     * add it manually to Publisher.
     */
    lstASF = asfTags->attribute("WM/Publisher");
    if (!lstASF.isEmpty()) {
        const auto attribute = lstASF.front();
        result->add(Property::Publisher, TStringToQString(attribute.toString()).trimmed());
    }
}

QMap<EmbeddedImageData::ImageType, QByteArray>
extractAsfCover(const TagLib::ASF::Tag* asfTags,
                const EmbeddedImageData::ImageTypes types)
{
    QMap<EmbeddedImageData::ImageType, QByteArray> images;
    if (!types || asfTags->isEmpty()) {
        return images;
    }

    // Attached Picture.
    TagLib::ASF::AttributeList lstASF = asfTags->attribute("WM/Picture");

    using Picture = TagLib::ASF::Picture;
    for (const auto& attribute: std::as_const(lstASF)) {
        Picture picture = attribute.toPicture();
        const auto imageType = mapTaglibType<Picture::Type>(picture.type());
        if (types & imageType) {
            const auto& pictureData = picture.picture();
            images.insert(imageType, QByteArray(pictureData.data(), pictureData.size()));
        }
    }
    return images;
}

} // anonymous namespace

TagLibExtractor::TagLibExtractor(QObject* parent)
    : ExtractorPlugin(parent)
{
}

QStringList TagLibExtractor::mimetypes() const
{
    return supportedMimeTypes;
}

void TagLibExtractor::extract(ExtractionResult* result)
{
    const QString fileUrl = result->inputUrl();
    const QString mimeType = getSupportedMimeType(result->inputMimetype());

    // Open the file readonly. Important if we're sandboxed.
#if defined Q_OS_WINDOWS
    TagLib::FileStream stream(fileUrl.toLocal8Bit().constData(), true);
#else
    TagLib::FileStream stream(fileUrl.toUtf8().constData(), true);
#endif
    if (!stream.isOpen()) {
        qCWarning(KFILEMETADATA_LOG) << "Unable to open file readonly: " << fileUrl;
        return;
    }

    const EmbeddedImageData::ImageTypes imageTypes{
        result->inputFlags() & ExtractionResult::ExtractImageData ? EmbeddedImageData::AllImages : 0
    };

    if (mimeType == QLatin1String("audio/mpeg")) {
        TagLib::MPEG::File file(&stream, TagLib::ID3v2::FrameFactory::instance(), true);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
            if (file.hasID3v2Tag()) {
                result->addImageData(extractId3Cover(file.ID3v2Tag(), imageTypes));
                extractId3Tags(file.ID3v2Tag(), result);
            }
        }
    } else if (mimeType == QLatin1String("audio/x-aiff") || mimeType == QLatin1String("audio/x-aifc")) {
        TagLib::RIFF::AIFF::File file(&stream, true);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
            if (file.hasID3v2Tag()) {
                result->addImageData(extractId3Cover(file.tag(), imageTypes));
                extractId3Tags(file.tag(), result);
            }
        }
    } else if (mimeType == QLatin1String("audio/wav") ||
               mimeType == QLatin1String("audio/vnd.wave") ||
               mimeType == QLatin1String("audio/x-wav")) {
        TagLib::RIFF::WAV::File file(&stream, true);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
            if (file.hasID3v2Tag()) {
                result->addImageData(extractId3Cover(file.ID3v2Tag(), imageTypes));
                extractId3Tags(file.ID3v2Tag(), result);
            }
        }
    } else if (mimeType == QLatin1String("audio/x-musepack")) {
        TagLib::MPC::File file(&stream, true);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
            if (file.APETag()) {
                result->addImageData(extractApeCover(file.APETag(), imageTypes));
            }
        }
    } else if (mimeType == QLatin1String("audio/x-ape")) {
        TagLib::APE::File file(&stream, true);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
            if (file.APETag()) {
                result->addImageData(extractApeCover(file.APETag(), imageTypes));
            }
        }
    } else if (mimeType == QLatin1String("audio/x-wavpack")) {
        TagLib::WavPack::File file(&stream, true);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
            if (file.APETag()) {
                result->addImageData(extractApeCover(file.APETag(), imageTypes));
            }
        }
    } else if ((mimeType == QLatin1String("audio/mp4")) ||
               (mimeType == QLatin1String("audio/vnd.audible.aax")) ||
               (mimeType == QLatin1String("audio/vnd.audible.aaxc"))) {
        TagLib::MP4::File file(&stream, true);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
            extractMp4Tags(file.tag(), result);
            result->addImageData(extractMp4Cover(file.tag(), imageTypes));
        }
    } else if (mimeType == QLatin1String("audio/flac")) {
        TagLib::FLAC::File file(&stream, TagLib::ID3v2::FrameFactory::instance(), true);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
            result->addImageData(extractFlacCover(file.pictureList(), imageTypes));
        }
    } else if (mimeType == QLatin1String("audio/x-flac+ogg")) {
        TagLib::Ogg::FLAC::File file(&stream, true);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
            if (file.tag()) {
                result->addImageData(extractFlacCover(file.tag()->pictureList(), imageTypes));
            }
        }
    } else if (mimeType == QLatin1String("audio/ogg") || mimeType == QLatin1String("audio/x-vorbis+ogg")) {
        TagLib::Ogg::Vorbis::File file(&stream, true);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
            if (file.tag()) {
                result->addImageData(extractFlacCover(file.tag()->pictureList(), imageTypes));
            }
        }
    } else if (mimeType == QLatin1String("audio/x-opus+ogg")) {
        TagLib::Ogg::Opus::File file(&stream, true);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
            if (file.tag()) {
                result->addImageData(extractFlacCover(file.tag()->pictureList(), imageTypes));
            }
        }
    } else if (mimeType == QLatin1String("audio/x-speex+ogg")) {
        TagLib::Ogg::Speex::File file(&stream, true);
        // Workaround for buggy taglib:
        // isValid() returns true for invalid files, but XiphComment* tag() returns a nullptr
        if (file.isValid() && file.tag()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
            result->addImageData(extractFlacCover(file.tag()->pictureList(), imageTypes));
        }
    } else if (mimeType == QLatin1String("audio/x-ms-wma")) {
        TagLib::ASF::File file(&stream, true);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
            extractAsfTags(file.tag(), result);
            TagLib::ASF::Tag* asfTags = dynamic_cast<TagLib::ASF::Tag*>(file.tag());
            if (asfTags) {
                result->addImageData(extractAsfCover(asfTags, imageTypes));
            }
        }
    } else if (mimeType == QLatin1String("audio/x-mod")) {
        TagLib::Mod::File file(&stream);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
        }
    } else if (mimeType == QLatin1String("audio/x-s3m")) {
        TagLib::S3M::File file(&stream);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
        }
    } else if (mimeType == QLatin1String("audio/x-xm")) {
        TagLib::XM::File file(&stream);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
        }
    } else if (mimeType == QLatin1String("audio/x-it")) {
        TagLib::IT::File file(&stream);
        if (file.isValid()) {
            extractAudioProperties(&file, result);
            readGenericProperties(file.properties(), result);
        }
    }

    result->addType(Type::Audio);
}

// TAG information (incomplete).
// https://xiph.org/vorbis/doc/v-comment.html
// https://help.mp3tag.de/main_tags.html
// http://id3.org/
// https://www.legroom.net/2009/05/09/ogg-vorbis-and-flac-comment-field-recommendations
// https://kodi.wiki/view/Music_tagging#Tags_Kodi_reads
// https://wiki.hydrogenaud.io/index.php?title=Tag_Mapping
// https://picard.musicbrainz.org/docs/mappings/
// -- FLAC/OGG --
// Artist:          ARTIST, PERFORMER
// Album artist:    ALBUMARTIST
// Composer:        COMPOSER
// Lyricist:        LYRICIST
// Conductor:       CONDUCTOR
// Disc number:     DISCNUMBER
// Total discs:     TOTALDISCS, DISCTOTAL
// Track number:    TRACKNUMBER
// Total tracks:    TOTALTRACKS, TRACKTOTAL
// Genre: GENRE
// -- ID3v2 --
// Artist:                          TPE1
// Album artist:                    TPE2
// Composer:                        TCOM
// Lyricist:                        TEXT
// Conductor:                       TPE3
// Disc number[/total dics]:        TPOS
// Track number[/total tracks]:     TRCK
// Genre:                           TCON

#include "moc_taglibextractor.cpp"
