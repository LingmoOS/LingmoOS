/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Dominik Seichter <domseichter@web.de>
    SPDX-FileCopyrightText: 2004 Ignacio Castaño <castano@ludicon.com>
    SPDX-FileCopyrightText: 2010 Troy Unrau <troy@kde.org>
    SPDX-FileCopyrightText: 2023 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ras_p.h"
#include "util_p.h"

#include <QDataStream>
#include <QDebug>
#include <QImage>

namespace // Private.
{
// format info from http://www.fileformat.info/format/sunraster/egff.htm

// Header format of saved files.
quint32 rasMagicBigEndian = 0x59a66a95;
// quint32 rasMagicLittleEndian = 0x956aa659; # used to support wrong encoded files

enum RASType {
    RAS_TYPE_OLD = 0x0,
    RAS_TYPE_STANDARD = 0x1,
    RAS_TYPE_BYTE_ENCODED = 0x2,
    RAS_TYPE_RGB_FORMAT = 0x3,
    RAS_TYPE_TIFF_FORMAT = 0x4,
    RAS_TYPE_IFF_FORMAT = 0x5,
    RAS_TYPE_EXPERIMENTAL = 0xFFFF,
};

enum RASColorMapType {
    RAS_COLOR_MAP_TYPE_NONE = 0x0,
    RAS_COLOR_MAP_TYPE_RGB = 0x1,
    RAS_COLOR_MAP_TYPE_RAW = 0x2,
};

struct RasHeader {
    quint32 MagicNumber = 0;
    quint32 Width = 0;
    quint32 Height = 0;
    quint32 Depth = 0;
    quint32 Length = 0;
    quint32 Type = 0;
    quint32 ColorMapType = 0;
    quint32 ColorMapLength = 0;
    enum {
        SIZE = 32,
    }; // 8 fields of four bytes each
};

static QDataStream &operator>>(QDataStream &s, RasHeader &head)
{
    s >> head.MagicNumber;
    s >> head.Width;
    s >> head.Height;
    s >> head.Depth;
    s >> head.Length;
    s >> head.Type;
    s >> head.ColorMapType;
    s >> head.ColorMapLength;
    /*qDebug() << "MagicNumber: " << head.MagicNumber
             << "Width: " << head.Width
             << "Height: " << head.Height
             << "Depth: " << head.Depth
             << "Length: " << head.Length
             << "Type: " << head.Type
             << "ColorMapType: " << head.ColorMapType
             << "ColorMapLength: " << head.ColorMapLength;*/
    return s;
}

static bool IsSupported(const RasHeader &head)
{
    // check magic number
    if (head.MagicNumber != rasMagicBigEndian) {
        return false;
    }
    // check for an appropriate depth
    if (head.Depth != 1 && head.Depth != 8 && head.Depth != 24 && head.Depth != 32) {
        return false;
    }
    if (head.Width == 0 || head.Height == 0) {
        return false;
    }
    // the Type field adds support for RLE(BGR), RGB and other encodings
    // we support Type 1: Normal(BGR), Type 2: RLE(BGR) and Type 3: Normal(RGB) ONLY!
    // TODO: add support for Type 4,5: TIFF/IFF
    if (!(head.Type == RAS_TYPE_STANDARD || head.Type == RAS_TYPE_RGB_FORMAT || head.Type == RAS_TYPE_BYTE_ENCODED)) {
        return false;
    }
    return true;
}

static QImage::Format imageFormat(const RasHeader &header)
{
    if (header.ColorMapType == RAS_COLOR_MAP_TYPE_RGB) {
        return QImage::Format_Indexed8;
    }
    if (header.Depth == 8 && header.ColorMapType == RAS_COLOR_MAP_TYPE_NONE) {
        return QImage::Format_Grayscale8;
    }
    if (header.Depth == 1) {
        return QImage::Format_Mono;
    }
    return QImage::Format_RGB32;
}

class LineDecoder
{
public:
    LineDecoder(QIODevice *d, const RasHeader &ras)
        : device(d)
        , header(ras)
    {
    }

    QByteArray readLine(qint64 size)
    {
        /* *** uncompressed
         */
        if (header.Type != RAS_TYPE_BYTE_ENCODED) {
            return device->read(size);
        }

        /* *** rle compressed
         * The Run-length encoding (RLE) scheme optionally used in Sun Raster
         * files (Type = 0002h) is used to encode bytes of image data
         * separately. RLE encoding may be found in any Sun Raster file
         * regardless of the type of image data it contains.
         *
         * The RLE packets are typically three bytes in size:
         * - The first byte is a Flag Value indicating the type of RLE packet.
         * - The second byte is the Run Count.
         * - The third byte is the Run Value.
         *
         * A Flag Value of 80h is followed by a Run Count in the range of 01h
         * to FFh. The Run Value follows the Run count and is in the range of
         * 00h to FFh. The pixel run is the Run Value repeated Run Count times.
         * There are two exceptions to this algorithm. First, if the Run Count
         * following the Flag Value is 00h, this is an indication that the run
         * is a single byte in length and has a value of 80h. And second, if
         * the Flag Value is not 80h, then it is assumed that the data is
         * unencoded pixel data and is written directly to the output stream.
         *
         * source: http://www.fileformat.info/format/sunraster/egff.htm
         */
        for (qsizetype psz = 0, ptr = 0; uncBuffer.size() < size;) {
            rleBuffer.append(device->read(std::min(qint64(32768), size)));
            qsizetype sz = rleBuffer.size();
            if (psz == sz) {
                break; // avoid infinite loop (data corrupted?!)
            }
            auto data = reinterpret_cast<uchar *>(rleBuffer.data());
            for (; ptr < sz;) {
                auto flag = data[ptr++];
                if (flag == 0x80) {
                    if (ptr >= sz) {
                        ptr -= 1;
                        break;
                    }
                    auto cnt = data[ptr++];
                    if (cnt == 0) {
                        uncBuffer.append(char(0x80));
                        continue;
                    } else if (ptr >= sz) {
                        ptr -= 2;
                        break;
                    }
                    auto val = data[ptr++];
                    uncBuffer.append(QByteArray(1 + cnt, char(val)));
                } else {
                    uncBuffer.append(char(flag));
                }
            }
            if (ptr) { // remove consumed data
                rleBuffer.remove(0, ptr);
                ptr = 0;
            }
            psz = rleBuffer.size();
        }
        if (uncBuffer.size() < size) {
            return QByteArray(); // something wrong
        }
        auto line = uncBuffer.mid(0, size);
        uncBuffer.remove(0, line.size()); // remove consumed data
        return line;
    }

private:
    QIODevice *device;
    RasHeader header;

    // RLE decoding buffers
    QByteArray rleBuffer;
    QByteArray uncBuffer;
};

static bool LoadRAS(QDataStream &s, const RasHeader &ras, QImage &img)
{
    s.device()->seek(RasHeader::SIZE);

    // The width of a scan line is always a multiple of 16 bits, padded when necessary.
    auto rasLineSize = (qint64(ras.Width) * ras.Depth + 7) / 8;
    if (rasLineSize & 1)
        ++rasLineSize;
    if (rasLineSize > kMaxQVectorSize) {
        qWarning() << "LoadRAS() unsupported line size" << rasLineSize;
        return false;
    }

    // Allocate image
    img = imageAlloc(ras.Width, ras.Height, imageFormat(ras));
    if (img.isNull()) {
        return false;
    }

    // Read palette if needed.
    if (ras.ColorMapType == RAS_COLOR_MAP_TYPE_RGB) {
        // max 256 rgb elements palette is supported
        if (ras.ColorMapLength > 768) {
            return false;
        }
        QList<quint8> palette(ras.ColorMapLength);
        for (quint32 i = 0; i < ras.ColorMapLength; ++i) {
            s >> palette[i];
            if (s.status() != QDataStream::Ok) {
                return false;
            }
        }
        QList<QRgb> colorTable;
        for (quint32 i = 0, n = ras.ColorMapLength / 3; i < n; ++i) {
            colorTable << qRgb(palette.at(i), palette.at(i + n), palette.at(i + 2 * n));
        }
        for (; colorTable.size() < 256;) {
            colorTable << qRgb(255, 255, 255);
        }
        img.setColorTable(colorTable);
    }

    LineDecoder dec(s.device(), ras);
    auto bytesPerLine = std::min(img.bytesPerLine(), qsizetype(rasLineSize));
    for (quint32 y = 0; y < ras.Height; ++y) {
        auto rasLine = dec.readLine(rasLineSize);
        if (rasLine.size() != rasLineSize) {
            qWarning() << "LoadRAS() unable to read line" << y << ": the seems corrupted!";
            return false;
        }

        // Grayscale 1-bit / Grayscale 8-bit (never seen)
        if (ras.ColorMapType == RAS_COLOR_MAP_TYPE_NONE && (ras.Depth == 1 || ras.Depth == 8)) {
            for (auto &&b : rasLine) {
                b = ~b;
            }
            std::memcpy(img.scanLine(y), rasLine.constData(), bytesPerLine);
            continue;
        }

        // Image with palette
        if (ras.ColorMapType == RAS_COLOR_MAP_TYPE_RGB && (ras.Depth == 1 || ras.Depth == 8)) {
            std::memcpy(img.scanLine(y), rasLine.constData(), bytesPerLine);
            continue;
        }

        // BGR 24-bit
        if (ras.ColorMapType == RAS_COLOR_MAP_TYPE_NONE && ras.Depth == 24 && (ras.Type == RAS_TYPE_STANDARD || ras.Type == RAS_TYPE_BYTE_ENCODED)) {
            quint8 red;
            quint8 green;
            quint8 blue;
            auto scanLine = reinterpret_cast<QRgb *>(img.scanLine(y));
            for (quint32 x = 0; x < ras.Width; x++) {
                red = rasLine.at(x * 3 + 2);
                green = rasLine.at(x * 3 + 1);
                blue = rasLine.at(x * 3);
                *(scanLine + x) = qRgb(red, green, blue);
            }
            continue;
        }

        // RGB 24-bit
        if (ras.ColorMapType == RAS_COLOR_MAP_TYPE_NONE && ras.Depth == 24 && ras.Type == RAS_TYPE_RGB_FORMAT) {
            quint8 red;
            quint8 green;
            quint8 blue;
            auto scanLine = reinterpret_cast<QRgb *>(img.scanLine(y));
            for (quint32 x = 0; x < ras.Width; x++) {
                red = rasLine.at(x * 3);
                green = rasLine.at(x * 3 + 1);
                blue = rasLine.at(x * 3 + 2);
                *(scanLine + x) = qRgb(red, green, blue);
            }
            continue;
        }

        // BGR 32-bit (not tested: test case missing)
        if (ras.ColorMapType == RAS_COLOR_MAP_TYPE_NONE && ras.Depth == 32 && (ras.Type == RAS_TYPE_STANDARD || ras.Type == RAS_TYPE_BYTE_ENCODED)) {
            quint8 red;
            quint8 green;
            quint8 blue;
            auto scanLine = reinterpret_cast<QRgb *>(img.scanLine(y));
            for (quint32 x = 0; x < ras.Width; x++) {
                red = rasLine.at(x * 4 + 3);
                green = rasLine.at(x * 4 + 2);
                blue = rasLine.at(x * 4 + 1);
                *(scanLine + x) = qRgb(red, green, blue);
            }

            continue;
        }

        // RGB 32-bit (tested: test case missing due to image too large)
        if (ras.ColorMapType == RAS_COLOR_MAP_TYPE_NONE && ras.Depth == 32 && ras.Type == RAS_TYPE_RGB_FORMAT) {
            quint8 red;
            quint8 green;
            quint8 blue;
            auto scanLine = reinterpret_cast<QRgb *>(img.scanLine(y));
            for (quint32 x = 0; x < ras.Width; x++) {
                red = rasLine.at(x * 4 + 1);
                green = rasLine.at(x * 4 + 2);
                blue = rasLine.at(x * 4 + 3);
                *(scanLine + x) = qRgb(red, green, blue);
            }
            continue;
        }

        qWarning() << "LoadRAS() unsupported format!"
                   << "ColorMapType:" << ras.ColorMapType << "Type:" << ras.Type << "Depth:" << ras.Depth;
        return false;
    }

    return true;
}
} // namespace

class RASHandlerPrivate
{
public:
    RASHandlerPrivate() {}
    ~RASHandlerPrivate() {}

    RasHeader m_header;
};


RASHandler::RASHandler()
    : QImageIOHandler()
    , d(new RASHandlerPrivate)
{
}

bool RASHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("ras");
        return true;
    }
    return false;
}

bool RASHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("RASHandler::canRead() called with no device");
        return false;
    }

    if (device->isSequential()) {
        // qWarning("Reading ras files from sequential devices not supported");
        return false;
    }

    qint64 oldPos = device->pos();
    QByteArray head = device->read(RasHeader::SIZE); // header is exactly 32 bytes, always FIXME
    int readBytes = head.size(); // this should always be 32 bytes

    device->seek(oldPos);

    if (readBytes < RasHeader::SIZE) {
        return false;
    }

    QDataStream stream(head);
    stream.setByteOrder(QDataStream::BigEndian);
    RasHeader ras;
    stream >> ras;
    return IsSupported(ras);
}

bool RASHandler::read(QImage *outImage)
{
    QDataStream s(device());
    s.setByteOrder(QDataStream::BigEndian);

    // Read image header.
    auto&& ras = d->m_header;
    s >> ras;

    if (ras.ColorMapLength > kMaxQVectorSize) {
        qWarning() << "LoadRAS() unsupported image color map length in file header" << ras.ColorMapLength;
        return false;
    }

    // Check supported file types.
    if (!IsSupported(ras)) {
        //         qDebug() << "This RAS file is not supported.";
        return false;
    }

    QImage img;
    bool result = LoadRAS(s, ras, img);

    if (result == false) {
        //         qDebug() << "Error loading RAS file.";
        return false;
    }

    *outImage = img;
    return true;
}

bool RASHandler::supportsOption(ImageOption option) const
{
    if (option == QImageIOHandler::Size) {
        return true;
    }
    if (option == QImageIOHandler::ImageFormat) {
        return true;
    }
    return false;
}

QVariant RASHandler::option(ImageOption option) const
{
    QVariant v;

    if (option == QImageIOHandler::Size) {
        auto&& header = d->m_header;
        if (IsSupported(header)) {
            v = QVariant::fromValue(QSize(header.Width, header.Height));
        }
        else if (auto dev = device()) {
            // transactions works on both random and sequential devices
            dev->startTransaction();
            auto ba = dev->read(RasHeader::SIZE);
            dev->rollbackTransaction();

            QDataStream s(ba);
            s.setByteOrder(QDataStream::BigEndian);
            s >> header;
            if (s.status() == QDataStream::Ok && IsSupported(header)) {
                v = QVariant::fromValue(QSize(header.Width, header.Height));
            }
        }
    }

    if (option == QImageIOHandler::ImageFormat) {
        auto&& header = d->m_header;
        if (IsSupported(header)) {
            v = QVariant::fromValue(imageFormat(header));
        }
        else if (auto dev = device()) {
            // transactions works on both random and sequential devices
            dev->startTransaction();
            auto ba = dev->read(RasHeader::SIZE);
            dev->rollbackTransaction();

            QDataStream s(ba);
            s.setByteOrder(QDataStream::BigEndian);
            s >> header;
            if (s.status() == QDataStream::Ok && IsSupported(header)) {
                v = QVariant::fromValue(imageFormat(header));
            }
        }
    }

    return v;
}

QImageIOPlugin::Capabilities RASPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "im1" || format == "im8" || format == "im24" || format == "im32" || format == "ras" || format == "sun") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && RASHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *RASPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new RASHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_ras_p.cpp"
