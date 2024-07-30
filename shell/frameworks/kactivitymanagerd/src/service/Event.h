/*
 *   SPDX-FileCopyrightText: 2011-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "kactivitymanagerd_plugin_export.h"

// Qt
#include <QDateTime>
#include <QMetaType>
#include <QString>

/**
 *
 */
class KACTIVITYMANAGERD_PLUGIN_EXPORT Event
{
public:
    enum Type {
        Accessed = 0, ///< resource was accessed, but we don't know for how long it will be open/used

        Opened = 1, ///< resource was opened
        Modified = 2, ///< previously opened resource was modified
        Closed = 3, ///< previously opened resource was closed

        FocussedIn = 4, ///< resource get the keyboard focus
        FocussedOut = 5, ///< resource lost the focus

        LastEventType = 5,
        UserEventType = 32,
    };

    // These events can't come outside of the activity manager daemon,
    // they are intended to provide some additional functionality
    // to the daemon plugins
    enum UserType {
        UpdateScore = UserEventType + 1,
    };

    Event();

    explicit Event(const QString &application, quintptr wid, const QString &uri, int type = Accessed);

    Event deriveWithType(Type type) const;

    bool operator==(const Event &other) const;

public:
    QString application;
    quintptr wid;
    QString uri;
    int type;
    QDateTime timestamp;

    QString typeName() const;
};

typedef QList<Event> EventList;

Q_DECLARE_METATYPE(Event)
Q_DECLARE_METATYPE(EventList)
