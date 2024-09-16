// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGLXINTEGRATION_H
#define QGLXINTEGRATION_H

#include "qxcbwindow.h"
#include "qxcbscreen.h"

#include <qpa/qplatformopenglcontext.h>
#include <qpa/qplatformoffscreensurface.h>
#include <QtGui/QSurfaceFormat>

#include <QtCore/QMutex>

#include <GL/glx.h>

QT_BEGIN_NAMESPACE

class QGLXContext : public QPlatformOpenGLContext
{
public:
    QGLXContext(QXcbScreen *xd, const QSurfaceFormat &format, QPlatformOpenGLContext *share);
    ~QGLXContext();

    bool makeCurrent(QPlatformSurface *surface);
    void doneCurrent();
    void swapBuffers(QPlatformSurface *surface);
    void (*getProcAddress(const QByteArray &procName)) ();

    QSurfaceFormat format() const;
    bool isSharing() const;
    bool isValid() const;

    GLXContext glxContext() const { return m_context; }

    static bool supportsThreading();
    static void queryDummyContext();

private:
    Display *m_display;
    GLXContext m_context;
    GLXContext m_shareContext;
    QSurfaceFormat m_format;
    bool m_isPBufferCurrent;
    int m_swapInterval;
    static bool m_queriedDummyContext;
    static bool m_supportsThreading;
};


class QGLXPbuffer : public QPlatformOffscreenSurface
{
public:
    explicit QGLXPbuffer(QOffscreenSurface *offscreenSurface);
    ~QGLXPbuffer();

    QSurfaceFormat format() const { return m_format; }
    bool isValid() const { return m_pbuffer != 0; }

    GLXPbuffer pbuffer() const { return m_pbuffer; }

private:
    QSurfaceFormat m_format;
    QXcbScreen *m_screen;
    GLXPbuffer m_pbuffer;
};

QT_END_NAMESPACE

#endif
