/*
    SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ACTIVITIES_INFO_H
#define ACTIVITIES_INFO_H

#include <QFuture>
#include <QObject>
#include <QString>

#include "lingmo_activities_export.h"

#include <memory>

namespace KActivities
{
class InfoPrivate;

/**
 * This class provides info about an activity. Most methods in it require a
 * semantic backend running to function properly.
 *
 * This class is not thread-safe.
 *
 * @see Consumer for info about activities
 *
 * The API of the class is synchronous, but the most used properties
 * are pre-fetched and cached. This means that, in order to get the least
 * amount of d-bus related locks, you should declare long-lived instances
 * of this class.
 *
 * Before relying on the values retrieved by the class, make sure that the
 * state is not Info::Unknown. You can get invalid data either because the
 * service is not functioning properly (or at all) or because the class did
 * not have enough time to synchronize the data with it.
 *
 * For example, if this is the only existing instance of the Info class, the
 * name method will return an empty string.
 *
 * For example, this is wrong (works, but blocks):
 * @code
 * void someMethod(const QString & activity) {
 *     // Do not copy. This approach is not a good one!
 *     Info info(activity);
 *     doSomethingWith(info.name());
 * }
 * @endcode
 *
 * Instances of the Info class should be long-lived. For example, members
 * of the classes that use them, and you should listen for the changes in the
 * provided properties.
 *
 * @since 4.5
 */
class LINGMO_ACTIVITIES_EXPORT Info : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(bool isCurrent READ isCurrent NOTIFY isCurrentChanged)
    Q_PROPERTY(Info::State state READ state NOTIFY stateChanged)

public:
    explicit Info(const QString &activity, QObject *parent = nullptr);
    ~Info() override;

    /**
     * @return true if the activity represented by this object exists and is valid
     */
    bool isValid() const;

    /**
     * Specifies which parts of this class are functional
     */
    enum Availability {
        Nothing = 0, ///< No activity info provided (isValid is false)
        BasicInfo = 1, ///< Basic info is provided
        Everything = 2, ///< Everything is available
    };

    /**
     * State of the activity
     */
    enum State {
        Invalid = 0, ///< This activity does not exist
        Unknown = 1, ///< Information is not yet retrieved from the service
        Running = 2, ///< Activity is running
        Starting = 3, ///< Activity is begin started
        Stopped = 4, ///< Activity is stopped
        Stopping = 5, ///< Activity is begin started
    };

    /**
     * @returns what info is provided by this instance of Info
     */
    Availability availability() const;

    /**
     * @returns the URI of this activity. The same URI is used by activities
     * KIO worker.
     */
    QString uri() const;

    /**
     * @returns the id of the activity
     */
    QString id() const;

    /**
     * @returns whether this activity is the current one
     */
    bool isCurrent() const;

    /**
     * @returns the name of the activity
     */
    QString name() const;

    /**
     * @returns the description of the activity
     */
    QString description() const;

    /**
     * @returns the icon of the activity. Icon can be a freedesktop.org name or
     * a file path. Or empty if no icon is set.
     */
    QString icon() const;

    /**
     * @returns the state of the activity
     */
    State state() const;

    /**
     * Links the specified resource to the activity
     * @param resourceUri resource URI
     * @note This method is <b>asynchronous</b>. It will return before the
     * resource is actually linked to the activity.
     */
    // QFuture<void> linkResource(const QString &resourceUri);

    /**
     * Unlinks the specified resource from the activity
     * @param resourceUri resource URI
     * @note This method is <b>asynchronous</b>. It will return before the
     * resource is actually unlinked from the activity.
     */
    // QFuture<void> unlinkResource(const QString &resourceUri);

    /**
     * @returns whether a resource is linked to this activity
     * @note This QFuture is not thread-based, you can not call synchronous
     * methods like waitForFinished, cancel, pause on it.
     * @since 5.0
     */
    // QFuture<bool> isResourceLinked(const QString &resourceUri);

Q_SIGNALS:
    /**
     * Emitted when the activity's name, icon or some custom property is changed
     */
    void infoChanged();

    /**
     * Emitted when the name is changed
     */
    void nameChanged(const QString &name);

    /**
     * Emitted when the activity becomes the current one, or when it stops
     * being the current one
     */
    void isCurrentChanged(bool current);

    /**
     * Emitted when the description is changed
     */
    void descriptionChanged(const QString &description);

    /**
     * Emitted when the icon was changed
     */
    void iconChanged(const QString &icon);

    /**
     * Emitted when the activity is added
     */
    void added();

    /**
     * Emitted when the activity is removed
     */
    void removed();

    /**
     * Emitted when the activity is started
     */
    void started();

    /**
     * Emitted when the activity is stopped
     */
    void stopped();

    /**
     * Emitted when the activity changes state
     * @param state new state of the activity
     */
    void stateChanged(KActivities::Info::State state);

private:
    const std::unique_ptr<InfoPrivate> d;

    Q_PRIVATE_SLOT(d, void activityStateChanged(const QString &, int))
    Q_PRIVATE_SLOT(d, void added(const QString &))
    Q_PRIVATE_SLOT(d, void removed(const QString &))
    Q_PRIVATE_SLOT(d, void started(const QString &))
    Q_PRIVATE_SLOT(d, void stopped(const QString &))
    Q_PRIVATE_SLOT(d, void infoChanged(const QString &))
    Q_PRIVATE_SLOT(d, void nameChanged(const QString &, const QString &))
    Q_PRIVATE_SLOT(d, void descriptionChanged(const QString &, const QString &))
    Q_PRIVATE_SLOT(d, void iconChanged(const QString &, const QString &))
    Q_PRIVATE_SLOT(d, void setServiceStatus(Consumer::ServiceStatus))
    Q_PRIVATE_SLOT(d, void setCurrentActivity(const QString &))

    friend class InfoPrivate;
};

} // namespace KActivities

#endif // ACTIVITIES_INFO_H
