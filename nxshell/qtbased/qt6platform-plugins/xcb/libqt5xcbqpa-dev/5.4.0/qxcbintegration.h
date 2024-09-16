// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBINTEGRATION_H
#define QXCBINTEGRATION_H

#include <qpa/qplatformintegration.h>
#include <qpa/qplatformscreen.h>

QT_BEGIN_NAMESPACE

class QXcbConnection;
class QAbstractEventDispatcher;
class QXcbNativeInterface;
class QXcbScreen;

class QXcbIntegration : public QPlatformIntegration
{
public:
    QXcbIntegration(const QStringList &parameters, int &argc, char **argv);
    ~QXcbIntegration();

    QPlatformWindow *createPlatformWindow(QWindow *window) const;
#ifndef QT_NO_OPENGL
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const;
#endif
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const;

    QPlatformOffscreenSurface *createPlatformOffscreenSurface(QOffscreenSurface *surface) const;

    bool hasCapability(Capability cap) const;
    QAbstractEventDispatcher *createEventDispatcher() const;
    void initialize();

    void moveToScreen(QWindow *window, int screen);

    QPlatformFontDatabase *fontDatabase() const;

    QPlatformNativeInterface *nativeInterface()const;

#ifndef QT_NO_CLIPBOARD
    QPlatformClipboard *clipboard() const;
#endif
#ifndef QT_NO_DRAGANDDROP
    QPlatformDrag *drag() const;
#endif

    QPlatformInputContext *inputContext() const;

#ifndef QT_NO_ACCESSIBILITY
    QPlatformAccessibility *accessibility() const;
#endif

    QPlatformServices *services() const;

    Qt::KeyboardModifiers queryKeyboardModifiers() const;
    QList<int> possibleKeys(const QKeyEvent *e) const;

    QStringList themeNames() const;
    QPlatformTheme *createPlatformTheme(const QString &name) const;
    QVariant styleHint(StyleHint hint) const;

    QXcbConnection *defaultConnection() const { return m_connections.first(); }

    QByteArray wmClass() const;

#if !defined(QT_NO_SESSIONMANAGER) && defined(XCB_USE_SM)
    QPlatformSessionManager *createPlatformSessionManager(const QString &id, const QString &key) const Q_DECL_OVERRIDE;
#endif

    void sync();
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
};

QT_END_NAMESPACE

#endif
