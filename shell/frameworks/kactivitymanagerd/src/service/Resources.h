/*
 *   SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

// Qt
#include <QString>

// Utils
#include <utils/d_ptr.h>

// Local
#include "Event.h"
#include "Module.h"

/**
 * Resources
 */
class Resources : public Module
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.Resources")

public:
    explicit Resources(QObject *parent = nullptr);
    ~Resources() override;

public Q_SLOTS:
    /**
     * Registers a new event
     * @param application the name of application that sent the event. Ignored
     *                    if the event is not of type Opened
     * @param windowId ID of the window that displays the resource. Ignored if
     *                 the event is of type Accessed
     * @param uri URI of the resource on which the event happened
     * @param event type of the event
     */
    void RegisterResourceEvent(const QString &application, uint windowId, const QString &uri, uint event);

    /**
     * Registers resource's mimetype.
     * Note that this will be forgotten when the resource in question is closed.
     * @param uri URI of the resource
     */
    void RegisterResourceMimetype(const QString &uri, const QString &mimetype);

    /**
     * Registers resource's title. If not manually specified, it will be a
     * shortened version of the uri
     *
     * Note that this will be forgotten when the resource in question is closed.
     * @param uri URI of the resource
     */
    void RegisterResourceTitle(const QString &uri, const QString &title);

Q_SIGNALS:
    void RegisteredResourceEvent(const Event &event);
    void ProcessedResourceEvents(const EventList &events);
    void RegisteredResourceMimetype(const QString &uri, const QString &mimetype);
    void RegisteredResourceTitle(const QString &uri, const QString &title);

private:
    D_PTR;
};
