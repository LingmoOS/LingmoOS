// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "private/dimagehandlerlibs_p.h"

#ifndef DTK_DISABLE_EX_IMAGE_FORMAT

#include <QLibrary>
#include <QFileInfo>
#include <QDateTime>
#include <QImageReader>

DLibFreeImage::DLibFreeImage()
{
    freeImage = new QLibrary("freeimage", "3");

    if (!freeImage->load()) {
        delete freeImage;
        freeImage = nullptr;
        return;
    }

    auto initFunctionError = [this]() {
        freeImage->unload();
        delete freeImage;
        freeImage = nullptr;
    };

#define INIT_FUNCTION_FREEIMAGE(Name)                                                                                                      \
    Name = reinterpret_cast<decltype(Name)>(freeImage->resolve(#Name));                                                          \
    if (!Name) {                                                                                                                 \
        initFunctionError();                                                                                                     \
        return;                                                                                                                  \
    }

    INIT_FUNCTION_FREEIMAGE(FreeImage_Load);
    INIT_FUNCTION_FREEIMAGE(FreeImage_Unload);
    INIT_FUNCTION_FREEIMAGE(FreeImage_Save);
    INIT_FUNCTION_FREEIMAGE(FreeImage_FIFSupportsReading);
    INIT_FUNCTION_FREEIMAGE(FreeImage_GetFileType);
    INIT_FUNCTION_FREEIMAGE(FreeImage_GetFIFFromFilename);
    INIT_FUNCTION_FREEIMAGE(FreeImage_GetImageType);

    INIT_FUNCTION_FREEIMAGE(FreeImage_GetBPP);
    INIT_FUNCTION_FREEIMAGE(FreeImage_GetWidth);
    INIT_FUNCTION_FREEIMAGE(FreeImage_GetHeight);
    INIT_FUNCTION_FREEIMAGE(FreeImage_GetRedMask);
    INIT_FUNCTION_FREEIMAGE(FreeImage_GetGreenMask);
    INIT_FUNCTION_FREEIMAGE(FreeImage_GetBlueMask);

    INIT_FUNCTION_FREEIMAGE(FreeImage_GetThumbnail);
    INIT_FUNCTION_FREEIMAGE(FreeImage_SetThumbnail);
    INIT_FUNCTION_FREEIMAGE(FreeImage_ConvertToRawBits);

    INIT_FUNCTION_FREEIMAGE(FreeImage_GetMetadataCount);
    INIT_FUNCTION_FREEIMAGE(FreeImage_FindFirstMetadata);
    INIT_FUNCTION_FREEIMAGE(FreeImage_FindNextMetadata);
    INIT_FUNCTION_FREEIMAGE(FreeImage_FindCloseMetadata);

    INIT_FUNCTION_FREEIMAGE(FreeImage_GetTagKey);
    INIT_FUNCTION_FREEIMAGE(FreeImage_GetTagValue);
    INIT_FUNCTION_FREEIMAGE(FreeImage_TagToString);
    INIT_FUNCTION_FREEIMAGE(FreeImage_Rotate);
}

DLibFreeImage::~DLibFreeImage()
{
    if (freeImage) {
        delete freeImage;
    }
}

bool DLibFreeImage::isValid()
{
    return freeImage;
}

bool DLibFreeImage::findMetaData(FREE_IMAGE_MDMODEL model, FIBITMAP *dib, QHash<QString, QString> &data)
{
    bool ret = false;
    if (freeImage) {
        if (FreeImage_GetMetadataCount(model, dib)) {
            FITAG *tag = nullptr;
            FIMETADATA *mdhandle = nullptr;
            mdhandle = FreeImage_FindFirstMetadata(model, dib, &tag);
            if (mdhandle) {
                do {
                    QString value;

                    // Warning: FreeImage_TagToString is not thread-safe, add mutex protection.
                    apiMutex.lock();
                    value = QString(FreeImage_TagToString(model, tag, nullptr));
                    apiMutex.unlock();

                    data.insert(FreeImage_GetTagKey(tag), value);
                    ret = true;
                } while (FreeImage_FindNextMetadata(mdhandle, &tag));
                FreeImage_FindCloseMetadata(mdhandle);
            }
        }
    }

    return ret;
}

QHash<QString, QString> DLibFreeImage::findAllMetaData(const QString &fileName)
{
    FIBITMAP *dib = readFileToFIBITMAP(fileName, FIF_LOAD_NOPIXELS);
    QHash<QString, QString> admMap;
    for (int i = FIMD_EXIF_MAIN; i <= FIMD_IPTC; ++i) {
        findMetaData(FREE_IMAGE_MDMODEL(i), dib, admMap);
    }

    QFileInfo info(fileName);
    if (admMap.contains("DateTime")) {
        // Get first DateTime.
        QDateTime time = QDateTime::fromString(admMap.value("DateTime"), "yyyy:MM:dd hh:mm:ss");
        admMap.insert("DateTimeOriginal", time.toString("yyyy/MM/dd hh:mm"));
    } else {
        admMap.insert("DateTimeOriginal", info.lastModified().toString("yyyy/MM/dd HH:mm"));
    }
    admMap.insert("DateTimeDigitized", info.lastModified().toString("yyyy/MM/dd HH:mm"));

    // The value of width and height might incorrect
    QImageReader reader(fileName);
    int w = reader.size().width();
    w = w > 0 ? w : static_cast<int>(FreeImage_GetWidth(dib));
    int h = reader.size().height();
    h = h > 0 ? h : static_cast<int>(FreeImage_GetHeight(dib));
    admMap.insert("Dimension", QString::number(w) + "x" + QString::number(h));
    admMap.insert("FileName", info.fileName());
    admMap.insert("FileFormat", info.suffix());

    static auto formatDataSize = [](qint64 size) -> QString {
        static const QStringList suffix = {"B", "KB", "MB", "GB", "TB", "PB"};
        int level = 0;
        double bytes = size;
        while (bytes >= 1024 && level < (suffix.length() - 1)) {
            bytes /= 1024;
            ++level;
        }
        return QString("%1%2").arg(bytes, 0, 'f', 1).arg(suffix[level]);
    };
    admMap.insert("FileSize", formatDataSize(info.size()));

    FreeImage_Unload(dib);
    return admMap;
}

FIBITMAP *DLibFreeImage::readFileToFIBITMAP(const QString &fileName, int flags, FREE_IMAGE_FORMAT fif)
{
    QByteArray b = fileName.toUtf8();
    const char *pc = b.data();

    // Detect file format.
    if (FIF_UNKNOWN == fif) {
        fif = FreeImage_GetFileType(pc, 0);
        if (FIF_UNKNOWN == fif) {
            fif = FreeImage_GetFIFFromFilename(pc);
        }
    }

    if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
        FIBITMAP *dib = FreeImage_Load(fif, pc, flags);
        return dib;
    }
    return nullptr;
}

bool DLibFreeImage::writeFIBITMAPToFile(FIBITMAP *dib, const QString &fileName, int flags)
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    bool bSuccess = false;
    const QByteArray ba = fileName.toUtf8();
    const char *pc = ba.data();
    // Try to guess the file format from the file extension
    fif = FreeImage_GetFIFFromFilename(pc);
    FREE_IMAGE_FORMAT realfif = FIF_UNKNOWN;

    if (fif == FIF_UNKNOWN) {
        realfif = FreeImage_GetFileType(pc, 0);
    }

    if (fif != FIF_UNKNOWN) {
        bSuccess = FreeImage_Save(fif, dib, pc, flags);
    } else if (realfif != FIF_UNKNOWN) {
        bSuccess = FreeImage_Save(realfif, dib, pc, flags);
    }

    return bSuccess;
}

QImage DLibFreeImage::FIBITMAPToQImage(FIBITMAP *dib) const
{
    if (!dib || FreeImage_GetImageType(dib) == FIT_UNKNOWN)
        return QImage(0, 0, QImage::Format_Invalid);
    int width = static_cast<int>(FreeImage_GetWidth(dib));
    int height = static_cast<int>(FreeImage_GetHeight(dib));
    int depth = static_cast<int>(FreeImage_GetBPP(dib));

    switch (depth) {
        case 1: {
            QImage result(width, height, QImage::Format_Mono);
            FreeImage_ConvertToRawBits(result.scanLine(0), dib, result.bytesPerLine(), 1, 0, 0, 0, true);
            return result;
        }
        case 4: {
            // NOTE: QImage do not support 4-bit, convert it to 8-bit
            QImage result(width, height, QImage::Format_Indexed8);
            FreeImage_ConvertToRawBits(result.scanLine(0), dib, result.bytesPerLine(), 8, 0, 0, 0, true);
            return result;
        }
        case 8: {
            QImage result(width, height, QImage::Format_Indexed8);
            FreeImage_ConvertToRawBits(result.scanLine(0), dib, result.bytesPerLine(), 8, 0, 0, 0, true);
            return result;
        }
        case 16:
            if ((FreeImage_GetRedMask(dib) == FI16_555_RED_MASK) && (FreeImage_GetGreenMask(dib) == FI16_555_GREEN_MASK) &&
                (FreeImage_GetBlueMask(dib) == FI16_555_BLUE_MASK)) {
                // 5-5-5
                QImage result(width, height, QImage::Format_RGB555);
                FreeImage_ConvertToRawBits(result.scanLine(0),
                                           dib,
                                           result.bytesPerLine(),
                                           16,
                                           FI16_555_RED_MASK,
                                           FI16_555_GREEN_MASK,
                                           FI16_555_BLUE_MASK,
                                           true);
                return result;
            } else {
                // 5-6-5
                QImage result(width, height, QImage::Format_RGB16);
                FreeImage_ConvertToRawBits(result.scanLine(0),
                                           dib,
                                           result.bytesPerLine(),
                                           16,
                                           FI16_565_RED_MASK,
                                           FI16_565_GREEN_MASK,
                                           FI16_565_BLUE_MASK,
                                           true);
                return result;
            }
        case 24: {
            QImage result(width, height, QImage::Format_RGB32);
            FreeImage_ConvertToRawBits(result.scanLine(0),
                                       dib,
                                       result.bytesPerLine(),
                                       32,
                                       FI_RGBA_RED_MASK,
                                       FI_RGBA_GREEN_MASK,
                                       FI_RGBA_BLUE_MASK,
                                       true);
            return result;
        }
        case 32: {
            QImage result(width, height, QImage::Format_ARGB32);
            FreeImage_ConvertToRawBits(result.scanLine(0),
                                       dib,
                                       result.bytesPerLine(),
                                       32,
                                       FI_RGBA_RED_MASK,
                                       FI_RGBA_GREEN_MASK,
                                       FI_RGBA_BLUE_MASK,
                                       true);
            return result;
        }
        case 48:
        case 64:
        case 96:
        case 128:
        default:
            break;
    }

    return QImage(0, 0, QImage::Format_Invalid);
}

ExifImageOrientation DLibFreeImage::imageOrientation(const QString &fileName)
{
    ExifImageOrientation oreintation = Undefined;

    FIBITMAP *dib = readFileToFIBITMAP(fileName, FIF_LOAD_NOPIXELS);
    // Metadata may be null sometimes, check if metadata exist.
    if (0 == FreeImage_GetMetadataCount(FIMD_EXIF_MAIN, dib)) {
        FreeImage_Unload(dib);
        return oreintation;
    }

    FITAG *tag = nullptr;
    FIMETADATA *mdhandle = nullptr;
    mdhandle = FreeImage_FindFirstMetadata(FIMD_EXIF_MAIN, dib, &tag);
    if (mdhandle) {
        do {
            if (qstrcmp(FreeImage_GetTagKey(tag), "Orientation") == 0) {
                oreintation = ExifImageOrientation(*static_cast<const WORD *>(FreeImage_GetTagValue(tag)));
                break;
            }
        } while (FreeImage_FindNextMetadata(mdhandle, &tag));
        FreeImage_FindCloseMetadata(mdhandle);
    }

    FreeImage_Unload(dib);
    return oreintation;
}

bool DLibFreeImage::rotateImageFile(const QString &fileName, int angle, QString &errorString)
{
    FIBITMAP *dib = readFileToFIBITMAP(fileName);
    if (nullptr == dib) {
        errorString = "Unsupported format";
        return false;
    }

    FIBITMAP *rotateRes = FreeImage_Rotate(dib, -angle, nullptr);
    if (rotateRes) {
        // Regenerate thumbnail if it's exits
        // Image formats that currently support thumbnail saving are
        // JPEG (JFIF formats), EXR, TGA and TIFF.
        if (FreeImage_GetThumbnail(dib)) {
            FIBITMAP *thumb = FreeImage_GetThumbnail(dib);
            FIBITMAP *rotateThumb = FreeImage_Rotate(thumb, -angle, nullptr);

            FreeImage_SetThumbnail(rotateRes, rotateThumb);
            FreeImage_Unload(rotateThumb);
        }
    }

    FREE_IMAGE_FORMAT f = FreeImage_GetFIFFromFilename(fileName.toUtf8().data());
    if (f == FIF_UNKNOWN) {
        FreeImage_Unload(dib);
        FreeImage_Unload(rotateRes);
        errorString = "Rotate image format error";
        return false;
    }

    if (!writeFIBITMAPToFile(rotateRes, fileName)) {
        FreeImage_Unload(dib);
        FreeImage_Unload(rotateRes);
        errorString = "Rotate image save failed, unknown format";
        return false;
    }

    FreeImage_Unload(dib);
    FreeImage_Unload(rotateRes);
    return true;
}

DLibRaw::DLibRaw()
{
    libraw = new QLibrary("libraw");
    if (!libraw->load()) {
        delete libraw;
        libraw = nullptr;
        return;
    }

    auto initFunctionError = [this]() {
        libraw->unload();
        delete libraw;
        libraw = nullptr;
    };

#define INIT_FUNCTION_LIBRAW(Name)                                                                                                      \
    Name = reinterpret_cast<decltype(Name)>(libraw->resolve(#Name));                                                             \
    if (!Name) {                                                                                                                 \
        initFunctionError();                                                                                                     \
        return;                                                                                                                  \
    }

    INIT_FUNCTION_LIBRAW(libraw_strerror);
    INIT_FUNCTION_LIBRAW(libraw_init);
    INIT_FUNCTION_LIBRAW(libraw_open_file);
    INIT_FUNCTION_LIBRAW(libraw_open_buffer);
    INIT_FUNCTION_LIBRAW(libraw_unpack);
    INIT_FUNCTION_LIBRAW(libraw_unpack_thumb);
    INIT_FUNCTION_LIBRAW(libraw_close);
    INIT_FUNCTION_LIBRAW(libraw_dcraw_process);
    INIT_FUNCTION_LIBRAW(libraw_dcraw_make_mem_image);
    INIT_FUNCTION_LIBRAW(libraw_dcraw_make_mem_thumb);
    INIT_FUNCTION_LIBRAW(libraw_dcraw_clear_mem);
}

DLibRaw::~DLibRaw()
{
    if (libraw) {
        delete libraw;
    }
}

bool DLibRaw::isValid()
{
    return libraw;
}

QImage DLibRaw::loadImage(const QString &fileName, QString &errString, QSize requestSize)
{
    QImage image;
    libraw_data_t *rawData = libraw_init(0);
    if (!rawData) {
        errString = QStringLiteral("Create new libraw object failed!");
        return image;
    }

    int ret = libraw_open_file(rawData, fileName.toUtf8().data());
    if (LIBRAW_SUCCESS == ret) {
        ret = readImage(rawData, image, requestSize);
    }
    libraw_close(rawData);

    if (LIBRAW_SUCCESS != ret) {
        errString = errorString(ret);
    }
    return image;
}

QImage DLibRaw::loadImage(QByteArray &data, QString &errString, QSize requestSize)
{
    QImage image;
    libraw_data_t *rawData = libraw_init(0);
    if (!rawData) {
        errString = QStringLiteral("Create new libraw object failed!");
        return image;
    }

    int ret = libraw_open_buffer(rawData, reinterpret_cast<void *>(data.data()), static_cast<size_t>(data.size()));
    if (LIBRAW_SUCCESS == ret) {
        ret = readImage(rawData, image, requestSize);
    }
    libraw_close(rawData);

    if (LIBRAW_SUCCESS != ret) {
        errString = errorString(ret);
    }
    return image;
}

int DLibRaw::readImage(libraw_data_t *rawData, QImage &image, QSize requestSize)
{
    if (!rawData) {
        return LIBRAW_INPUT_CLOSED;
    }

    int ret = LIBRAW_UNSPECIFIED_ERROR;
    int errCode = LIBRAW_SUCCESS;
    libraw_processed_image_t *output = nullptr;
    // Try to read thumbnail image if possible.
    if (!requestSize.isEmpty() &&
        (requestSize.width() < rawData->thumbnail.twidth || requestSize.height() < rawData->thumbnail.theight)) {
        ret = libraw_unpack_thumb(rawData);
        if (LIBRAW_SUCCESS == ret) {
            output = libraw_dcraw_make_mem_thumb(rawData, &errCode);
            if (LIBRAW_SUCCESS != errCode && output) {
                libraw_dcraw_clear_mem(output);
            }
        }
    }

    // Read default image if no thumbnail.
    if (!output) {
        ret = libraw_unpack(rawData);
        if (LIBRAW_SUCCESS != ret) {
            return ret;
        }
        ret = libraw_dcraw_process(rawData);
        if (LIBRAW_SUCCESS != ret) {
            return ret;
        }

        output = libraw_dcraw_make_mem_image(rawData, &errCode);
        if (LIBRAW_SUCCESS != errCode) {
            if (output) {
                libraw_dcraw_clear_mem(output);
            }

            // Can't read data, return.
            return errCode;
        }
    }

    if (LIBRAW_IMAGE_JPEG == output->type) {
        image.loadFromData(output->data, static_cast<int>(output->data_size), "JPEG");
        if (rawData->sizes.flip) {
            QTransform rotation;
            int angle = 0;

            // Image orientation (0 if does not require rotation; 3 if requires 180-deg rotation;
            // 5 if 90 deg counterclockwise, 6 if 90 deg clockwise).
            switch (rawData->sizes.flip) {
                case 3:
                    angle = 180;
                    break;
                case 5:
                    angle = -90;
                    break;
                case 6:
                    angle = 90;
                    break;
                default:
                    break;
            }

            if (angle != 0) {
                rotation.rotate(angle);
                image = image.transformed(rotation);
            }
        }

    } else {
        int numPixels = output->width * output->height;
        int colorSize = output->bits / 8;
        int pixelSize = output->colors * colorSize;
        uchar *pixels = new uchar[numPixels * 4];
        uchar *data = output->data;

        for (int i = 0; i < numPixels; i++, data += pixelSize) {
            if (output->colors == 3) {
                pixels[i * 4] = data[2 * colorSize];
                pixels[i * 4 + 1] = data[1 * colorSize];
                pixels[i * 4 + 2] = data[0];
            } else {
                pixels[i * 4] = data[0];
                pixels[i * 4 + 1] = data[0];
                pixels[i * 4 + 2] = data[0];
            }
        }

        // QImage::Format_RGB32 will cause window transparent
        image = QImage(pixels, output->width, output->height, QImage::Format_RGB32).convertToFormat(QImage::Format_ARGB32);
        delete[] pixels;
    }

    libraw_dcraw_clear_mem(output);
    return LIBRAW_SUCCESS;
}

QString DLibRaw::errorString(int errorCode)
{
    return QString(libraw_strerror(errorCode));
}

#endif
