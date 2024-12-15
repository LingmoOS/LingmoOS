// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOPENGLPAINTDEVICE_H
#define DOPENGLPAINTDEVICE_H

#include "global.h"

#include <QOffscreenSurface>
#include <QOpenGLPaintDevice>
#include <QImage>

DPP_BEGIN_NAMESPACE

class DOpenGLPaintDevicePrivate;
class DOpenGLPaintDevice : public QOpenGLPaintDevice
{
    Q_DECLARE_PRIVATE(DOpenGLPaintDevice)

public:
    enum UpdateBehavior {
        NoPartialUpdate,
        PartialUpdateBlit,
        PartialUpdateBlend
    };

    explicit DOpenGLPaintDevice(QSurface *surface, UpdateBehavior updateBehavior = PartialUpdateBlit);
    explicit DOpenGLPaintDevice(const QSize &size, UpdateBehavior updateBehavior = PartialUpdateBlit);
    explicit DOpenGLPaintDevice(QOpenGLContext *shareContext, QSurface *surface = nullptr, UpdateBehavior updateBehavior = PartialUpdateBlit);
    explicit DOpenGLPaintDevice(QOpenGLContext *shareContext, const QSize &size, UpdateBehavior updateBehavior = PartialUpdateBlit);
    ~DOpenGLPaintDevice();

    void resize(const QSize &size);

    UpdateBehavior updateBehavior() const;
    bool isValid() const;

    void makeCurrent();
    void doneCurrent();
    void flush();

    QOpenGLContext *context() const;
    QOpenGLContext *shareContext() const;

    GLuint defaultFramebufferObject() const;

    QImage grabFramebuffer();

private:
    using QOpenGLPaintDevice::setSize;
    void ensureActiveTarget() override;

    Q_DISABLE_COPY(DOpenGLPaintDevice)
};

DPP_END_NAMESPACE

#endif // DOPENGLPAINTDEVICE_H
