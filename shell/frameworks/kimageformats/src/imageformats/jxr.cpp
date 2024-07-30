/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/*
 * Info about JXR:
 * - https://learn.microsoft.com/en-us/windows/win32/wic/jpeg-xr-codec
 *
 * Sample images:
 * - http://fileformats.archiveteam.org/wiki/JPEG_XR
 * - https://github.com/bvibber/hdrfix/tree/main/samples
 */

#include "jxr_p.h"
#include "util_p.h"

#include <QColorSpace>
#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QFloat16>
#include <QHash>
#include <QImage>
#include <QImageReader>
#include <QLoggingCategory>
#include <QSet>
#include <QSharedData>
#include <QTemporaryDir>

#include <JXRGlue.h>
#include <cstring>

Q_DECLARE_LOGGING_CATEGORY(LOG_JXRPLUGIN)
Q_LOGGING_CATEGORY(LOG_JXRPLUGIN, "kf.imageformats.plugins.jxr", QtWarningMsg)

/*!
 * Support for float images
 *
 * NOTE: Float images have values greater than 1 so they need an additional in place conversion.
 */
// #define JXR_DENY_FLOAT_IMAGE

/*!
 * Remove the neeeds of additional memory by disabling the conversion between
 * different color depths (e.g. RGBA64bpp to RGBA32bpp).
 *
 * NOTE: Leaving deptch conversion enabled (default) ensures maximum read compatibility.
 */
// #define JXR_DISABLE_DEPTH_CONVERSION // default commented

/*!
 * Windows displays and opens JXR files correctly out of the box. Unfortunately it doesn't
 * seem to open (P)RGBA @32bpp files as it only wants (P)BGRA32bpp files (a format not supported by Qt).
 * Only for this format an hack is activated to guarantee total compatibility of the plugin with Windows.
 */
// #define JXR_DISABLE_BGRA_HACK // default commented

/*!
 * The following functions are present in the Debian headers but not in the SUSE ones even if the source version is 1.0.1 on both.
 *
 * - ERR PKImageDecode_GetXMPMetadata_WMP(PKImageDecode *pID, U8 *pbXMPMetadata, U32 *pcbXMPMetadata);
 * - ERR PKImageDecode_GetEXIFMetadata_WMP(PKImageDecode *pID, U8 *pbEXIFMetadata, U32 *pcbEXIFMetadata);
 * - ERR PKImageDecode_GetGPSInfoMetadata_WMP(PKImageDecode *pID, U8 *pbGPSInfoMetadata, U32 *pcbGPSInfoMetadata);
 * - ERR PKImageDecode_GetIPTCNAAMetadata_WMP(PKImageDecode *pID, U8 *pbIPTCNAAMetadata, U32 *pcbIPTCNAAMetadata);
 * - ERR PKImageDecode_GetPhotoshopMetadata_WMP(PKImageDecode *pID, U8 *pbPhotoshopMetadata, U32 *pcbPhotoshopMetadata);
 *
 * As a result, their use is disabled by default. It is possible to activate their use by defining the
 * JXR_ENABLE_ADVANCED_METADATA preprocessor directive
 */

// #define JXR_ENABLE_ADVANCED_METADATA

class JXRHandlerPrivate : public QSharedData
{
private:
    QSharedPointer<QTemporaryDir> tempDir;
    mutable QSharedPointer<QFile> jxrFile;
    mutable QHash<QString, QString> txtMeta;

public:
    PKFactory *pFactory = nullptr;
    PKCodecFactory *pCodecFactory = nullptr;
    PKImageDecode *pDecoder = nullptr;
    PKImageEncode *pEncoder = nullptr;

    JXRHandlerPrivate()
    {
        tempDir = QSharedPointer<QTemporaryDir>(new QTemporaryDir);
        if (PKCreateFactory(&pFactory, PK_SDK_VERSION) == WMP_errSuccess) {
            PKCreateCodecFactory(&pCodecFactory, WMP_SDK_VERSION);
        }
        if (pFactory == nullptr || pCodecFactory == nullptr) {
            qCWarning(LOG_JXRPLUGIN) << "JXRHandlerPrivate::JXRHandlerPrivate() initialization error of JXR library!";
        }
    }
    JXRHandlerPrivate(const JXRHandlerPrivate &other) = default;

    ~JXRHandlerPrivate()
    {
        if (pCodecFactory) {
            PKCreateCodecFactory_Release(&pCodecFactory);
        }
        if (pFactory) {
            PKCreateFactory_Release(&pFactory);
        }
        if (pDecoder) {
            PKImageDecode_Release(&pDecoder);
        }
        if (pEncoder) {
            PKImageEncode_Release(&pEncoder);
        }
    }

    QString fileName() const
    {
        return jxrFile->fileName();
    }

    /* *** READ *** */

    /*!
     * \brief initForReading
     * Initialize the device for reading.
     * \param device The source device.
     * \return True on success, otherwise false.
     */
    bool initForReading(QIODevice *device)
    {
        if (!readDevice(device)) {
            return false;
        }
        if (!initDecoder()) {
            return false;
        }
        return true;
    }

    /*!
     * \brief jxrFormat
     * \return The JXR format.
     */
    PKPixelFormatGUID jxrFormat() const
    {
        PKPixelFormatGUID pixelFormatGUID = GUID_PKPixelFormatUndefined;
        if (pDecoder) {
            pDecoder->GetPixelFormat(pDecoder, &pixelFormatGUID);
        }
        return pixelFormatGUID;
    }

    /*!
     * \brief imageFormat
     * Calculate the image format from the JXR format. In conversionFormat it returns the possible conversion format of the JXR to match the returned Qt format.
     * \return The QImage format. If invalid, the image cannot be read.
     */
    QImage::Format imageFormat(PKPixelFormatGUID *conversionFormat = nullptr) const
    {
        PKPixelFormatGUID tmp;
        if (conversionFormat == nullptr) {
            conversionFormat = &tmp;
        }
        *conversionFormat = GUID_PKPixelFormatUndefined;

        auto jxrfmt = jxrFormat();
        auto qtFormat = exactFormat(jxrfmt);
        if (qtFormat != QImage::Format_Invalid) {
            return qtFormat;
        }

        // *** CONVERSION WITH THE SAME DEPTH ***
        // IMPORTANT: For supported conversions see JXRGluePFC.c

        // 32-bit
        if (IsEqualGUID(jxrfmt, GUID_PKPixelFormat32bppBGR)) {
            *conversionFormat = GUID_PKPixelFormat32bppRGB;
            return QImage::Format_RGBX8888; // Format_RGB32 (?)
        };
        if (IsEqualGUID(jxrfmt, GUID_PKPixelFormat32bppBGRA)) {
            *conversionFormat = GUID_PKPixelFormat32bppRGBA;
            return QImage::Format_RGBA8888;
        };
        if (IsEqualGUID(jxrfmt, GUID_PKPixelFormat32bppPBGRA)) {
            *conversionFormat = GUID_PKPixelFormat32bppPRGBA;
            return QImage::Format_RGBA8888_Premultiplied;
        };

#ifndef JXR_DENY_FLOAT_IMAGE
        if (IsEqualGUID(jxrfmt, GUID_PKPixelFormat128bppRGBAFixedPoint)) {
            *conversionFormat = GUID_PKPixelFormat128bppRGBAFloat;
            return QImage::Format_RGBA32FPx4;
        };
#endif // !JXR_DENY_FLOAT_IMAGE

        //  *** CONVERSION TO A LOWER DEPTH ***
        // IMPORTANT: For supported conversions see JXRGluePFC.c

#ifndef JXR_DISABLE_DEPTH_CONVERSION

#ifndef JXR_DENY_FLOAT_IMAGE
        // RGB FLOAT
        if (IsEqualGUID(jxrfmt, GUID_PKPixelFormat96bppRGBFloat)) {
            *conversionFormat = GUID_PKPixelFormat64bppRGBHalf;
            return QImage::Format_RGBX16FPx4;
        };
#endif // !JXR_DENY_FLOAT_IMAGE

        // RGBA
        // clang-format off
        if (IsEqualGUID(jxrfmt, GUID_PKPixelFormat64bppRGBAHalf) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat64bppRGBAFixedPoint) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat128bppRGBAFixedPoint) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat128bppRGBAFloat)) {

            *conversionFormat = GUID_PKPixelFormat32bppRGBA;
            return QImage::Format_RGBA8888;
        };
        // clang-format on

        // RGB
        // clang-format off
        if (IsEqualGUID(jxrfmt, GUID_PKPixelFormat128bppRGBFloat) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat96bppRGBFloat) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat64bppRGBFixedPoint) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat96bppRGBFixedPoint) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat128bppRGBFixedPoint) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat48bppRGBHalf) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat64bppRGBHalf) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat48bppRGBFixedPoint) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat32bppRGB101010) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat48bppRGB) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat32bppRGBE) ) {

            *conversionFormat = GUID_PKPixelFormat24bppRGB;
            return QImage::Format_RGB888;
        };
        // clang-format on

        // Gray
        // clang-format off
        if (IsEqualGUID(jxrfmt, GUID_PKPixelFormat32bppGrayFloat) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat16bppGrayFixedPoint) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat32bppGrayFixedPoint) ||
            IsEqualGUID(jxrfmt, GUID_PKPixelFormat16bppGrayHalf)) {

            *conversionFormat = GUID_PKPixelFormat8bppGray;
            return QImage::Format_Grayscale8;
        };
        // clang-format on
#endif // !JXR_DISABLE_DEPTH_CONVERSION

        return QImage::Format_Invalid;
    }

    /*!
     * \brief imageSize
     * \return The image size in pixels.
     */
    QSize imageSize() const
    {
        if (pDecoder) {
            qint32 w, h;
            pDecoder->GetSize(pDecoder, &w, &h);
            return QSize(w, h);
        }
        return {};
    }

    /*!
     * \brief colorSpace
     * \return The ICC profile if exists.
     */
    QColorSpace colorSpace() const
    {
        QColorSpace cs;
        if (pDecoder == nullptr) {
            return cs;
        }
        quint32 size;
        if (!pDecoder->GetColorContext(pDecoder, nullptr, &size) && size) {
            QByteArray ba(size, 0);
            if (!pDecoder->GetColorContext(pDecoder, reinterpret_cast<quint8 *>(ba.data()), &size)) {
                cs = QColorSpace::fromIccProfile(ba);
            }
        }
        return cs;
    }

    /*!
     * \brief xmpData
     * \return The XMP data if exists.
     */
    QString xmpData() const
    {
        QString xmp;
        if (pDecoder == nullptr) {
            return xmp;
        }
#ifdef JXR_ENABLE_ADVANCED_METADATA
        quint32 size;
        if (!PKImageDecode_GetXMPMetadata_WMP(pDecoder, nullptr, &size) && size) {
            QByteArray ba(size, 0);
            if (!PKImageDecode_GetXMPMetadata_WMP(pDecoder, reinterpret_cast<quint8 *>(ba.data()), &size)) {
                xmp = QString::fromUtf8(ba);
            }
        }
#endif
        return xmp;
    }

    /*!
     * \brief setTextMetadata
     * Set the text metadata into \a image
     * \param image Image on which to write metadata
     */
    void setTextMetadata(QImage& image)
    {
        auto xmp = xmpData();
        if (!xmp.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_XMP_ADOBE), xmp);
        }
        auto descr = description();
        if (!descr.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_DESCRIPTION), descr);
        }
        auto softw = software();
        if (!softw.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_SOFTWARE), softw);
        }
        auto make = cameraMake();
        if (!make.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_MANUFACTURER), make);
        }
        auto model = cameraModel();
        if (!model.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_MODEL), model);
        }
        auto cDate = dateTime();
        if (!cDate.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_CREATIONDATE), cDate);
        }
        auto author = artist();
        if (!author.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_AUTHOR), author);
        }
        auto copy = copyright();
        if (!copy.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_COPYRIGHT), copy);
        }
        auto capt = caption();
        if (!capt.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_TITLE), capt);
        }
        auto host = hostComputer();
        if (!host.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_HOSTCOMPUTER), capt);
        }
        auto docn = documentName();
        if (!docn.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_DOCUMENTNAME), docn);
        }
    }

#define META_TEXT(name, key)                                                                                                                                   \
    QString name() const                                                                                                                                       \
    {                                                                                                                                                          \
        readTextMeta();                                                                                                                                        \
        return txtMeta.value(QStringLiteral(key));                                                                                                             \
    }

    META_TEXT(description, META_KEY_DESCRIPTION)
    META_TEXT(cameraMake, META_KEY_MANUFACTURER)
    META_TEXT(cameraModel, META_KEY_MODEL)
    META_TEXT(software, META_KEY_SOFTWARE)
    META_TEXT(dateTime, META_KEY_CREATIONDATE)
    META_TEXT(artist, META_KEY_AUTHOR)
    META_TEXT(copyright, META_KEY_COPYRIGHT)
    META_TEXT(caption, META_KEY_TITLE)
    META_TEXT(documentName, META_KEY_DOCUMENTNAME)
    META_TEXT(hostComputer, META_KEY_HOSTCOMPUTER)

#undef META_TEXT

    /* *** WRITE *** */

    /*!
     * \brief initForWriting
     * Initialize the stream for writing.
     * \return True on success, otherwise false.
     */
    bool initForWriting()
    {
        // I have to use QFile because, on Windows, the QTemporary file is locked (even if I close it)
        auto fileName = QStringLiteral("%1.jxr").arg(tempDir->filePath(QUuid::createUuid().toString(QUuid::WithoutBraces).left(8)));
        QSharedPointer<QFile> file(new QFile(fileName));
        jxrFile = file;
        return initEncoder();
    }

    /*!
     * \brief finalizeWriting
     * \param device
     * Finalize the writing operation. Must be called as last peration.
     * \return True on success, otherwise false.
     */
    bool finalizeWriting(QIODevice *device)
    {
        if (device == nullptr || pEncoder == nullptr) {
            return false;
        }
        if (auto err = PKImageEncode_Release(&pEncoder)) {
            qCWarning(LOG_JXRPLUGIN) << "JXRHandlerPrivate::finalizeWriting() error while releasing the encoder:" << err;
            return false;
        }

        if (!deviceCopy(device, jxrFile.data())) {
            qCWarning(LOG_JXRPLUGIN) << "JXRHandlerPrivate::finalizeWriting() error while writing in the target device";
            return false;
        }
        return true;
    }

    /*!
     * \brief imageToSave
     * If necessary it converts the image to be saved into the appropriate format otherwise it does nothing.
     * \param source The image to save.
     * \return The image to use for save operation.
     */
    QImage imageToSave(const QImage &source) const
    {
        // IMPORTANT: these values must be in exactMatchingFormat()
        // clang-format off
        auto valid = QSet<QImage::Format>()
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
            << QImage::Format_CMYK8888
#endif
#ifndef JXR_DENY_FLOAT_IMAGE
            << QImage::Format_RGBA16FPx4
            << QImage::Format_RGBX16FPx4
            << QImage::Format_RGBA32FPx4
            << QImage::Format_RGBA32FPx4_Premultiplied
            << QImage::Format_RGBX32FPx4
#endif // JXR_DENY_FLOAT_IMAGE
            << QImage::Format_RGBA64
            << QImage::Format_RGBA64_Premultiplied
            << QImage::Format_RGBA8888
            << QImage::Format_RGBA8888_Premultiplied
            << QImage::Format_RGBX8888
            << QImage::Format_BGR888
            << QImage::Format_RGB888
            << QImage::Format_RGB555
            << QImage::Format_RGB16
            << QImage::Format_Grayscale16
            << QImage::Format_Grayscale8
            << QImage::Format_Mono;
        // clang-format on

        // To avoid complex code, I will save only inetger formats.
        auto qi = source;
        if (qi.format() == QImage::Format_MonoLSB) {
            qi = qi.convertToFormat(QImage::Format_Mono);
        }
        if (qi.format() == QImage::Format_Indexed8) {
            if (qi.allGray())
                qi = qi.convertToFormat(QImage::Format_Grayscale8);
            else
                qi = qi.convertToFormat(QImage::Format_RGBA8888);
        }

        // generic
        if (!valid.contains(qi.format())) {
            auto alpha = qi.hasAlphaChannel();
            auto depth = qi.depth();
            if (depth >= 12 && depth <= 24 && !alpha) {
                qi = qi.convertToFormat(QImage::Format_RGB888);
            } else if (depth >= 48) {
                // JXR don't have RGBX64 format so I have two possibilities:
                // - convert to 32 bpp (convertToFormat(alpha ? QImage::Format_RGBA64 : QImage::Format_RGB888))
                // - convert to 64 bpp with fake alpha (preferred)
                qi = qi.convertToFormat(QImage::Format_RGBA64);
            } else {
                qi = qi.convertToFormat(alpha ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
            }
#ifndef JXR_DENY_FLOAT_IMAGE
        } else if(qi.format() == QImage::Format_RGBA16FPx4 ||
                  qi.format() == QImage::Format_RGBX16FPx4 ||
                  qi.format() == QImage::Format_RGBA32FPx4 ||
                  qi.format() == QImage::Format_RGBA32FPx4_Premultiplied ||
                  qi.format() == QImage::Format_RGBX32FPx4) {
            auto cs = qi.colorSpace();
            if (cs.isValid() && cs.transferFunction() != QColorSpace::TransferFunction::Linear) {
                qi = qi.convertedToColorSpace(QColorSpace(QColorSpace::SRgbLinear));
            }
        }
#endif // JXR_DENY_FLOAT_IMAGE

        return qi;
    }

    /*!
     * \brief initCodecParameters
     * Initialize the JXR codec parameters.
     * \param wmiSCP
     * \param image The image to save.
     * \return True on success, otherwise false.
     */
    bool initCodecParameters(CWMIStrCodecParam *wmiSCP, const QImage &image)
    {
        if (wmiSCP == nullptr || image.isNull()) {
            return false;
        }
        memset(wmiSCP, 0, sizeof(CWMIStrCodecParam));

        auto fmt = image.format();

        wmiSCP->bVerbose = FALSE;
        if (fmt == QImage::Format_Grayscale8 || fmt == QImage::Format_Grayscale16 || fmt == QImage::Format_Mono)
            wmiSCP->cfColorFormat = Y_ONLY;
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        else if (fmt == QImage::Format_CMYK8888)
            wmiSCP->cfColorFormat = CMYK;
#endif
        else
            wmiSCP->cfColorFormat = YUV_444;
        wmiSCP->bdBitDepth = BD_LONG;
        wmiSCP->bfBitstreamFormat = FREQUENCY;
        wmiSCP->bProgressiveMode = TRUE;
        wmiSCP->olOverlap = OL_ONE;
        wmiSCP->cNumOfSliceMinus1H = wmiSCP->cNumOfSliceMinus1V = 0;
        wmiSCP->sbSubband = SB_ALL;
        wmiSCP->uAlphaMode = image.hasAlphaChannel() ? 2 : 0;
        return true;
    }

    /*!
     * \brief updateTextMetadata
     * Read the metadata from the image and set it in the encoder.
     * \param image The image to save.
     */
    void updateTextMetadata(const QImage &image)
    {
        if (pEncoder == nullptr) {
            return;
        }

        DESCRIPTIVEMETADATA meta;
        memset(&meta, 0, sizeof(meta));

#define META_CTEXT(name, field)                                                                                                                                \
    auto field = image.text(QStringLiteral(name)).toUtf8();                                                                                                    \
    if (!field.isEmpty()) {                                                                                                                                    \
        meta.field.vt = DPKVT_LPSTR;                                                                                                                           \
        meta.field.VT.pszVal = field.data();                                                                                                                   \
    }
#define META_WTEXT(name, field)                                                                                                                                \
    auto field = image.text(QStringLiteral(name));                                                                                                             \
    if (!field.isEmpty()) {                                                                                                                                    \
        meta.field.vt = DPKVT_LPWSTR;                                                                                                                          \
        meta.field.VT.pwszVal = const_cast<quint16 *>(field.utf16());                                                                                          \
    }

        META_CTEXT(META_KEY_DESCRIPTION, pvarImageDescription)
        META_CTEXT(META_KEY_MANUFACTURER, pvarCameraMake)
        META_CTEXT(META_KEY_MODEL, pvarCameraModel)
        META_CTEXT(META_KEY_AUTHOR, pvarArtist)
        META_CTEXT(META_KEY_COPYRIGHT, pvarCopyright)
        META_CTEXT(META_KEY_CREATIONDATE, pvarDateTime)
        META_CTEXT(META_KEY_DOCUMENTNAME, pvarDocumentName)
        META_CTEXT(META_KEY_HOSTCOMPUTER, pvarHostComputer)
        META_WTEXT(META_KEY_TITLE, pvarCaption)

#undef META_CTEXT
#undef META_WTEXT

        // Software must be updated
        auto software = QStringLiteral("%1 %2").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion()).toUtf8();
        if (!software.isEmpty()) {
            meta.pvarSoftware.vt = DPKVT_LPSTR;
            meta.pvarSoftware.VT.pszVal = software.data();
        }

        auto xmp = image.text(QStringLiteral(META_KEY_XMP_ADOBE)).toUtf8();
        if (!xmp.isNull()) {
            if (auto err = PKImageEncode_SetXMPMetadata_WMP(pEncoder, reinterpret_cast<quint8 *>(xmp.data()), xmp.size())) {
                qCWarning(LOG_JXRPLUGIN) << "JXRHandler::write() error while setting XMP data:" << err;
            }
        }
        if (auto err = pEncoder->SetDescriptiveMetadata(pEncoder, &meta)) {
            qCWarning(LOG_JXRPLUGIN) << "JXRHandler::write() error while setting descriptive data:" << err;
        }
    }

    /*!
     * \brief exactFormat
     * JXR and Qt use support image formats, some of which are identical. Use this function to convert a JXR format to Qt format.
     * \param jxrFormat Format to be converted.
     * \return A valid Qt format or QImage::Format_Invalid if there is no match
     */
    static QImage::Format exactFormat(const PKPixelFormatGUID &jxrFormat)
    {
        auto l = exactMatchingFormats();
        for (auto &&p : l) {
            if (IsEqualGUID(p.second, jxrFormat))
                return p.first;
        }
        return QImage::Format_Invalid;
    }

    /*!
     * \brief exactFormat
     * JXR and Qt use support image formats, some of which are identical. Use this function to convert a JXR format to Qt format.
     * \param qtFormat Format to be converted.
     * \return A valid JXR format or GUID_PKPixelFormatUndefined if there is no match
     */
    static PKPixelFormatGUID exactFormat(const QImage::Format &qtFormat)
    {
        auto l = exactMatchingFormats();
        for (auto &&p : l) {
            if (p.first == qtFormat)
                return p.second;
        }
        return GUID_PKPixelFormatUndefined;
    }

private:
    static QList<std::pair<QImage::Format, PKPixelFormatGUID>> exactMatchingFormats()
    {
        // clang-format off
        auto list = QList<std::pair<QImage::Format, PKPixelFormatGUID>>()
#ifndef JXR_DENY_FLOAT_IMAGE
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_RGBA16FPx4, GUID_PKPixelFormat64bppRGBAHalf)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_RGBX16FPx4, GUID_PKPixelFormat64bppRGBHalf)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_RGBA32FPx4, GUID_PKPixelFormat128bppRGBAFloat)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_RGBA32FPx4_Premultiplied, GUID_PKPixelFormat128bppPRGBAFloat)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_RGBX32FPx4, GUID_PKPixelFormat128bppRGBFloat)
#endif // JXR_DENY_FLOAT_IMAGE
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_CMYK8888, GUID_PKPixelFormat32bppCMYK)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_CMYK8888, GUID_PKPixelFormat32bppCMYKDIRECT)
#endif
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_Mono, GUID_PKPixelFormatBlackWhite)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_Grayscale8, GUID_PKPixelFormat8bppGray)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_Grayscale16, GUID_PKPixelFormat16bppGray)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_RGB555, GUID_PKPixelFormat16bppRGB565)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_RGB16, GUID_PKPixelFormat16bppRGB565)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_BGR888, GUID_PKPixelFormat24bppBGR)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_RGB888, GUID_PKPixelFormat24bppRGB)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_RGBX8888, GUID_PKPixelFormat32bppRGB)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_RGBA8888, GUID_PKPixelFormat32bppRGBA)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_RGBA8888_Premultiplied, GUID_PKPixelFormat32bppPRGBA)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_RGBA64, GUID_PKPixelFormat64bppRGBA)
            << std::pair<QImage::Format, PKPixelFormatGUID>(QImage::Format_RGBA64_Premultiplied, GUID_PKPixelFormat64bppPRGBA);
        // clang-format on
        return list;
    }

    bool deviceCopy(QIODevice *target, QIODevice *source)
    {
        if (target == nullptr || source == nullptr) {
            return false;
        }
        auto isTargetOpen = target->isOpen();
        if (!isTargetOpen && !target->open(QIODevice::WriteOnly)) {
            return false;
        }
        auto isSourceOpen = source->isOpen();
        if (!isSourceOpen && !source->open(QIODevice::ReadOnly)) {
            return false;
        }
        QByteArray buff(32768 * 4, char());
        for (; !source->atEnd();) {
            auto read = source->read(buff.data(), buff.size());
            if (read < 1) {
                return false;
            }
            if (target->write(buff.data(), read) != read) {
                return false;
            }
        }
        if (!isSourceOpen) {
            source->close();
        }
        if (!isTargetOpen) {
            target->close();
        }
        return true;
    }

    bool readDevice(QIODevice *device)
    {
        if (device == nullptr) {
            return false;
        }
        if (!jxrFile.isNull()) {
            return true;
        }
        // I have to use QFile because, on Windows, the QTemporary file is locked (even if I close it)
        auto fileName = QStringLiteral("%1.jxr").arg(tempDir->filePath(QUuid::createUuid().toString(QUuid::WithoutBraces).left(8)));
        QSharedPointer<QFile> file(new QFile(fileName));
        if (!file->open(QFile::WriteOnly)) {
            return false;
        }
        if (!deviceCopy(file.data(), device)) {
            qCWarning(LOG_JXRPLUGIN) << "JXRHandlerPrivate::readDevice() error while writing in the target device";
            return false;
        }
        file->close();
        jxrFile = file;
        return true;
    }

    bool initDecoder()
    {
        if (pDecoder) {
            return true;
        }
        if (pCodecFactory == nullptr) {
            return false;
        }
        if (auto err = pCodecFactory->CreateDecoderFromFile(qUtf8Printable(fileName()), &pDecoder)) {
            qCWarning(LOG_JXRPLUGIN) << "JXRHandlerPrivate::initDecoder() unable to create decoder:" << err;
            return false;
        }
        return true;
    }

    bool initEncoder()
    {
        if (pDecoder) {
            return true;
        }
        if (pCodecFactory == nullptr) {
            return false;
        }
        if (auto err = pCodecFactory->CreateCodec(&IID_PKImageWmpEncode, (void **)&pEncoder)) {
            qCWarning(LOG_JXRPLUGIN) << "JXRHandlerPrivate::initEncoder() unable to create encoder:" << err;
            return false;
        }
        return true;
    }

    bool readTextMeta() const {
        if (pDecoder == nullptr) {
            return false;
        }
        if (!txtMeta.isEmpty()) {
            return true;
        }

        DESCRIPTIVEMETADATA meta;
        if (pDecoder->GetDescriptiveMetadata(pDecoder, &meta)) {
            return false;
        }

#define META_TEXT(name, field)                                                                                                                                 \
    if (meta.field.vt == DPKVT_LPSTR)                                                                                                                          \
        txtMeta.insert(QStringLiteral(name), QString::fromUtf8(meta.field.VT.pszVal));                                                                         \
    else if (meta.field.vt == DPKVT_LPWSTR)                                                                                                                    \
        txtMeta.insert(QStringLiteral(name), QString::fromUtf16(reinterpret_cast<char16_t *>(meta.field.VT.pwszVal)));

        META_TEXT(META_KEY_DESCRIPTION, pvarImageDescription)
        META_TEXT(META_KEY_MANUFACTURER, pvarCameraMake)
        META_TEXT(META_KEY_MODEL, pvarCameraModel)
        META_TEXT(META_KEY_SOFTWARE, pvarSoftware)
        META_TEXT(META_KEY_CREATIONDATE, pvarDateTime)
        META_TEXT(META_KEY_AUTHOR, pvarArtist)
        META_TEXT(META_KEY_COPYRIGHT, pvarCopyright)
        META_TEXT(META_KEY_TITLE, pvarCaption)
        META_TEXT(META_KEY_DOCUMENTNAME, pvarDocumentName)
        META_TEXT(META_KEY_HOSTCOMPUTER, pvarHostComputer)

#undef META_TEXT

        return true;
    }
};

bool JXRHandler::read(QImage *outImage)
{
    if (!d->initForReading(device())) {
        return false;
    }

    PKPixelFormatGUID convFmt;
    auto imageFmt = d->imageFormat(&convFmt);
    auto img = imageAlloc(d->imageSize(), imageFmt);
    if (img.isNull()) {
        return false;
    }

    // resolution
    float hres, vres;
    if (auto err = d->pDecoder->GetResolution(d->pDecoder, &hres, &vres)) {
        qCWarning(LOG_JXRPLUGIN) << "JXRHandler::read() error while reading resolution:" << err;
    } else {
        img.setDotsPerMeterX(qRound(hres * 1000 / 25.4));
        img.setDotsPerMeterY(qRound(vres * 1000 / 25.4));
    }

    // alpha copy mode
    if (img.hasAlphaChannel()) {
        d->pDecoder->WMP.wmiSCP.uAlphaMode = 2; // or 1 (?)
    }

    PKRect rect = {0, 0, img.width(), img.height()};
    if (IsEqualGUID(convFmt, GUID_PKPixelFormatUndefined)) { // direct storing
        if (auto err = d->pDecoder->Copy(d->pDecoder, &rect, img.bits(), img.bytesPerLine())) {
            qCWarning(LOG_JXRPLUGIN) << "JXRHandler::read() unable to copy data:" << err;
            return false;
        }
    } else { // conversion to a known format
        PKFormatConverter *pConverter = nullptr;
        if (auto err = d->pCodecFactory->CreateFormatConverter(&pConverter)) {
            qCWarning(LOG_JXRPLUGIN) << "JXRHandler::read() unable to create the converter:" << err;
            return false;
        }
        if (auto err = pConverter->Initialize(pConverter, d->pDecoder, nullptr, convFmt)) {
            PKFormatConverter_Release(&pConverter);
            qCWarning(LOG_JXRPLUGIN) << "JXRHandler::read() unable to initialize the converter:" << err;
            return false;
        }
        if (d->pDecoder->WMP.wmiI.cBitsPerUnit == size_t(img.depth())) { // in place conversion
            if (auto err = pConverter->Copy(pConverter, &rect, img.bits(), img.bytesPerLine())) {
                PKFormatConverter_Release(&pConverter);
                qCWarning(LOG_JXRPLUGIN) << "JXRHandler::read() unable to copy converted data:" << err;
                return false;
            }
        } else { // additional buffer needed
            qint64 convStrideSize = (img.width() * d->pDecoder->WMP.wmiI.cBitsPerUnit + 7) / 8;
            qint64 buffSize = convStrideSize * img.height();
            qint64 limit = QImageReader::allocationLimit();
            if (limit && (buffSize + img.sizeInBytes()) > limit * 1024 * 1024) {
                qCWarning(LOG_JXRPLUGIN) << "JXRHandler::read() unable to covert due to allocation limit set:" << limit << "MiB";
                return false;
            }
            QVector<quint8> ba(buffSize);
            if (auto err = pConverter->Copy(pConverter, &rect, ba.data(), convStrideSize)) {
                PKFormatConverter_Release(&pConverter);
                qCWarning(LOG_JXRPLUGIN) << "JXRHandler::read() unable to copy converted data:" << err;
                return false;
            }
            for (qint32 y = 0, h = img.height(); y < h; ++y) {
                std::memcpy(img.scanLine(y), ba.data() + convStrideSize * y, (std::min)(convStrideSize, img.bytesPerLine()));
            }
        }
        PKFormatConverter_Release(&pConverter);
    }

    // Metadata (e.g.: icc profile, description, etc...)
    img.setColorSpace(d->colorSpace());
    d->setTextMetadata(img);

#ifndef JXR_DENY_FLOAT_IMAGE
    // JXR float are stored in scRGB.
    if (img.format() == QImage::Format_RGBX16FPx4 || img.format() == QImage::Format_RGBA16FPx4 || img.format() == QImage::Format_RGBA16FPx4_Premultiplied ||
        img.format() == QImage::Format_RGBX32FPx4 || img.format() == QImage::Format_RGBA32FPx4 || img.format() == QImage::Format_RGBA32FPx4_Premultiplied) {
        auto hasAlpha = img.hasAlphaChannel();
        for (qint32 y = 0, h = img.height(); y < h; ++y) {
            if (img.depth() == 64) {
                auto line = reinterpret_cast<qfloat16 *>(img.scanLine(y));
                for (int x = 0, w = img.width() * 4; x < w; x += 4)
                    line[x + 3] = hasAlpha ? std::clamp(line[x + 3], qfloat16(0), qfloat16(1)) : qfloat16(1);
            } else {
                auto line = reinterpret_cast<float *>(img.scanLine(y));
                for (int x = 0, w = img.width() * 4; x < w; x += 4)
                    line[x + 3] = hasAlpha ? std::clamp(line[x + 3], float(0), float(1)) : float(1);
            }
        }
        if(!img.colorSpace().isValid()) {
            img.setColorSpace(QColorSpace(QColorSpace::SRgbLinear));
        }
    }
#endif

    *outImage = img;
    return true;
}

bool JXRHandler::write(const QImage &image)
{
    if (!d->initForWriting()) {
        return false;
    }
    struct WMPStream *pEncodeStream = nullptr;
    if (auto err = d->pFactory->CreateStreamFromFilename(&pEncodeStream, qUtf8Printable(d->fileName()), "wb")) {
        qCWarning(LOG_JXRPLUGIN) << "JXRHandler::write() unable to create stream:" << err;
        return false;
    }

    // convert the image to a supported format
    auto qi = d->imageToSave(image);
    auto jxlfmt = d->exactFormat(qi.format());
    if (IsEqualGUID(jxlfmt, GUID_PKPixelFormatUndefined)) {
        qCWarning(LOG_JXRPLUGIN) << "JXRHandler::write() something wrong when calculating the target format for" << qi.format();
        return false;
    }
#ifndef JXR_DISABLE_BGRA_HACK
    if (IsEqualGUID(jxlfmt, GUID_PKPixelFormat32bppRGBA)) {
        jxlfmt = GUID_PKPixelFormat32bppBGRA;
        qi.rgbSwap();
    }
    if (IsEqualGUID(jxlfmt, GUID_PKPixelFormat32bppPRGBA)) {
        jxlfmt = GUID_PKPixelFormat32bppPBGRA;
        qi.rgbSwap();
    }
#endif

    // initialize the codec parameters
    CWMIStrCodecParam wmiSCP;
    if (!d->initCodecParameters(&wmiSCP, qi)) {
        qCWarning(LOG_JXRPLUGIN) << "JXRHandler::write() something wrong when calculating encoder parameters for" << qi.format();
        return false;
    }
    if (m_quality > -1) {
        wmiSCP.uiDefaultQPIndex = qBound(0, 100 - m_quality, 100);
    }

    if (auto err = d->pEncoder->Initialize(d->pEncoder, pEncodeStream, &wmiSCP, sizeof(wmiSCP))) {
        qCWarning(LOG_JXRPLUGIN) << "JXRHandler::write() error while initializing the encoder:" << err;
        return false;
    }

    // setting mandatory image info
    if (auto err = d->pEncoder->SetPixelFormat(d->pEncoder, jxlfmt)) {
        qCWarning(LOG_JXRPLUGIN) << "JXRHandler::write() error while setting the image format:" << err;
        return false;
    }
    if (auto err = d->pEncoder->SetSize(d->pEncoder, qi.width(), qi.height())) {
        qCWarning(LOG_JXRPLUGIN) << "JXRHandler::write() error while setting the image size:" << err;
        return false;
    }
    if (auto err = d->pEncoder->SetResolution(d->pEncoder, qi.dotsPerMeterX() * 25.4 / 1000, qi.dotsPerMeterY() * 25.4 / 1000)) {
        qCWarning(LOG_JXRPLUGIN) << "JXRHandler::write() error while setting the image resolution:" << err;
        return false;
    }

    // setting metadata (a failure of setting metadata doesn't stop the encoding)
    auto cs = qi.colorSpace().iccProfile();
    if (!cs.isEmpty()) {
        if (auto err = d->pEncoder->SetColorContext(d->pEncoder, reinterpret_cast<quint8 *>(cs.data()), cs.size())) {
            qCWarning(LOG_JXRPLUGIN) << "JXRHandler::write() error while setting ICC profile:" << err;
        }
    }
    d->updateTextMetadata(image);

    // writing the image
    if (auto err = d->pEncoder->WritePixels(d->pEncoder, qi.height(), qi.bits(), qi.bytesPerLine())) {
        qCWarning(LOG_JXRPLUGIN) << "JXRHandler::write() error while encoding the image:" << err;
        return false;
    }
    if (!d->finalizeWriting(device())) {
        return false;
    }
    return true;
}

void JXRHandler::setOption(ImageOption option, const QVariant &value)
{
    if (option == QImageIOHandler::Quality) {
        bool ok = false;
        auto q = value.toInt(&ok);
        if (ok) {
            m_quality = q;
        }
    }
}

bool JXRHandler::supportsOption(ImageOption option) const
{
    if (option == QImageIOHandler::Size) {
        return true;
    }
    if (option == QImageIOHandler::ImageFormat) {
        return true;
    }
    if (option == QImageIOHandler::Quality) {
        return true;
    }
    if (option == QImageIOHandler::ImageTransformation) {
        return false; // disabled because test cases are missing
    }
    return false;
}

QVariant JXRHandler::option(ImageOption option) const
{
    QVariant v;

    if (option == QImageIOHandler::Size) {
        if (d->initForReading(device())) {
            auto size = d->imageSize();
            if (size.isValid()) {
                v = QVariant::fromValue(size);
            }
        }
    }

    if (option == QImageIOHandler::ImageFormat) {
        if (d->initForReading(device())) {
            v = QVariant::fromValue(d->imageFormat());
        }
    }

    if (option == QImageIOHandler::Quality) {
        v = m_quality;
    }

    if (option == QImageIOHandler::ImageTransformation) {
        // TODO: rotation info (test case needed)
        if (d->initForReading(device())) {
            switch (d->pDecoder->WMP.oOrientationFromContainer) {
            case O_FLIPV:
                v = int(QImageIOHandler::TransformationFlip);
                break;
            case O_FLIPH:
                v = int(QImageIOHandler::TransformationMirror);
                break;
            case O_FLIPVH:
                v = int(QImageIOHandler::TransformationRotate180);
                break;
            case O_RCW:
                v = int(QImageIOHandler::TransformationRotate90);
                break;
            case O_RCW_FLIPV:
                v = int(QImageIOHandler::TransformationFlipAndRotate90);
                break;
            case O_RCW_FLIPH:
                v = int(QImageIOHandler::TransformationMirrorAndRotate90);
                break;
            case O_RCW_FLIPVH:
                v = int(QImageIOHandler::TransformationRotate270);
                break;
            default:
                v = int(QImageIOHandler::TransformationNone);
                break;
            }
        }
    }

    return v;
}

JXRHandler::JXRHandler()
    : d(new JXRHandlerPrivate)
    , m_quality(-1)
{
}

bool JXRHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("jxr");
        return true;
    }
    return false;
}

bool JXRHandler::canRead(QIODevice *device)
{
    if (!device) {
        qCWarning(LOG_JXRPLUGIN) << "JXRHandler::canRead() called with no device";
        return false;
    }

    // Some tests on sequential devices fail: I reject them for now
    if (device->isSequential()) {
        return false;
    }

    // JPEG XR image data is stored in TIFF-like container format (II and 0xBC01 version)
    if (device->peek(4) == QByteArray::fromRawData("\x49\x49\xbc\x01", 4)) {
        return true;
    }

    return false;
}

QImageIOPlugin::Capabilities JXRPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "jxr" || format == "wdp" || format == "hdp") {
        return Capabilities(CanRead | CanWrite);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && JXRHandler::canRead(device)) {
        cap |= CanRead;
    }
    if (device->isWritable()) {
        cap |= CanWrite;
    }
    return cap;
}

QImageIOHandler *JXRPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new JXRHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_jxr_p.cpp"
