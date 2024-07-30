/*
    The high dynamic range EXR format support for QImage.

    SPDX-FileCopyrightText: 2003 Brad Hards <bradh@frogmouth.net>
    SPDX-FileCopyrightText: 2023 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_EXR_P_H
#define KIMG_EXR_P_H

#include <QImageIOPlugin>

/*!
 * \brief The EXRHandler class
 * The handler uses the OpenEXR reference implementation of the EXR file format.
 *
 * The purpose of EXR format is to accurately and efficiently represent high-dynamic-range scene-linear
 * image data and associated metadata.
 *
 * Both reading and writing of EXR files is supported. When saving, the image is converted to 16-bit
 * and sRGB Linear color space (if not already so). If no color space is set in the image, sRGB is assumed.
 * When the handler is compiled with the default compile options, the loaded image is a 16-bit image
 * with linear color space.
 * Multiview images are also supported (read only) via imageCount(), jumpToImage(), etc....
 *
 * The following QImageIOHandler options are supported:
 * - ImageFormat: The image's data format returned by the handler.
 * - Size: The size of the image.
 * - CompressionRatio: The compression ratio of the image data (see OpenEXR compression schemes).
 * - Quality: The quality level of the image (see OpenEXR compression level of lossy codecs).
 *
 * The following metadata are set/get via QImage::setText()/QImage::text() in both read/write (if any):
 * - Latitude, Longitude, Altitude: Geographic coordinates (Float converted to string).
 * - CreationDate: Date the image was captured/created (QDateTime converted to string using Qt::ISODate).
 * - Comment: Additional image information in human-readable form, for example a verbal description of the image (QString).
 * - Owner: Name of the owner of the image (QString).
 *
 * In addition, information about image resolution is preserved and the preview is written for images larger
 * than 1024px.
 *
 * The following compile options are supported (defines):
 * - EXR_MAX_IMAGE_WIDTH: Maximum image width supported (read/write, default: 300000 px).
 * - EXR_MAX_IMAGE_HEIGHT: Maximum image height supported (read/write, default: 300000 px).
 * - EXR_LINES_PER_BLOCK: The number of scanlines buffered on both read and write operations.\n
 *                        The higher the value, the greater the parallelization but the RAM consumption increases (default: 128)
 * - EXR_USE_LEGACY_CONVERSIONS: The result image is an 8-bit RGB(A) converted without icc profiles (read, default: undefined).
 * - EXR_CONVERT_TO_SRGB: The resulting image is convertef in the sRGB color space (read, default: undefined).
 * - EXR_DISABLE_XMP_ATTRIBUTE: Disable the stores of XMP values in a non-standard attribute named "xmp".\n
 *                              The QImage metadata used is "XML:com.adobe.xmp" (write, default: undefined).
 */
class EXRHandler : public QImageIOHandler
{
public:
    EXRHandler();

    bool canRead() const override;
    bool read(QImage *outImage) override;
    bool write(const QImage &image) override;

    void setOption(ImageOption option, const QVariant &value) override;
    bool supportsOption(QImageIOHandler::ImageOption option) const override;
    QVariant option(QImageIOHandler::ImageOption option) const override;

    bool jumpToNextImage() override;
    bool jumpToImage(int imageNumber) override;
    int imageCount() const override;
    int currentImageNumber() const override;

    static bool canRead(QIODevice *device);

private:
    /*!
     * \brief m_compressionRatio
     * Value set by QImageWriter::setCompression().
     *
     * The compression scheme is the same as defined by OpenEXR library:
     * - 0: no compression
     * - 1: run length encoding
     * - 2: zlib compression, one scan line at a time
     * - 3: zlib compression, in blocks of 16 scan lines
     * - 4: piz-based wavelet compression (default)
     * - 5: lossy 24-bit float compression
     * - 6: lossy 4-by-4 pixel block compression, fixed compression rate
     * - 7: lossy 4-by-4 pixel block compression, fields are compressed more
     * - 8: lossy DCT based compression, in blocks of 32 scanlines. More efficient for partial buffer access.
     * - 9: lossy DCT based compression, in blocks of 256 scanlines. More efficient space wise and faster to decode full frames than DWAA_COMPRESSION.
     */
    qint32 m_compressionRatio;

    /*!
     * \brief m_quality
     * Value set by QImageWriter::setQuality().
     *
     * The quality is used on DCT compression schemes only with a
     * supposed value between 0 and 100 (default: 45).
     */
    qint32 m_quality;

    /*!
     * \brief m_imageNumber
     * Value set by QImageReader::jumpToImage() or QImageReader::jumpToNextImage().
     * The number of view selected in a multiview image.
     */
    qint32 m_imageNumber;

    /*!
     * \brief m_imageCount
     * The total number of views (cache value)
     */
    mutable qint32 m_imageCount;

    /*!
     * \brief m_startPos
     * The initial device position to allow multi image load (cache value).
     */
    qint64 m_startPos;
};

class EXRPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "exr.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_EXR_P_H
