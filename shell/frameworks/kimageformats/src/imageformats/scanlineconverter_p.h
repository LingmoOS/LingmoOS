/*
    SPDX-FileCopyrightText: 2023 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SCANLINECONVERTER_P_H
#define SCANLINECONVERTER_P_H

#include <QColorSpace>
#include <QImage>

/*!
 * \brief The scanlineFormatConversion class
 * A class to convert an image scan line. It introduces some overhead on small images
 * but performs better on large images. :)
 */
class ScanLineConverter
{
public:
    ScanLineConverter(const QImage::Format &targetFormat);
    ScanLineConverter(const ScanLineConverter &other);
    ScanLineConverter &operator=(const ScanLineConverter &other);

    /*!
     * \brief targetFormat
     * \return The target format set in the constructor.
     */
    QImage::Format targetFormat() const;

    /*!
     * \brief setTargetColorSpace
     * Set the colorspace conversion.
     *
     * In addition to format conversion, it is also possible to convert the color
     * space if the source image has a different one set.
     * The conversion is done on the source format if and only if the image
     * has a color depth greater than 24 bit and the color profile set is different
     * from that of the image itself.
     * \param colorSpace
     */
    void setTargetColorSpace(const QColorSpace &colorSpace);
    QColorSpace targetColorSpace() const;

    /*!
     * \brief setDefaultSourceColorSpace
     * Set the source color space to use when the image does not have a color space.
     * \param colorSpace
     */
    void setDefaultSourceColorSpace(const QColorSpace &colorSpace);
    QColorSpace defaultSourceColorSpace() const;

    /*!
     * \brief convertedScanLine
     * Convert the scanline \a y.
     * \note If the image format (and color space) is the same of converted format, it returns the image scan line.
     * \return The scan line converted.
     */
    const uchar *convertedScanLine(const QImage &image, qint32 y);

    /*!
     * \brief bytesPerLine
     * \return The size of the last converted scanline.
     */
    qsizetype bytesPerLine() const;

    /*!
     * \brief isColorSpaceConversionNeeded
     * Calculates if a color space conversion is needed.
     * \note Only 24 bit or grater images.
     * \param image The source image.
     * \param targetColorSpace The target color space.
     * \param defaultColorSpace The default color space to use it image does not contains one.
     * \return True if the conversion should be done otherwise false.
     */
    static bool isColorSpaceConversionNeeded(const QImage &image, const QColorSpace &targetColorSpace, const QColorSpace &defaultColorSpace = QColorSpace());

    /*!
     * \brief isColorSpaceConversionNeeded
     */
    inline bool isColorSpaceConversionNeeded(const QImage &image) const
    {
        return isColorSpaceConversionNeeded(image, _colorSpace, _defaultColorSpace);
    }

private:
    // data
    QImage::Format _targetFormat;
    QColorSpace _colorSpace;
    QColorSpace _defaultColorSpace;

    // internal buffers
    QImage _tmpBuffer;
    QImage _convBuffer;
};

#endif // SCANLINECONVERTER_P_H
