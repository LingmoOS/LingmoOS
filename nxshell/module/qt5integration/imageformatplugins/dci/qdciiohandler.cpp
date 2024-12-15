// Copyright (C) 2022 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "qdciiohandler.h"

#ifndef QT_NO_SVGRENDERER

#include "qimage.h"
#include "qpixmap.h"
#include "qpainter.h"
#include "qvariant.h"
#include "qbuffer.h"
#include "qdebug.h"

#include <DDciIcon>

DGUI_USE_NAMESPACE

QT_BEGIN_NAMESPACE

class QDciIOHandlerPrivate
{
public:
    QDciIOHandlerPrivate(QDciIOHandler *qq)
        : q(qq)
    {}

    bool load(QIODevice *device);

    QDciIOHandler *q;
    DDciIcon icon;
    DDciIconMatchResult current = nullptr;
    DDciIcon::Theme theme = DDciIcon::Light;
    DDciIcon::Mode mode = DDciIcon::Normal;

    bool loaded = false;
    bool readDone = false;

    int defaultSize = 0;
    int scaledSize = 0;
    QColor backColor;
};

bool QDciIOHandlerPrivate::load(QIODevice *device)
{
    if (!device)
        return false;

    if (loaded)
        return current;
    if (q->format().isEmpty() && !q->canRead())
        return false;

    loaded = true;

    QBuffer *buf = qobject_cast<QBuffer *>(device);
    if (buf) {
        const QByteArray &ba = buf->data();
        icon = DDciIcon(QByteArray::fromRawData(ba.constData() + buf->pos(), ba.size() - buf->pos()));
        buf->seek(ba.size());
    } else {
        icon = DDciIcon(device->readAll());
    }

    current = nullptr;
    if (icon.isNull())
        return false;

    if (q->format().startsWith(QByteArrayLiteral("dci-"))) {
        const auto sections = q->format().split('-');
        if (sections.size() == 3) {
            if (sections[1] == QByteArrayLiteral("dark"))
                theme = DDciIcon::Dark;
            if (sections[2] == QByteArrayLiteral("disabled"))
                mode = DDciIcon::Disabled;
            else if (sections[2] == QByteArrayLiteral("hover"))
                mode = DDciIcon::Hover;
            else if (sections[2] == QByteArrayLiteral("pressed"))
                mode = DDciIcon::Pressed;
        }
    }

    current = icon.matchIcon(0, theme, mode, DDciIcon::DontFallbackMode);

    if (!current)
        return false;

    defaultSize = icon.actualSize(current);

    return true;
}

QDciIOHandler::QDciIOHandler()
    : d(new QDciIOHandlerPrivate(this))
{

}

QDciIOHandler::~QDciIOHandler()
{
    delete d;
}

bool QDciIOHandler::canRead() const
{
    if (!device())
        return false;
    if (d->loaded && !d->readDone)
        return true;

    QByteArray buf = device()->peek(4);
    if (buf.startsWith(QByteArrayLiteral("DCI"))) {
        // Decide format from content..
        if (format().isEmpty())
            setFormat("dci");

        return true;
    }
    return false;
}

QByteArray QDciIOHandler::name() const
{
    return "dci";
}

bool QDciIOHandler::read(QImage *image)
{
    if (d->readDone || d->load(device())) {
        Q_ASSERT(d->current);
        int finalSize = d->scaledSize > 0 ? d->scaledSize : d->defaultSize;

        if (finalSize > 0) {
            DDciIconPalette palette(QColor::Invalid, d->backColor);
            *image = d->icon.pixmap(1.0, finalSize, d->current, palette).toImage();
        }
        d->readDone = true;
        return true;
    }

    return false;
}

QVariant QDciIOHandler::option(ImageOption option) const
{
    switch(option) {
    case ImageFormat:
        return QImage::Format_ARGB32_Premultiplied;
    case Size:
        d->load(device());
        return QSize(d->defaultSize, d->defaultSize);
    case ScaledSize:
        return QSize(d->scaledSize, d->scaledSize);
    case BackgroundColor:
        return d->backColor;
    case Animation:
        // TODO: Support animation for DDciIcon
        return false;
    case Endianness:
        return QSysInfo::LittleEndian;
    case SupportedSubTypes:
        if (d->load(device()) && d->icon.isSupportedAttribute(d->current, DDciIcon::HasPalette))
            return QVariant::fromValue(QByteArrayList { QByteArrayLiteral("HasPalette") });
        return QVariant::fromValue(QByteArrayList());
    default:
        break;
    }
    return QVariant();
}

void QDciIOHandler::setOption(ImageOption option, const QVariant &value)
{
    switch(option) {
    case ScaledSize: {
        const QSize &size = value.toSize();
        d->scaledSize = qMin(size.width(), size.height());
        break;
    }
    case BackgroundColor:
        d->backColor = qvariant_cast<QColor>(value);
    default:
        break;
    }
}

bool QDciIOHandler::supportsOption(ImageOption option) const
{
    switch(option)
    {
    case Size:
    case ScaledSize:
    case ImageFormat:
    case Animation:
    case BackgroundColor:
    case Endianness:
    case SupportedSubTypes:
        return true;
    default:
        break;
    }
    return false;
}

bool QDciIOHandler::canRead(QIODevice *device)
{
    QByteArray buf = device->peek(4);
    return buf.startsWith(QByteArrayLiteral("DCI"));
}

QT_END_NAMESPACE

#endif // QT_NO_SVGRENDERER
