// Copyright (C) 2019 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPLATFORMINTEGRATION_WAYLAND_H
#define QPLATFORMINTEGRATION_WAYLAND_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <qpa/qplatformintegration.h>
#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandBuffer;
class QWaylandDisplay;
class QWaylandClientBufferIntegration;
class QWaylandServerBufferIntegration;
class QWaylandShellIntegration;
class QWaylandInputDeviceIntegration;
class QWaylandInputDevice;

class Q_WAYLAND_CLIENT_EXPORT QWaylandIntegration : public QPlatformIntegration
{
public:
    QWaylandIntegration();
    ~QWaylandIntegration() override;

    bool hasFailed() { return mFailed; }

    bool hasCapability(QPlatformIntegration::Capability cap) const override;
    QPlatformWindow *createPlatformWindow(QWindow *window) const override;
#if QT_CONFIG(opengl)
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const override;
#endif
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const override;

    QAbstractEventDispatcher *createEventDispatcher() const override;
    void initialize() override;

    QPlatformFontDatabase *fontDatabase() const override;

    QPlatformNativeInterface *nativeInterface() const override;
#if QT_CONFIG(clipboard)
    QPlatformClipboard *clipboard() const override;
#endif
#if QT_CONFIG(draganddrop)
    QPlatformDrag *drag() const override;
#endif
    QPlatformInputContext *inputContext() const override;

    QVariant styleHint(StyleHint hint) const override;

#if QT_CONFIG(accessibility)
    QPlatformAccessibility *accessibility() const override;
#endif

    QPlatformServices *services() const override;

    QWaylandDisplay *display() const;

    QStringList themeNames() const override;

    QPlatformTheme *createPlatformTheme(const QString &name) const override;

    QWaylandInputDevice *createInputDevice(QWaylandDisplay *display, int version, uint32_t id);

    virtual QWaylandClientBufferIntegration *clientBufferIntegration() const;
    virtual QWaylandServerBufferIntegration *serverBufferIntegration() const;
    virtual QWaylandShellIntegration *shellIntegration() const;

private:
    // NOTE: mDisplay *must* be destructed after mDrag and mClientBufferIntegration.
    // Do not move this definition into the private section at the bottom.
    QScopedPointer<QWaylandDisplay> mDisplay;

protected:
    QScopedPointer<QWaylandClientBufferIntegration> mClientBufferIntegration;
    QScopedPointer<QWaylandServerBufferIntegration> mServerBufferIntegration;
    QScopedPointer<QWaylandShellIntegration> mShellIntegration;
    QScopedPointer<QWaylandInputDeviceIntegration> mInputDeviceIntegration;

private:
    void initializeClientBufferIntegration();
    void initializeServerBufferIntegration();
    void initializeShellIntegration();
    void initializeInputDeviceIntegration();
    QWaylandShellIntegration *createShellIntegration(const QString& interfaceName);

    QScopedPointer<QPlatformFontDatabase> mFontDb;
#if QT_CONFIG(clipboard)
    QScopedPointer<QPlatformClipboard> mClipboard;
#endif
#if QT_CONFIG(draganddrop)
    QScopedPointer<QPlatformDrag> mDrag;
#endif
    QScopedPointer<QPlatformNativeInterface> mNativeInterface;
    QScopedPointer<QPlatformInputContext> mInputContext;
#if QT_CONFIG(accessibility)
    QScopedPointer<QPlatformAccessibility> mAccessibility;
#endif
    bool mFailed = false;
    bool mClientBufferIntegrationInitialized = false;
    bool mServerBufferIntegrationInitialized = false;
    bool mShellIntegrationInitialized = false;

    friend class QWaylandDisplay;
};

}

QT_END_NAMESPACE

#endif
