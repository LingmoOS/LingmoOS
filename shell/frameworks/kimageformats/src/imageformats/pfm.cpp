/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/*
 * See also: https://www.pauldebevec.com/Research/HDR/PFM/
 */

#include "pfm_p.h"
#include "util_p.h"

#include <QColorSpace>
#include <QDataStream>
#include <QIODevice>
#include <QImage>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(LOG_PFMPLUGIN)
Q_LOGGING_CATEGORY(LOG_PFMPLUGIN, "kf.imageformats.plugins.pfm", QtWarningMsg)

class PFMHeader
{
private:
    /*!
     * \brief m_bw True if grayscale.
     */
    bool m_bw;

    /*!
     * \brief m_ps True if saved by Photoshop (Photoshop variant).
     *
     * When \a false the format of the header is the following (GIMP):
     * [type]
     * [xres] [yres]
     * [byte_order]
     *
     * When \a true the format of the header is the following (Photoshop):
     * [type]
     * [xres]
     * [yres]
     * [byte_order]
     */
    bool m_ps;

    /*!
     * \brief m_width The image width.
     */
    qint32 m_width;

    /*!
     * \brief m_height The image height.
     */
    qint32 m_height;

    /*!
     * \brief m_byteOrder The image byte orger.
     */
    QDataStream::ByteOrder m_byteOrder;

public:
    PFMHeader() :
        m_bw(false),
        m_ps(false),
        m_width(0),
        m_height(0),
        m_byteOrder(QDataStream::BigEndian)
    {

    }

    bool isValid() const
    {
        return (m_width > 0 && m_height > 0);
    }

    bool isBlackAndWhite() const
    {
        return m_bw;
    }

    bool isPhotoshop() const
    {
        return m_ps;
    }

    qint32 width() const
    {
        return m_width;
    }

    qint32 height() const
    {
        return m_height;
    }

    QSize size() const
    {
        return QSize(m_width, m_height);
    }

    QDataStream::ByteOrder byteOrder() const
    {
        return m_byteOrder;
    }

    QImage::Format format() const
    {
        if (isValid()) {
            return QImage::Format_RGBX32FPx4;
        }
        return QImage::Format_Invalid;
    }

    bool read(QIODevice *d)
    {
        auto pf = d->read(3);
        if (pf == QByteArray("PF\n")) {
            m_bw = false;
        } else if (pf == QByteArray("Pf\n")) {
            m_bw = true;
        } else {
            return false;
        }
        auto wh = QString::fromLatin1(d->readLine(128));
        auto list = wh.split(QStringLiteral(" "));
        if (list.size() == 1) {
            m_ps = true; // try for Photoshop version
            list << QString::fromLatin1(d->readLine(128));
        }
        if (list.size() != 2) {
            return false;
        }
        auto ok_o = false;
        auto ok_w = false;
        auto ok_h = false;
        auto o = QString::fromLatin1(d->readLine(128)).toDouble(&ok_o);
        auto w = list.first().toInt(&ok_w);
        auto h = list.last().toInt(&ok_h);
        if (!ok_o || !ok_w || !ok_h || o == 0) {
            return false;
        }
        m_width = w;
        m_height = h;
        m_byteOrder = o > 0 ? QDataStream::BigEndian : QDataStream::LittleEndian;
        return isValid();
    }

    bool peek(QIODevice *d)
    {
        d->startTransaction();
        auto ok = read(d);
        d->rollbackTransaction();
        return ok;
    }
};

class PFMHandlerPrivate
{
public:
    PFMHandlerPrivate() {}
    ~PFMHandlerPrivate() {}

    PFMHeader m_header;
};

PFMHandler::PFMHandler()
    : QImageIOHandler()
    , d(new PFMHandlerPrivate)
{
}

bool PFMHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("pfm");
        return true;
    }
    return false;
}

bool PFMHandler::canRead(QIODevice *device)
{
    if (!device) {
        qCWarning(LOG_PFMPLUGIN) << "PFMHandler::canRead() called with no device";
        return false;
    }

    PFMHeader h;
    if (!h.peek(device)) {
        return false;
    }

    return h.isValid();
}

bool PFMHandler::read(QImage *image)
{
    auto&& header = d->m_header;
    if (!header.read(device())) {
        qCWarning(LOG_PFMPLUGIN) << "PFMHandler::read() invalid header";
        return false;
    }

    QDataStream s(device());
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    s.setByteOrder(header.byteOrder());

    auto img = imageAlloc(header.size(), header.format());
    if (img.isNull()) {
        qCWarning(LOG_PFMPLUGIN) << "PFMHandler::read() error while allocating the image";
        return false;
    }

    for (auto y = 0, h = img.height(); y < h; ++y) {
        auto bw = header.isBlackAndWhite();
        auto line = reinterpret_cast<float *>(img.scanLine(header.isPhotoshop() ? y : h - y - 1));
        for (auto x = 0, n = img.width() * 4; x < n; x += 4) {
            line[x + 3] = float(1);
            s >> line[x];
            if (bw) {
                line[x + 1] = line[x];
                line[x + 2] = line[x];
            } else {
                s >> line[x + 1];
                s >> line[x + 2];
            }
            if (s.status() != QDataStream::Ok) {
                qCWarning(LOG_PFMPLUGIN) << "PFMHandler::read() detected corrupted data";
                return false;
            }
        }
    }

    img.setColorSpace(QColorSpace(QColorSpace::SRgbLinear));

    *image = img;
    return true;
}

bool PFMHandler::supportsOption(ImageOption option) const
{
    if (option == QImageIOHandler::Size) {
        return true;
    }
    if (option == QImageIOHandler::ImageFormat) {
        return true;
    }
    if (option == QImageIOHandler::Endianness) {
        return true;
    }
    return false;
}

QVariant PFMHandler::option(ImageOption option) const
{
    QVariant v;

    if (option == QImageIOHandler::Size) {
        auto&& h = d->m_header;
        if (h.isValid()) {
            v = QVariant::fromValue(h.size());
        } else if (auto dev = device()) {
            if (h.peek(dev)) {
                v = QVariant::fromValue(h.size());
            }
        }
    }

    if (option == QImageIOHandler::ImageFormat) {
        auto&& h = d->m_header;
        if (h.isValid()) {
            v = QVariant::fromValue(h.format());
        } else if (auto dev = device()) {
            if (h.peek(dev)) {
                v = QVariant::fromValue(h.format());
            }
        }
    }

    if (option == QImageIOHandler::Endianness) {
        auto&& h = d->m_header;
        if (h.isValid()) {
            v = QVariant::fromValue(h.byteOrder());
        } else if (auto dev = device()) {
            if (h.peek(dev)) {
                v = QVariant::fromValue(h.byteOrder());
            }
        }
    }

    return v;
}

QImageIOPlugin::Capabilities PFMPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "pfm") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && PFMHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *PFMPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new PFMHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_pfm_p.cpp"
