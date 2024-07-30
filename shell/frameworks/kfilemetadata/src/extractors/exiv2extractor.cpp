/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "exiv2extractor.h"
#include "kfilemetadata_debug.h"
#include <QTimeZone>
#include <cmath>
#include <exiv2/exiv2.hpp>

using namespace KFileMetaData;

namespace {
void add(ExtractionResult* result, const Exiv2::ExifData& data,
         Property::Property prop, const Exiv2::ExifKey& key,
         QMetaType::Type type);

double fetchGpsDouble(const Exiv2::ExifData& data, const Exiv2::ExifKey& key);
double fetchGpsAltitude(const Exiv2::ExifData& data);
QByteArray fetchByteArray(const Exiv2::ExifData& data, const Exiv2::ExifKey& key);
}

Exiv2Extractor::Exiv2Extractor(QObject* parent)
    : ExtractorPlugin(parent)
{
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF(true);
#endif
}

namespace
{
static const QStringList supportedMimeTypes = {
    QStringLiteral("image/bmp"),
    QStringLiteral("image/gif"),
    QStringLiteral("image/jp2"),
    QStringLiteral("image/jpeg"),
    QStringLiteral("image/pgf"),
    QStringLiteral("image/png"),
    QStringLiteral("image/tiff"),
    QStringLiteral("image/webp"),
#ifdef EXV_ENABLE_BMFF
    QStringLiteral("image/avif"),
    QStringLiteral("image/heif"),
    QStringLiteral("image/jxl"),
    QStringLiteral("image/x-canon-cr3"),
#endif
    QStringLiteral("image/x-exv"),
    QStringLiteral("image/x-canon-cr2"),
    QStringLiteral("image/x-canon-crw"),
    QStringLiteral("image/x-fuji-raf"),
    QStringLiteral("image/x-minolta-mrw"),
    QStringLiteral("image/x-nikon-nef"),
    QStringLiteral("image/x-olympus-orf"),
    QStringLiteral("image/x-panasonic-rw2"),
    QStringLiteral("image/x-pentax-pef"),
    QStringLiteral("image/x-photoshop"),
    QStringLiteral("image/x-samsung-srw"),
    QStringLiteral("image/x-tga"),
};

QString toString(const Exiv2::Value& value)
{
    const std::string str = value.toString();
    return QString::fromUtf8(str.c_str(), str.length());
}

QVariant toVariantDateTime(const Exiv2::Value& value)
{
    if (value.typeId() == Exiv2::asciiString) {
        QDateTime val = ExtractorPlugin::dateTimeFromString(QString::fromLatin1(value.toString().c_str()));
        if (val.isValid()) {
            // Datetime is stored in exif as local time.
            val.setTimeZone(QTimeZone::fromSecondsAheadOfUtc(0));
            return QVariant(val);
        }
    }

    return QVariant();
}

QVariant toVariantLong(const Exiv2::Value& value)
{
    if (value.typeId() == Exiv2::unsignedLong || value.typeId() == Exiv2::signedLong) {
#if EXIV2_TEST_VERSION(0,28,0)
        qlonglong val = value.toInt64();
#else
        qlonglong val = value.toLong();
#endif
        return QVariant(val);
    }

    QString str(toString(value));
    bool ok = false;
    int val = str.toInt(&ok);
    if (ok) {
        return QVariant(val);
    }

    return QVariant();
}

QVariant toVariantDouble(const Exiv2::Value& value)
{
    if (value.typeId() == Exiv2::tiffFloat || value.typeId() == Exiv2::tiffDouble
        || value.typeId() == Exiv2::unsignedRational || value.typeId() == Exiv2::signedRational) {
        return QVariant(static_cast<double>(value.toFloat()));
    }

    QString str(toString(value));
    bool ok = false;
    double val = str.toDouble(&ok);
    if (ok) {
        return QVariant(val);
    }

    return QVariant();
}

QVariant toVariantString(const Exiv2::Value& value)
{
    QString str = toString(value);
    if (!str.isEmpty()) {
        return QVariant(str);
    }

    return QVariant();
}

QVariant toVariant(const Exiv2::Value& value, QMetaType::Type type) {
    if (value.count() == 0) {
        return QVariant();
    }
    switch (type) {
    case QMetaType::Int:
        return toVariantLong(value);

    case QMetaType::QDateTime:
        return toVariantDateTime(value);

    case QMetaType::Double:
        return toVariantDouble(value);

    case QMetaType::QString:
    default:
        return toVariantString(value);
    }
}
}

QStringList Exiv2Extractor::mimetypes() const
{
    return supportedMimeTypes;
}

void Exiv2Extractor::extract(ExtractionResult* result)
{
    using namespace std::string_literals;

    QByteArray arr = result->inputUrl().toUtf8();
    std::string fileString(arr.data(), arr.length());

#if EXIV2_TEST_VERSION(0, 28, 0)
    Exiv2::Image::UniquePtr image;
#else
    Exiv2::Image::AutoPtr image;
#endif
    try {
        image = Exiv2::ImageFactory::open(fileString);
    } catch (const std::exception&) {
        return;
    }
    if (!image.get()) {
        return;
    }

    try {
        image->readMetadata();
    } catch (const std::exception&) {
        return;
    }
    result->addType(Type::Image);

    if (!(result->inputFlags() & ExtractionResult::ExtractMetaData)) {
        return;
    }

    if (image->pixelHeight()) {
        result->add(Property::Height, image->pixelHeight());
    }

    if (image->pixelWidth()) {
        result->add(Property::Width, image->pixelWidth());
    }

    std::string comment = image->comment();
    if (!comment.empty()) {
        result->add(Property::Comment, QString::fromUtf8(comment.c_str(), comment.length()));
    }

    const Exiv2::ExifData& data = image->exifData();

    using EK = Exiv2::ExifKey;
    add(result, data, Property::Manufacturer, EK{"Exif.Image.Make"s}, QMetaType::QString);
    add(result, data, Property::Model, EK{"Exif.Image.Model"s}, QMetaType::QString);
    add(result, data, Property::Description, EK{"Exif.Image.ImageDescription"s}, QMetaType::QString);
    add(result, data, Property::Artist, EK{"Exif.Image.Artist"s}, QMetaType::QString);
    add(result, data, Property::Copyright, EK{"Exif.Image.Copyright"s}, QMetaType::QString);
    add(result, data, Property::Generator, EK{"Exif.Image.Software"s}, QMetaType::QString);
    add(result, data, Property::ImageDateTime, EK{"Exif.Image.DateTime"s}, QMetaType::QDateTime);
    add(result, data, Property::ImageOrientation, EK{"Exif.Image.Orientation"s}, QMetaType::Int);
    add(result, data, Property::PhotoFlash, EK{"Exif.Photo.Flash"s}, QMetaType::Int);
    add(result, data, Property::PhotoPixelXDimension, EK{"Exif.Photo.PixelXDimension"s}, QMetaType::Int);
    add(result, data, Property::PhotoPixelYDimension, EK{"Exif.Photo.PixelYDimension"s}, QMetaType::Int);
    add(result, data, Property::PhotoDateTimeOriginal, EK{"Exif.Photo.DateTimeOriginal"s}, QMetaType::QDateTime);
    add(result, data, Property::PhotoFocalLength, EK{"Exif.Photo.FocalLength"s}, QMetaType::Double);
    add(result, data, Property::PhotoFocalLengthIn35mmFilm, EK{"Exif.Photo.FocalLengthIn35mmFilm"s}, QMetaType::Double);
    add(result, data, Property::PhotoExposureTime, EK{"Exif.Photo.ExposureTime"s}, QMetaType::Double);
    add(result, data, Property::PhotoExposureBiasValue, EK{"Exif.Photo.ExposureBiasValue"s}, QMetaType::Double);
    add(result, data, Property::PhotoFNumber, EK{"Exif.Photo.FNumber"s}, QMetaType::Double);
    add(result, data, Property::PhotoApertureValue, EK{"Exif.Photo.ApertureValue"s}, QMetaType::Double);
    add(result, data, Property::PhotoWhiteBalance, EK{"Exif.Photo.WhiteBalance"s}, QMetaType::Int);
    add(result, data, Property::PhotoMeteringMode, EK{"Exif.Photo.MeteringMode"s}, QMetaType::Int);
    add(result, data, Property::PhotoISOSpeedRatings, EK{"Exif.Photo.ISOSpeedRatings"s}, QMetaType::Int);
    add(result, data, Property::PhotoSaturation, EK{"Exif.Photo.Saturation"s}, QMetaType::Int);
    add(result, data, Property::PhotoSharpness, EK{"Exif.Photo.Sharpness"s}, QMetaType::Int);
    // https://exiv2.org/tags.html "Exif.Photo.ImageTitle" not natively supported, use tag value
    add(result, data, Property::Title, EK{0xa436, "Photo"s}, QMetaType::QString);

    double latitude = fetchGpsDouble(data, EK{"Exif.GPSInfo.GPSLatitude"s});
    double longitude = fetchGpsDouble(data, EK{"Exif.GPSInfo.GPSLongitude"s});
    double altitude = fetchGpsAltitude(data);

    QByteArray latRef = fetchByteArray(data, EK{"Exif.GPSInfo.GPSLatitudeRef"s});
    if (!latRef.isEmpty() && latRef[0] == 'S') {
        latitude *= -1;
    }

    QByteArray longRef = fetchByteArray(data, EK{"Exif.GPSInfo.GPSLongitudeRef"s});
    if (!longRef.isEmpty() && longRef[0] == 'W') {
        longitude *= -1;
    }

    if (!std::isnan(latitude)) {
        result->add(Property::PhotoGpsLatitude, latitude);
    }

    if (!std::isnan(longitude)) {
        result->add(Property::PhotoGpsLongitude, longitude);
    }

    if (!std::isnan(altitude)) {
        result->add(Property::PhotoGpsAltitude, altitude);
    }

    const Exiv2::XmpData& xmpData = image->xmpData();
    for (const auto& entry : xmpData) {
        if (entry.groupName() != "dc"s) {
            continue;
        }

        std::map<std::string, Property::Property> propMap = {
            { "subject"s, Property::Subject },
            { "title"s, Property::Title},
            { "description"s, Property::Description},
        };
        if (auto type = propMap.find(entry.tagName()); type != propMap.end()) {
            auto xmpType = Exiv2::XmpValue::xmpArrayType(entry.value().typeId());
            size_t limit = ((xmpType == Exiv2::XmpValue::xaBag) || (xmpType == Exiv2::XmpValue::xaSeq)) ? entry.count() : 1;
            for (size_t i = 0; i < limit; i++) {
                auto value = QString::fromStdString(entry.toString(i));
                if (!value.isEmpty()) {
                    result->add(type->second, value);
                }
            }
        }
    }
}

namespace {
void add(ExtractionResult* result, const Exiv2::ExifData& data,
         Property::Property prop, const Exiv2::ExifKey& key,
         QMetaType::Type type)
{
    Exiv2::ExifData::const_iterator it = data.findKey(key);
    if (it != data.end()) {
        QVariant value = toVariant(it->value(), type);
        if (!value.isNull()) {
            result->add(prop, value);
        }
    }
}

double fetchGpsDouble(const Exiv2::ExifData& data, const Exiv2::ExifKey& key)
{
    Exiv2::ExifData::const_iterator it = data.findKey(key);
    if (it != data.end() && it->count() == 3) {
        double n = 0.0;
        double d = 0.0;

        n = (*it).toRational(0).first;
        d = (*it).toRational(0).second;

        if (d == 0.0) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        double deg = n / d;

        n = (*it).toRational(1).first;
        d = (*it).toRational(1).second;

        if (d == 0.0) {
            return deg;
        }

        double min = n / d;
        if (min != -1.0) {
            deg += min / 60.0;
        }

        n = (*it).toRational(2).first;
        d = (*it).toRational(2).second;

        if (d == 0.0) {
            return deg;
        }

        double sec = n / d;
        if (sec != -1.0) {
            deg += sec / 3600.0;
        }

        return deg;
    }

    return std::numeric_limits<double>::quiet_NaN();
}

double fetchGpsAltitude(const Exiv2::ExifData& data)
{
    using namespace std::string_literals;

    double alt = std::numeric_limits<double>::quiet_NaN();
    Exiv2::ExifData::const_iterator it = data.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitude"s));
    if (it != data.end() && it->count() > 0 &&
        (it->value().typeId() == Exiv2::unsignedRational || it->value().typeId() == Exiv2::signedRational)) {
        auto ratio = it->value().toRational();
        if (ratio.second == 0) {
            return alt;
        }
        it = data.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitudeRef"s));
        if (it != data.end() && it->count() > 0 &&
            (it->value().typeId() == Exiv2::unsignedByte || it->value().typeId() == Exiv2::signedByte)) {
#if EXIV2_TEST_VERSION(0,28,0)
            auto altRef = it->value().toInt64();
#else
            auto altRef = it->value().toLong();
#endif
            if (altRef) {
                alt = -1.0 * ratio.first / ratio.second;
            } else {
                alt = 1.0 * ratio.first / ratio.second;
            }
        }
    }
    return alt;
}
QByteArray fetchByteArray(const Exiv2::ExifData& data, const Exiv2::ExifKey& key)
{
    Exiv2::ExifData::const_iterator it = data.findKey(key);
    if (it != data.end() && it->count() > 0) {
        std::string str = it->value().toString();
        return QByteArray(str.c_str(), str.size());
    }

    return QByteArray();
}
} // <anonymous> namespace

#include "moc_exiv2extractor.cpp"
