/*
    AV1 Image File Format (AVIF) support for QImage.

    SPDX-FileCopyrightText: 2020 Daniel Novomesky <dnovomesky@gmail.com>

    SPDX-License-Identifier: BSD-2-Clause
*/

#include <QThread>
#include <QtGlobal>

#include <QColorSpace>

#include "avif_p.h"
#include "util_p.h"

#include <cfloat>

/*
Quality range - compression/subsampling
100 - lossless RGB compression
< KIMG_AVIF_QUALITY_BEST, 100 ) - YUV444 color subsampling
< KIMG_AVIF_QUALITY_HIGH, KIMG_AVIF_QUALITY_BEST ) - YUV422 color subsampling
< 0, KIMG_AVIF_QUALITY_HIGH ) - YUV420 color subsampling
< 0, KIMG_AVIF_QUALITY_LOW ) - lossy compression of alpha channel
*/

#ifndef KIMG_AVIF_DEFAULT_QUALITY
#define KIMG_AVIF_DEFAULT_QUALITY 68
#endif

#ifndef KIMG_AVIF_QUALITY_BEST
#define KIMG_AVIF_QUALITY_BEST 90
#endif

#ifndef KIMG_AVIF_QUALITY_HIGH
#define KIMG_AVIF_QUALITY_HIGH 80
#endif

#ifndef KIMG_AVIF_QUALITY_LOW
#define KIMG_AVIF_QUALITY_LOW 51
#endif

QAVIFHandler::QAVIFHandler()
    : m_parseState(ParseAvifNotParsed)
    , m_quality(KIMG_AVIF_DEFAULT_QUALITY)
    , m_container_width(0)
    , m_container_height(0)
    , m_rawAvifData(AVIF_DATA_EMPTY)
    , m_decoder(nullptr)
    , m_must_jump_to_next_image(false)
{
}

QAVIFHandler::~QAVIFHandler()
{
    if (m_decoder) {
        avifDecoderDestroy(m_decoder);
    }
}

bool QAVIFHandler::canRead() const
{
    if (m_parseState == ParseAvifNotParsed && !canRead(device())) {
        return false;
    }

    if (m_parseState != ParseAvifError) {
        setFormat("avif");

        if (m_parseState == ParseAvifFinished) {
            return false;
        }

        return true;
    }
    return false;
}

bool QAVIFHandler::canRead(QIODevice *device)
{
    if (!device) {
        return false;
    }
    QByteArray header = device->peek(144);
    if (header.size() < 12) {
        return false;
    }

    avifROData input;
    input.data = reinterpret_cast<const uint8_t *>(header.constData());
    input.size = header.size();

    if (avifPeekCompatibleFileType(&input)) {
        return true;
    }
    return false;
}

bool QAVIFHandler::ensureParsed() const
{
    if (m_parseState == ParseAvifSuccess || m_parseState == ParseAvifMetadata || m_parseState == ParseAvifFinished) {
        return true;
    }
    if (m_parseState == ParseAvifError) {
        return false;
    }

    QAVIFHandler *that = const_cast<QAVIFHandler *>(this);

    return that->ensureDecoder();
}

bool QAVIFHandler::ensureOpened() const
{
    if (m_parseState == ParseAvifSuccess || m_parseState == ParseAvifFinished) {
        return true;
    }
    if (m_parseState == ParseAvifError) {
        return false;
    }

    QAVIFHandler *that = const_cast<QAVIFHandler *>(this);
    if (ensureParsed()) {
        if (m_parseState == ParseAvifMetadata) {
            bool success = that->jumpToNextImage();
            that->m_parseState = success ? ParseAvifSuccess : ParseAvifError;
            return success;
        }
    }

    that->m_parseState = ParseAvifError;
    return false;
}

bool QAVIFHandler::ensureDecoder()
{
    if (m_decoder) {
        return true;
    }

    m_rawData = device()->readAll();

    m_rawAvifData.data = reinterpret_cast<const uint8_t *>(m_rawData.constData());
    m_rawAvifData.size = m_rawData.size();

    if (avifPeekCompatibleFileType(&m_rawAvifData) == AVIF_FALSE) {
        m_parseState = ParseAvifError;
        return false;
    }

    m_decoder = avifDecoderCreate();

    m_decoder->ignoreExif = AVIF_TRUE;
    m_decoder->ignoreXMP = AVIF_TRUE;

#if AVIF_VERSION >= 80400
    m_decoder->maxThreads = qBound(1, QThread::idealThreadCount(), 64);
#endif

#if AVIF_VERSION >= 90100
    m_decoder->strictFlags = AVIF_STRICT_DISABLED;
#endif

#if AVIF_VERSION >= 110000
    m_decoder->imageDimensionLimit = 65535;
#endif

    avifResult decodeResult;

    decodeResult = avifDecoderSetIOMemory(m_decoder, m_rawAvifData.data, m_rawAvifData.size);
    if (decodeResult != AVIF_RESULT_OK) {
        qWarning("ERROR: avifDecoderSetIOMemory failed: %s", avifResultToString(decodeResult));

        avifDecoderDestroy(m_decoder);
        m_decoder = nullptr;
        m_parseState = ParseAvifError;
        return false;
    }

    decodeResult = avifDecoderParse(m_decoder);
    if (decodeResult != AVIF_RESULT_OK) {
        qWarning("ERROR: Failed to parse input: %s", avifResultToString(decodeResult));

        avifDecoderDestroy(m_decoder);
        m_decoder = nullptr;
        m_parseState = ParseAvifError;
        return false;
    }

    m_container_width = m_decoder->image->width;
    m_container_height = m_decoder->image->height;

    if ((m_container_width > 65535) || (m_container_height > 65535)) {
        qWarning("AVIF image (%dx%d) is too large!", m_container_width, m_container_height);
        m_parseState = ParseAvifError;
        return false;
    }

    if ((m_container_width == 0) || (m_container_height == 0)) {
        qWarning("Empty image, nothing to decode");
        m_parseState = ParseAvifError;
        return false;
    }

    if (m_container_width > ((16384 * 16384) / m_container_height)) {
        qWarning("AVIF image (%dx%d) has more than 256 megapixels!", m_container_width, m_container_height);
        m_parseState = ParseAvifError;
        return false;
    }

    // calculate final dimensions with crop and rotate operations applied
    int new_width = m_container_width;
    int new_height = m_container_height;

    if (m_decoder->image->transformFlags & AVIF_TRANSFORM_CLAP) {
        if ((m_decoder->image->clap.widthD > 0) && (m_decoder->image->clap.heightD > 0) && (m_decoder->image->clap.horizOffD > 0)
            && (m_decoder->image->clap.vertOffD > 0)) {
            int crop_width = (int)((double)(m_decoder->image->clap.widthN) / (m_decoder->image->clap.widthD) + 0.5);
            if (crop_width < new_width && crop_width > 0) {
                new_width = crop_width;
            }
            int crop_height = (int)((double)(m_decoder->image->clap.heightN) / (m_decoder->image->clap.heightD) + 0.5);
            if (crop_height < new_height && crop_height > 0) {
                new_height = crop_height;
            }
        }
    }

    if (m_decoder->image->transformFlags & AVIF_TRANSFORM_IROT) {
        if (m_decoder->image->irot.angle == 1 || m_decoder->image->irot.angle == 3) {
            int tmp = new_width;
            new_width = new_height;
            new_height = tmp;
        }
    }

    m_estimated_dimensions.setWidth(new_width);
    m_estimated_dimensions.setHeight(new_height);

    m_parseState = ParseAvifMetadata;
    return true;
}

bool QAVIFHandler::decode_one_frame()
{
    if (!ensureParsed()) {
        return false;
    }

    bool loadalpha;

    if (m_decoder->image->alphaPlane) {
        loadalpha = true;
    } else {
        loadalpha = false;
    }

    QImage::Format resultformat;

    if (m_decoder->image->depth > 8) {
        if (loadalpha) {
            resultformat = QImage::Format_RGBA64;
        } else {
            resultformat = QImage::Format_RGBX64;
        }
    } else {
        if (loadalpha) {
            resultformat = QImage::Format_ARGB32;
        } else {
            resultformat = QImage::Format_RGB32;
        }
    }

    QImage result = imageAlloc(m_decoder->image->width, m_decoder->image->height, resultformat);
    if (result.isNull()) {
        qWarning("Memory cannot be allocated");
        return false;
    }

    QColorSpace colorspace;
    if (m_decoder->image->icc.data && (m_decoder->image->icc.size > 0)) {
        const QByteArray icc_data(reinterpret_cast<const char *>(m_decoder->image->icc.data), m_decoder->image->icc.size);
        colorspace = QColorSpace::fromIccProfile(icc_data);
        if (!colorspace.isValid()) {
            qWarning("AVIF image has Qt-unsupported or invalid ICC profile!");
        }
    } else {
        float prim[8] = {0.64f, 0.33f, 0.3f, 0.6f, 0.15f, 0.06f, 0.3127f, 0.329f};
        // outPrimaries: rX, rY, gX, gY, bX, bY, wX, wY
        avifColorPrimariesGetValues(m_decoder->image->colorPrimaries, prim);

        const QPointF redPoint(QAVIFHandler::CompatibleChromacity(prim[0], prim[1]));
        const QPointF greenPoint(QAVIFHandler::CompatibleChromacity(prim[2], prim[3]));
        const QPointF bluePoint(QAVIFHandler::CompatibleChromacity(prim[4], prim[5]));
        const QPointF whitePoint(QAVIFHandler::CompatibleChromacity(prim[6], prim[7]));

        QColorSpace::TransferFunction q_trc = QColorSpace::TransferFunction::Custom;
        float q_trc_gamma = 0.0f;

        switch (m_decoder->image->transferCharacteristics) {
        /* AVIF_TRANSFER_CHARACTERISTICS_BT470M */
        case 4:
            q_trc = QColorSpace::TransferFunction::Gamma;
            q_trc_gamma = 2.2f;
            break;
        /* AVIF_TRANSFER_CHARACTERISTICS_BT470BG */
        case 5:
            q_trc = QColorSpace::TransferFunction::Gamma;
            q_trc_gamma = 2.8f;
            break;
        /* AVIF_TRANSFER_CHARACTERISTICS_LINEAR */
        case 8:
            q_trc = QColorSpace::TransferFunction::Linear;
            break;
        /* AVIF_TRANSFER_CHARACTERISTICS_SRGB */
        case 0:
        case 2: /* AVIF_TRANSFER_CHARACTERISTICS_UNSPECIFIED */
        case 13:
            q_trc = QColorSpace::TransferFunction::SRgb;
            break;
        default:
            qWarning("CICP colorPrimaries: %d, transferCharacteristics: %d\nThe colorspace is unsupported by this plug-in yet.",
                     m_decoder->image->colorPrimaries,
                     m_decoder->image->transferCharacteristics);
            q_trc = QColorSpace::TransferFunction::SRgb;
            break;
        }

        if (q_trc != QColorSpace::TransferFunction::Custom) { // we create new colorspace using Qt
            switch (m_decoder->image->colorPrimaries) {
            /* AVIF_COLOR_PRIMARIES_BT709 */
            case 0:
            case 1:
            case 2: /* AVIF_COLOR_PRIMARIES_UNSPECIFIED */
                colorspace = QColorSpace(QColorSpace::Primaries::SRgb, q_trc, q_trc_gamma);
                break;
            /* AVIF_COLOR_PRIMARIES_SMPTE432 */
            case 12:
                colorspace = QColorSpace(QColorSpace::Primaries::DciP3D65, q_trc, q_trc_gamma);
                break;
            default:
                colorspace = QColorSpace(whitePoint, redPoint, greenPoint, bluePoint, q_trc, q_trc_gamma);
                break;
            }
        }

        if (!colorspace.isValid()) {
            qWarning("AVIF plugin created invalid QColorSpace from NCLX/CICP!");
        }
    }

    result.setColorSpace(colorspace);

    avifRGBImage rgb;
    avifRGBImageSetDefaults(&rgb, m_decoder->image);

#if AVIF_VERSION >= 1000000
    rgb.maxThreads = m_decoder->maxThreads;
#endif

    if (m_decoder->image->depth > 8) {
        rgb.depth = 16;
        rgb.format = AVIF_RGB_FORMAT_RGBA;

        if (!loadalpha && (m_decoder->image->yuvFormat == AVIF_PIXEL_FORMAT_YUV400)) {
            resultformat = QImage::Format_Grayscale16;
        }
    } else {
        rgb.depth = 8;
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        rgb.format = AVIF_RGB_FORMAT_BGRA;
#else
        rgb.format = AVIF_RGB_FORMAT_ARGB;
#endif

#if AVIF_VERSION >= 80400
        if (m_decoder->imageCount > 1) {
            /* accelerate animated AVIF */
            rgb.chromaUpsampling = AVIF_CHROMA_UPSAMPLING_FASTEST;
        }
#endif

        if (!loadalpha && (m_decoder->image->yuvFormat == AVIF_PIXEL_FORMAT_YUV400)) {
            resultformat = QImage::Format_Grayscale8;
        }
    }

    rgb.rowBytes = result.bytesPerLine();
    rgb.pixels = result.bits();

    avifResult res = avifImageYUVToRGB(m_decoder->image, &rgb);
    if (res != AVIF_RESULT_OK) {
        qWarning("ERROR in avifImageYUVToRGB: %s", avifResultToString(res));
        return false;
    }

    if (m_decoder->image->transformFlags & AVIF_TRANSFORM_CLAP) {
        if ((m_decoder->image->clap.widthD > 0) && (m_decoder->image->clap.heightD > 0) && (m_decoder->image->clap.horizOffD > 0)
            && (m_decoder->image->clap.vertOffD > 0)) {
            int new_width = (int)((double)(m_decoder->image->clap.widthN) / (m_decoder->image->clap.widthD) + 0.5);
            if (new_width > result.width()) {
                new_width = result.width();
            }

            int new_height = (int)((double)(m_decoder->image->clap.heightN) / (m_decoder->image->clap.heightD) + 0.5);
            if (new_height > result.height()) {
                new_height = result.height();
            }

            if (new_width > 0 && new_height > 0) {
                int offx =
                    ((double)((int32_t)m_decoder->image->clap.horizOffN)) / (m_decoder->image->clap.horizOffD) + (result.width() - new_width) / 2.0 + 0.5;
                if (offx < 0) {
                    offx = 0;
                } else if (offx > (result.width() - new_width)) {
                    offx = result.width() - new_width;
                }

                int offy =
                    ((double)((int32_t)m_decoder->image->clap.vertOffN)) / (m_decoder->image->clap.vertOffD) + (result.height() - new_height) / 2.0 + 0.5;
                if (offy < 0) {
                    offy = 0;
                } else if (offy > (result.height() - new_height)) {
                    offy = result.height() - new_height;
                }

                result = result.copy(offx, offy, new_width, new_height);
            }
        }

        else { // Zero values, we need to avoid 0 divide.
            qWarning("ERROR: Wrong values in avifCleanApertureBox");
        }
    }

    if (m_decoder->image->transformFlags & AVIF_TRANSFORM_IROT) {
        QTransform transform;
        switch (m_decoder->image->irot.angle) {
        case 1:
            transform.rotate(-90);
            result = result.transformed(transform);
            break;
        case 2:
            transform.rotate(180);
            result = result.transformed(transform);
            break;
        case 3:
            transform.rotate(90);
            result = result.transformed(transform);
            break;
        }
    }

    if (m_decoder->image->transformFlags & AVIF_TRANSFORM_IMIR) {
#if AVIF_VERSION > 90100 && AVIF_VERSION < 1000000
        switch (m_decoder->image->imir.mode) {
#else
        switch (m_decoder->image->imir.axis) {
#endif
        case 0: // top-to-bottom
            result = result.mirrored(false, true);
            break;
        case 1: // left-to-right
            result = result.mirrored(true, false);
            break;
        }
    }

    if (resultformat == result.format()) {
        m_current_image = result;
    } else {
        m_current_image = result.convertToFormat(resultformat);
    }

    m_estimated_dimensions = m_current_image.size();

    m_must_jump_to_next_image = false;
    return true;
}

bool QAVIFHandler::read(QImage *image)
{
    if (!ensureOpened()) {
        return false;
    }

    if (m_must_jump_to_next_image) {
        jumpToNextImage();
    }

    *image = m_current_image;
    if (imageCount() >= 2) {
        m_must_jump_to_next_image = true;
        if (m_decoder->imageIndex >= m_decoder->imageCount - 1) {
            // all frames in animation have been read
            m_parseState = ParseAvifFinished;
        }
    } else {
        // the static image has been read
        m_parseState = ParseAvifFinished;
    }
    return true;
}

bool QAVIFHandler::write(const QImage &image)
{
    if (image.format() == QImage::Format_Invalid) {
        qWarning("No image data to save!");
        return false;
    }

    if ((image.width() > 0) && (image.height() > 0)) {
        if ((image.width() > 65535) || (image.height() > 65535)) {
            qWarning("Image (%dx%d) is too large to save!", image.width(), image.height());
            return false;
        }

        if (image.width() > ((16384 * 16384) / image.height())) {
            qWarning("Image (%dx%d) will not be saved because it has more than 256 megapixels!", image.width(), image.height());
            return false;
        }

        if ((image.width() > 32768) || (image.height() > 32768)) {
            qWarning("Image (%dx%d) has a dimension above 32768 pixels, saved AVIF may not work in other software!", image.width(), image.height());
        }
    } else {
        qWarning("Image has zero dimension!");
        return false;
    }

    const char *encoder_name = avifCodecName(AVIF_CODEC_CHOICE_AUTO, AVIF_CODEC_FLAG_CAN_ENCODE);
    if (!encoder_name) {
        qWarning("Cannot save AVIF images because libavif was built without AV1 encoders!");
        return false;
    }

    bool lossless = false;
    if (m_quality >= 100) {
        if (avifCodecName(AVIF_CODEC_CHOICE_AOM, AVIF_CODEC_FLAG_CAN_ENCODE)) {
            lossless = true;
        } else {
            qWarning("You are using %s encoder. It is recommended to enable libAOM encoder in libavif to use lossless compression.", encoder_name);
        }
    }

    if (m_quality > 100) {
        m_quality = 100;
    } else if (m_quality < 0) {
        m_quality = KIMG_AVIF_DEFAULT_QUALITY;
    }

#if AVIF_VERSION < 1000000
    int maxQuantizer = AVIF_QUANTIZER_WORST_QUALITY * (100 - qBound(0, m_quality, 100)) / 100;
    int minQuantizer = 0;
    int maxQuantizerAlpha = 0;
#endif
    avifResult res;

    bool save_grayscale; // true - monochrome, false - colors
    int save_depth; // 8 or 10bit per channel
    QImage::Format tmpformat; // format for temporary image

    avifImage *avif = nullptr;

    // grayscale detection
    switch (image.format()) {
    case QImage::Format_Mono:
    case QImage::Format_MonoLSB:
    case QImage::Format_Grayscale8:
    case QImage::Format_Grayscale16:
        save_grayscale = true;
        break;
    case QImage::Format_Indexed8:
        save_grayscale = image.isGrayscale();
        break;
    default:
        save_grayscale = false;
        break;
    }

    // depth detection
    switch (image.format()) {
    case QImage::Format_BGR30:
    case QImage::Format_A2BGR30_Premultiplied:
    case QImage::Format_RGB30:
    case QImage::Format_A2RGB30_Premultiplied:
    case QImage::Format_Grayscale16:
    case QImage::Format_RGBX64:
    case QImage::Format_RGBA64:
    case QImage::Format_RGBA64_Premultiplied:
        save_depth = 10;
        break;
    default:
        if (image.depth() > 32) {
            save_depth = 10;
        } else {
            save_depth = 8;
        }
        break;
    }

#if AVIF_VERSION < 1000000
    // deprecated quality settings
    if (maxQuantizer > 20) {
        minQuantizer = maxQuantizer - 20;
        if (maxQuantizer > 40) { // we decrease quality of alpha channel here
            maxQuantizerAlpha = maxQuantizer - 40;
        }
    }
#endif

    if (save_grayscale && !image.hasAlphaChannel()) { // we are going to save grayscale image without alpha channel
        if (save_depth > 8) {
            tmpformat = QImage::Format_Grayscale16;
        } else {
            tmpformat = QImage::Format_Grayscale8;
        }
        QImage tmpgrayimage = image.convertToFormat(tmpformat);

        avif = avifImageCreate(tmpgrayimage.width(), tmpgrayimage.height(), save_depth, AVIF_PIXEL_FORMAT_YUV400);
#if AVIF_VERSION >= 110000
        res = avifImageAllocatePlanes(avif, AVIF_PLANES_YUV);
        if (res != AVIF_RESULT_OK) {
            qWarning("ERROR in avifImageAllocatePlanes: %s", avifResultToString(res));
            return false;
        }
#else
        avifImageAllocatePlanes(avif, AVIF_PLANES_YUV);
#endif

        if (tmpgrayimage.colorSpace().isValid()) {
            avif->colorPrimaries = (avifColorPrimaries)1;
            avif->matrixCoefficients = (avifMatrixCoefficients)1;

            switch (tmpgrayimage.colorSpace().transferFunction()) {
            case QColorSpace::TransferFunction::Linear:
                /* AVIF_TRANSFER_CHARACTERISTICS_LINEAR */
                avif->transferCharacteristics = (avifTransferCharacteristics)8;
                break;
            case QColorSpace::TransferFunction::SRgb:
                /* AVIF_TRANSFER_CHARACTERISTICS_SRGB */
                avif->transferCharacteristics = (avifTransferCharacteristics)13;
                break;
            default:
                /* AVIF_TRANSFER_CHARACTERISTICS_UNSPECIFIED */
                break;
            }
        }

        if (save_depth > 8) { // QImage::Format_Grayscale16
            for (int y = 0; y < tmpgrayimage.height(); y++) {
                const uint16_t *src16bit = reinterpret_cast<const uint16_t *>(tmpgrayimage.constScanLine(y));
                uint16_t *dest16bit = reinterpret_cast<uint16_t *>(avif->yuvPlanes[0] + y * avif->yuvRowBytes[0]);
                for (int x = 0; x < tmpgrayimage.width(); x++) {
                    int tmp_pixelval = (int)(((float)(*src16bit) / 65535.0f) * 1023.0f + 0.5f); // downgrade to 10 bits
                    *dest16bit = qBound(0, tmp_pixelval, 1023);
                    dest16bit++;
                    src16bit++;
                }
            }
        } else { // QImage::Format_Grayscale8
            for (int y = 0; y < tmpgrayimage.height(); y++) {
                const uchar *src8bit = tmpgrayimage.constScanLine(y);
                uint8_t *dest8bit = avif->yuvPlanes[0] + y * avif->yuvRowBytes[0];
                for (int x = 0; x < tmpgrayimage.width(); x++) {
                    *dest8bit = *src8bit;
                    dest8bit++;
                    src8bit++;
                }
            }
        }

    } else { // we are going to save color image
        if (save_depth > 8) {
            if (image.hasAlphaChannel()) {
                tmpformat = QImage::Format_RGBA64;
            } else {
                tmpformat = QImage::Format_RGBX64;
            }
        } else { // 8bit depth
            if (image.hasAlphaChannel()) {
                tmpformat = QImage::Format_RGBA8888;
            } else {
                tmpformat = QImage::Format_RGB888;
            }
        }

        QImage tmpcolorimage = image.convertToFormat(tmpformat);

        avifPixelFormat pixel_format = AVIF_PIXEL_FORMAT_YUV420;
        if (m_quality >= KIMG_AVIF_QUALITY_HIGH) {
            if (m_quality >= KIMG_AVIF_QUALITY_BEST) {
                pixel_format = AVIF_PIXEL_FORMAT_YUV444; // best quality
            } else {
                pixel_format = AVIF_PIXEL_FORMAT_YUV422; // high quality
            }
        }

        avifMatrixCoefficients matrix_to_save = (avifMatrixCoefficients)1; // default for Qt 5.12 and 5.13;

        avifColorPrimaries primaries_to_save = (avifColorPrimaries)2;
        avifTransferCharacteristics transfer_to_save = (avifTransferCharacteristics)2;
        QByteArray iccprofile;

        if (tmpcolorimage.colorSpace().isValid()) {
            switch (tmpcolorimage.colorSpace().primaries()) {
            case QColorSpace::Primaries::SRgb:
                /* AVIF_COLOR_PRIMARIES_BT709 */
                primaries_to_save = (avifColorPrimaries)1;
                /* AVIF_MATRIX_COEFFICIENTS_BT709 */
                matrix_to_save = (avifMatrixCoefficients)1;
                break;
            case QColorSpace::Primaries::DciP3D65:
                /* AVIF_NCLX_COLOUR_PRIMARIES_P3, AVIF_NCLX_COLOUR_PRIMARIES_SMPTE432 */
                primaries_to_save = (avifColorPrimaries)12;
                /* AVIF_MATRIX_COEFFICIENTS_CHROMA_DERIVED_NCL */
                matrix_to_save = (avifMatrixCoefficients)12;
                break;
            default:
                /* AVIF_TRANSFER_CHARACTERISTICS_UNSPECIFIED */
                primaries_to_save = (avifColorPrimaries)2;
                /* AVIF_MATRIX_COEFFICIENTS_UNSPECIFIED */
                matrix_to_save = (avifMatrixCoefficients)2;
                break;
            }

            switch (tmpcolorimage.colorSpace().transferFunction()) {
            case QColorSpace::TransferFunction::Linear:
                /* AVIF_TRANSFER_CHARACTERISTICS_LINEAR */
                transfer_to_save = (avifTransferCharacteristics)8;
                break;
            case QColorSpace::TransferFunction::Gamma:
                if (qAbs(tmpcolorimage.colorSpace().gamma() - 2.2f) < 0.1f) {
                    /* AVIF_TRANSFER_CHARACTERISTICS_BT470M */
                    transfer_to_save = (avifTransferCharacteristics)4;
                } else if (qAbs(tmpcolorimage.colorSpace().gamma() - 2.8f) < 0.1f) {
                    /* AVIF_TRANSFER_CHARACTERISTICS_BT470BG */
                    transfer_to_save = (avifTransferCharacteristics)5;
                } else {
                    /* AVIF_TRANSFER_CHARACTERISTICS_UNSPECIFIED */
                    transfer_to_save = (avifTransferCharacteristics)2;
                }
                break;
            case QColorSpace::TransferFunction::SRgb:
                /* AVIF_TRANSFER_CHARACTERISTICS_SRGB */
                transfer_to_save = (avifTransferCharacteristics)13;
                break;
            default:
                /* AVIF_TRANSFER_CHARACTERISTICS_UNSPECIFIED */
                transfer_to_save = (avifTransferCharacteristics)2;
                break;
            }

            // in case primaries or trc were not identified
            if ((primaries_to_save == 2) || (transfer_to_save == 2)) {
                if (lossless) {
                    iccprofile = tmpcolorimage.colorSpace().iccProfile();
                } else {
                    // upgrade image to higher bit depth
                    if (save_depth == 8) {
                        save_depth = 10;
                        if (tmpcolorimage.hasAlphaChannel()) {
                            tmpcolorimage.convertTo(QImage::Format_RGBA64);
                        } else {
                            tmpcolorimage.convertTo(QImage::Format_RGBX64);
                        }
                    }

                    if ((primaries_to_save == 2) && (transfer_to_save != 2)) { // other primaries but known trc
                        primaries_to_save = (avifColorPrimaries)1; // AVIF_COLOR_PRIMARIES_BT709
                        matrix_to_save = (avifMatrixCoefficients)1; // AVIF_MATRIX_COEFFICIENTS_BT709

                        switch (transfer_to_save) {
                        case 8: // AVIF_TRANSFER_CHARACTERISTICS_LINEAR
                            tmpcolorimage.convertToColorSpace(QColorSpace(QColorSpace::Primaries::SRgb, QColorSpace::TransferFunction::Linear));
                            break;
                        case 4: // AVIF_TRANSFER_CHARACTERISTICS_BT470M
                            tmpcolorimage.convertToColorSpace(QColorSpace(QColorSpace::Primaries::SRgb, 2.2f));
                            break;
                        case 5: // AVIF_TRANSFER_CHARACTERISTICS_BT470BG
                            tmpcolorimage.convertToColorSpace(QColorSpace(QColorSpace::Primaries::SRgb, 2.8f));
                            break;
                        default: // AVIF_TRANSFER_CHARACTERISTICS_SRGB + any other
                            tmpcolorimage.convertToColorSpace(QColorSpace(QColorSpace::Primaries::SRgb, QColorSpace::TransferFunction::SRgb));
                            transfer_to_save = (avifTransferCharacteristics)13;
                            break;
                        }
                    } else if ((primaries_to_save != 2) && (transfer_to_save == 2)) { // recognized primaries but other trc
                        transfer_to_save = (avifTransferCharacteristics)13;
                        tmpcolorimage.convertToColorSpace(tmpcolorimage.colorSpace().withTransferFunction(QColorSpace::TransferFunction::SRgb));
                    } else { // unrecognized profile
                        primaries_to_save = (avifColorPrimaries)1; // AVIF_COLOR_PRIMARIES_BT709
                        transfer_to_save = (avifTransferCharacteristics)13;
                        matrix_to_save = (avifMatrixCoefficients)1; // AVIF_MATRIX_COEFFICIENTS_BT709
                        tmpcolorimage.convertToColorSpace(QColorSpace(QColorSpace::Primaries::SRgb, QColorSpace::TransferFunction::SRgb));
                    }
                }
            }
        } else { // profile is unsupported by Qt
            iccprofile = tmpcolorimage.colorSpace().iccProfile();
            if (iccprofile.size() > 0) {
                matrix_to_save = (avifMatrixCoefficients)6;
            }
        }

        if (lossless && pixel_format == AVIF_PIXEL_FORMAT_YUV444) {
            matrix_to_save = (avifMatrixCoefficients)0;
        }
        avif = avifImageCreate(tmpcolorimage.width(), tmpcolorimage.height(), save_depth, pixel_format);
        avif->matrixCoefficients = matrix_to_save;

        avif->colorPrimaries = primaries_to_save;
        avif->transferCharacteristics = transfer_to_save;

        if (iccprofile.size() > 0) {
#if AVIF_VERSION >= 1000000
            res = avifImageSetProfileICC(avif, reinterpret_cast<const uint8_t *>(iccprofile.constData()), iccprofile.size());
            if (res != AVIF_RESULT_OK) {
                qWarning("ERROR in avifImageSetProfileICC: %s", avifResultToString(res));
                return false;
            }
#else
            avifImageSetProfileICC(avif, reinterpret_cast<const uint8_t *>(iccprofile.constData()), iccprofile.size());
#endif
        }

        avifRGBImage rgb;
        avifRGBImageSetDefaults(&rgb, avif);
        rgb.rowBytes = tmpcolorimage.bytesPerLine();
        rgb.pixels = const_cast<uint8_t *>(tmpcolorimage.constBits());

        if (save_depth > 8) { // 10bit depth
            rgb.depth = 16;

            if (!tmpcolorimage.hasAlphaChannel()) {
                rgb.ignoreAlpha = AVIF_TRUE;
            }

            rgb.format = AVIF_RGB_FORMAT_RGBA;
        } else { // 8bit depth
            rgb.depth = 8;

            if (tmpcolorimage.hasAlphaChannel()) {
                rgb.format = AVIF_RGB_FORMAT_RGBA;
            } else {
                rgb.format = AVIF_RGB_FORMAT_RGB;
            }
        }

        res = avifImageRGBToYUV(avif, &rgb);
        if (res != AVIF_RESULT_OK) {
            qWarning("ERROR in avifImageRGBToYUV: %s", avifResultToString(res));
            return false;
        }
    }

    avifRWData raw = AVIF_DATA_EMPTY;
    avifEncoder *encoder = avifEncoderCreate();
    encoder->maxThreads = qBound(1, QThread::idealThreadCount(), 64);

#if AVIF_VERSION < 1000000
    encoder->minQuantizer = minQuantizer;
    encoder->maxQuantizer = maxQuantizer;

    if (image.hasAlphaChannel()) {
        encoder->minQuantizerAlpha = AVIF_QUANTIZER_LOSSLESS;
        encoder->maxQuantizerAlpha = maxQuantizerAlpha;
    }
#else
    encoder->quality = m_quality;

    if (image.hasAlphaChannel()) {
        if (m_quality >= KIMG_AVIF_QUALITY_LOW) {
            encoder->qualityAlpha = 100;
        } else {
            encoder->qualityAlpha = 100 - (KIMG_AVIF_QUALITY_LOW - m_quality) / 2;
        }
    }
#endif

    encoder->speed = 6;

    res = avifEncoderWrite(encoder, avif, &raw);
    avifEncoderDestroy(encoder);
    avifImageDestroy(avif);

    if (res == AVIF_RESULT_OK) {
        qint64 status = device()->write(reinterpret_cast<const char *>(raw.data), raw.size);
        avifRWDataFree(&raw);

        if (status > 0) {
            return true;
        } else if (status == -1) {
            qWarning("Write error: %s", qUtf8Printable(device()->errorString()));
            return false;
        }
    } else {
        qWarning("ERROR: Failed to encode: %s", avifResultToString(res));
    }

    return false;
}

QVariant QAVIFHandler::option(ImageOption option) const
{
    if (option == Quality) {
        return m_quality;
    }

    if (!supportsOption(option) || !ensureParsed()) {
        return QVariant();
    }

    switch (option) {
    case Size:
        return m_estimated_dimensions;
    case Animation:
        if (imageCount() >= 2) {
            return true;
        } else {
            return false;
        }
    default:
        return QVariant();
    }
}

void QAVIFHandler::setOption(ImageOption option, const QVariant &value)
{
    switch (option) {
    case Quality:
        m_quality = value.toInt();
        if (m_quality > 100) {
            m_quality = 100;
        } else if (m_quality < 0) {
            m_quality = KIMG_AVIF_DEFAULT_QUALITY;
        }
        return;
    default:
        break;
    }
    QImageIOHandler::setOption(option, value);
}

bool QAVIFHandler::supportsOption(ImageOption option) const
{
    return option == Quality || option == Size || option == Animation;
}

int QAVIFHandler::imageCount() const
{
    if (!ensureParsed()) {
        return 0;
    }

    if (m_decoder->imageCount >= 1) {
        return m_decoder->imageCount;
    }
    return 0;
}

int QAVIFHandler::currentImageNumber() const
{
    if (m_parseState == ParseAvifNotParsed) {
        return -1;
    }

    if (m_parseState == ParseAvifError || !m_decoder) {
        return 0;
    }

    if (m_parseState == ParseAvifMetadata) {
        if (m_decoder->imageCount >= 2) {
            return -1;
        } else {
            return 0;
        }
    }

    return m_decoder->imageIndex;
}

bool QAVIFHandler::jumpToNextImage()
{
    if (!ensureParsed()) {
        return false;
    }

    avifResult decodeResult;

    if (m_decoder->imageIndex >= 0) {
        if (m_decoder->imageCount < 2) {
            m_parseState = ParseAvifSuccess;
            return true;
        }

        if (m_decoder->imageIndex >= m_decoder->imageCount - 1) { // start from beginning
            decodeResult = avifDecoderReset(m_decoder);
            if (decodeResult != AVIF_RESULT_OK) {
                qWarning("ERROR in avifDecoderReset: %s", avifResultToString(decodeResult));
                m_parseState = ParseAvifError;
                return false;
            }
        }
    }

    decodeResult = avifDecoderNextImage(m_decoder);

    if (decodeResult != AVIF_RESULT_OK) {
        qWarning("ERROR: Failed to decode Next image in sequence: %s", avifResultToString(decodeResult));
        m_parseState = ParseAvifError;
        return false;
    }

    if ((m_container_width != m_decoder->image->width) || (m_container_height != m_decoder->image->height)) {
        qWarning("Decoded image sequence size (%dx%d) do not match first image size (%dx%d)!",
                 m_decoder->image->width,
                 m_decoder->image->height,
                 m_container_width,
                 m_container_height);

        m_parseState = ParseAvifError;
        return false;
    }

    if (decode_one_frame()) {
        m_parseState = ParseAvifSuccess;
        return true;
    } else {
        m_parseState = ParseAvifError;
        return false;
    }
}

bool QAVIFHandler::jumpToImage(int imageNumber)
{
    if (!ensureParsed()) {
        return false;
    }

    if (m_decoder->imageCount < 2) { // not an animation
        if (imageNumber == 0) {
            if (ensureOpened()) {
                m_parseState = ParseAvifSuccess;
                return true;
            }
        }
        return false;
    }

    if (imageNumber < 0 || imageNumber >= m_decoder->imageCount) { // wrong index
        return false;
    }

    if (imageNumber == m_decoder->imageIndex) { // we are here already
        m_must_jump_to_next_image = false;
        m_parseState = ParseAvifSuccess;
        return true;
    }

    avifResult decodeResult = avifDecoderNthImage(m_decoder, imageNumber);

    if (decodeResult != AVIF_RESULT_OK) {
        qWarning("ERROR: Failed to decode %d th Image in sequence: %s", imageNumber, avifResultToString(decodeResult));
        m_parseState = ParseAvifError;
        return false;
    }

    if ((m_container_width != m_decoder->image->width) || (m_container_height != m_decoder->image->height)) {
        qWarning("Decoded image sequence size (%dx%d) do not match declared container size (%dx%d)!",
                 m_decoder->image->width,
                 m_decoder->image->height,
                 m_container_width,
                 m_container_height);

        m_parseState = ParseAvifError;
        return false;
    }

    if (decode_one_frame()) {
        m_parseState = ParseAvifSuccess;
        return true;
    } else {
        m_parseState = ParseAvifError;
        return false;
    }
}

int QAVIFHandler::nextImageDelay() const
{
    if (!ensureOpened()) {
        return 0;
    }

    if (m_decoder->imageCount < 2) {
        return 0;
    }

    int delay_ms = 1000.0 * m_decoder->imageTiming.duration;
    if (delay_ms < 1) {
        delay_ms = 1;
    }
    return delay_ms;
}

int QAVIFHandler::loopCount() const
{
    if (!ensureParsed()) {
        return 0;
    }

    if (m_decoder->imageCount < 2) {
        return 0;
    }

#if AVIF_VERSION >= 1000000
    if (m_decoder->repetitionCount >= 0) {
        return m_decoder->repetitionCount;
    }
#endif
    // Endless loop to work around https://github.com/AOMediaCodec/libavif/issues/347
    return -1;
}

QPointF QAVIFHandler::CompatibleChromacity(qreal chrX, qreal chrY)
{
    chrX = qBound(qreal(0.0), chrX, qreal(1.0));
    chrY = qBound(qreal(DBL_MIN), chrY, qreal(1.0));

    if ((chrX + chrY) > qreal(1.0)) {
        chrX = qreal(1.0) - chrY;
    }

    return QPointF(chrX, chrY);
}

QImageIOPlugin::Capabilities QAVIFPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    static const bool isAvifDecoderAvailable(avifCodecName(AVIF_CODEC_CHOICE_AUTO, AVIF_CODEC_FLAG_CAN_DECODE) != nullptr);
    static const bool isAvifEncoderAvailable(avifCodecName(AVIF_CODEC_CHOICE_AUTO, AVIF_CODEC_FLAG_CAN_ENCODE) != nullptr);

    if (format == "avif") {
        Capabilities format_cap;
        if (isAvifDecoderAvailable) {
            format_cap |= CanRead;
        }
        if (isAvifEncoderAvailable) {
            format_cap |= CanWrite;
        }
        return format_cap;
    }

    if (format == "avifs") {
        Capabilities format_cap;
        if (isAvifDecoderAvailable) {
            format_cap |= CanRead;
        }
        return format_cap;
    }

    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && QAVIFHandler::canRead(device) && isAvifDecoderAvailable) {
        cap |= CanRead;
    }
    if (device->isWritable() && isAvifEncoderAvailable) {
        cap |= CanWrite;
    }
    return cap;
}

QImageIOHandler *QAVIFPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new QAVIFHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_avif_p.cpp"
