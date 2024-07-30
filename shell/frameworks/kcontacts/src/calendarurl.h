/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CALENDARURL_H
#define CALENDARURL_H
#include "kcontacts_export.h"

#include <QMap>
#include <QSharedDataPointer>
#include <QString>
class QUrl;

class CalendarUrlTest;

namespace KContacts
{
class ParameterMap;

/**
 * @short Class that holds a Calendar Url (FBURL/CALADRURI/CALURI)
 *
 * @see RFC 6350 Section 6.9 (https://datatracker.ietf.org/doc/html/rfc6350#section-6.9)
 * @since 4.14.6
 */
class KCONTACTS_EXPORT CalendarUrl
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const CalendarUrl &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, CalendarUrl &);
    friend class VCardTool;
    friend class ::CalendarUrlTest;

public:
    enum CalendarType {
        Unknown = 0, ///< Unknow calendar type
        FBUrl, ///< Specify the calendar containing the FreeBusy time information
        CALUri, ///< Specify the calendar associated with the contact
        CALADRUri, ///< Specify the calendar which should received the sheduling requests
        EndCalendarType,
    };

    CalendarUrl();
    CalendarUrl(CalendarUrl::CalendarType type);
    CalendarUrl(const CalendarUrl &other);

    ~CalendarUrl();

    typedef QList<CalendarUrl> List;

    Q_REQUIRED_RESULT bool isValid() const;

    void setType(CalendarUrl::CalendarType type);
    Q_REQUIRED_RESULT CalendarUrl::CalendarType type() const;

    void setUrl(const QUrl &url);
    QUrl url() const;

    Q_REQUIRED_RESULT bool operator==(const CalendarUrl &other) const;
    Q_REQUIRED_RESULT bool operator!=(const CalendarUrl &other) const;

    CalendarUrl &operator=(const CalendarUrl &other);

    Q_REQUIRED_RESULT QString toString() const;

private:
    // exported for CalendarUrlTest
    void setParams(const ParameterMap &params);
    Q_REQUIRED_RESULT ParameterMap params() const;

    class Private;
    QSharedDataPointer<Private> d;
};

KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const CalendarUrl &object);

KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, CalendarUrl &object);
}
Q_DECLARE_TYPEINFO(KContacts::CalendarUrl, Q_RELOCATABLE_TYPE);
#endif // CALENDARURL_H
