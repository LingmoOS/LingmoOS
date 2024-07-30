/*
 *   SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

// Self
#include "Resources.h"

// Qt
#include <QString>
#include <QWindow> // for WId

// Local
#include "resourcesadaptor.h"

class Resources::Private : public QThread
{
    Q_OBJECT

public:
    Private(Resources *parent);
    ~Private() override;

    void run() override;

    // Inserts the event directly into the queue
    void insertEvent(const Event &newEvent);

    // Processes the event and inserts it into the queue
    void addEvent(const QString &application, WId wid, const QString &uri, int type);

    // Processes the event and inserts it into the queue
    void addEvent(const Event &newEvent);

    QStringList resourcesLinkedToActivity(const QString &activity) const;

public Q_SLOTS:
    // Reacting to window manager signals
    void windowClosed(WId windowId);

    void activeWindowChanged(WId windowId);

private:
    struct WindowData {
        QSet<QString> resources;
        QString focussedResource;
        QString application;
    };

    Event lastEvent;

    QHash<WId, WindowData> windows;
    WId focussedWindow;

    Resources *const q;
};
