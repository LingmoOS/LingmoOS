// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dimagehandler.h"
#include "private/dimagehandlerlibs_p.h"

#include <DObjectPrivate>

#include <QSet>
#include <QHash>
#include <QFileInfo>
#include <QImageReader>
#include <QPainter>

#include <omp.h>
#include <cmath>

#define SAVE_QUAITY_VALUE 100

struct SupportFormats
{
    QHash<QString, int> freeImageFormats;
    QHash<QString, int> movieFormats;
    QStringList libRawFormats;
    QStringList qtSupportFormats;
    QStringList saveableFormats;
    QStringList qtRotatableFormats;

    QStringList supportFormats;

    SupportFormats();
};

Q_GLOBAL_STATIC(SupportFormats, SupportFormatsInstance)
#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
Q_GLOBAL_STATIC(DLibFreeImage, DLibFreeImageInstance)
Q_GLOBAL_STATIC(DLibRaw, DLibRawInstance)
#endif

SupportFormats::SupportFormats()
{
#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
    // LibFreeImage support formats
    freeImageFormats["BMP"] = FIF_BMP;
    freeImageFormats["ICO"] = FIF_ICO;
    freeImageFormats["JPG"] = FIF_JPEG;
    freeImageFormats["JPE"] = FIF_JPEG;
    freeImageFormats["JPS"] = FIF_JPEG;
    freeImageFormats["JPEG"] = FIF_JPEG;
    freeImageFormats["JNG"] = FIF_JNG;
    freeImageFormats["KOALA"] = FIF_KOALA;
    freeImageFormats["KOA"] = FIF_KOALA;
    freeImageFormats["LBM"] = FIF_LBM;
    freeImageFormats["IFF"] = FIF_LBM;
    freeImageFormats["MNG"] = FIF_MNG;
    freeImageFormats["PBM"] = FIF_PBM;
    freeImageFormats["PBMRAW"] = FIF_PBMRAW;
    freeImageFormats["PCD"] = FIF_PCD;
    freeImageFormats["PCX"] = FIF_PCX;
    freeImageFormats["PGM"] = FIF_PGM;
    freeImageFormats["PGMRAW"] = FIF_PGMRAW;
    freeImageFormats["PNG"] = FIF_PNG;
    freeImageFormats["PPM"] = FIF_PPM;
    freeImageFormats["PPMRAW"] = FIF_PPMRAW;
    freeImageFormats["RAS"] = FIF_RAS;
    freeImageFormats["TGA"] = FIF_TARGA;
    freeImageFormats["TARGA"] = FIF_TARGA;
    freeImageFormats["TIFF"] = FIF_TIFF;
    freeImageFormats["TIF"] = FIF_TIFF;
    freeImageFormats["WBMP"] = FIF_WBMP;
    freeImageFormats["PSD"] = FIF_PSD;
    freeImageFormats["CUT"] = FIF_CUT;
    freeImageFormats["XBM"] = FIF_XBM;
    freeImageFormats["XPM"] = FIF_XPM;
    freeImageFormats["DDS"] = FIF_DDS;
    freeImageFormats["GIF"] = FIF_GIF;
    // Warning: Build fails in arch linux.
    // In libfreeimage.h from arch linux freeimage3.18.0-15 package:
    // The G3 fax format plugin is deliberately disabled in the Fedora build of
    // FreeImage as it requires that FreeImage uses a private copy of libtiff
    // which is a no no because of security reasons.
#    if 0
    freeImageFormats["FAX"] = FIF_FAXG3;
    freeImageFormats["G3"] = FIF_FAXG3;
#    endif
    freeImageFormats["SGI"] = FIF_SGI;
    freeImageFormats["EXR"] = FIF_EXR;
    freeImageFormats["PCT"] = FIF_PICT;
    freeImageFormats["PIC"] = FIF_PICT;
    freeImageFormats["PICT"] = FIF_PICT;
    freeImageFormats["PIC"] = FIF_PICT;
    freeImageFormats["WEBP"] = FIF_WEBP;
    freeImageFormats["JXR"] = FIF_JXR;
    movieFormats["MNG"] = FIF_MNG;
    movieFormats["GIF"] = FIF_GIF;
    movieFormats["WEBP"] = FIF_WEBP;

    // LibRaw support images
    libRawFormats << "CR2"
                  << "CRW"
                  << "DCR"
                  << "KDC"
                  << "MRW"
                  << "NEF"
                  << "ORF"
                  << "PEF"
                  << "RAF"
                  << "SRF"
                  << "DNG"
                  << "RAW";
#endif

    // For some formats, Qt will support after loading image plugins.
    QList<QByteArray> qtReadableFormats = QImageReader::supportedImageFormats();
    for (QByteArray &format : qtReadableFormats) {
        qtSupportFormats << QString::fromUtf8(format).toUpper();
    }
    // Qt support formats internal.
    qtSupportFormats << "PNM"
                     << "MEF"
                     << "PXM";

    saveableFormats << "BMP"
                    << "JPG"
                    << "JPEG"
                    << "JPS"
                    << "JPE"
                    << "PNG"
                    << "PGM"
                    << "PPM"
                    << "PNM"
                    << "TGA"
                    << "XPM"
                    << "ICO"
                    << "JNG"
#ifndef ISSW_64
                    << "WBMP"
#endif
                    << "RAS";

    // For those formats, Qt process faster than FreeImage.
    qtRotatableFormats << "ICNS"
                       << "JPG"
                       << "JPEG"
                       << "PNG"
                       << "BMP";

    QSet<QString> formats;

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
    // Check libFreeImage load succeeded.
    if (DLibFreeImageInstance()->isValid()) {
        // Use FreeImage load .pcx image.
        qtSupportFormats.removeAll("PCX");

        for (auto itr = freeImageFormats.begin(); itr != freeImageFormats.end(); ++itr) {
            formats.insert(itr.key());
        }

        for (auto itr = movieFormats.begin(); itr != movieFormats.end(); ++itr) {
            formats.insert(itr.key());
        }
    }

    // Add libRaw support formats.
    if (DLibRawInstance()->isValid()) {
        for (const QString &format : libRawFormats) {
            formats.insert(format);
        }
    }
#endif

    if (formats.isEmpty()) {
        supportFormats = qtSupportFormats;
    } else {
        // Add qt default support formats.
        for (const QString &format : qtSupportFormats) {
            formats.insert(format);
        }

        supportFormats = formats.values();
    }
}

DCORE_USE_NAMESPACE
DGUI_BEGIN_NAMESPACE

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
QString detectImageFormatInternal(const QString &fileName, FREE_IMAGE_FORMAT &format);
#else
QString detectImageFormatInternal(const QString &fileName);
#endif

class DImageHandlerPrivate : public DObjectPrivate
{
    D_DECLARE_PUBLIC(DImageHandler)
public:
    enum ImageOption { Readable = 0x1, Wirteable = 0x2, Rotatable = 0x4, SupportFreeImage = 0x8 };
    Q_DECLARE_FLAGS(ImageOptions, ImageOption);

    explicit DImageHandlerPrivate(DImageHandler *qq);

    bool formatReadable(const QString &fileFormat) const;
    bool formatWriteable(const QString &fileFormat) const;

    bool loadStaticImageFromFile(const QString &fileName, QImage &image);
#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
    bool loadImageWithFreeImage(const QString &fileName, QImage &image, FREE_IMAGE_FORMAT fifForamt, QString fileFormat);
#endif
    bool rotateImageFile(const QString &fileName, int angle);
    bool rotateImage(QImage &image, int angle);

    void adjustImageToRealOrientation(QImage &image, ExifImageOrientation orientation);

    QString fileName;
    ImageOptions options;
    QImage cachedImage;
    QString cachedFormat;
    QString lastError;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(DImageHandlerPrivate::ImageOptions)

DImageHandlerPrivate::DImageHandlerPrivate(DImageHandler *qq)
    : DObjectPrivate(qq)
{
}

bool DImageHandlerPrivate::formatReadable(const QString &fileFormat) const
{
    if (fileFormat.isEmpty()) {
        return false;
    }

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
    if (DLibFreeImageInstance()->isValid()) {
        if (SupportFormatsInstance()->supportFormats.contains(fileFormat)) {
            return true;
        }
    }
#endif

    return SupportFormatsInstance()->supportFormats.contains(fileFormat);
}

bool DImageHandlerPrivate::formatWriteable(const QString &fileFormat) const
{
    if (fileFormat.isEmpty()) {
        return false;
    }

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
    if (DLibFreeImageInstance()->isValid()) {
        if (SupportFormatsInstance()->saveableFormats.contains(fileFormat)) {
            return true;
        }
    }
#endif

    return SupportFormatsInstance()->saveableFormats.contains(fileFormat);
}

bool DImageHandlerPrivate::loadStaticImageFromFile(const QString &fileName, QImage &image)
{
    D_Q(DImageHandler);

    QFileInfo fileInfo(fileName);
    if (0 == fileInfo.size()) {
        lastError = QString("Error file!");
        return false;
    }

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
    FREE_IMAGE_FORMAT fifFormat = FIF_UNKNOWN;
    QString fileFormat = detectImageFormatInternal(fileName, fifFormat);
    if (DLibFreeImageInstance()->isValid()) {
        // Using free image meta type.
        QHash<QString, QString> dataMap = DLibFreeImageInstance()->findAllMetaData(fileName);
        fileFormat = dataMap.value("FileFormat").toUpper();
    }

    // For some formats, need use Qt image reader load, to aviod some errors on different hardware architectures.
    bool usingQImage = ((FIF_PICT == fifFormat) && ("PCT" != fileFormat));

    if (usingQImage || SupportFormatsInstance()->qtSupportFormats.contains(fileFormat)) {
#else
    QString fileFormat = detectImageFormatInternal(fileName);
#endif
        QImageReader reader(fileName);
        reader.setAutoTransform(true);

        if (reader.imageCount() > 0 || "ICNS" != fileFormat) {
            image = reader.read();
            if (!image.isNull()) {
                return true;
            } else {
                // Try loading image with a forced format.
                reader.setFormat(fileFormat.toLower().toUtf8());
                image = reader.read();
                if (!image.isNull()) {
                    return true;

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
                } else if (DLibFreeImageInstance()->isValid() &&
                           DLibFreeImageInstance()->FreeImage_FIFSupportsReading(fifFormat)) {
                    // Try load with FreeImage.
                    return loadImageWithFreeImage(fileName, image, fifFormat, fileFormat);
#endif
                } else {
                    lastError = QString("Load image by qt failed, %1, use format: %2").arg(reader.errorString()).arg(fileFormat);
                    return false;
                }
            }
        }

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
    } else if (SupportFormatsInstance()->libRawFormats.contains(fileFormat) && DLibRawInstance()->isValid()) {
        // Use libRaw load.
        image = DLibRawInstance()->loadImage(fileName, lastError);
        return !image.isNull();
    } else if (DLibFreeImageInstance()->isValid()) {
        // Use FreeImage load.
        return loadImageWithFreeImage(fileName, image, fifFormat, fileFormat);
    }
#endif

    lastError = QString("Unsupport image format: %1").arg(fileFormat);
    return false;
}

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
bool DImageHandlerPrivate::loadImageWithFreeImage(const QString &fileName,
                                                  QImage &image,
                                                  FREE_IMAGE_FORMAT fifFormat,
                                                  QString fileFormat)
{
    if (!DLibFreeImageInstance()->isValid()) {
        return false;
    }

    if (FIF_UNKNOWN != fifFormat || SupportFormatsInstance()->freeImageFormats.contains(fileFormat)) {
        if (FIF_UNKNOWN == fifFormat) {
            fifFormat = FREE_IMAGE_FORMAT(SupportFormatsInstance()->freeImageFormats.value(fileFormat, FIF_UNKNOWN));
        }

        static const int MAX_JP2_SUPPORT_SIZE = 40960000;
        if (FIF_JP2 == fifFormat && QFileInfo(fileName).size() > MAX_JP2_SUPPORT_SIZE) {
            lastError = QString("Load image failed, JP2 image size to big, format: %1").arg(fileFormat);
            return false;
        }

        FIBITMAP *dib = DLibFreeImageInstance()->FreeImage_Load(fifFormat, fileName.toUtf8().data(), 0);
        if (!dib) {
            lastError = QString("Load image failed, format: %1").arg(fileFormat);
            return false;
        }

        image = DLibFreeImageInstance()->FIBITMAPToQImage(dib);
        if (image.isNull()) {
            DLibFreeImageInstance()->FreeImage_Unload(dib);
            lastError = QString("Convert to QImage failed: %1").arg(fileFormat);
            return false;
        }

        DLibFreeImageInstance()->FreeImage_Unload(dib);
        return true;
    }

    lastError = QString("Unsupport image format: %1").arg(fileFormat);
    return false;
}
#endif

bool DImageHandlerPrivate::rotateImageFile(const QString &fileName, int angle)
{
    D_Q(DImageHandler);

    if (0 != (angle % 90)) {
        lastError = QString("Unsupported angle.");
        return false;
    }

    QString format = q->detectImageFormat(fileName);
    if (SupportFormatsInstance()->qtRotatableFormats.contains(format)) {
        QImage image(fileName);

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
        // Some image formats support use EXIF info store orientation info.
        if (DLibFreeImageInstance()->isValid()) {
            // Using libFreeImage to get EXIF information.
            ExifImageOrientation orientation = DLibFreeImageInstance()->imageOrientation(fileName);
            adjustImageToRealOrientation(image, orientation);
        }
#endif

        if (rotateImage(image, angle)) {
            image.save(fileName, format.toLatin1().data(), SAVE_QUAITY_VALUE);
            return true;
        }
        return false;
    }

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
    if (DLibFreeImageInstance()->isValid()) {
        return DLibFreeImageInstance()->rotateImageFile(fileName, angle, lastError);
    }
#endif

    lastError = QString("Unsupported format: %1").arg(format);
    return false;
}

bool DImageHandlerPrivate::rotateImage(QImage &image, int angle)
{
    if (image.isNull()) {
        lastError = QString("Image is null.");
        return false;
    }

    if (0 != (angle % 90)) {
        lastError = QString("Rotate angle not base of 90, angle: %1").arg(angle);
        return false;
    }

    QImage imageCopy(image);
    if (!imageCopy.isNull()) {
        QTransform rotateMatrix;
        rotateMatrix.rotate(angle);
        image = imageCopy.transformed(rotateMatrix, Qt::SmoothTransformation);
        return true;
    }

    lastError = QString("Image is null.");
    return false;
}

void DImageHandlerPrivate::adjustImageToRealOrientation(QImage &image, ExifImageOrientation orientation)
{
    switch (orientation) {
            // TopLeft unnecessary rotate/flip
        case TopRight:  // Horizontal flip
            image = image.mirrored(true, false);
            break;
        case BottomRight:
            rotateImage(image, 180);
            break;
        case BottomLeft:  // Vertical flip
            image = image.mirrored(false, true);
            break;
        case LeftTop:  // Clockwise 90 degrees and horizontal flip
            rotateImage(image, 90);
            image = image.mirrored(true, false);
            break;
        case RightTop:
            rotateImage(image, 90);
            break;
        case RightBottom:  // Clockwise 90 degrees and vertical flip
            rotateImage(image, 90);
            image = image.mirrored(false, true);
            break;
        case LeftBottom:  // Counterclockwise 90 degrees
            rotateImage(image, -90);
            break;
        default:
            break;
    }
}

DImageHandler::DImageHandler(QObject *parent)
    : QObject(parent)
    , DObject(*new DImageHandlerPrivate(this))
{
}

DImageHandler::~DImageHandler() {}

void DImageHandler::setFileName(const QString &fileName)
{
    D_D(DImageHandler);
    if (fileName == d->fileName) {
        return;
    }

    d->fileName = fileName;
    d->options &= 0;
    clearCache();

    if (!d->fileName.isEmpty()) {
        d->cachedFormat = detectImageFormat(fileName);
        d->options.setFlag(DImageHandlerPrivate::Readable, d->formatReadable(d->cachedFormat));
        if (d->formatWriteable(d->cachedFormat)) {
            d->options.setFlag(DImageHandlerPrivate::Wirteable);
            d->options.setFlag(DImageHandlerPrivate::Rotatable);
        }
    }
}

QString DImageHandler::fileName() const
{
    D_DC(DImageHandler);
    return d->fileName;
}

QImage DImageHandler::readImage()
{
    D_D(DImageHandler);

    if (!isReadable()) {
        d->lastError = QString("File is not readable");
        return QImage();
    }

    if (!d->cachedImage.isNull()) {
        return d->cachedImage;
    }

    d->loadStaticImageFromFile(d->fileName, d->cachedImage);
    return d->cachedImage;
}

QImage DImageHandler::thumbnail(const QSize &size, Qt::AspectRatioMode mode)
{
    D_D(DImageHandler);
    if (d->cachedImage.isNull()) {
        d->loadStaticImageFromFile(d->fileName, d->cachedImage);
    }

    return d->cachedImage.scaled(size, mode);
}

QString DImageHandler::imageFormat() const
{
    D_DC(DImageHandler);
    return d->cachedFormat;
}

QSize DImageHandler::imageSize()
{
    D_D(DImageHandler);
    if (isReadable() && d->cachedImage.isNull()) {
        d->loadStaticImageFromFile(d->fileName, d->cachedImage);
    }

    return d->cachedImage.size();
}

QHash<QString, QString> DImageHandler::findAllMetaData()
{
    D_D(DImageHandler);
    if (!isReadable()) {
        return {};
    }

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
    if (DLibFreeImageInstance()->isValid()) {
        return DLibFreeImageInstance()->findAllMetaData(d->fileName);
    }
#endif

    return {};
}

bool DImageHandler::saveImage(const QString &fileName, const QString &format)
{
    D_D(DImageHandler);

    if (d->cachedImage.isNull()) {
        if (!d->loadStaticImageFromFile(d->fileName, d->cachedImage)) {
            return false;
        }
    }

    return saveImage(d->cachedImage, fileName, format);
}

bool DImageHandler::saveImage(const QImage &image, const QString &fileName, const QString &format)
{
    D_D(DImageHandler);

    QString realFormat = format.toUpper();
    if (realFormat.isEmpty()) {
        // Detect save file format.
#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
        FREE_IMAGE_FORMAT fifFormat = FIF_UNKNOWN;
        realFormat = detectImageFormatInternal(fileName, fifFormat);
        if (FIF_UNKNOWN != fifFormat) {
            realFormat = SupportFormatsInstance()->freeImageFormats.key(fifFormat);
        }
#else
        realFormat = detectImageFormatInternal(fileName);
#endif
    }

    if (!SupportFormatsInstance()->saveableFormats.contains(realFormat.toUpper())) {
        d->lastError = QString("Unsupport image save format: %1").arg(realFormat);
        return false;
    }

    if (!image.save(fileName, realFormat.toUtf8().data(), SAVE_QUAITY_VALUE)) {
        d->lastError = QString("Save image by qt failed, format: %1").arg(realFormat);
        return false;
    }
    return true;
}

bool DImageHandler::rotateImage(QImage &image, int angle)
{
    D_D(DImageHandler);
    return d->rotateImage(image, angle);
}

bool DImageHandler::rotateImageFile(const QString &fileName, int angle)
{
    D_D(DImageHandler);
    return d->rotateImageFile(fileName, angle);
}

bool DImageHandler::isReadable() const
{
    D_DC(DImageHandler);
    return d->options.testFlag(DImageHandlerPrivate::Readable);
}

bool DImageHandler::isWriteable() const
{
    D_DC(DImageHandler);
    return d->options.testFlag(DImageHandlerPrivate::Wirteable);
}

bool DImageHandler::isRotatable() const
{
    D_DC(DImageHandler);
    return d->options.testFlag(DImageHandlerPrivate::Rotatable);
}

void DImageHandler::clearCache()
{
    D_D(DImageHandler);
    d->cachedImage = QImage();
    d->cachedFormat.clear();
    d->lastError.clear();
}

QString DImageHandler::lastError() const
{
    return d_func()->lastError;
}

QStringList DImageHandler::supportFormats()
{
    return SupportFormatsInstance()->supportFormats;
}

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
QString detectImageFormatInternal(const QString &fileName, FREE_IMAGE_FORMAT &format)
#else
QString detectImageFormatInternal(const QString &fileName)
#endif
{
    QFileInfo info(fileName);
    QString fileSuffix = info.suffix().toUpper();
    QByteArray tempPath = fileName.toUtf8();

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
    format = FIF_UNKNOWN;
    
    // Check load lib FreeImage succeeded.
    if (DLibFreeImageInstance()->isValid()) {
        format = DLibFreeImageInstance()->FreeImage_GetFileType(tempPath.data(), 0);
        if (FIF_UNKNOWN != format && format != SupportFormatsInstance()->freeImageFormats[fileSuffix]) {
            // Some format don't load with FreeImage.
            QString fifFileFormat = SupportFormatsInstance()->freeImageFormats.key(format);
            if (!fifFileFormat.isEmpty()) {
                fileSuffix = fifFileFormat;
            }
        }

        if (format == FIF_TIFF) {
            fileSuffix = "TIFF";
        }
    }
#endif

    if (fileSuffix.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly)) {
            return QString("");
        }

        const QByteArray data = file.read(64);

        // Check bmp file.
        if (data.startsWith("BM")) {
            return "BMP";
        }

        // Check dds file.
        if (data.startsWith("DDS")) {
            return "DDS";
        }

        // Check gif file.
        if (data.startsWith("GIF8")) {
            return "GIF";
        }

        // Check Max OS icons file.
        if (data.startsWith("icns")) {
            return "ICNS";
        }

        // Check jpeg file.
        if (data.startsWith("\xff\xd8")) {
            return "JPG";
        }

        // Check mng file.
        if (data.startsWith("\x8a\x4d\x4e\x47\x0d\x0a\x1a\x0a")) {
            return "MNG";
        }

        // Check net pbm file (BitMap).
        if (data.startsWith("P1") || data.startsWith("P4")) {
            return "PBM";
        }

        // Check pgm file (GrayMap).
        if (data.startsWith("P2") || data.startsWith("P5")) {
            return "PGM";
        }

        // Check ppm file (PixMap).
        if (data.startsWith("P3") || data.startsWith("P6")) {
            return "PPM";
        }

        // Check png file.
        if (data.startsWith("\x89PNG\x0d\x0a\x1a\x0a")) {
            return "PNG";
        }

        // Check svg file.
        if (data.indexOf("<svg") > -1) {
            return "SVG";
        }

        // Check tiff file.
        if (data.startsWith("MM\x00\x2a") || data.startsWith("II\x2a\x00")) {
            // big-endian, little-endian.
            return "TIFF";
        }

        // Check webp file.
        if (data.startsWith("RIFFr\x00\x00\x00WEBPVP")) {
            return "WEBP";
        }

        // Check xbm file.
        if (data.indexOf("#define max_width ") > -1 && data.indexOf("#define max_height ") > -1) {
            return "XBM";
        }

        // Check xpm file.
        if (data.startsWith("/* XPM */")) {
            return "XPM";
        }

        return QString("");
    }

    return fileSuffix;
}

QString DImageHandler::detectImageFormat(const QString &fileName)
{
#ifndef DTK_DISABLE_EX_IMAGE_FORMAT
    FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
    return detectImageFormatInternal(fileName, format);
#else
    return detectImageFormatInternal(fileName);
#endif
}

QImage DImageHandler::oldColorFilter(const QImage &img)
{
    QImage imgCopy;
    if (img.format() != QImage::Format_RGB888) {
        imgCopy = QImage(img).convertToFormat(QImage::Format_RGB888);
    } else {
        imgCopy = QImage(img);
    }
    uint8_t *rgb = imgCopy.bits();
    if (nullptr == rgb) {
        return QImage();
    }
    int size = img.width() * img.height();

#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        float r = 0.393 * rgb[i * 3] + 0.769 * rgb[i * 3 + 1] + 0.189 * rgb[i * 3 + 2];
        float g = 0.349 * rgb[i * 3] + 0.686 * rgb[i * 3 + 1] + 0.168 * rgb[i * 3 + 2];
        float b = 0.272 * rgb[i * 3] + 0.534 * rgb[i * 3 + 1] + 0.131 * rgb[i * 3 + 2];
        r = qBound<float>(0, r, 255.0);
        g = qBound<float>(0, g, 255.0);
        b = qBound<float>(0, b, 255.0);
        rgb[i * 3] = r;
        rgb[i * 3 + 1] = g;
        rgb[i * 3 + 2] = b;
    }

    return imgCopy;
}

QImage DImageHandler::warmColorFilter(const QImage &img, int intensity)
{
    QImage imgCopy;
    if (img.format() != QImage::Format_RGB888) {
        imgCopy = QImage(img).convertToFormat(QImage::Format_RGB888);
    } else {
        imgCopy = QImage(img);
    }
    uint8_t *rgb = imgCopy.bits();
    if (nullptr == rgb) {
        return QImage();
    }
    QColor frontColor;
    int size = img.width() * img.height();

#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        int r = rgb[i * 3] + intensity;
        int g = rgb[i * 3 + 1] + intensity;
        int b = rgb[i * 3 + 2];

        rgb[i * 3] = r > 255 ? 255 : r;
        rgb[i * 3 + 1] = g > 255 ? 255 : g;
        rgb[i * 3 + 2] = b > 255 ? 255 : b;
    }

    return imgCopy;
}

QImage DImageHandler::coolColorFilter(const QImage &img, int intensity)
{
    QImage imgCopy;
    if (img.format() != QImage::Format_RGB888) {
        imgCopy = QImage(img).convertToFormat(QImage::Format_RGB888);
    } else {
        imgCopy = QImage(img);
    }
    uint8_t *rgb = imgCopy.bits();
    if (nullptr == rgb) {
        return QImage();
    }
    QColor frontColor;
    int size = img.width() * img.height();

#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        int r = rgb[i * 3];
        int g = rgb[i * 3 + 1];
        int b = rgb[i * 3 + 2] + intensity;

        rgb[i * 3] = r > 255 ? 255 : r;
        rgb[i * 3 + 1] = g > 255 ? 255 : g;
        rgb[i * 3 + 2] = b > 255 ? 255 : b;
    }

    return imgCopy;
}

QImage DImageHandler::grayScaleColorFilter(const QImage &img)
{
    QImage imgCopy;
    if (img.format() != QImage::Format_RGB888) {
        imgCopy = QImage(img).convertToFormat(QImage::Format_RGB888);
    } else {
        imgCopy = QImage(img);
    }
    uint8_t *rgb = imgCopy.bits();
    if (nullptr == rgb) {
        return QImage();
    }
    QColor frontColor;
    int size = img.width() * img.height();

#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        int average = (rgb[i * 3] + rgb[i * 3 + 1] + rgb[i * 3 + 2]) / 3;
        rgb[i * 3] = average > 255 ? 255 : average;
        rgb[i * 3 + 1] = average > 255 ? 255 : average;
        rgb[i * 3 + 2] = average > 255 ? 255 : average;
    }

    return imgCopy;
}

QImage DImageHandler::antiColorFilter(const QImage &img)
{
    QImage imgCopy;
    if (img.format() != QImage::Format_RGB888) {
        imgCopy = QImage(img).convertToFormat(QImage::Format_RGB888);
    } else {
        imgCopy = QImage(img);
    }
    uint8_t *rgb = imgCopy.bits();
    if (nullptr == rgb) {
        return QImage();
    }
    int size = img.width() * img.height();

#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        rgb[i * 3] = 255 - rgb[i * 3];
        rgb[i * 3 + 1] = 255 - rgb[i * 3 + 1];
        rgb[i * 3 + 2] = 255 - rgb[i * 3 + 2];
    }

    return imgCopy;
}

QImage DImageHandler::metalColorFilter(const QImage &img)
{
    QImage baseImage = QImage(img);
    QImage darkImage = DImageHandler::changeBrightness(img, -100);
    QImage greyImage = DImageHandler::grayScale(darkImage);
    QPainter painter;

    QImage newImage = baseImage.scaled(QSize(img.width(), img.height()));

    painter.begin(&newImage);
    painter.setOpacity(0.5);
    painter.drawImage(0, 0, greyImage);
    painter.end();

    return newImage;
}

QImage DImageHandler::bilateralFilter(const QImage &img, double spatialDecay, double photometricStandardDeviation)
{
    QImage imgCopy = QImage(img).convertToFormat(QImage::Format_RGB888);

    double c = -0.5 / (photometricStandardDeviation * photometricStandardDeviation);
    double mu = spatialDecay / (2 - spatialDecay);

    double *exptable = new double[256];
    double *g_table = new double[256];
#pragma omp parallel for
    for (int i = 0; i <= 255; i++) {
        exptable[i] = (1 - spatialDecay) * exp(c * i * i);
        g_table[i] = mu * i;
    }
    int width = img.width();
    int height = img.height();
    int length = width * height;
    double *data2Red = new double[length];
    double *data2Green = new double[length];
    double *data2Blue = new double[length];

    int size = imgCopy.width() * imgCopy.height();
    uint8_t *rgb = imgCopy.bits();
#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        data2Red[i] = rgb[i * 3];
        data2Green[i] = rgb[i * 3 + 1];
        data2Blue[i] = rgb[i * 3 + 2];
    }

    double *gRed = new double[length];
    double *pRed = new double[length];
    double *rRed = new double[length];

    double *gGreen = new double[length];
    double *pGreen = new double[length];
    double *rGreen = new double[length];

    double *gBlue = new double[length];
    double *pBlue = new double[length];
    double *rBlue = new double[length];
    memcpy(pRed, data2Red, sizeof(double) * length);
    memcpy(rRed, data2Red, sizeof(double) * length);

    memcpy(pGreen, data2Green, sizeof(double) * length);
    memcpy(rGreen, data2Green, sizeof(double) * length);

    memcpy(pBlue, data2Blue, sizeof(double) * length);
    memcpy(rBlue, data2Blue, sizeof(double) * length);

    double rho0 = 1.0 / (2 - spatialDecay);
#pragma omp parallel for
    for (int k2 = 0; k2 < height; ++k2) {
        int startIndex = k2 * width;
        double mu2 = 0.0;

        for (int k = startIndex + 1, K = startIndex + width; k < K; ++k) {
            int div0Red = fabs(pRed[k] - pRed[k - 1]);
            mu2 = exptable[div0Red];
            pRed[k] = pRed[k - 1] * mu2 + pRed[k] * (1.0 - mu2);

            int div0Green = fabs(pGreen[k] - pGreen[k - 1]);
            mu2 = exptable[div0Green];
            pGreen[k] = pGreen[k - 1] * mu2 + pGreen[k] * (1.0 - mu2);

            int div0Blue = fabs(pBlue[k] - pBlue[k - 1]);
            mu2 = exptable[div0Blue];
            pBlue[k] = pBlue[k - 1] * mu2 + pBlue[k] * (1.0 - mu2);
        }

        for (int k = startIndex + width - 2; startIndex <= k; --k) {
            int div0Red = fabs(rRed[k] - rRed[k + 1]);
            mu2 = exptable[div0Red];
            rRed[k] = rRed[k + 1] * mu2 + rRed[k] * (1.0 - mu2);

            int div0Green = fabs(rGreen[k] - rGreen[k + 1]);
            mu2 = exptable[div0Green];
            rGreen[k] = rGreen[k + 1] * mu2 + rGreen[k] * (1.0 - mu2);

            int div0Blue = fabs(rBlue[k] - rBlue[k + 1]);
            mu2 = exptable[div0Blue];
            rBlue[k] = rBlue[k + 1] * mu2 + rBlue[k] * (1.0 - mu2);
        }

        for (int k = startIndex, K = startIndex + width; k < K; k++) {
            rRed[k] = (rRed[k] + pRed[k]) * rho0 - g_table[(int)data2Red[k]];
            rGreen[k] = (rGreen[k] + pGreen[k]) * rho0 - g_table[(int)data2Green[k]];
            rBlue[k] = (rBlue[k] + pBlue[k]) * rho0 - g_table[(int)data2Blue[k]];
        }
    }

    int m = 0;

    for (int k2 = 0; k2 < height; k2++) {
        int n = k2;

        for (int k1 = 0; k1 < width; k1++) {
            gRed[n] = rRed[m];
            gGreen[n] = rGreen[m];
            gBlue[n] = rBlue[m];
            m++;
            n += height;
        }
    }

    memcpy(pRed, gRed, sizeof(double) * height * width);
    memcpy(rRed, gRed, sizeof(double) * height * width);

    memcpy(pGreen, gGreen, sizeof(double) * height * width);
    memcpy(rGreen, gGreen, sizeof(double) * height * width);

    memcpy(pBlue, gBlue, sizeof(double) * height * width);
    memcpy(rBlue, gBlue, sizeof(double) * height * width);

#pragma omp parallel for
    for (int k1 = 0; k1 < width; ++k1) {
        int startIndex = k1 * height;
        double mu1 = 0.0;

        for (int k = startIndex + 1, K = startIndex + height; k < K; ++k) {
            int div0Red = fabs(pRed[k] - pRed[k - 1]);
            mu1 = exptable[div0Red];
            pRed[k] = pRed[k - 1] * mu1 + pRed[k] * (1.0 - mu1);

            int div0Green = fabs(pGreen[k] - pGreen[k - 1]);
            mu1 = exptable[div0Green];
            pGreen[k] = pGreen[k - 1] * mu1 + pGreen[k] * (1.0 - mu1);

            int div0Blue = fabs(pBlue[k] - pBlue[k - 1]);
            mu1 = exptable[div0Blue];
            pBlue[k] = pBlue[k - 1] * mu1 + pBlue[k] * (1.0 - mu1);
        }

        for (int k = startIndex + height - 2; startIndex <= k; --k) {
            int div0Red = fabs(rRed[k] - rRed[k + 1]);
            mu1 = exptable[div0Red];
            rRed[k] = rRed[k + 1] * mu1 + rRed[k] * (1.0 - mu1);

            int div0Green = fabs(rGreen[k] - rGreen[k + 1]);
            mu1 = exptable[div0Green];
            rGreen[k] = rGreen[k + 1] * mu1 + rGreen[k] * (1.0 - mu1);

            int div0Blue = fabs(rBlue[k] - rBlue[k + 1]);
            mu1 = exptable[div0Blue];
            rBlue[k] = rBlue[k + 1] * mu1 + rBlue[k] * (1.0 - mu1);
        }
    }

    double init_gain_mu = spatialDecay / (2 - spatialDecay);

#pragma omp parallel for
    for (int k = 0; k < length; ++k) {
        rRed[k] = (rRed[k] + pRed[k]) * rho0 - gRed[k] * init_gain_mu;

        rGreen[k] = (rGreen[k] + pGreen[k]) * rho0 - gGreen[k] * init_gain_mu;

        rBlue[k] = (rBlue[k] + pBlue[k]) * rho0 - gBlue[k] * init_gain_mu;
    }

    m = 0;
    int nRowBytes = (width * 24 + 31) / 32 * 4;
    int lineNum_24 = 0;

    for (int k1 = 0; k1 < width; ++k1) {
        int n = k1;
        for (int k2 = 0; k2 < height; ++k2) {
            data2Red[n] = rRed[m];
            data2Green[n] = rGreen[m];
            data2Blue[n] = rBlue[m];
            lineNum_24 = k2 * nRowBytes;
            rgb[lineNum_24 + k1 * 3] = data2Red[n];
            rgb[lineNum_24 + k1 * 3 + 1] = data2Green[n];
            rgb[lineNum_24 + k1 * 3 + 2] = data2Blue[n];
            m++;
            n += width;
        }
    }
    delete[] data2Red;
    data2Red = nullptr;
    delete[] data2Green;
    data2Green = nullptr;
    delete[] data2Blue;
    data2Blue = nullptr;

    delete[] pRed;
    pRed = nullptr;
    delete[] rRed;
    rRed = nullptr;
    delete[] gRed;
    gRed = nullptr;

    delete[] pGreen;
    pGreen = nullptr;
    delete[] rGreen;
    rGreen = nullptr;
    delete[] gGreen;
    gGreen = nullptr;

    delete[] pBlue;
    pBlue = nullptr;
    delete[] rBlue;
    rBlue = nullptr;
    delete[] gBlue;
    gBlue = nullptr;

    delete[] exptable;
    exptable = nullptr;
    delete[] g_table;
    g_table = nullptr;

    return imgCopy;
}

QImage DImageHandler::contourExtraction(const QImage &img)
{
    int width = img.width();
    int height = img.height();

    QImage binImg = binaryzation(img);
    QImage newImg = QImage(width, height, QImage::Format_RGB888);
    newImg.fill(Qt::white);

    uint8_t *rgb = newImg.bits();
    uint8_t *binrgb = binImg.bits();
    int nRowBytes = (width * 24 + 31) / 32 * 4;

#pragma omp parallel for
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int pixel[8];
            memset(pixel, 0, 8 * sizeof(int));
            int lineNum_24 = y * nRowBytes;
            if (binrgb[lineNum_24 + x * 3] == 0) {
                rgb[lineNum_24 + x * 3] = 0;
                rgb[lineNum_24 + x * 3 + 1] = 0;
                rgb[lineNum_24 + x * 3 + 2] = 0;
                pixel[0] = binrgb[(y - 1) * nRowBytes + (x - 1) * 3];
                pixel[1] = binrgb[(y)*nRowBytes + (x - 1) * 3];
                pixel[2] = binrgb[(y + 1) * nRowBytes + (x - 1) * 3];
                pixel[3] = binrgb[(y - 1) * nRowBytes + (x)*3];
                pixel[4] = binrgb[(y + 1) * nRowBytes + (x)*3];
                pixel[5] = binrgb[(y - 1) * nRowBytes + (x + 1) * 3];
                pixel[6] = binrgb[(y)*nRowBytes + (x + 1) * 3];
                pixel[7] = binrgb[(y + 1) * nRowBytes + (x + 1) * 3];

                if (pixel[0] + pixel[1] + pixel[2] + pixel[3] + pixel[4] + pixel[5] + pixel[6] + pixel[7] == 0) {
                    rgb[lineNum_24 + x * 3] = 255;
                    rgb[lineNum_24 + x * 3 + 1] = 255;
                    rgb[lineNum_24 + x * 3 + 2] = 255;
                }
            }
        }
    }

    return newImg;
}

QImage DImageHandler::binaryzation(const QImage &img)
{
    QImage imgCopy;

    if (img.format() != QImage::Format_RGB888) {
        imgCopy = QImage(img).convertToFormat(QImage::Format_RGB888);
    } else {
        imgCopy = QImage(img);
    }
    uint8_t *rgb = imgCopy.bits();

    int size = img.width() * img.height();

#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        int gray = (rgb[i * 3] + rgb[i * 3 + 1] + rgb[i * 3 + 2]) / 3;
        int newGray = 0;
        if (gray > 128)
            newGray = 255;
        else
            newGray = 0;
        rgb[i * 3] = newGray;
        rgb[i * 3 + 1] = newGray;
        rgb[i * 3 + 2] = newGray;
    }

    return imgCopy;
}

QImage DImageHandler::grayScale(const QImage &img)
{
    QImage imgCopy;

    if (img.format() != QImage::Format_RGB888) {
        imgCopy = QImage(img).convertToFormat(QImage::Format_RGB888);
    } else {
        imgCopy = QImage(img);
    }
    uint8_t *rgb = imgCopy.bits();
    int size = img.width() * img.height();

#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        int average = (rgb[i * 3] * 299 + rgb[i * 3 + 1] * 587 + rgb[i * 3 + 1] * 114 + 500) / 1000;
        rgb[i * 3] = average;
        rgb[i * 3 + 1] = average;
        rgb[i * 3 + 2] = average;
    }

    return imgCopy;
}

QImage DImageHandler::laplaceSharpen(const QImage &img)
{
    QImage imgCopy;
    int width = img.width();
    int height = img.height();
    int window[3][3] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
    if (img.format() != QImage::Format_RGB888) {
        imgCopy = QImage(width, height, QImage::Format_RGB888);
    } else {
        imgCopy = QImage(img);
    }
    QImage imgCopyrgbImg = QImage(img).convertToFormat(QImage::Format_RGB888);
    uint8_t *rgbImg = imgCopyrgbImg.bits();
    uint8_t *rgb = imgCopy.bits();
    int nRowBytes = (width * 24 + 31) / 32 * 4;

#pragma omp parallel for
    for (int x = 1; x < img.width(); x++) {
        for (int y = 1; y < img.height(); y++) {
            int sumR = 0;
            int sumG = 0;
            int sumB = 0;
            int lineNum_24 = 0;

            for (int m = x - 1; m <= x + 1; m++)
                for (int n = y - 1; n <= y + 1; n++) {
                    if (m >= 0 && m < width && n < height) {
                        lineNum_24 = n * nRowBytes;
                        sumR += rgbImg[lineNum_24 + m * 3] * window[n - y + 1][m - x + 1];
                        sumG += rgbImg[lineNum_24 + m * 3 + 1] * window[n - y + 1][m - x + 1];
                        sumB += rgbImg[lineNum_24 + m * 3 + 2] * window[n - y + 1][m - x + 1];
                    }
                }

            int old_r = rgbImg[lineNum_24 + x * 3];
            sumR += old_r;
            sumR = qBound(0, sumR, 255);

            int old_g = rgbImg[lineNum_24 + x * 3 + 1];
            sumG += old_g;
            sumG = qBound(0, sumG, 255);

            int old_b = rgbImg[lineNum_24 + x * 3 + 2];
            sumB += old_b;
            sumB = qBound(0, sumB, 255);
            lineNum_24 = y * nRowBytes;
            rgb[lineNum_24 + x * 3] = sumR;
            rgb[lineNum_24 + x * 3 + 1] = sumG;
            rgb[lineNum_24 + x * 3 + 2] = sumB;
        }
    }

    return imgCopy;
}

QImage DImageHandler::sobelEdgeDetector(const QImage &img)
{
    double *Gx = new double[9];
    double *Gy = new double[9];

    /* Sobel */
    Gx[0] = 1.0;
    Gx[1] = 0.0;
    Gx[2] = -1.0;
    Gx[3] = 2.0;
    Gx[4] = 0.0;
    Gx[5] = -2.0;
    Gx[6] = 1.0;
    Gx[7] = 0.0;
    Gx[8] = -1.0;

    Gy[0] = -1.0;
    Gy[1] = -2.0;
    Gy[2] = -1.0;
    Gy[3] = 0.0;
    Gy[4] = 0.0;
    Gy[5] = 0.0;
    Gy[6] = 1.0;
    Gy[7] = 2.0;
    Gy[8] = 1.0;

    QImage grayImage = grayScale(img);
    int height = grayImage.height();
    int width = grayImage.width();
    QImage imgCopy = QImage(width, height, QImage::Format_RGB888);

    uint8_t *rgbImg = grayImage.bits();
    uint8_t *rgb = imgCopy.bits();

    int nRowBytes = (width * 24 + 31) / 32 * 4;

    float *sobel_norm = new float[width * height];
    float max = 0.0;
    QColor my_color;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double value_gx = 0.0;
            double value_gy = 0.0;

            for (int k = 0; k < 3; k++) {
                for (int p = 0; p < 3; p++) {
                    if ((x + 1 + 1 - k < width) && (y + 1 + 1 - p < height)) {
                        int lineNum_24 = (y + 1 + 1 - p) * nRowBytes;
                        value_gx += Gx[p * 3 + k] * rgbImg[lineNum_24 + (x + 1 + 1 - k) * 3];
                        value_gy += Gy[p * 3 + k] * rgbImg[lineNum_24 + (x + 1 + 1 - k) * 3];
                    }
                }
                sobel_norm[x + y * width] = abs(value_gx) + abs(value_gy);

                max = sobel_norm[x + y * width] > max ? sobel_norm[x + y * width] : max;
            }
        }
    }

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            my_color.setHsv(0, 0, 255 - int(255.0 * sobel_norm[i + j * width] / max));

            int lineNum_24 = j * nRowBytes;
            rgb[lineNum_24 + i * 3] = my_color.red();
            rgb[lineNum_24 + i * 3 + 1] = my_color.green();
            rgb[lineNum_24 + i * 3 + 2] = my_color.blue();
        }
    }
    delete[] sobel_norm;

    return imgCopy;
}

QImage DImageHandler::changeLightAndContrast(const QImage &img, int light, int contrast)
{
    QImage imgCopy;
    if (img.format() != QImage::Format_RGB888) {
        imgCopy = QImage(img).convertToFormat(QImage::Format_RGB888);
    } else {
        imgCopy = QImage(img);
    }
    uint8_t *rgb = imgCopy.bits();
    if (nullptr == rgb) {
        return QImage();
    }

    int size = img.width() * img.height();

#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        int r;
        int g;
        int b;
        r = light * 0.01 * rgb[i * 3] - 150 + contrast;
        g = light * 0.01 * rgb[i * 3 + 1] - 150 + contrast;
        b = light * 0.01 * rgb[i * 3 + 2] - 150 + contrast;

        rgb[i * 3] = qBound(0, r, 255);
        rgb[i * 3 + 1] = qBound(0, g, 255);
        rgb[i * 3 + 2] = qBound(0, b, 255);
    }

    return imgCopy;
}

QImage DImageHandler::changeBrightness(const QImage &img, int brightness)
{
    QImage imgCopy;

    if (img.format() != QImage::Format_RGB888) {
        imgCopy = QImage(img).convertToFormat(QImage::Format_RGB888);
    } else {
        imgCopy = QImage(img);
    }
    uint8_t *rgb = imgCopy.bits();
    int size = img.width() * img.height();

#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        int r = rgb[i * 3] + brightness;
        int g = rgb[i * 3 + 1] + brightness;
        int b = rgb[i * 3 + 2] + brightness;
        r = qBound(0, r, 255);
        g = qBound(0, g, 255);
        b = qBound(0, b, 255);
        rgb[i * 3] = r;
        rgb[i * 3 + 1] = g;
        rgb[i * 3 + 2] = b;
    }

    return imgCopy;
}

QImage DImageHandler::changeTransparency(const QImage &img, int transparency)
{
    QImage newImage(img.width(), img.height(), QImage::Format_ARGB32);
    QColor oldColor;
    int r, g, b;
    for (int x = 0; x < newImage.width(); x++) {
        for (int y = 0; y < newImage.height(); y++) {
            oldColor = QColor(img.pixel(x, y));

            r = oldColor.red();
            g = oldColor.green();
            b = oldColor.blue();

            newImage.setPixel(x, y, qRgba(r, g, b, transparency));
        }
    }

    return newImage;
}

QImage DImageHandler::changeStauration(const QImage &img, int saturation)
{
    int r, g, b, rgbMin, rgbMax;
    float k = saturation / 100.0f * 128;
    int alpha = 0;

    QImage newImage(img);
    QColor tmpColor;

    for (int x = 0; x < newImage.width(); x++) {
        for (int y = 0; y < newImage.height(); y++) {
            tmpColor = QColor(img.pixel(x, y));
            r = tmpColor.red();
            g = tmpColor.green();
            b = tmpColor.blue();

            rgbMin = qMin(qMin(r, g), b);
            rgbMax = qMax(qMax(r, g), b);

            int delta = (rgbMax - rgbMin);
            int value = (rgbMax + rgbMin);
            if (delta == 0) {
                continue;
            }
            int L = value >> 1;
            if (k >= 0) {
                int S = L < 128 ? (delta << 7) / value : (delta << 7) / (510 - value);
                alpha = k + S >= 128 ? S : 128 - k;
                alpha = 128 * 128 / alpha - 128;
            } else {
                alpha = k;
            }
            r = r + ((r - L) * alpha >> 7);
            g = g + ((g - L) * alpha >> 7);
            b = b + ((b - L) * alpha >> 7);
            r = qBound(0, r, 255);
            g = qBound(0, g, 255);
            b = qBound(0, b, 255);
            newImage.setPixel(x, y, qRgb(r, g, b));
        }
    }

    return newImage;
}

QImage DImageHandler::replacePointColor(const QImage &img, QColor oldColor, QColor newColor)
{
    QImage imgCopy = img;
    if (img.format() != QImage::Format_RGB888) {
        imgCopy = QImage(img).convertToFormat(QImage::Format_RGB888);
    } else {
        imgCopy = QImage(img);
    }
    uint8_t *rgb = imgCopy.bits();
    if (nullptr == rgb) {
        return QImage();
    }
    QColor frontColor;

    for (int x = 0; x < img.width(); ++x) {
        for (int y = 0; y < img.height(); ++y) {
            if (imgCopy.pixelColor(x, y) == oldColor) {
                imgCopy.setPixelColor(x, y, newColor);
            }
        }
    }

    return imgCopy;
}

QImage DImageHandler::flipHorizontal(const QImage &img)
{
    return img.mirrored(true, false);
}

QImage DImageHandler::flipVertical(const QImage &img)
{
    return img.mirrored(false, true);
}

DGUI_END_NAMESPACE
