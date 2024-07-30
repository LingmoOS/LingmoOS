/*
    SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ACTIVITIES_CONTROLLER_H
#define ACTIVITIES_CONTROLLER_H

#include <QFuture>
#include <QObject>
#include <QString>

#include "consumer.h"

#include "lingmo_activities_export.h"

#include <memory>

namespace KActivities
{
class ControllerPrivate;

/**
 * This class provides methods for controlling and managing
 * the activities.
 *
 * @note The QFuture objects returned by these methods are not thread-based,
 * you can not call synchronous methods like waitForFinished, cancel, pause on
 * them. You need either to register watchers to check when those have finished,
 * or to check whether they are ready from time to time manually.
 *
 * @see Consumer for info about activities
 *
 * @since 5.0
 */
class LINGMO_ACTIVITIES_EXPORT Controller : public Consumer
{
    Q_OBJECT

    Q_PROPERTY(QString currentActivity READ currentActivity WRITE setCurrentActivity)

public:
    explicit Controller(QObject *parent = nullptr);

    ~Controller() override;

    /**
     * Sets the name of the specified activity
     * @param id id of the activity
     * @param name name to be set
     */
    QFuture<void> setActivityName(const QString &id, const QString &name);

    /**
     * Sets the description of the specified activity
     * @param id id of the activity
     * @param description description to be set
     */
    QFuture<void> setActivityDescription(const QString &id, const QString &description);

    /**
     * Sets the icon of the specified activity
     * @param id id of the activity
     * @param icon icon to be set - freedesktop.org name or file path
     */
    QFuture<void> setActivityIcon(const QString &id, const QString &icon);

    /**
     * Sets the current activity
     * @param id id of the activity to make current
     * @returns true if successful
     */
    QFuture<bool> setCurrentActivity(const QString &id);

    /**
     * Adds a new activity
     * @param name name of the activity
     * @returns id of the newly created activity
     */
    QFuture<QString> addActivity(const QString &name);

    /**
     * Removes the specified activity
     * @param id id of the activity to delete
     */
    QFuture<void> removeActivity(const QString &id);

    /**
     * Stops the activity
     * @param id id of the activity to stop
     */
    QFuture<void> stopActivity(const QString &id);

    /**
     * Starts the activity
     * @param id id of the activity to start
     */
    QFuture<void> startActivity(const QString &id);

    /**
     * Switches to the previous activity
     */
    QFuture<void> previousActivity();

    /**
     * Switches to the next activity
     */
    QFuture<void> nextActivity();

private:
    const std::unique_ptr<ControllerPrivate> d;
};

} // namespace KActivities

#endif // ACTIVITIES_CONTROLLER_H
