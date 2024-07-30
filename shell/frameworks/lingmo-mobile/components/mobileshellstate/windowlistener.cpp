// SPDX-FileCopyrightText: 2022 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "windowlistener.h"

WindowListener::WindowListener(QObject *parent)
    : QObject{parent}
{
    // initialize wayland window checking
    KWayland::Client::ConnectionThread *connection = KWayland::Client::ConnectionThread::fromApplication(this);
    if (!connection) {
        return;
    }

    auto *registry = new KWayland::Client::Registry(this);
    registry->create(connection);

    connect(registry, &KWayland::Client::Registry::lingmoWindowManagementAnnounced, this, [this, registry](quint32 name, quint32 version) {
        m_windowManagement = registry->createLingmoWindowManagement(name, version, this);
        connect(m_windowManagement, &KWayland::Client::LingmoWindowManagement::windowCreated, this, &WindowListener::onWindowCreated);
        connect(m_windowManagement, &KWayland::Client::LingmoWindowManagement::activeWindowChanged, this, [this]() {
            Q_EMIT activeWindowChanged(m_windowManagement->activeWindow());
        });
    });

    registry->setup();
    connection->roundtrip();
}

WindowListener *WindowListener::instance()
{
    static WindowListener *listener = new WindowListener();
    return listener;
}

void WindowListener::onWindowCreated(KWayland::Client::LingmoWindow *window)
{
    QString storageId = window->appId();

    // Ignore empty windows
    if (storageId == "") {
        return;
    }

    // Special handling for lingmoshell windows, don't track them
    if (storageId == "org.kde.lingmoshell") {
        Q_EMIT lingmoWindowCreated(window);
        return;
    }

    // listen for window close
    connect(window, &KWayland::Client::LingmoWindow::unmapped, this, [this, storageId]() {
        Q_EMIT windowRemoved(storageId);
    });

    Q_EMIT windowCreated(window);
}
