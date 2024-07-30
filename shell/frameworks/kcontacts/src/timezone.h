/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_TIMEZONE_H
#define KCONTACTS_TIMEZONE_H

#include "kcontacts_export.h"
#include <QSharedDataPointer>
#include <QString>

namespace KContacts
{
/**
 * @short Time zone information.
 *
 * This class stores information about a time zone.
 */
class KCONTACTS_EXPORT TimeZone
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const TimeZone &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, TimeZone &);

public:
    /**
     * Construct invalid time zone.
     */
    TimeZone();

    /**
     * Construct time zone.
     *
     * @param offset Offset in minutes relative to UTC.
     */
    TimeZone(int offset);

    /**
     * Copy constructor.
     */
    TimeZone(const TimeZone &other);

    /**
     * Destroys the time zone.
     */
    ~TimeZone();

    /**
     * Set time zone offset relative to UTC.
     *
     * @param offset Offset in minutes.
     */
    void setOffset(int offset);

    /**
     * Return offset in minutes relative to UTC.
     */
    Q_REQUIRED_RESULT int offset() const;

    /**
     * Return, if this time zone object is valid.
     */
    Q_REQUIRED_RESULT bool isValid() const;

    Q_REQUIRED_RESULT bool operator==(const TimeZone &other) const;
    Q_REQUIRED_RESULT bool operator!=(const TimeZone &other) const;
    TimeZone &operator=(const TimeZone &other);

    /**
     * Return string representation of time zone offset.
     */
    Q_REQUIRED_RESULT QString toString() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

/**
 * Serializes the @p timezone object into the @p stream.
 */
KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const TimeZone &timeZone);

/**
 * Initializes the @p timezone object from the @p stream.
 */
KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, TimeZone &timeZone);
}
Q_DECLARE_TYPEINFO(KContacts::TimeZone, Q_RELOCATABLE_TYPE);

#endif
