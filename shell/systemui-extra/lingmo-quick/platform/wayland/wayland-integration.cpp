/*
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    Based on WaylandIntegration from kwayland-integration

    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2024 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "wayland-integration_p.h"
#include <KWayland/Client/connection_thread.h>
#include <windowmanager/lingmoshell.h>
#include "registry.h"

#include <QCoreApplication>
#include <QDebug>

class WaylandIntegrationSingleton
{
public:
    WaylandIntegration self;
};

Q_GLOBAL_STATIC(WaylandIntegrationSingleton, privateWaylandIntegrationSelf)

WaylandIntegration::WaylandIntegration(QObject *parent)
    : QObject(parent)
{
    setupKWaylandIntegration();
}

WaylandIntegration::~WaylandIntegration()
{
}

LingmoUIShell *WaylandIntegration::waylandLingmoUIShell()
{
    if (!m_waylandLingmoUIShell && m_registry) {
        const LingmoUIQuick::WaylandClient::Registry::AnnouncedInterface interface = m_registry->interface(LingmoUIQuick::WaylandClient::Registry::Interface::LingmoUIShell);

        if (interface.name == 0) {
            qWarning() << "The compositor does not support the lingmo shell protocol";
            return nullptr;
        }

        m_waylandLingmoUIShell = m_registry->createLingmoUIShell(interface.name, interface.version, qApp);

        connect(m_waylandLingmoUIShell, &LingmoUIShell::removed, this, [this]() {
            m_waylandLingmoUIShell->deleteLater();
        });
    }

    return m_waylandLingmoUIShell;
}

WaylandIntegration *WaylandIntegration::self()
{
    return &privateWaylandIntegrationSelf()->self;
}

void WaylandIntegration::sync()
{
    wl_display_roundtrip(m_connection->display());
}

void WaylandIntegration::setupKWaylandIntegration()
{
    m_connection = KWayland::Client::ConnectionThread::fromApplication(this);
    if (!m_connection) {
        return;
    }

    m_registry = new LingmoUIQuick::WaylandClient::Registry(qApp);
    m_registry->create(m_connection);
    m_registry->setup();
    m_connection->roundtrip();
}
