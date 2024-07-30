/*
    Softimage PIC support for QImage.

    SPDX-FileCopyrightText: 1998 Halfdan Ingvarsson
    SPDX-FileCopyrightText: 2007 Ruben Lopez <r.lopez@bren.es>
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/* This code is based on the GIMP-PIC plugin by Halfdan Ingvarsson,
 * and relicensed from GPL to LGPL to accommodate the KDE licensing policy
 * with his permission.
 */

#include "pic_p.h"
#include "rle_p.h"
#include "util_p.h"

#include <QDataStream>
#include <QDebug>
#include <QImage>
#include <QVariant>
#include <algorithm>
#include <functional>
#include <qendian.h>
#include <utility>

/**
 * Reads a PIC file header from a data stream.
 *
 * @param s         The data stream to read from.
 * @param channels  Where the read header will be stored.
 * @returns  @p s
 *
 * @relates PicHeader
 */
static QDataStream &operator>>(QDataStream &s, PicHeader &header)
{
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    s >> header.magic;
    s >> header.version;

    // the comment should be truncated to the first null byte
    char comment[81] = {};
    s.readRawData(comment, 80);
    header.comment = QByteArray(comment);

    header.id.resize(4);
    const int bytesRead = s.readRawData(header.id.data(), 4);
    if (bytesRead != 4) {
        header.id.resize(bytesRead);
    }

    s >> header.width;
    s >> header.height;
    s >> header.ratio;
    qint16 fields;
    s >> fields;
    header.fields = static_cast<PicFields>(fields);
    qint16 pad;
    s >> pad;
    return s;
}

/**
 * Writes a PIC file header to a data stream.
 *
 * @param s         The data stream to write to.
 * @param channels  The header to write.
 * @returns  @p s
 *
 * @relates PicHeader
 */
static QDataStream &operator<<(QDataStream &s, const PicHeader &header)
{
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    s << header.magic;
    s << header.version;

    char comment[80] = {};
    strncpy(comment, header.comment.constData(), sizeof(comment));
    s.writeRawData(comment, sizeof(comment));

    char id[4] = {};
    strncpy(id, header.id.constData(), sizeof(id));
    s.writeRawData(id, sizeof(id));

    s << header.width;
    s << header.height;
    s << header.ratio;
    s << quint16(header.fields);
    s << quint16(0);
    return s;
}

/**
 * Reads a series of channel descriptions from a data stream.
 *
 * If the stream contains more than 8 channel descriptions, the status of @p s
 * will be set to QDataStream::ReadCorruptData (note that more than 4 channels
 * - one for each component - does not really make sense anyway).
 *
 * @param s         The data stream to read from.
 * @param channels  The location to place the read channel descriptions; any
 *                  existing entries will be cleared.
 * @returns  @p s
 *
 * @relates PicChannel
 */
static QDataStream &operator>>(QDataStream &s, QList<PicChannel> &channels)
{
    const unsigned maxChannels = 8;
    unsigned count = 0;
    quint8 chained = 1;
    channels.clear();
    while (chained && count < maxChannels && s.status() == QDataStream::Ok) {
        PicChannel channel;
        s >> chained;
        s >> channel.size;
        s >> channel.encoding;
        s >> channel.code;
        channels << channel;
        ++count;
    }
    if (chained) {
        // too many channels!
        s.setStatus(QDataStream::ReadCorruptData);
    }
    return s;
}

/**
 * Writes a series of channel descriptions to a data stream.
 *
 * Note that the corresponding read operation will not read more than 8 channel
 * descriptions, although there should be no reason to have more than 4 channels
 * anyway.
 *
 * @param s         The data stream to write to.
 * @param channels  The channel descriptions to write.
 * @returns  @p s
 *
 * @relates PicChannel
 */
static QDataStream &operator<<(QDataStream &s, const QList<PicChannel> &channels)
{
    Q_ASSERT(channels.size() > 0);
    for (int i = 0; i < channels.size() - 1; ++i) {
        s << quint8(1); // chained
        s << channels[i].size;
        s << quint8(channels[i].encoding);
        s << channels[i].code;
    }
    s << quint8(0); // chained
    s << channels.last().size;
    s << quint8(channels.last().encoding);
    s << channels.last().code;
    return s;
}

static bool readRow(QDataStream &stream, QRgb *row, quint16 width, const QList<PicChannel> &channels)
{
    for (const PicChannel &channel : channels) {
        auto readPixel = [&](QDataStream &str) -> QRgb {
            quint8 red = 0;
            if (channel.code & RED) {
                str >> red;
            }
            quint8 green = 0;
            if (channel.code & GREEN) {
                str >> green;
            }
            quint8 blue = 0;
            if (channel.code & BLUE) {
                str >> blue;
            }
            quint8 alpha = 0;
            if (channel.code & ALPHA) {
                str >> alpha;
            }
            return qRgba(red, green, blue, alpha);
        };
        auto updatePixel = [&](QRgb oldPixel, QRgb newPixel) -> QRgb {
            return qRgba(qRed((channel.code & RED) ? newPixel : oldPixel),
                         qGreen((channel.code & GREEN) ? newPixel : oldPixel),
                         qBlue((channel.code & BLUE) ? newPixel : oldPixel),
                         qAlpha((channel.code & ALPHA) ? newPixel : oldPixel));
        };
        if (channel.encoding == MixedRLE) {
            bool success = decodeRLEData(RLEVariant::PIC, stream, row, width, readPixel, updatePixel);
            if (!success) {
                qDebug() << "decodeRLEData failed";
                return false;
            }
        } else if (channel.encoding == Uncompressed) {
            for (quint16 i = 0; i < width; ++i) {
                QRgb pixel = readPixel(stream);
                row[i] = updatePixel(row[i], pixel);
            }
        } else {
            // unknown encoding
            qDebug() << "Unknown encoding";
            return false;
        }
    }
    if (stream.status() != QDataStream::Ok) {
        qDebug() << "DataStream status was" << stream.status();
    }
    return stream.status() == QDataStream::Ok;
}

bool SoftimagePICHandler::canRead() const
{
    if (!SoftimagePICHandler::canRead(device())) {
        return false;
    }
    setFormat("pic");
    return true;
}

bool SoftimagePICHandler::read(QImage *image)
{
    if (!readChannels()) {
        return false;
    }

    QImage::Format fmt = QImage::Format_RGB32;
    for (const PicChannel &channel : std::as_const(m_channels)) {
        if (channel.size != 8) {
            // we cannot read images that do not come in bytes
            qDebug() << "Channel size was" << channel.size;
            m_state = Error;
            return false;
        }
        if (channel.code & ALPHA) {
            fmt = QImage::Format_ARGB32;
        }
    }

    QImage img = imageAlloc(m_header.width, m_header.height, fmt);
    if (img.isNull()) {
        qDebug() << "Failed to allocate image, invalid dimensions?" << QSize(m_header.width, m_header.height) << fmt;
        return false;
    }

    img.fill(qRgb(0, 0, 0));

    for (int y = 0; y < m_header.height; y++) {
        QRgb *row = reinterpret_cast<QRgb *>(img.scanLine(y));
        if (!readRow(m_dataStream, row, m_header.width, m_channels)) {
            qDebug() << "readRow failed";
            m_state = Error;
            return false;
        }
    }

    *image = img;
    m_state = Ready;

    return true;
}

bool SoftimagePICHandler::write(const QImage &_image)
{
    bool alpha = _image.hasAlphaChannel();
    const QImage image = _image.convertToFormat(alpha ? QImage::Format_ARGB32 : QImage::Format_RGB32);

    if (image.width() < 0 || image.height() < 0) {
        qDebug() << "Image size invalid:" << image.width() << image.height();
        return false;
    }
    if (image.width() > 65535 || image.height() > 65535) {
        qDebug() << "Image too big:" << image.width() << image.height();
        // there are only two bytes for each dimension
        return false;
    }

    QDataStream stream(device());

    stream << PicHeader(image.width(), image.height(), m_description);

    PicChannelEncoding encoding = m_compression ? MixedRLE : Uncompressed;
    QList<PicChannel> channels;
    channels << PicChannel(encoding, RED | GREEN | BLUE);
    if (alpha) {
        channels << PicChannel(encoding, ALPHA);
    }
    stream << channels;

    for (int r = 0; r < image.height(); r++) {
        const QRgb *row = reinterpret_cast<const QRgb *>(image.scanLine(r));

        /* Write the RGB part of the scanline */
        auto rgbEqual = [](QRgb p1, QRgb p2) -> bool {
            return qRed(p1) == qRed(p2) && qGreen(p1) == qGreen(p2) && qBlue(p1) == qBlue(p2);
        };
        auto writeRgb = [](QDataStream &str, QRgb pixel) -> void {
            str << quint8(qRed(pixel)) << quint8(qGreen(pixel)) << quint8(qBlue(pixel));
        };
        if (m_compression) {
            encodeRLEData(RLEVariant::PIC, stream, row, image.width(), rgbEqual, writeRgb);
        } else {
            for (int i = 0; i < image.width(); ++i) {
                writeRgb(stream, row[i]);
            }
        }

        /* Write the alpha channel */
        if (alpha) {
            auto alphaEqual = [](QRgb p1, QRgb p2) -> bool {
                return qAlpha(p1) == qAlpha(p2);
            };
            auto writeAlpha = [](QDataStream &str, QRgb pixel) -> void {
                str << quint8(qAlpha(pixel));
            };
            if (m_compression) {
                encodeRLEData(RLEVariant::PIC, stream, row, image.width(), alphaEqual, writeAlpha);
            } else {
                for (int i = 0; i < image.width(); ++i) {
                    writeAlpha(stream, row[i]);
                }
            }
        }
    }
    return stream.status() == QDataStream::Ok;
}

bool SoftimagePICHandler::canRead(QIODevice *device)
{
    char data[4];
    if (device->peek(data, 4) != 4) {
        return false;
    }
    return qFromBigEndian<qint32>(reinterpret_cast<uchar *>(data)) == PIC_MAGIC_NUMBER;
}

bool SoftimagePICHandler::readHeader()
{
    if (m_state == Ready) {
        m_state = Error;
        m_dataStream.setDevice(device());
        m_dataStream >> m_header;
        if (m_header.isValid() && m_dataStream.status() == QDataStream::Ok) {
            m_state = ReadHeader;
        }
    }

    return m_state != Error;
}

bool SoftimagePICHandler::readChannels()
{
    readHeader();
    if (m_state == ReadHeader) {
        m_state = Error;
        m_dataStream >> m_channels;
        if (m_dataStream.status() == QDataStream::Ok) {
            m_state = ReadChannels;
        }
    }
    return m_state != Error;
}

void SoftimagePICHandler::setOption(ImageOption option, const QVariant &value)
{
    switch (option) {
    case CompressionRatio:
        m_compression = value.toBool();
        break;
    case Description: {
        m_description.clear();
        const QStringList entries = value.toString().split(QStringLiteral("\n\n"));
        for (const QString &entry : entries) {
            if (entry.startsWith(QStringLiteral("Description: "))) {
                m_description = entry.mid(13).simplified().toUtf8();
            }
        }
        break;
    }
    default:
        break;
    }
}

QVariant SoftimagePICHandler::option(ImageOption option) const
{
    const_cast<SoftimagePICHandler *>(this)->readHeader();
    switch (option) {
    case Size:
        if (const_cast<SoftimagePICHandler *>(this)->readHeader()) {
            return QSize(m_header.width, m_header.height);
        } else {
            return QVariant();
        }
    case CompressionRatio:
        return m_compression;
    case Description:
        if (const_cast<SoftimagePICHandler *>(this)->readHeader()) {
            QString descStr = QString::fromUtf8(m_header.comment);
            if (!descStr.isEmpty()) {
                return QString(QStringLiteral("Description: ") + descStr + QStringLiteral("\n\n"));
            }
        }
        return QString();
    case ImageFormat:
        if (const_cast<SoftimagePICHandler *>(this)->readChannels()) {
            for (const PicChannel &channel : std::as_const(m_channels)) {
                if (channel.code & ALPHA) {
                    return QImage::Format_ARGB32;
                }
            }
            return QImage::Format_RGB32;
        }
        return QVariant();
    default:
        return QVariant();
    }
}

bool SoftimagePICHandler::supportsOption(ImageOption option) const
{
    return (option == CompressionRatio || option == Description || option == ImageFormat || option == Size);
}

QImageIOPlugin::Capabilities SoftimagePICPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "pic") {
        return Capabilities(CanRead | CanWrite);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && SoftimagePICHandler::canRead(device)) {
        cap |= CanRead;
    }
    if (device->isWritable()) {
        cap |= CanWrite;
    }
    return cap;
}

QImageIOHandler *SoftimagePICPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new SoftimagePICHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_pic_p.cpp"
