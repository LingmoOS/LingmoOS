/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Lubos Lunak <l.lunak@kde.org>
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "cptr_p.h"
#include "kxutils_p.h"
#include <QBitmap>
#include <QDebug>

#include <private/qtx11extras_p.h>

#include <xcb/xcb.h>

namespace KXUtils
{
template<typename T>
T fromNative(xcb_pixmap_t pixmap, xcb_connection_t *c)
{
    const xcb_get_geometry_cookie_t geoCookie = xcb_get_geometry_unchecked(c, pixmap);
    UniqueCPointer<xcb_get_geometry_reply_t> geo(xcb_get_geometry_reply(c, geoCookie, nullptr));
    if (!geo) {
        // getting geometry for the pixmap failed
        return T();
    }

    const xcb_get_image_cookie_t imageCookie = xcb_get_image_unchecked(c, XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap, 0, 0, geo->width, geo->height, ~0);
    UniqueCPointer<xcb_get_image_reply_t> xImage(xcb_get_image_reply(c, imageCookie, nullptr));
    if (!xImage) {
        // request for image data failed
        return T();
    }
    QImage::Format format = QImage::Format_Invalid;
    switch (xImage->depth) {
    case 1:
        format = QImage::Format_MonoLSB;
        break;
    case 16:
        format = QImage::Format_RGB16;
        break;
    case 24:
        format = QImage::Format_RGB32;
        break;
    case 30: {
        // Qt doesn't have a matching image format. We need to convert manually
        uint32_t *pixels = reinterpret_cast<uint32_t *>(xcb_get_image_data(xImage.get()));
        for (uint i = 0; i < xImage.get()->length; ++i) {
            int r = (pixels[i] >> 22) & 0xff;
            int g = (pixels[i] >> 12) & 0xff;
            int b = (pixels[i] >> 2) & 0xff;

            pixels[i] = qRgba(r, g, b, 0xff);
        }
        // fall through, Qt format is still Format_ARGB32_Premultiplied
        Q_FALLTHROUGH();
    }
    case 32:
        format = QImage::Format_ARGB32_Premultiplied;
        break;
    default:
        return T(); // we don't know
    }
    QImage image(xcb_get_image_data(xImage.get()), geo->width, geo->height, xcb_get_image_data_length(xImage.get()) / geo->height, format, free, xImage.get());
    xImage.release();
    if (image.isNull()) {
        return T();
    }
    if (image.format() == QImage::Format_MonoLSB) {
        // work around an abort in QImage::color
        image.setColorCount(2);
        image.setColor(0, QColor(Qt::white).rgb());
        image.setColor(1, QColor(Qt::black).rgb());
    }
    return T::fromImage(image);
}

// Create QPixmap from X pixmap. Take care of different depths if needed.
QPixmap createPixmapFromHandle(WId pixmap, WId pixmap_mask)
{
    return createPixmapFromHandle(QX11Info::connection(), pixmap, pixmap_mask);
}

QPixmap createPixmapFromHandle(xcb_connection_t *c, WId pixmap, WId pixmap_mask)
{
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    qDebug() << "Byte order not supported";
    return QPixmap();
#endif
    const xcb_setup_t *setup = xcb_get_setup(c);
    if (setup->image_byte_order != XCB_IMAGE_ORDER_LSB_FIRST) {
        qDebug() << "Byte order not supported";
        return QPixmap();
    }

    QPixmap pix = fromNative<QPixmap>(pixmap, c);
    if (pixmap_mask != XCB_PIXMAP_NONE) {
        QBitmap mask = fromNative<QBitmap>(pixmap_mask, c);
        if (mask.size() != pix.size()) {
            return QPixmap();
        }
        pix.setMask(mask);
    }
    return pix;
}

// Functions for X timestamp comparing. For Time being 32bit they're fairly simple
// (the #if 0 part), but on 64bit architectures Time is 64bit unsigned long,
// so there special care needs to be taken to always use only the lower 32bits.
#if 0
int timestampCompare(Time time1, Time time2)   // like strcmp()
{
    if (time1 == time2) {
        return 0;
    }
    return (time1 - time2) < 0x7fffffffU ? 1 : -1;   // time1 > time2 -> 1, handle wrapping
}

Time timestampDiff(Time time1, Time time2)   // returns time2 - time1
{
    // no need to handle wrapping?
    return time2 - time1;
}
#else
int timestampCompare(unsigned long time1_, unsigned long time2_) // like strcmp()
{
    quint32 time1 = time1_;
    quint32 time2 = time2_;
    if (time1 == time2) {
        return 0;
    }
    return quint32(time1 - time2) < 0x7fffffffU ? 1 : -1; // time1 > time2 -> 1, handle wrapping
}

int timestampDiff(unsigned long time1_, unsigned long time2_) // returns time2 - time1
{
    // no need to handle wrapping?
    quint32 time1 = time1_;
    quint32 time2 = time2_;
    return quint32(time2 - time1);
}
#endif

} // namespace
