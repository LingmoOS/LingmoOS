/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "waylandintegration.h"
#include "logging.h"

#include <KWayland/Client/compositor.h>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/plasmawindowmanagement.h>
#include <KWayland/Client/region.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/shadow.h>
#include <KWayland/Client/shm_pool.h>
#include <KWayland/Client/surface.h>

#include <KWindowSystem>

#include "waylandxdgactivationv1_p.h"
#include <QGuiApplication>

class WaylandIntegrationSingleton
{
public:
    WaylandIntegration self;
};

Q_GLOBAL_STATIC(WaylandIntegrationSingleton, privateWaylandIntegrationSelf)

WaylandIntegration::WaylandIntegration()
    : QObject()
    , m_activation(new WaylandXdgActivationV1)
{
    setupKWaylandIntegration();
}

WaylandIntegration::~WaylandIntegration()
{
}

void WaylandIntegration::setupKWaylandIntegration()
{
    using namespace KWayland::Client;
    m_waylandConnection = ConnectionThread::fromApplication(this);
    if (!m_waylandConnection) {
        qCWarning(KWAYLAND_KWS) << "Failed getting Wayland connection from QPA";
        return;
    }
    m_registry = new Registry(qApp);
    m_registry->create(m_waylandConnection);
    m_waylandCompositor = Compositor::fromApplication(this);

    m_registry->setup();
    m_waylandConnection->roundtrip();
}

WaylandIntegration *WaylandIntegration::self()
{
    return &privateWaylandIntegrationSelf()->self;
}

KWayland::Client::Registry *WaylandIntegration::registry() const
{
    return m_registry;
}

KWayland::Client::PlasmaShell *WaylandIntegration::waylandPlasmaShell()
{
    if (!m_waylandPlasmaShell && m_registry) {
        const KWayland::Client::Registry::AnnouncedInterface wmInterface = m_registry->interface(KWayland::Client::Registry::Interface::PlasmaShell);

        if (wmInterface.name == 0) {
            return nullptr;
        }

        m_waylandPlasmaShell = m_registry->createPlasmaShell(wmInterface.name, wmInterface.version, qApp);
    }
    return m_waylandPlasmaShell;
}

WaylandXdgActivationV1 *WaylandIntegration::activation()
{
    return m_activation.get();
}
