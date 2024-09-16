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

    QPaintDevice *paintDevice() Q_DECL_OVERRIDE;
    void flush(QWindow *window, const QRegion &region, const QPoint &offset) Q_DECL_OVERRIDE;
#ifndef QT_NO_OPENGL
    void composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                         QPlatformTextureList *textures, QOpenGLContext *context,
                         bool translucentBackground) Q_DECL_OVERRIDE;
    QImage toImage() const Q_DECL_OVERRIDE;
#endif

    QPlatformGraphicsBuffer *graphicsBuffer() const Q_DECL_OVERRIDE;

    void resize(const QSize &size, const QRegion &staticContents) Q_DECL_OVERRIDE;
    bool scroll(const QRegion &area, int dx, int dy) Q_DECL_OVERRIDE;

    void beginPaint(const QRegion &) Q_DECL_OVERRIDE;
    void endPaint() Q_DECL_OVERRIDE;

private:
    QXcbShmImage *m_image;
    QRegion m_paintRegion;
    QImage m_rgbImage;
};

QT_END_NAMESPACE

#endif
