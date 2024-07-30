/*
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>
    SPDX-FileCopyrightText: 2018 Alexander Stippich <a.stippich@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "taglibwriter.h"
#include "embeddedimagedata.h"
#include "kfilemetadata_debug.h"

#include <array>

#include <taglib.h>
#include <tfilestream.h>
#include <tpropertymap.h>
#include <tstring.h>
#include <aifffile.h>
#include <apefile.h>
#include <apetag.h>
#include <asffile.h>
#include <asftag.h>
#include <flacfile.h>
#include <mp4file.h>
#include <mp4tag.h>
#include <mpcfile.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <oggfile.h>
#include <opusfile.h>
#include <vorbisfile.h>
#include <speexfile.h>
#include <wavpackfile.h>
#include <wavfile.h>
#include <popularimeterframe.h>
#include <attachedpictureframe.h>

namespace {

const QStringList supportedMimeTypes = {
    QStringLiteral("audio/flac"),
    QStringLiteral("audio/mp4"),
    QStringLiteral("audio/mpeg"),
    QStringLiteral("audio/ogg"),
    QStringLiteral("audio/wav"),
    QStringLiteral("audio/vnd.wave"),
    QStringLiteral("audio/x-aiff"),
    QStringLiteral("audio/x-aifc"),
    QStringLiteral("audio/x-ape"),
    QStringLiteral("audio/x-ms-wma"),
    QStringLiteral("audio/x-musepack"),
    QStringLiteral("audio/x-opus+ogg"),
    QStringLiteral("audio/x-speex+ogg"),
    QStringLiteral("audio/x-vorbis+ogg"),
    QStringLiteral("audio/x-wav"),
    QStringLiteral("audio/x-wavpack"),
};

int id3v2RatingTranslation[11] = {
    0, 1, 13, 54, 64, 118, 128, 186, 196, 242, 255
};

using namespace KFileMetaData;

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

template<typename ImageType>
static const std::array<typename ImageType::Type, 21> allImageTypes = {
    ImageType::FrontCover,
    ImageType::Other,
    ImageType::FileIcon,
    ImageType::OtherFileIcon,
    ImageType::BackCover,
    ImageType::LeafletPage,
    ImageType::Media,
    ImageType::LeadArtist,
    ImageType::Artist,
    ImageType::Conductor,
    ImageType::Band,
    ImageType::Composer,
    ImageType::Lyricist,
    ImageType::RecordingLocation,
    ImageType::DuringRecording,
    ImageType::DuringPerformance,
    ImageType::MovieScreenCapture,
    ImageType::ColouredFish,
    ImageType::Illustration,
    ImageType::BandLogo,
    ImageType::PublisherLogo,
};

TagLib::String determineMimeType(const QByteArray &pictureData)
{
    if (pictureData.startsWith(QByteArray::fromHex("89504E470D0A1A0A"))) {
        return TagLib::String("image/png");
    } else if (pictureData.startsWith(QByteArray::fromHex("FFD8FFDB")) ||
               pictureData.startsWith(QByteArray::fromHex("FFD8FFE000104A4649460001")) ||
               pictureData.startsWith(QByteArray::fromHex("FFD8FFEE")) ||
               pictureData.startsWith(QByteArray::fromHex("FFD8FFE1"))) {
        return TagLib::String("image/jpeg");
    } else {
        return TagLib::String();
    }
}

void writeID3v2Tags(TagLib::ID3v2::Tag *id3Tags, const PropertyMultiMap &newProperties)
{
    if (newProperties.contains(Property::Rating)) {
        int rating = newProperties.value(Property::Rating).toInt();
        if (rating >= 0 && rating <= 10) {
            id3Tags->removeFrames("POPM");
            // ID3v2::Tag::addFrame takes ownership
            auto ratingFrame = new TagLib::ID3v2::PopularimeterFrame;
            ratingFrame->setEmail("org.kde.kfilemetadata");
            ratingFrame->setRating(id3v2RatingTranslation[rating]);
            id3Tags->addFrame(ratingFrame);
        }
    }
}

void writeID3v2Cover(TagLib::ID3v2::Tag *id3Tags,
                     const QMap<EmbeddedImageData::ImageType, QByteArray> images)
{
    EmbeddedImageData::ImageTypes wantedTypes;
    EmbeddedImageData::ImageTypes removeTypes;
    std::for_each(images.keyValueBegin(),images.keyValueEnd(),
        [&](const std::pair<EmbeddedImageData::ImageType, QByteArray> it) {
            if (it.second.isEmpty()) {
                removeTypes |= it.first;
            } else {
                wantedTypes |= it.first;
            }
        });

    using PictureFrame = TagLib::ID3v2::AttachedPictureFrame;
    auto updateFrame = [&wantedTypes, &images](PictureFrame* coverFrame, const EmbeddedImageData::ImageType kfmType) {
        wantedTypes &= ~kfmType;
        auto newCover = images[kfmType];
        auto newMimeType = determineMimeType(newCover);
        if (!newMimeType.isEmpty()) {
            coverFrame->setPicture(TagLib::ByteVector(static_cast<const char *>(newCover.data()), newCover.size()));
            coverFrame->setMimeType(newMimeType);
        }
    };

    // Update existing covers
    TagLib::ID3v2::FrameList lstID3v2 = id3Tags->frameListMap()["APIC"];
    for (auto& frame : std::as_const(lstID3v2)) {
        auto* coverFrame = static_cast<PictureFrame *>(frame);
        const auto kfmType = mapTaglibType<PictureFrame::Type>(coverFrame->type());
        if (kfmType & wantedTypes) {
            updateFrame(coverFrame, kfmType);
        } else if (kfmType & removeTypes) {
            id3Tags->removeFrame(coverFrame);
        }
    }
    // Add new covers
    for (const auto type : allImageTypes<PictureFrame>) {
        const auto kfmType = mapTaglibType<PictureFrame::Type>(type);
        if (kfmType & wantedTypes) {
            // ID3v2::Tag::addFrame takes ownership
            auto* coverFrame = new PictureFrame;
            coverFrame->setType(type);
            updateFrame(coverFrame, kfmType);
            id3Tags->addFrame(coverFrame);
        }
    }
}

// Instantiated for either FLAC::File or
// Ogg::XiphComment (Ogg::*::File::tag())
template<typename Container>
void writeFlacCover(Container* tags,
                    const QMap<EmbeddedImageData::ImageType, QByteArray> images)
{
    EmbeddedImageData::ImageTypes wantedTypes;
    EmbeddedImageData::ImageTypes removeTypes;
    std::for_each(images.keyValueBegin(),images.keyValueEnd(),
        [&](const std::pair<EmbeddedImageData::ImageType, QByteArray> it) {
            if (it.second.isEmpty()) {
                removeTypes |= it.first;
            } else {
                wantedTypes |= it.first;
            }
        });

    using PictureFrame = TagLib::FLAC::Picture;
    auto updateFrame = [&wantedTypes, &images](PictureFrame* coverFrame, const EmbeddedImageData::ImageType kfmType) {
        wantedTypes &= ~kfmType;
        auto newCover = images[kfmType];
        auto newMimeType = determineMimeType(newCover);
        if (!newMimeType.isEmpty()) {
            coverFrame->setData(TagLib::ByteVector(static_cast<const char *>(newCover.data()), newCover.size()));
            coverFrame->setMimeType(newMimeType);
        }
    };

    // Update existing covers
    auto lstPic = tags->pictureList();
    for (auto it = lstPic.begin(); it != lstPic.end(); ++it) {
        const auto kfmType = mapTaglibType<PictureFrame::Type>((*it)->type());
        if (kfmType & wantedTypes) {
            updateFrame((*it), kfmType);
        } else if (kfmType & removeTypes) {
            tags->removePicture(*it);
        }
    }
    // Add new covers
    for (const auto type : allImageTypes<PictureFrame>) {
        const auto kfmType = mapTaglibType<PictureFrame::Type>(type);
        if (kfmType & wantedTypes) {
            // FLAC::File::addPicture takes ownership (dito XiphComment)
            auto* coverFrame = new PictureFrame;
            coverFrame->setType(type);
            updateFrame(coverFrame, kfmType);
            tags->addPicture(coverFrame);
        }
    }
}

void writeApeTags(TagLib::PropertyMap &oldProperties, const PropertyMultiMap &newProperties)
{
    if (newProperties.contains(Property::Rating)) {
        oldProperties.replace("RATING", TagLib::String::number(newProperties.value(Property::Rating).toInt() * 10));
    }
}

void writeApeCover(TagLib::APE::Tag* apeTags,
                   const QMap<EmbeddedImageData::ImageType, QByteArray> images)
{
    if (images.empty()) {
        return;
    }
    auto imageIt = images.constFind(EmbeddedImageData::FrontCover);
    if ((images.size() > 1) || (imageIt == images.constEnd())) {
        // TODO warn
    }
    if (imageIt == images.constEnd()) {
        return;
    }

    const auto newCover = *imageIt;
    if (newCover.isEmpty()) {
        apeTags->removeItem("COVER ART (FRONT)");
        return;
    }

    TagLib::ByteVector imageData;
    if (determineMimeType(newCover) == TagLib::String("image/png")) {
        imageData.setData("frontCover.png\0", 15);
    } else {
        imageData.setData("frontCover.jpeg\0", 16);
    }
    imageData.append(TagLib::ByteVector(newCover.constData(), newCover.size()));
    apeTags->setData("COVER ART (FRONT)", imageData);
}

void writeVorbisTags(TagLib::PropertyMap &oldProperties, const PropertyMultiMap &newProperties)
{
    if (newProperties.contains(Property::Rating)) {
        oldProperties.replace("RATING", TagLib::String::number(newProperties.value(Property::Rating).toInt() * 10));
    }
}

void writeAsfTags(TagLib::ASF::Tag *asfTags, const PropertyMultiMap &properties)
{
    if (properties.contains(Property::Rating)) {
        //map the rating values of WMP to Baloo rating
        //0->0, 1->2, 4->25, 6->50, 8->75, 10->99
        int rating = properties.value(Property::Rating).toInt();
        if (rating == 0) {
            rating = 0;
        } else if (rating <= 2) {
            rating = 1;
        } else if (rating == 10){
            rating = 99;
        } else {
            rating = static_cast<int>(12.5 * rating - 25);
        }
        asfTags->setAttribute("WM/SharedUserRating", TagLib::String::number(rating));
    }
}

void writeAsfCover(TagLib::ASF::Tag* asfTags,
                   const QMap<EmbeddedImageData::ImageType, QByteArray> images)
{
    EmbeddedImageData::ImageTypes wantedTypes;
    EmbeddedImageData::ImageTypes removeTypes;
    std::for_each(images.keyValueBegin(),images.keyValueEnd(),
        [&](const std::pair<EmbeddedImageData::ImageType, QByteArray> it) {
            if (it.second.isEmpty()) {
                removeTypes |= it.first;
            } else {
                wantedTypes |= it.first;
            }
        });

    using PictureFrame = TagLib::ASF::Picture;
    auto updateFrame = [&wantedTypes, &images](PictureFrame* coverFrame, const EmbeddedImageData::ImageType kfmType) {
        wantedTypes &= ~kfmType;
        auto newCover = images[kfmType];
        auto newMimeType = determineMimeType(newCover);
        if (!newMimeType.isEmpty()) {
            coverFrame->setPicture(TagLib::ByteVector(static_cast<const char *>(newCover.data()), newCover.size()));
            coverFrame->setMimeType(newMimeType);
        }
    };

    // Update existing covers
    TagLib::ASF::AttributeList lstPic = asfTags->attribute("WM/Picture");
    for (auto it = lstPic.begin(); it != lstPic.end(); ) {
        PictureFrame picture = (*it).toPicture();
        const auto kfmType = mapTaglibType<PictureFrame::Type>(picture.type());
        if (kfmType & wantedTypes) {
            updateFrame(&picture, kfmType);
            ++it;
        } else if (kfmType & removeTypes) {
            it = lstPic.erase(it);
        } else {
            ++it;
        }
    }
    // Add new covers
    for (const auto type : allImageTypes<PictureFrame>) {
        const auto kfmType = mapTaglibType<PictureFrame::Type>(type);
        if (kfmType & wantedTypes) {
            PictureFrame coverFrame;
            updateFrame(&coverFrame, kfmType);
            coverFrame.setType(type);
            lstPic.append(coverFrame);
        }
    }
    asfTags->setAttribute("WM/Picture", lstPic);
}
void writeMp4Tags(TagLib::MP4::Tag *mp4Tags, const PropertyMultiMap &newProperties)
{
    if (newProperties.contains(Property::Rating)) {
        mp4Tags->setItem("rate", TagLib::StringList(TagLib::String::number(newProperties.value(Property::Rating).toInt() * 10)));
    }
}

void writeMp4Cover(TagLib::MP4::Tag *mp4Tags,
                   const QMap<EmbeddedImageData::ImageType, QByteArray> images)
{
    if (images.empty()) {
        return;
    }
    auto imageIt = images.constFind(EmbeddedImageData::FrontCover);
    if ((images.size() > 1) || (imageIt == images.constEnd())) {
        // TODO warn
    }
    if (imageIt == images.constEnd()) {
        return;
    }

    TagLib::MP4::CoverArtList coverArtList;
    const auto newCover = *imageIt;
    if (!newCover.isEmpty()) {
        TagLib::ByteVector imageData(newCover.data(), newCover.size());
        TagLib::MP4::CoverArt coverArt(TagLib::MP4::CoverArt::Format::Unknown, imageData);
        coverArtList.append(coverArt);
    }
    mp4Tags->setItem("covr", coverArtList);
}

} // anonymous namespace

void writeGenericProperties(TagLib::PropertyMap &oldProperties, const PropertyMultiMap &newProperties)
{
    if (newProperties.empty()) {
        return;
    }

    if (newProperties.contains(Property::Title)) {
        oldProperties.replace("TITLE", QStringToTString(newProperties.value(Property::Title).toString()));
    }

    if (newProperties.contains(Property::Artist)) {
        oldProperties.replace("ARTIST", QStringToTString(newProperties.value(Property::Artist).toString()));
    }

    if (newProperties.contains(Property::AlbumArtist)) {
        oldProperties.replace("ALBUMARTIST", QStringToTString(newProperties.value(Property::AlbumArtist).toString()));
    }

    if (newProperties.contains(Property::Album)) {
        oldProperties.replace("ALBUM", QStringToTString(newProperties.value(Property::Album).toString()));
    }

    if (newProperties.contains(Property::TrackNumber)) {
        int trackNumber = newProperties.value(Property::TrackNumber).toInt();
        //taglib requires uint
        if (trackNumber >= 0) {
            oldProperties.replace("TRACKNUMBER", QStringToTString(newProperties.value(Property::TrackNumber).toString()));
        }
    }

    if (newProperties.contains(Property::DiscNumber)) {
        int discNumber = newProperties.value(Property::DiscNumber).toInt();
        //taglib requires uint
        if (discNumber >= 0) {
            oldProperties.replace("DISCNUMBER", QStringToTString(newProperties.value(Property::DiscNumber).toString()));
        }
    }

    if (newProperties.contains(Property::ReleaseYear)) {
        int year = newProperties.value(Property::ReleaseYear).toInt();
        //taglib requires uint
        if (year >= 0) {
            oldProperties.replace("DATE", QStringToTString(newProperties.value(Property::ReleaseYear).toString()));
        }
    }

    if (newProperties.contains(Property::Genre)) {
        oldProperties.replace("GENRE", QStringToTString(newProperties.value(Property::Genre).toString()));
    }

    if (newProperties.contains(Property::Comment)) {
        oldProperties.replace("COMMENT", QStringToTString(newProperties.value(Property::Comment).toString()));
    }

    if (newProperties.contains(Property::Composer)) {
        oldProperties.replace("COMPOSER", QStringToTString(newProperties.value(Property::Composer).toString()));
    }

    if (newProperties.contains(Property::Lyricist)) {
        oldProperties.replace("LYRICIST", QStringToTString(newProperties.value(Property::Lyricist).toString()));
    }

    if (newProperties.contains(Property::Conductor)) {
        oldProperties.replace("CONDUCTOR", QStringToTString(newProperties.value(Property::Conductor).toString()));
    }

    if (newProperties.contains(Property::Copyright)) {
        oldProperties.replace("COPYRIGHT", QStringToTString(newProperties.value(Property::Copyright).toString()));
    }

    if (newProperties.contains(Property::Lyrics)) {
        oldProperties.replace("LYRICS", QStringToTString(newProperties.value(Property::Lyrics).toString()));
    }

    if (newProperties.contains(Property::Language)) {
        oldProperties.replace("LANGUAGE", QStringToTString(newProperties.value(Property::Language).toString()));
    }
}

TagLibWriter::TagLibWriter(QObject* parent)
    : WriterPlugin(parent)
{
}

QStringList TagLibWriter::writeMimetypes() const
{
    return supportedMimeTypes;
}

void TagLibWriter::write(const WriteData& data)
{
    const QString fileUrl = data.inputUrl();
    const PropertyMultiMap properties = data.properties();
    const QString mimeType = data.inputMimetype();

#if defined Q_OS_WINDOWS
    TagLib::FileStream stream(fileUrl.toLocal8Bit().constData(), false);
#else
    TagLib::FileStream stream(fileUrl.toUtf8().constData(), false);
#endif
    if (!stream.isOpen() || stream.readOnly()) {
        qCWarning(KFILEMETADATA_LOG) << "Unable to open file in write mode: " << fileUrl;
        return;
    }

    if (mimeType == QLatin1String("audio/mpeg")) {
        TagLib::MPEG::File file(&stream, TagLib::ID3v2::FrameFactory::instance(), false);
        if (file.isValid()) {
            auto savedProperties = file.properties();
            writeGenericProperties(savedProperties, properties);
            file.setProperties(savedProperties);
            if (file.hasID3v2Tag()) {
                writeID3v2Tags(file.ID3v2Tag(), properties);
                writeID3v2Cover(file.ID3v2Tag(), data.imageData());
            }
            file.save();
        }
    } else if (mimeType == QLatin1String("audio/x-aiff") || mimeType == QLatin1String("audio/x-aifc"))  {
        TagLib::RIFF::AIFF::File file(&stream, false);
        if (file.isValid()) {
            auto savedProperties = file.properties();
            writeGenericProperties(savedProperties, properties);
            file.setProperties(savedProperties);
            auto id3Tags = dynamic_cast<TagLib::ID3v2::Tag*>(file.tag());
            if (id3Tags) {
                writeID3v2Tags(id3Tags, properties);
                writeID3v2Cover(id3Tags, data.imageData());
            }
            file.save();
        }
    } else if (mimeType == QLatin1String("audio/wav") ||
               mimeType == QLatin1String("audio/vnd.wave") ||
               mimeType == QLatin1String("audio/x-wav")) {
        TagLib::RIFF::WAV::File file(&stream, false);
        if (file.isValid()) {
            auto savedProperties = file.properties();
            writeGenericProperties(savedProperties, properties);
            file.setProperties(savedProperties);
            auto id3Tags = file.ID3v2Tag();
            if (id3Tags) {
                writeID3v2Tags(id3Tags, properties);
                writeID3v2Cover(id3Tags, data.imageData());
            }
            file.save();
        }
    } else if (mimeType == QLatin1String("audio/x-musepack")) {
        TagLib::MPC::File file(&stream, false);
        if (file.isValid()) {
            auto savedProperties = file.properties();
            writeGenericProperties(savedProperties, properties);
            writeApeTags(savedProperties, properties);
            file.setProperties(savedProperties);
            if (file.hasAPETag()) {
                writeApeCover(file.APETag(), data.imageData());
            }
            file.save();
        }
    } else if (mimeType == QLatin1String("audio/x-ape")) {
        TagLib::APE::File file(&stream, false);
        if (file.isValid()) {
            auto savedProperties = file.properties();
            writeGenericProperties(savedProperties, properties);
            writeApeTags(savedProperties, properties);
            file.setProperties(savedProperties);
            if (file.hasAPETag()) {
                writeApeCover(file.APETag(), data.imageData());
            }
            file.save();
        }
    } else if (mimeType == QLatin1String("audio/x-wavpack")) {
        TagLib::WavPack::File file(&stream, false);
        if (file.isValid()) {
            auto savedProperties = file.properties();
            writeGenericProperties(savedProperties, properties);
            writeApeTags(savedProperties, properties);
            file.setProperties(savedProperties);
            if (file.hasAPETag()) {
                writeApeCover(file.APETag(), data.imageData());
            }
            file.save();
        }
    } else if (mimeType == QLatin1String("audio/mp4")) {
        TagLib::MP4::File file(&stream, false);
        if (file.isValid()) {
            auto savedProperties = file.properties();
            writeGenericProperties(savedProperties, properties);
            auto mp4Tags = dynamic_cast<TagLib::MP4::Tag*>(file.tag());
            if (mp4Tags) {
                writeMp4Tags(mp4Tags, properties);
                writeMp4Cover(mp4Tags, data.imageData());
            }
            file.setProperties(savedProperties);
            file.save();
        }
    } else if (mimeType == QLatin1String("audio/flac")) {
        TagLib::FLAC::File file(&stream, TagLib::ID3v2::FrameFactory::instance(), false);
        if (file.isValid()) {
            auto savedProperties = file.properties();
            writeGenericProperties(savedProperties, properties);
            writeVorbisTags(savedProperties, properties);
            file.setProperties(savedProperties);
            writeFlacCover(&file, data.imageData());
            file.save();
        }
    } else if (mimeType == QLatin1String("audio/ogg") || mimeType == QLatin1String("audio/x-vorbis+ogg")) {
        TagLib::Ogg::Vorbis::File file(&stream, false);
        if (file.isValid()) {
            auto savedProperties = file.properties();
            writeGenericProperties(savedProperties, properties);
            writeVorbisTags(savedProperties, properties);
            file.setProperties(savedProperties);
            writeFlacCover(file.tag(), data.imageData());
            file.save();
        }
    } else if (mimeType == QLatin1String("audio/x-opus+ogg")) {
        TagLib::Ogg::Opus::File file(&stream, false);
        if (file.isValid()) {
            auto savedProperties = file.properties();
            writeGenericProperties(savedProperties, properties);
            writeVorbisTags(savedProperties, properties);
            file.setProperties(savedProperties);
            writeFlacCover(file.tag(), data.imageData());
            file.save();
        }
    } else if (mimeType == QLatin1String("audio/x-speex+ogg")) {
        TagLib::Ogg::Speex::File file(&stream, false);
        if (file.isValid()) {
            auto savedProperties = file.properties();
            writeGenericProperties(savedProperties, properties);
            writeVorbisTags(savedProperties, properties);
            file.setProperties(savedProperties);
            writeFlacCover(file.tag(), data.imageData());
            file.save();
        }
    } else if (mimeType == QLatin1String("audio/x-ms-wma")) {
        TagLib::ASF::File file(&stream, false);
        if (file.isValid()) {
            auto savedProperties = file.properties();
            writeGenericProperties(savedProperties, properties);
            file.setProperties(savedProperties);
            auto asfTags = dynamic_cast<TagLib::ASF::Tag*>(file.tag());
            if (asfTags){
                writeAsfTags(asfTags, properties);
                writeAsfCover(asfTags, data.imageData());
            }
            file.save();
        }
    }
}

#include "moc_taglibwriter.cpp"
