/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_GEO_H
#define KCONTACTS_GEO_H

#include "kcontacts_export.h"

#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>

namespace KContacts
{
/**
 * @short Geographic position
 *
 * This class represents a geographic position.
 */
class KCONTACTS_EXPORT Geo
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const Geo &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, Geo &);

    Q_GADGET
    Q_PROPERTY(float latitude READ latitude WRITE setLatitude)
    Q_PROPERTY(float longitude READ longitude WRITE setLongitude)
    Q_PROPERTY(bool isValid READ isValid)

public:
    /**
     * Creates an invalid geographics position object.
     */
    Geo();

    /**
     * Creates a geographics position object.
     *
     * @param latitude  Geographical latitude
     * @param longitude Geographical longitude
     */
    Geo(float latitude, float longitude);

    /**
     * Copy constructor.
     */
    Geo(const Geo &other);

    /**
     * Destroys the geographics position object.
     */
    ~Geo();

    /**
     * Sets the @p latitude.
     *
     * @param latitude The location's latitude coordinate
     */
    void setLatitude(float latitude);

    /**
     * Returns the latitude.
     */
    Q_REQUIRED_RESULT float latitude() const;

    /**
     * Sets the @p longitude.
     *
     * @param longitude The location's longitude coordinate
     */
    void setLongitude(float longitude);

    /**
     * Returns the longitude.
     */
    Q_REQUIRED_RESULT float longitude() const;

    /**
     * Returns, whether this object contains a valid geographical position.
     */
    Q_REQUIRED_RESULT bool isValid() const;

    /**
     * Equality operator.
     *
     * @note Two invalid Geo instance will return @c true
     */
    Q_REQUIRED_RESULT bool operator==(const Geo &other) const;

    /**
     * Not-Equal operator.
     */
    bool operator!=(const Geo &other) const;

    /**
     * Assignment operator.
     *
     * @param other The Geo instance to assign to @c this
     */
    Geo &operator=(const Geo &other);

    /**
     * Returns string representation of geographical position.
     */
    Q_REQUIRED_RESULT QString toString() const;

    /**
     * Clears the class, marking it as invalid.
     *
     * @since 5.6
     */
    void clear();

private:
    class Private;
    QSharedDataPointer<Private> d;
};

/**
 * Serializes the geographical position @p object into the @p stream.
 */
KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const Geo &object);

/**
 * Initializes the geographical position @p object from the @p stream.
 */
KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, Geo &object);
}

Q_DECLARE_TYPEINFO(KContacts::Geo, Q_RELOCATABLE_TYPE);

#endif
