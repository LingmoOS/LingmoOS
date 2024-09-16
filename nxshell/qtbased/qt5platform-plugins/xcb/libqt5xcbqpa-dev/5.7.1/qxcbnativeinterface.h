// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBNATIVEINTERFACE_H
#define QXCBNATIVEINTERFACE_H

#include <qpa/qplatformnativeinterface.h>
#include <xcb/xcb.h>

#include <QtCore/QRect>

#include "qxcbexport.h"

QT_BEGIN_NAMESPACE

class QWidget;
class QXcbScreen;
class QXcbConnection;
class QXcbNativeInterfaceHandler;
class QDBusMenuConnection;

class Q_XCB_EXPORT QXcbNativeInterface : public QPlatformNativeInterface
{
    Q_OBJECT
public:
    enum ResourceType {
        Display,
        Connection,
        Screen,
        AppTime,
        AppUserTime,
        ScreenHintStyle,
        StartupId,
        TrayWindow,
        GetTimestamp,
        X11Screen,
        RootWindow,
        ScreenSubpixelType,
        ScreenAntialiasingEnabled,
        NoFontHinting,
        AtspiBus,
        CompositingEnabled
    };

    QXcbNativeInterface();

    void *nativeResourceForIntegration(const QByteArray &resource) Q_DECL_OVERRIDE;
    void *nativeResourceForContext(const QByteArray &resourceString, QOpenGLContext *context) Q_DECL_OVERRIDE;
    void *nativeResourceForScreen(const QByteArray &resource, QScreen *screen) Q_DECL_OVERRIDE;
    void *nativeResourceForWindow(const QByteArray &resourceString, QWindow *window) Q_DECL_OVERRIDE;
    void *nativeResourceForBackingStore(const QByteArray &resource, QBackingStore *backingStore) Q_DECL_OVERRIDE;
#ifndef QT_NO_CURSOR
    void *nativeResourceForCursor(const QByteArray &resource, const QCursor &cursor) Q_DECL_OVERRIDE;
#endif

    NativeResourceForIntegrationFunction nativeResourceFunctionForIntegration(const QByteArray &resource) Q_DECL_OVERRIDE;
    NativeResourceForContextFunction nativeResourceFunctionForContext(const QByteArray &resource) Q_DECL_OVERRIDE;
    NativeResourceForScreenFunction nativeResourceFunctionForScreen(const QByteArray &resource) Q_DECL_OVERRIDE;
    NativeResourceForWindowFunction nativeResourceFunctionForWindow(const QByteArray &resource) Q_DECL_OVERRIDE;
    NativeResourceForBackingStoreFunction nativeResourceFunctionForBackingStore(const QByteArray &resource) Q_DECL_OVERRIDE;

    QFunctionPointer platformFunction(const QByteArray &function) const Q_DECL_OVERRIDE;

    inline const QByteArray &genericEventFilterType() const { return m_genericEventFilterType; }

    void *displayForWindow(QWindow *window);
    void *connectionForWindow(QWindow *window);
    void *screenForWindow(QWindow *window);
    void *appTime(const QXcbScreen *screen);
    void *appUserTime(const QXcbScreen *screen);
    void *getTimestamp(const QXcbScreen *screen);
    void *startupId();
    void *x11Screen();
    void *rootWindow();
    void *display();
    void *atspiBus();
    void *connection();
    static void setStartupId(const char *);
    static void setAppTime(QScreen *screen, xcb_timestamp_t time);
    static void setAppUserTime(QScreen *screen, xcb_timestamp_t time);

    Q_INVOKABLE bool systemTrayAvailable(const QScreen *screen) const;
    Q_INVOKABLE void setParentRelativeBackPixmap(QWindow *window);
    Q_INVOKABLE bool systrayVisualHasAlphaChannel();
    Q_INVOKABLE bool requestSystemTrayWindowDock(const QWindow *window);
    Q_INVOKABLE QRect systemTrayWindowGlobalGeometry(const QWindow *window);

    void addHandler(QXcbNativeInterfaceHandler *handler);
    void removeHandler(QXcbNativeInterfaceHandler *handler);
signals:
    void systemTrayWindowChanged(QScreen *screen);

private:
    xcb_window_t locateSystemTray(xcb_connection_t *conn, const QXcbScreen *screen);

    const QByteArray m_genericEventFilterType;

    xcb_atom_t m_sysTraySelectionAtom;

    static QXcbScreen *qPlatformScreenForWindow(QWindow *window);

    QList<QXcbNativeInterfaceHandler *> m_handlers;
    NativeResourceForIntegrationFunction handlerNativeResourceFunctionForIntegration(const QByteArray &resource) const;
    NativeResourceForContextFunction handlerNativeResourceFunctionForContext(const QByteArray &resource) const;
    NativeResourceForScreenFunction handlerNativeResourceFunctionForScreen(const QByteArray &resource) const;
    NativeResourceForWindowFunction handlerNativeResourceFunctionForWindow(const QByteArray &resource) const;
    NativeResourceForBackingStoreFunction handlerNativeResourceFunctionForBackingStore(const QByteArray &resource) const;
    QFunctionPointer handlerPlatformFunction(const QByteArray &function) const;
    void *handlerNativeResourceForIntegration(const QByteArray &resource) const;
    void *handlerNativeResourceForContext(const QByteArray &resource, QOpenGLContext *context) const;
    void *handlerNativeResourceForScreen(const QByteArray &resource, QScreen *screen) const;
    void *handlerNativeResourceForWindow(const QByteArray &resource, QWindow *window) const;
    void *handlerNativeResourceForBackingStore(const QByteArray &resource, QBackingStore *backingStore) const;
};

QT_END_NAMESPACE

#endif // QXCBNATIVEINTERFACE_H
