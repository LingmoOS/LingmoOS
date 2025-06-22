/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QSocketNotifier>

#include <wayland-server.h>

namespace ScreenLocker
{
class WaylandServer : public QObject
{
    Q_OBJECT
public:
    explicit WaylandServer(QObject *parent = nullptr);
    ~WaylandServer() override;
    int start();
    void stop();

Q_SIGNALS:
    void x11WindowAdded(quint32 window);

private:
    void flush();
    void dispatchEvents();

    static void bind(wl_client *client, void *data, uint32_t version, uint32_t id);

    QSocketNotifier *m_notifier = nullptr;
    ::wl_display *m_display = nullptr;
    ::wl_client *m_greeter = nullptr;
    ::wl_global *m_interface = nullptr;

    struct Listener {
        ::wl_listener listener;
        WaylandServer *server;
    } m_listener;
};

}
