// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBBACKINGSTORE_H
#define QXCBBACKINGSTORE_H

#include <qpa/qplatformbackingstore.h>

#include <xcb/xcb.h>

#include "qxcbobject.h"

            QT_BEGIN_NAMESPACE

        class QXcbShmImage;

class QXcbBackingStore : public QXcbObject, public QPlatformBackingStore
{
public:
    QXcbBackingStore(QWindow *widget);
    ~QXcbBackingStore();

    QPaintDevice *paintDevice();
    void flush(QWindow *window, const QRegion &region, const QPoint &offset);
#ifndef QT_NO_OPENGL
    void composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                         QPlatformTextureList *textures, QOpenGLContext *context);
#endif
    QImage toImage() const;
    void resize(const QSize &size, const QRegion &staticContents);
    bool scroll(const QRegion &area, int dx, int dy);

    void beginPaint(const QRegion &);

private:
    QXcbShmImage *m_image;
};

QT_END_NAMESPACE

#endif
