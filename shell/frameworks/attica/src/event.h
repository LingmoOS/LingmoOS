/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Eckhart Wörner <ewoerner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_EVENT_H
#define ATTICA_EVENT_H

#include "attica_export.h"

#include <QDate>
#include <QList>
#include <QMap>
#include <QSharedDataPointer>
#include <QUrl>

namespace Attica
{
/**
 * @class Event event.h <Attica/Event>
 *
 * Represents a single event
 */
class ATTICA_EXPORT Event
{
public:
    typedef QList<Event> List;
    class Parser;

    /**
     * Creates an empty Event
     */
    Event();

    /**
     * Copy constructor.
     * @param other the Event to copy from
     */
    Event(const Event &other);

    /**
     * Assignment operator.
     * @param other the Event to assign from
     * @return pointer to this Event
     */
    Event &operator=(const Event &other);

    /**
     * Destructor.
     */
    ~Event();

    /**
     * Sets the id of the Event.
     * The id uniquely identifies a Event with the OCS API.
     * @param id the new id
     */
    void setId(const QString &id);

    /**
     * Gets the id of the Event.
     * The id uniquely identifies a Event with the OCS API.
     * @return the id
     */
    QString id() const;

    /**
     * Sets the name of the Event.
     * @param name the new name
     */
    void setName(const QString &name);

    /**
     * Gets the name of the Event.
     * @return the name
     */
    QString name() const;

    /**
     * Sets the description of the Event.
     * @param description the new description
     */
    void setDescription(const QString &description);

    /**
     * Gets the description of the Event.
     * @return the description
     */
    QString description() const;

    /**
     * Sets the id of the user bound to the Event.
     * @param user the new user id
     */
    void setUser(const QString &user);

    /**
     * Gets the id of the user bound to the Event.
     * @return the user id
     */
    QString user() const;

    /**
     * Sets the start date of the Event.
     * @param startDate the start date
     */
    void setStartDate(const QDate &startDate);

    /**
     * Gets the start date of the Event.
     * @return the start date
     */
    QDate startDate() const;

    /**
     * Sets the end date of the Event.
     * @param endDate the end date
     */
    void setEndDate(const QDate &endDate);

    /**
     * Gets the start date of the Event.
     * @return the end date
     */
    QDate endDate() const;

    /**
     * Sets the latitude of the position the Event takes place.
     * @param latitude the new latitude
     */
    void setLatitude(qreal latitude);

    /**
     * Gets the latitude of the position the Event takes place.
     * @return the latitude
     */
    qreal latitude() const;

    /**
     * Sets the longitude of the position the Event takes place.
     * @param longitude the new latitude
     */
    void setLongitude(qreal longitude);

    /**
     * Gets the longitude of the position the Event takes place.
     * @return the latitude
     */
    qreal longitude() const;

    /**
     * Sets the homepage of the Event.
     * @param homepage the new homepage
     */
    void setHomepage(const QUrl &homepage);

    /**
     * Gets the homepage of the Event.
     * @return the homepage
     */
    QUrl homepage() const;

    /**
     * Sets the country where the Event takes place.
     * @param country the new country
     */
    void setCountry(const QString &country);

    /**
     * Gets the country where the Event takes place.
     * @return the country
     */
    QString country() const;

    /**
     * Sets the city where the Event takes place.
     * @param city the new city
     */
    void setCity(const QString &city);

    /**
     * Gets the city where the Event takes place.
     * @return the city
     */
    QString city() const;

    /**
     * Add an attribute that is not included in the basis set of attributes exposed by the Event class.
     * If the attribute already exists it gets overwritten.
     * @param key the key of the attribute
     * @param value the value of the attribute
     */
    void addExtendedAttribute(const QString &key, const QString &value);

    /**
     * Get an attribute that is not included in the basis set of attributes exposed by the Event class.
     * @param key the key of the attribute
     * @return the value of the attribute with the specified key, or an empty string, if the key has not been found
     */
    QString extendedAttribute(const QString &key) const;

    /**
     * Get all attributes that are not included in the basis set of attributes exposed by the Event class.
     * @return the attribute mappings
     */
    QMap<QString, QString> extendedAttributes() const;

    /**
     * Checks whether this Event has an id
     * @return @c true if an id has been set, @c false otherwise
     */
    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
