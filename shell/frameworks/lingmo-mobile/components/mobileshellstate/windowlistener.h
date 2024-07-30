// SPDX-FileCopyrightText: 2022 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QList>
#include <QObject>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/lingmowindowmanagement.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>

class WindowListener : public QObject
{
    Q_OBJECT

public:
    WindowListener(QObject *parent = nullptr);

    static WindowListener *instance();

    QList<KWayland::Client::LingmoWindow *> windowsFromStorageId(QString &storageId) const;

public Q_SLOTS:
    void onWindowCreated(KWayland::Client::LingmoWindow *window);

Q_SIGNALS:
    void windowCreated(KWayland::Client::LingmoWindow *window);
    void lingmoWindowCreated(KWayland::Client::LingmoWindow *window);
    void windowRemoved(QString storageId);
    void activeWindowChanged(KWayland::Client::LingmoWindow *activeWindow);

private:
    KWayland::Client::LingmoWindowManagement *m_windowManagement{nullptr};
};
