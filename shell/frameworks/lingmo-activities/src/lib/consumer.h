/*
    SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ACTIVITIES_CONSUMER_H
#define ACTIVITIES_CONSUMER_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "info.h"

#include "lingmo_activities_export.h"

#include <memory>

namespace KActivities
{
class ConsumerPrivate;

/**
 * Contextual information can be, from the user's point of view, divided
 * into three aspects - "who am I?", "where am I?" (what are my surroundings?)
 * and "what am I doing?".
 *
 * Activities deal with the last one - "what am I doing?". The current activity
 * refers to what the user is doing at the moment, while the other activities
 * represent things that he/she was doing before, and probably will be doing
 * again.
 *
 * Activity is an abstract concept whose meaning can differ from one user to
 * another. Typical examples of activities are "developing a KDE project",
 * "studying the 19th century art", "composing music", "lazing on a Sunday
 * afternoon" etc.
 *
 * Consumer provides read-only information about activities.
 *
 * Before relying on the values retrieved by the class, make sure that the
 * serviceStatus is set to Running. Otherwise, you can get invalid data either
 * because the service is not functioning properly (or at all) or because
 * the class did not have enough time to synchronize the data with it.
 *
 * For example, if this is the only existing instance of the Consumer class,
 * the listActivities method will return an empty list.
 *
 * @code
 * void someMethod() {
 *     // Do not copy. This approach is not a good one!
 *     Consumer c;
 *     doSomethingWith(c.listActivities());
 * }
 * @endcode
 *
 * Instances of the Consumer class should be long-lived. For example, members
 * of the classes that use them, and you should listen for the changes in the
 * provided properties.
 *
 * @since 4.5
 */
class LINGMO_ACTIVITIES_EXPORT Consumer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentActivity READ currentActivity NOTIFY currentActivityChanged)
    Q_PROPERTY(QStringList activities READ activities NOTIFY activitiesChanged)
    Q_PROPERTY(QStringList runningActivities READ runningActivities NOTIFY runningActivitiesChanged)
    Q_PROPERTY(ServiceStatus serviceStatus READ serviceStatus NOTIFY serviceStatusChanged)

public:
    /**
     * Different states of the activities service
     */
    enum ServiceStatus {
        NotRunning, ///< Service is not running
        Unknown, ///< Unable to determine the status of the service
        Running, ///< Service is running properly
    };

    explicit Consumer(QObject *parent = nullptr);

    ~Consumer() override;

    /**
     * @returns the id of the current activity
     * @note Activity ID is a UUID-formatted string. If the serviceStatus
     *       is not Running, a null UUID is returned. The ID can also be an empty
     *       string in the case there is no current activity.
     */
    QString currentActivity() const;

    /**
     * @returns the list of activities filtered by state
     * @param state state of the activity
     * @note If the serviceStatus is not Running, only a null activity will be
     *       returned.
     */
    QStringList activities(Info::State state) const;

    /**
     * @returns a list of running activities
     * This is a convenience method that returns Running and Stopping activities
     */
    QStringList runningActivities() const;

    /**
     * @returns the list of all existing activities
     * @note If the serviceStatus is not Running, only a null activity will be
     *       returned.
     */
    QStringList activities() const;

    /**
     * @returns status of the activities service
     */
    ServiceStatus serviceStatus();

Q_SIGNALS:
    /**
     * This signal is emitted when the current activity is changed
     * @param id id of the new current activity
     */
    void currentActivityChanged(const QString &id);

    /**
     * This signal is emitted when the activity service goes online or offline,
     * or when the class manages to synchronize the data with the service.
     * @param status new status of the service
     */
    void serviceStatusChanged(Consumer::ServiceStatus status);

    /**
     * This signal is emitted when a new activity is added
     * @param id id of the new activity
     */
    void activityAdded(const QString &id);

    /**
     * This signal is emitted when an activity has been removed
     * @param id id of the removed activity
     */
    void activityRemoved(const QString &id);

    /**
     * This signal is emitted when the activity list changes
     * @param activities list of activities
     */
    void activitiesChanged(const QStringList &activities);

    /**
     * This signal is emitted when the list of running activities changes
     * @param runningActivities list of running activities
     */
    void runningActivitiesChanged(const QStringList &runningActivities);

private:
    const std::unique_ptr<ConsumerPrivate> d;
};

} // namespace KActivities

#endif // ACTIVITIES_CONSUMER_H
