/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef ATTICA_ACTIVITY_H
#define ATTICA_ACTIVITY_H

#include <QList>
#include <QSharedDataPointer>
#include <QUrl>

#include "attica_export.h"
#include "person.h"

class QDateTime;

namespace Attica
{
/**
 * @class Activity activity.h <Attica/Activity>
 *
 * Represents a single news item (also known as activity)
 */
class ATTICA_EXPORT Activity
{
public:
    typedef QList<Activity> List;
    class Parser;

    /**
     * Creates an empty Activity
     */
    Activity();

    /**
     * Copy constructor.
     * @param other the Activity to copy from
     */
    Activity(const Activity &other);

    /**
     * Assignment operator.
     * @param other the Activity to assign from
     * @return pointer to this Activity
     */
    Activity &operator=(const Activity &other);

    /**
     * Destructor.
     */
    ~Activity();

    /**
     * Sets the id of the Activity.
     * The id uniquely identifies an Activity with the OCS API.
     * @param id the new id
     */
    void setId(const QString &id);

    /**
     * Gets the id of the Activity.
     * The id uniquely identifies an Activity with the OCS API.
     * @return the id
     */
    QString id() const;

    /**
     * Sets the user bound to the Activity.
     * @param id the new user
     */
    void setAssociatedPerson(const Person &associatedPerson);

    /**
     * Gets the user bound to the Activity.
     * @return the user
     */
    Person associatedPerson() const;

    /**
     * Sets the timestamp the Activity has been published.
     * @param timestamp the new timestamp
     */
    void setTimestamp(const QDateTime &timestamp);

    /**
     * Gets the timestamp the Activity has been published.
     * @return the timestamp
     */
    QDateTime timestamp() const;

    /**
     * Sets the message of the Activity.
     * @param message the new message
     */
    void setMessage(const QString &message);

    /**
     * Gets the message of the Activity.
     * @return the message
     */
    QString message() const;

    /**
     * Sets the link to further information about this Activity.
     * @param link the new link
     */
    void setLink(const QUrl &link);

    /**
     * Gets the link to further information about this Activity.
     * @return the link
     */
    QUrl link() const;

    /**
     * Checks whether this Activity has an id
     * @return @c true if an id has been set, @c false otherwise
     */
    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
