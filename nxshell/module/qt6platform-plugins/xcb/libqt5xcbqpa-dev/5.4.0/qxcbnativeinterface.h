// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBNATIVEINTERFACE_H
#define QXCBNATIVEINTERFACE_H

#include <qpa/qplatformnativeinterface.h>
#include <xcb/xcb.h>

#include <QtCore/QRect>

QT_BEGIN_NAMESPACE

class QWidget;
class QXcbScreen;
class QXcbConnection;

class QXcbNativeInterface : public QPlatformNativeInterface
{
    Q_OBJECT
public:
    enum ResourceType {
        Display,
        EglDisplay,
        Connection,
        Screen,
        EglContext,
        EglConfig,
        GLXConfig,
        GLXContext,
        AppTime,
        AppUserTime,
        ScreenHintStyle,
        StartupId,
        TrayWindow,
        GetTimestamp,
        X11Screen,
        RootWindow,
        ScreenSubpixelType,
        ScreenAntialiasingEnabled
    };

    QXcbNativeInterface();

    void *nativeResourceForIntegration(const QByteArray &resource) Q_DECL_OVERRIDE;
    void *nativeResourceForContext(const QByteArray &resourceString, QOpenGLContext *context) Q_DECL_OVERRIDE;
    void *nativeResourceForScreen(const QByteArray &resource, QScreen *screen) Q_DECL_OVERRIDE;
    void *nativeResourceForWindow(const QByteArray &resourceString, QWindow *window) Q_DECL_OVERRIDE;

    NativeResourceForIntegrationFunction nativeResourceFunctionForIntegration(const QByteArray &resource) Q_DECL_OVERRIDE;
    NativeResourceForContextFunction nativeResourceFunctionForContext(const QByteArray &resource) Q_DECL_OVERRIDE;
    NativeResourceForScreenFunction nativeResourceFunctionForScreen(const QByteArray &resource) Q_DECL_OVERRIDE;

    QFunctionPointer platformFunction(const QByteArray &function) const Q_DECL_OVERRIDE;

    inline const QByteArray &genericEventFilterType() const { return m_genericEventFilterType; }

    void *displayForWindow(QWindow *window);
    void *eglDisplayForWindow(QWindow *window);
    void *connectionForWindow(QWindow *window);
    void *screenForWindow(QWindow *window);
    void *appTime(const QXcbScreen *screen);
    void *appUserTime(const QXcbScreen *screen);
    void *getTimestamp(const QXcbScreen *screen);
    void *startupId();
    void *x11Screen();
    void *rootWindow();
    static void setStartupId(const char *);
    static void setAppTime(QScreen *screen, xcb_timestamp_t time);
    static void setAppUserTime(QScreen *screen, xcb_timestamp_t time);
    static void *eglContextForContext(QOpenGLContext *context);
    static void *eglConfigForContext(QOpenGLContext *context);
    static void *glxContextForContext(QOpenGLContext *context);
    static void *glxConfigForContext(QOpenGLContext *context);

    Q_INVOKABLE void beep();
    Q_INVOKABLE bool systemTrayAvailable(const QScreen *screen) const;
    Q_INVOKABLE void clearRegion(const QWindow *qwindow, const QRect& rect);
    Q_INVOKABLE bool systrayVisualHasAlphaChannel();
    Q_INVOKABLE bool requestSystemTrayWindowDock(const QWindow *window);
    Q_INVOKABLE QRect systemTrayWindowGlobalGeometry(const QWindow *window);

signals:
    void systemTrayWindowChanged(QScreen *screen);

private:
    xcb_window_t locateSystemTray(xcb_connection_t *conn, const QXcbScreen *screen);

    const QByteArray m_genericEventFilterType;

    xcb_atom_t m_sysTraySelectionAtom;
    xcb_visualid_t m_systrayVisualId;

    static QXcbScreen *qPlatformScreenForWindow(QWindow *window);
};

QT_END_NAMESPACE

#endif // QXCBNATIVEINTERFACE_H
