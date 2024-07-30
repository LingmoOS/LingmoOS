/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2005 Christoph Hormann <chris_hormann@gmx.de>
    SPDX-FileCopyrightText: 2005 Ignacio Castaño <castanyo@yahoo.es>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "hdr_p.h"
#include "util_p.h"

#include <QColorSpace>
#include <QDataStream>
#include <QFloat16>
#include <QImage>
#include <QLoggingCategory>
#include <QRegularExpressionMatch>

#include <QDebug>

/* *** HDR_HALF_QUALITY ***
 * If defined, a 16-bits float image is created, otherwise a 32-bits float ones (default).
 */
//#define HDR_HALF_QUALITY // default commented -> you should define it in your cmake file

typedef unsigned char uchar;

Q_LOGGING_CATEGORY(HDRPLUGIN, "kf.imageformats.plugins.hdr", QtWarningMsg)

#define MAXLINE 1024
#define MINELEN 8 // minimum scanline length for encoding
#define MAXELEN 0x7fff // maximum scanline length for encoding

class Header
{
public:
    Header()
    {
        m_colorSpace = QColorSpace(QColorSpace::SRgbLinear);
        m_transformation = QImageIOHandler::TransformationNone;
    }
    Header(const Header&) = default;
    Header& operator=(const Header&) = default;

    bool isValid() const { return width() > 0 && height() > 0; }
    qint32 width() const { return(m_size.width()); }
    qint32 height() const { return(m_size.height()); }
    QString software() const { return(m_software); }
    QImageIOHandler::Transformations transformation() const { return(m_transformation); }

    /*!
     * \brief colorSpace
     *
     * The color space for the image.
     *
     * The CIE (x,y) chromaticity coordinates of the three (RGB)
     * primaries and the white point used to standardize the picture's
     * color system. This is used mainly by the ra_xyze program to
     * convert between color systems. If no PRIMARIES line
     * appears, we assume the standard primaries defined in
     * src/common/color.h, namely "0.640 0.330 0.290
     * 0.600 0.150 0.060 0.333 0.333" for red, green, blue
     * and white, respectively.
     */
    QColorSpace colorSpace() const { return(m_colorSpace); }

    /*!
     * \brief exposure
     *
     * A single floating point number indicating a multiplier that has
     * been applied to all the pixels in the file. EXPOSURE values are
     * cumulative, so the original pixel values (i.e., radiances in
     * watts/steradian/m^2) must be derived by taking the values in the
     * file and dividing by all the EXPOSURE settings multiplied
     * together. No EXPOSURE setting implies that no exposure
     * changes have taken place.
     */
    float exposure() const {
        float mul = 1;
        for (auto&& v : m_exposure)
            mul *= v;
        return mul;
    }

    QImageIOHandler::Transformations m_transformation;
    QColorSpace m_colorSpace;
    QString m_software;
    QSize m_size;
    QList<float> m_exposure;
};

class HDRHandlerPrivate
{
public:
    HDRHandlerPrivate()
    {
    }
    ~HDRHandlerPrivate()
    {
    }

    const Header& header(QIODevice *device)
    {
        auto&& h = m_header;
        if (h.isValid()) {
            return h;
        }
        h = readHeader(device);
        return h;
    }

    static Header readHeader(QIODevice *device)
    {
        Header h;

        int len;
        QByteArray line(MAXLINE + 1, Qt::Uninitialized);
        QByteArray format;

        // Parse header
        do {
            len = device->readLine(line.data(), MAXLINE);

            if (line.startsWith("FORMAT=")) {
                format = line.mid(7, len - 7).trimmed();
            }
            if (line.startsWith("SOFTWARE=")) {
                h.m_software = QString::fromUtf8(line.mid(9, len - 9)).trimmed();
            }
            if (line.startsWith("EXPOSURE=")) {
                auto ok = false;
                auto ex = QLocale::c().toFloat(QString::fromLatin1(line.mid(9, len - 9)).trimmed(), &ok);
                if (ok)
                    h.m_exposure << ex;
            }
            if (line.startsWith("PRIMARIES=")) {
                auto list = line.mid(10, len - 10).trimmed().split(' ');
                QList<double> primaries;
                for (auto&& v : list) {
                    auto ok = false;
                    auto d = QLocale::c().toDouble(QString::fromLatin1(v), &ok);
                    if (ok)
                        primaries << d;
                }
                if (primaries.size() == 8) {
                    auto cs = QColorSpace(QPointF(primaries.at(6), primaries.at(7)),
                                          QPointF(primaries.at(0), primaries.at(1)),
                                          QPointF(primaries.at(2), primaries.at(3)),
                                          QPointF(primaries.at(4), primaries.at(5)),
                                          QColorSpace::TransferFunction::Linear);
                    cs.setDescription(QStringLiteral("Embedded RGB"));
                    if (cs.isValid())
                        h.m_colorSpace = cs;
                }
            }

        } while ((len > 0) && (line[0] != '\n'));

        if (format != "32-bit_rle_rgbe") {
            qCDebug(HDRPLUGIN) << "Unknown HDR format:" << format;
            return h;
        }

        len = device->readLine(line.data(), MAXLINE);
        line.resize(len);

        /*
         * Handle flipping and rotation, as per the spec below.
         * The single resolution line consists of 4 values, a X and Y label each followed by a numerical
         * integer value. The X and Y are immediately preceded by a sign which can be used to indicate
         * flipping, the order of the X and Y indicate rotation. The standard coordinate system for
         * Radiance images would have the following resolution string -Y N +X N. This indicates that the
         * vertical axis runs down the file and the X axis is to the right (imagining the image as a
         * rectangular block of data). A -X would indicate a horizontal flip of the image. A +Y would
         * indicate a vertical flip. If the X value appears before the Y value then that indicates that
         * the image is stored in column order rather than row order, that is, it is rotated by 90 degrees.
         * The reader can convince themselves that the 8 combinations cover all the possible image orientations
         * and rotations.
         */
        QRegularExpression resolutionRegExp(QStringLiteral("([+\\-][XY])\\s+([0-9]+)\\s+([+\\-][XY])\\s+([0-9]+)\n"));
        QRegularExpressionMatch match = resolutionRegExp.match(QString::fromLatin1(line));
        if (!match.hasMatch()) {
            qCDebug(HDRPLUGIN) << "Invalid HDR file, the first line after the header didn't have the expected format:" << line;
            return h;
        }

        auto c0 = match.captured(1);
        auto c1 = match.captured(3);
        if (c0.at(1) == u'Y') {
            if (c0.at(0) == u'-' && c1.at(0) == u'+')
                h.m_transformation = QImageIOHandler::TransformationNone;
            if (c0.at(0) == u'-' && c1.at(0) == u'-')
                h.m_transformation = QImageIOHandler::TransformationMirror;
            if (c0.at(0) == u'+' && c1.at(0) == u'+')
                h.m_transformation = QImageIOHandler::TransformationFlip;
            if (c0.at(0) == u'+' && c1.at(0) == u'-')
                h.m_transformation = QImageIOHandler::TransformationRotate180;
        }
        else {
            if (c0.at(0) == u'-' && c1.at(0) == u'+')
                h.m_transformation = QImageIOHandler::TransformationRotate90;
            if (c0.at(0) == u'-' && c1.at(0) == u'-')
                h.m_transformation = QImageIOHandler::TransformationMirrorAndRotate90;
            if (c0.at(0) == u'+' && c1.at(0) == u'+')
                h.m_transformation = QImageIOHandler::TransformationFlipAndRotate90;
            if (c0.at(0) == u'+' && c1.at(0) == u'-')
                h.m_transformation = QImageIOHandler::TransformationRotate270;
        }

        h.m_size = QSize(match.captured(4).toInt(), match.captured(2).toInt());
        return h;
    }

private:
    Header m_header;
};

// read an old style line from the hdr image file
// if 'first' is true the first byte is already read
static bool Read_Old_Line(uchar *image, int width, QDataStream &s)
{
    int rshift = 0;
    int i;

    uchar *start = image;
    while (width > 0) {
        s >> image[0];
        s >> image[1];
        s >> image[2];
        s >> image[3];

        if (s.atEnd()) {
            return false;
        }

        if ((image[0] == 1) && (image[1] == 1) && (image[2] == 1)) {
            // NOTE: we don't have an image sample that cover this code
            if (rshift > 31) {
                return false;
            }
            for (i = image[3] << rshift; i > 0 && width > 0; i--) {
                if (image == start) {
                    return false; // you cannot be here at the first run
                }
                // memcpy(image, image-4, 4);
                (uint &)image[0] = (uint &)image[0 - 4];
                image += 4;
                width--;
            }
            rshift += 8;
        } else {
            image += 4;
            width--;
            rshift = 0;
        }
    }
    return true;
}

template<class float_T>
void RGBE_To_QRgbLine(uchar *image, float_T *scanline, const Header& h)
{
    auto exposure = h.exposure();
    for (int j = 0, width = h.width(); j < width; j++) {
        // v = ldexp(1.0, int(image[3]) - 128);
        float v;
        int e = qBound(-31, int(image[3]) - 128, 31);
        if (e > 0) {
            v = float(1 << e);
        } else {
            v = 1.0f / float(1 << -e);
        }

        auto j4 = j * 4;
        auto vn = v / 255.0f;
        if (exposure > 0) {
            vn /= exposure;
        }

        scanline[j4] = float_T(float(image[0]) * vn);
        scanline[j4 + 1] = float_T(float(image[1]) * vn);
        scanline[j4 + 2] = float_T(float(image[2]) * vn);
        scanline[j4 + 3] = float_T(1.0f);
        image += 4;
    }
}

QImage::Format imageFormat()
{
#ifdef HDR_HALF_QUALITY
    return QImage::Format_RGBX16FPx4;
#else
    return QImage::Format_RGBX32FPx4;
#endif
}

// Load the HDR image.
static bool LoadHDR(QDataStream &s, const Header& h, QImage &img)
{
    uchar val;
    uchar code;

    const int width = h.width();
    const int height = h.height();

    // Create dst image.
    img = imageAlloc(width, height, imageFormat());
    if (img.isNull()) {
        qCDebug(HDRPLUGIN) << "Couldn't create image with size" << width << height << "and format RGB32";
        return false;
    }

    QByteArray lineArray;
    lineArray.resize(4 * width);
    uchar *image = reinterpret_cast<uchar *>(lineArray.data());

    for (int cline = 0; cline < height; cline++) {
#ifdef HDR_HALF_QUALITY
        auto scanline = reinterpret_cast<qfloat16 *>(img.scanLine(cline));
#else
        auto scanline = reinterpret_cast<float *>(img.scanLine(cline));
#endif

        // determine scanline type
        if ((width < MINELEN) || (MAXELEN < width)) {
            Read_Old_Line(image, width, s);
            RGBE_To_QRgbLine(image, scanline, h);
            continue;
        }

        s >> val;

        if (s.atEnd()) {
            return true;
        }

        if (val != 2) {
            s.device()->ungetChar(val);
            Read_Old_Line(image, width, s);
            RGBE_To_QRgbLine(image, scanline, h);
            continue;
        }

        s >> image[1];
        s >> image[2];
        s >> image[3];

        if (s.atEnd()) {
            return true;
        }

        if ((image[1] != 2) || (image[2] & 128)) {
            image[0] = 2;
            Read_Old_Line(image + 4, width - 1, s);
            RGBE_To_QRgbLine(image, scanline, h);
            continue;
        }

        if ((image[2] << 8 | image[3]) != width) {
            qCDebug(HDRPLUGIN) << "Line of pixels had width" << (image[2] << 8 | image[3]) << "instead of" << width;
            return false;
        }

        // read each component
        for (int i = 0, len = int(lineArray.size()); i < 4; i++) {
            for (int j = 0; j < width;) {
                s >> code;
                if (s.atEnd()) {
                    qCDebug(HDRPLUGIN) << "Truncated HDR file";
                    return false;
                }
                if (code > 128) {
                    // run
                    code &= 127;
                    s >> val;
                    while (code != 0) {
                        auto idx = i + j * 4;
                        if (idx < len) {
                            image[idx] = val;
                        }
                        j++;
                        code--;
                    }
                } else {
                    // non-run
                    while (code != 0) {
                        auto idx = i + j * 4;
                        if (idx < len) {
                            s >> image[idx];
                        }
                        j++;
                        code--;
                    }
                }
            }
        }
        RGBE_To_QRgbLine(image, scanline, h);
    }

    return true;
}

bool HDRHandler::read(QImage *outImage)
{
    QDataStream s(device());

    const Header& h = d->header(s.device());
    if (!h.isValid()) {
        return false;
    }

    QImage img;
    if (!LoadHDR(s, h, img)) {
        // qDebug() << "Error loading HDR file.";
        return false;
    }

    // By setting the linear color space, programs that support profiles display HDR files as in GIMP and Photoshop.
    img.setColorSpace(h.colorSpace());

    // Metadata
    if (!h.software().isEmpty()) {
        img.setText(QStringLiteral(META_KEY_SOFTWARE), h.software());
    }

    *outImage = img;
    return true;
}

bool HDRHandler::supportsOption(ImageOption option) const
{
    if (option == QImageIOHandler::Size) {
        return true;
    }
    if (option == QImageIOHandler::ImageFormat) {
        return true;
    }
    if (option == QImageIOHandler::ImageTransformation) {
        return true;
    }
    return false;
}

QVariant HDRHandler::option(ImageOption option) const
{
    QVariant v;

    if (option == QImageIOHandler::Size) {
        if (auto dev = device()) {
            auto&& h = d->header(dev);
            if (h.isValid()) {
                v = QVariant::fromValue(h.m_size);
            }
        }
    }

    if (option == QImageIOHandler::ImageFormat) {
        v = QVariant::fromValue(imageFormat());
    }

    if (option == QImageIOHandler::ImageTransformation) {
        if (auto dev = device()) {
            auto&& h = d->header(dev);
            if (h.isValid()) {
                v = QVariant::fromValue(h.transformation());
            }
        }
    }

    return v;
}

HDRHandler::HDRHandler()
    : QImageIOHandler()
    , d(new HDRHandlerPrivate)
{
}

bool HDRHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("hdr");
        return true;
    }
    return false;
}

bool HDRHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("HDRHandler::canRead() called with no device");
        return false;
    }

    // the .pic taken from official test cases does not start with this string but can be loaded.
    if(device->peek(11) == "#?RADIANCE\n" || device->peek(7) == "#?RGBE\n") {
        return true;
    }

    // allow to load offical test cases: https://radsite.lbl.gov/radiance/framed.html
    device->startTransaction();
    auto h = HDRHandlerPrivate::readHeader(device);
    device->rollbackTransaction();
    if (h.isValid()) {
        return true;
    }

    return false;
}

QImageIOPlugin::Capabilities HDRPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "hdr") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && HDRHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *HDRPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new HDRHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_hdr_p.cpp"
