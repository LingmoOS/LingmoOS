/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pxr_p.h"
#include "util_p.h"

#include <QIODevice>
#include <QImage>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(LOG_PXRPLUGIN)
Q_LOGGING_CATEGORY(LOG_PXRPLUGIN, "kf.imageformats.plugins.pxr", QtWarningMsg)

class PXRHeader
{
private:
    QByteArray m_rawHeader;

    quint16 ui16(quint8 c1, quint8 c2) const {
        return (quint16(c2) << 8) | quint16(c1);
    }

    quint32 ui32(quint8 c1, quint8 c2, quint8 c3, quint8 c4) const {
        return (quint32(c4) << 24) | (quint32(c3) << 16) | (quint32(c2) << 8) | quint32(c1);
    }

public:
    PXRHeader()
    {

    }

    bool isValid() const
    {
        return (m_rawHeader.size() == 512 &&
                m_rawHeader.startsWith(QByteArray::fromRawData("\x80\xE8\x00\x00", 4)));
    }

    bool isSupported() const
    {
        return format() != QImage::Format_Invalid;
    }

    qint32 width() const
    {
        if (!isValid()) {
            return 0;
        }
        return qint32(ui16(m_rawHeader.at(418), m_rawHeader.at(419)));
    }

    qint32 height() const
    {
        if (!isValid()) {
            return 0;
        }
        return qint32(ui16(m_rawHeader.at(416), m_rawHeader.at(417)));
    }

    QSize size() const
    {
        return QSize(width(), height());
    }

    qint32 channel() const
    {
        if (!isValid()) {
            return 0;
        }
        return qint32(ui16(m_rawHeader.at(424), m_rawHeader.at(425)));
    }

    qint32 depth() const
    {
        if (!isValid()) {
            return 0;
        }
        return qint32(ui16(m_rawHeader.at(426), m_rawHeader.at(427)));
    }

    // supposing the image offset (always 1024 on sample files)
    qint32 offset() const
    {
        if (!isValid()) {
            return 0;
        }
        return qint32(ui16(m_rawHeader.at(428), m_rawHeader.at(429)));
    }

    QImage::Format format() const
    {
        if (channel() == 14 && depth() == 2) {
            return QImage::Format_RGB888;
        }
        if (channel() == 8 && depth() == 2) {
            return QImage::Format_Grayscale8;
        }
        return QImage::Format_Invalid;
    }

    qsizetype strideSize() const
    {
        if (format() == QImage::Format_RGB888) {
            return width() * 3;
        }
        if (format() == QImage::Format_Grayscale8) {
            return width();
        }
        return 0;
    }

    bool read(QIODevice *d)
    {
        m_rawHeader = d->read(512);
        return isValid();
    }

    bool peek(QIODevice *d)
    {
        d->startTransaction();
        auto ok = read(d);
        d->rollbackTransaction();
        return ok;
    }

    bool jumpToImageData(QIODevice *d) const
    {
        if (d->isSequential()) {
            if (auto sz = std::max(offset() - qint32(m_rawHeader.size()), 0)) {
                return d->read(sz).size() == sz;
            }
            return true;
        }
        return d->seek(offset());
    }
};

class PXRHandlerPrivate
{
public:
    PXRHandlerPrivate() {}
    ~PXRHandlerPrivate() {}

    PXRHeader m_header;
};

PXRHandler::PXRHandler()
    : QImageIOHandler()
    , d(new PXRHandlerPrivate)
{
}

bool PXRHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("pxr");
        return true;
    }
    return false;
}

bool PXRHandler::canRead(QIODevice *device)
{
    if (!device) {
        qCWarning(LOG_PXRPLUGIN) << "PXRHandler::canRead() called with no device";
        return false;
    }

    PXRHeader h;
    if (!h.peek(device)) {
        return false;
    }

    return h.isSupported();
}

bool PXRHandler::read(QImage *image)
{
    auto&& header = d->m_header;

    if (!header.read(device())) {
        qCWarning(LOG_PXRPLUGIN) << "PXRHandler::read() invalid header";
        return false;
    }

    auto img = imageAlloc(header.size(), header.format());
    if (img.isNull()) {
        qCWarning(LOG_PXRPLUGIN) << "PXRHandler::read() error while allocating the image";
        return false;
    }

    auto d = device();
    if (!header.jumpToImageData(d)) {
        qCWarning(LOG_PXRPLUGIN) << "PXRHandler::read() error while seeking image data";
        return false;
    }

    auto size = std::min(img.bytesPerLine(), header.strideSize());
    for (auto y = 0, h = img.height(); y < h; ++y) {
        auto line = reinterpret_cast<char*>(img.scanLine(y));
        if (d->read(line, size) != size) {
            qCWarning(LOG_PXRPLUGIN) << "PXRHandler::read() error while reading image scanline";
            return false;
        }
    }

    *image = img;
    return true;
}

bool PXRHandler::supportsOption(ImageOption option) const
{
    if (option == QImageIOHandler::Size) {
        return true;
    }
    if (option == QImageIOHandler::ImageFormat) {
        return true;
    }
    return false;
}

QVariant PXRHandler::option(ImageOption option) const
{
    QVariant v;

    if (option == QImageIOHandler::Size) {
        auto&& h = d->m_header;
        if (h.isValid()) {
            v = QVariant::fromValue(h.size());
        } else if (auto d = device()) {
            if (h.peek(d)) {
                v = QVariant::fromValue(h.size());
            }
        }
    }

    if (option == QImageIOHandler::ImageFormat) {
        auto&& h = d->m_header;
        if (h.isValid()) {
            v = QVariant::fromValue(h.format());
        } else if (auto d = device()) {
            if (h.peek(d)) {
                v = QVariant::fromValue(h.format());
            }
        }
    }

    return v;
}

QImageIOPlugin::Capabilities PXRPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "pxr") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && PXRHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *PXRPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new PXRHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_pxr_p.cpp"
