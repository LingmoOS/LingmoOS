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

    QPlatformWindow *createPlatformWindow(QWindow *window) const Q_DECL_OVERRIDE;
#ifndef QT_NO_OPENGL
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const Q_DECL_OVERRIDE;
#endif
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const Q_DECL_OVERRIDE;

    QPlatformOffscreenSurface *createPlatformOffscreenSurface(QOffscreenSurface *surface) const Q_DECL_OVERRIDE;

    bool hasCapability(Capability cap) const Q_DECL_OVERRIDE;
    QAbstractEventDispatcher *createEventDispatcher() const Q_DECL_OVERRIDE;
    void initialize() Q_DECL_OVERRIDE;

    void moveToScreen(QWindow *window, int screen);

    QPlatformFontDatabase *fontDatabase() const Q_DECL_OVERRIDE;

    QPlatformNativeInterface *nativeInterface()const Q_DECL_OVERRIDE;

#ifndef QT_NO_CLIPBOARD
    QPlatformClipboard *clipboard() const Q_DECL_OVERRIDE;
#endif
#ifndef QT_NO_DRAGANDDROP
    QPlatformDrag *drag() const Q_DECL_OVERRIDE;
#endif

    QPlatformInputContext *inputContext() const Q_DECL_OVERRIDE;

#ifndef QT_NO_ACCESSIBILITY
    QPlatformAccessibility *accessibility() const Q_DECL_OVERRIDE;
#endif

    QPlatformServices *services() const Q_DECL_OVERRIDE;

    Qt::KeyboardModifiers queryKeyboardModifiers() const Q_DECL_OVERRIDE;
    QList<int> possibleKeys(const QKeyEvent *e) const Q_DECL_OVERRIDE;

    QStringList themeNames() const Q_DECL_OVERRIDE;
    QPlatformTheme *createPlatformTheme(const QString &name) const Q_DECL_OVERRIDE;
    QVariant styleHint(StyleHint hint) const Q_DECL_OVERRIDE;

    QXcbConnection *defaultConnection() const { return m_connections.first(); }

    QByteArray wmClass() const;

#if !defined(QT_NO_SESSIONMANAGER) && defined(XCB_USE_SM)
    QPlatformSessionManager *createPlatformSessionManager(const QString &id, const QString &key) const Q_DECL_OVERRIDE;
#endif

    void sync() Q_DECL_OVERRIDE;

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
