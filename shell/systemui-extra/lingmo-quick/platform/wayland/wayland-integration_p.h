/*
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
    SPDX-FileCopyrightText: 2024 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WAYLAND_INTEGRATION_P_H
#define WAYLAND_INTEGRATION_P_H

#include <QObject>
#include <QPointer>

namespace KWayland::Client {
class ConnectionThread;
}

class LingmoUIShell;
namespace LingmoUIQuick::WaylandClient
{
class Registry;
}

class WaylandIntegration : public QObject
{
    Q_OBJECT

public:
    explicit WaylandIntegration(QObject *parent = nullptr);
    ~WaylandIntegration() override;

    LingmoUIShell *waylandLingmoUIShell();

    static WaylandIntegration *self();
    void sync();

private:
    void setupKWaylandIntegration();

    QPointer<LingmoUIQuick::WaylandClient::Registry> m_registry;
    QPointer<LingmoUIShell> m_waylandLingmoUIShell;
    QPointer<KWayland::Client::ConnectionThread> m_connection;

    Q_DISABLE_COPY(WaylandIntegration)
};

#endif // WAYLAND_INTEGRATION_P_H
