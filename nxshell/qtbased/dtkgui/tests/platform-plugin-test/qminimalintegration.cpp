// SPDX-FileCopyrightText: 2023 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "qminimalintegration.h"
#include "qminimalbackingstore.h"
#include "dplatformnativeinterface.h"

#include <QtGui/private/qpixmap_raster_p.h>
#include <QtGui/private/qguiapplication_p.h>
#include <qpa/qplatformfontdatabase.h>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow.h>
#include <qpa/qwindowsysteminterface.h>

#include <private/qgenericunixeventdispatcher_p.h>
#include <private/qgenericunixservices_p.h>

MinimalIntegration::MinimalIntegration(const QStringList &parameters)
{
    m_primaryScreen = new MinimalScreen();

    m_primaryScreen->mGeometry = QRect(0, 0, 240, 320);
    m_primaryScreen->mDepth = 32;
    m_primaryScreen->mFormat = QImage::Format_ARGB32_Premultiplied;

    QWindowSystemInterface::handleScreenAdded(m_primaryScreen);
}

MinimalIntegration::~MinimalIntegration()
{
    QWindowSystemInterface::handleScreenRemoved(m_primaryScreen);
}

QPlatformWindow *MinimalIntegration::createPlatformWindow(QWindow *window) const
{
    Q_UNUSED(window);
    QPlatformWindow *w = new QPlatformWindow(window);
    w->requestActivateWindow();
    return w;
}

QPlatformBackingStore *MinimalIntegration::createPlatformBackingStore(QWindow *window) const
{
    return new MinimalBackingStore(window);
}

QAbstractEventDispatcher *MinimalIntegration::createEventDispatcher() const
{
    return createUnixEventDispatcher();
}

QPlatformServices *MinimalIntegration::services() const
{
    if (!m_services)
        m_services.reset(new QGenericUnixServices);
    return m_services.get();
}

QPlatformNativeInterface *MinimalIntegration::nativeInterface() const
{
    if (!m_nativeInterface)
        m_nativeInterface.reset(new DPlatformNativeInterface);
    return m_nativeInterface.get();
}

QStringList MinimalIntegration::themeNames() const
{
    QStringList list = QPlatformIntegration::themeNames();
    const QByteArray desktop_session = qgetenv("DESKTOP_SESSION");

    if (desktop_session.isEmpty() || desktop_session.startsWith("deepin"))
        list.prepend("deepin");

    return list;
}

MinimalIntegration *MinimalIntegration::instance()
{
    return static_cast<MinimalIntegration *>(QGuiApplicationPrivate::platformIntegration());
}
