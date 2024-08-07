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
    void windowCreated(KWayland::Client::LingmoWindow *window);

Q_SIGNALS:
    void windowChanged(QString storageId);

private:
    KWayland::Client::LingmoWindowManagement *m_windowManagement = nullptr;
    QHash<QString, QList<KWayland::Client::LingmoWindow *>> m_windows; // <storageId, window>
};
