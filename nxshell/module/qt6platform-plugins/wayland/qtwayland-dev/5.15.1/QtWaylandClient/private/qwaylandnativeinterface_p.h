// Copyright (C) 2020 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDNATIVEINTERFACE_H
#define QWAYLANDNATIVEINTERFACE_H

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

#include <QVariantMap>
#include <qpa/qplatformnativeinterface.h>

#include <QtWaylandClient/qtwaylandclientglobal.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandIntegration;
class QWaylandScreen;

class Q_WAYLAND_CLIENT_EXPORT QWaylandNativeInterface : public QPlatformNativeInterface
{
public:
    QWaylandNativeInterface(QWaylandIntegration *integration);
    void *nativeResourceForIntegration(const QByteArray &resource) override;
    void *nativeResourceForWindow(const QByteArray &resourceString,
                                  QWindow *window) override;
    void *nativeResourceForScreen(const QByteArray &resourceString,
                                  QScreen *screen) override;
#if QT_CONFIG(opengl)
    void *nativeResourceForContext(const QByteArray &resource, QOpenGLContext *context) override;
#endif
    QVariantMap windowProperties(QPlatformWindow *window) const override;
    QVariant windowProperty(QPlatformWindow *window, const QString &name) const override;
    QVariant windowProperty(QPlatformWindow *window, const QString &name, const QVariant &defaultValue) const override;
    void setWindowProperty(QPlatformWindow *window, const QString &name, const QVariant &value) override;

    void emitWindowPropertyChanged(QPlatformWindow *window, const QString &name);

    QFunctionPointer platformFunction(const QByteArray &resource) const override;

private:
    QWaylandIntegration *m_integration = nullptr;
    QHash<QPlatformWindow*, QVariantMap> m_windowProperties;

    static void setSync(QWindow *window);
    static void setDeSync(QWindow *window);
    static bool isSync(QWindow *window);
};

}

QT_END_NAMESPACE

#endif // QWAYLANDNATIVEINTERFACE_H
