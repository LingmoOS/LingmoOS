/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "fullscreenoverlay.h"

#include <QStandardPaths>

#include <KX11Extras>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <kwindowsystem.h>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>

FullScreenOverlay::FullScreenOverlay(QQuickWindow *parent)
    : QQuickWindow(parent)
{
    setFlags(Qt::FramelessWindowHint);
    setWindowState(Qt::WindowFullScreen);
    initWayland();
    setWindowStates(Qt::WindowFullScreen);
}

FullScreenOverlay::~FullScreenOverlay()
{
}

void FullScreenOverlay::initWayland()
{
    if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        return;
    }
    using namespace KWayland::Client;
    ConnectionThread *connection = ConnectionThread::fromApplication(this);
    if (!connection) {
        return;
    }
    Registry *registry = new Registry(this);
    registry->create(connection);

    m_surface = Surface::fromWindow(this);
    if (!m_surface) {
        return;
    }
    connect(registry, &Registry::plasmaShellAnnounced, this, [this, registry](quint32 name, quint32 version) {
        m_plasmaShellInterface = registry->createPlasmaShell(name, version, this);

        m_plasmaShellSurface = m_plasmaShellInterface->createSurface(m_surface, this);
        m_plasmaShellSurface->setSkipTaskbar(true);
    });

    registry->setup();
    connection->roundtrip();
    // HACK: why the first time is shown fullscreen won't work?
    showFullScreen();
    hide();
}

bool FullScreenOverlay::event(QEvent *e)
{
    if (e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut) {
        emit activeChanged();
    } else if (e->type() == QEvent::PlatformSurface) {
        QPlatformSurfaceEvent *pe = static_cast<QPlatformSurfaceEvent *>(e);

        if (pe->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
            if (m_plasmaShellSurface) {
                m_plasmaShellSurface->setSkipTaskbar(true);
            }

            if (!m_acceptsFocus) {
                setFlags(flags() | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
            } else {
                setFlags(flags() | Qt::FramelessWindowHint);
            }
        }
    } else if (e->type() == QEvent::Show) {
        if (m_plasmaShellSurface) {
            m_plasmaShellSurface->setSkipTaskbar(true);
        }
    } else if (e->type() == QEvent::Expose) {
        if (KWindowSystem::isPlatformX11()) {
            KX11Extras::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
        } else {
            if (m_plasmaShellSurface) {
                m_plasmaShellSurface->setSkipTaskbar(true);
                m_plasmaShellSurface->setSkipSwitcher(true);
            }
        }
    }

    return QQuickWindow::event(e);
}

#include "fullscreenoverlay.moc"
