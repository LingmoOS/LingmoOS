// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBINTEGRATION_H
#define QXCBINTEGRATION_H

#include <qpa/qplatformintegration.h>
#include <qpa/qplatformscreen.h>

#include "qxcbexport.h"

#include <xcb/xcb.h>

QT_BEGIN_NAMESPACE

class QXcbConnection;
class QAbstractEventDispatcher;
class QXcbNativeInterface;
class QXcbScreen;

class Q_XCB_EXPORT QXcbIntegration : public QPlatformIntegration
{
public:
    QXcbIntegration(const QStringList &parameters, int &argc, char **argv);
    ~QXcbIntegration();

    QPlatformWindow *createPlatformWindow(QWindow *window) const override;
    QPlatformWindow *createForeignWindow(QWindow *window, WId nativeHandle) const override;
#ifndef QT_NO_OPENGL
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const override;
#endif
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const override;

    QPlatformOffscreenSurface *createPlatformOffscreenSurface(QOffscreenSurface *surface) const override;

    bool hasCapability(Capability cap) const override;
    QAbstractEventDispatcher *createEventDispatcher() const override;
    void initialize() override;

    void moveToScreen(QWindow *window, int screen);

    QPlatformFontDatabase *fontDatabase() const override;

    QPlatformNativeInterface *nativeInterface()const override;

#ifndef QT_NO_CLIPBOARD
    QPlatformClipboard *clipboard() const override;
#endif
#ifndef QT_NO_DRAGANDDROP
    QPlatformDrag *drag() const override;
#endif

    QPlatformInputContext *inputContext() const override;

#ifndef QT_NO_ACCESSIBILITY
    QPlatformAccessibility *accessibility() const override;
#endif

    QPlatformServices *services() const override;

    Qt::KeyboardModifiers queryKeyboardModifiers() const override;
    QList<int> possibleKeys(const QKeyEvent *e) const override;

    QStringList themeNames() const override;
    QPlatformTheme *createPlatformTheme(const QString &name) const override;
    QVariant styleHint(StyleHint hint) const override;

    QXcbConnection *defaultConnection() const { return m_connections.first(); }

    QByteArray wmClass() const;

#if !defined(QT_NO_SESSIONMANAGER) && defined(XCB_USE_SM)
    QPlatformSessionManager *createPlatformSessionManager(const QString &id, const QString &key) const override;
#endif

    void sync() override;

    void beep() const override;

    static QXcbIntegration *instance() { return m_instance; }

private:
    QList<QXcbConnection *> m_connections;

    QScopedPointer<QPlatformFontDatabase> m_fontDatabase;
    QScopedPointer<QXcbNativeInterface> m_nativeInterface;

    QScopedPointer<QPlatformInputContext> m_inputContext;

#ifndef QT_NO_ACCESSIBILITY
    mutable QScopedPointer<QPlatformAccessibility> m_accessibility;
#endif

    QScopedPointer<QPlatformServices> m_services;

    friend class QXcbConnection; // access QPlatformIntegration::screenAdded()

    mutable QByteArray m_wmClass;
    const char *m_instanceName;
    bool m_canGrab;
    xcb_visualid_t m_defaultVisualId;

    static QXcbIntegration *m_instance;
};

QT_END_NAMESPACE

#endif
