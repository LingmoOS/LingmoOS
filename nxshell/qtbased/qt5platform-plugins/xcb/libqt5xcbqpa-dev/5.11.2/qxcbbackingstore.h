// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBBACKINGSTORE_H
#define QXCBBACKINGSTORE_H

#include <qpa/qplatformbackingstore.h>
#include <QtCore/QStack>

#include <xcb/xcb.h>

#include "qxcbobject.h"

QT_BEGIN_NAMESPACE

class QXcbBackingStoreImage;

class QXcbBackingStore : public QXcbObject, public QPlatformBackingStore
{
public:
    QXcbBackingStore(QWindow *window);
    ~QXcbBackingStore();

    QPaintDevice *paintDevice() override;
    void flush(QWindow *window, const QRegion &region, const QPoint &offset) override;
#ifndef QT_NO_OPENGL
    void composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                         QPlatformTextureList *textures,
                         bool translucentBackground) override;
#endif
    QImage toImage() const override;

    QPlatformGraphicsBuffer *graphicsBuffer() const override;

    void resize(const QSize &size, const QRegion &staticContents) override;
    bool scroll(const QRegion &area, int dx, int dy) override;

    void beginPaint(const QRegion &) override;
    void endPaint() override;

    static bool createSystemVShmSegment(QXcbConnection *c, size_t segmentSize = 1,
                                        void *shmInfo = nullptr);

private:
    QXcbBackingStoreImage *m_image = nullptr;
    QStack<QRegion> m_paintRegions;
    QImage m_rgbImage;
};

QT_END_NAMESPACE

#endif
