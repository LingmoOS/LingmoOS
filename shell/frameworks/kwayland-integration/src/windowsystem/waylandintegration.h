/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLANDINTEGRATION_H
#define WAYLANDINTEGRATION_H
#include <private/kwindoweffects_p.h>

#include <QObject>
#include <QPointer>

namespace KWayland
{
namespace Client
{
class Compositor;
class ConnectionThread;
class PlasmaShell;
class Registry;
class ShadowManager;
class ShmPool;
}
}
class WaylandXdgActivationV1;

class WaylandIntegration : public QObject
{
public:
    explicit WaylandIntegration();
    ~WaylandIntegration();
    void setupKWaylandIntegration();

    static WaylandIntegration *self();

    KWayland::Client::Registry *registry() const;
    KWayland::Client::PlasmaShell *waylandPlasmaShell();

    WaylandXdgActivationV1 *activation();

private:
    QPointer<KWayland::Client::ConnectionThread> m_waylandConnection;
    QPointer<KWayland::Client::Compositor> m_waylandCompositor;
    QPointer<KWayland::Client::Registry> m_registry;
    QPointer<KWayland::Client::PlasmaShell> m_waylandPlasmaShell;
    std::unique_ptr<WaylandXdgActivationV1> m_activation;
    struct {
        quint32 name = 0;
        quint32 version = 0;
    } m_activationInterface;
};

#endif
