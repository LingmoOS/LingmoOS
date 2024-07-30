/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 1998 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the VCalFormat base class.

  This class implements the vCalendar format. It provides methods for
  loading/saving/converting vCalendar format data into the internal
  representation as Calendar and Incidences.

  @brief
  vCalendar format implementation.

  @author Preston Brown \<pbrown@kde.org\>
  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#ifndef KCALCORE_VCALFORMAT_H
#define KCALCORE_VCALFORMAT_H

#include "attendee.h"
#include "calformat.h"
#include "event.h"
#include "journal.h"
#include "kcalendarcore_export.h"
#include "todo.h"

struct VObject;

class QDate;

#define _VCAL_VERSION "1.0"

/* extensions for iMIP / iTIP */
#define ICOrganizerProp "X-ORGANIZER"
#define ICMethodProp "X-METHOD"
#define ICRequestStatusProp "X-REQUEST-STATUS"

namespace KCalendarCore
{
class Event;
class Todo;
class VCalFormatPrivate;

/**
  @brief
  vCalendar format implementation.

  This class implements the vCalendar format. It provides methods for
  loading/saving/converting vCalendar format data into the internal
  representation as Calendar and Incidences.
*/
class KCALENDARCORE_EXPORT VCalFormat : public CalFormat
{
public:
    /**
      Constructor a new vCalendar Format object.
    */
    VCalFormat();

    /**
      Destructor.
    */
    ~VCalFormat() override;

    /**
      @copydoc
      CalFormat::load()
    */
    bool load(const Calendar::Ptr &calendar, const QString &fileName) override;

    /**
      @copydoc
      CalFormat::save()
    */
    bool save(const Calendar::Ptr &calendar, const QString &fileName) override;

    /**
      @copydoc
      CalFormat::toString()
    */
    Q_REQUIRED_RESULT QString toString(const Calendar::Ptr &calendar) override;

    /**
      @copydoc
      CalFormat::fromRawString()
    */
    Q_REQUIRED_RESULT bool fromRawString(const Calendar::Ptr &calendar, const QByteArray &string) override;

protected:
    /**
      Translates a VObject of the TODO type into an Event.
      @param vtodo is a pointer to a valid VObject object.
    */
    Todo::Ptr VTodoToEvent(VObject *vtodo);

    /**
      Translates a VObject into a Event and returns a pointer to it.
      @param vevent is a pointer to a valid VObject object.
    */
    Event::Ptr VEventToEvent(VObject *vevent);

    /**
      Parse TZ tag from vtimezone.
    */
    QString parseTZ(const QByteArray &timezone) const;

    /**
      Parse DAYLIGHT tag from vtimezone.
    */
    QString parseDst(QByteArray &timezone) const;

    /**
      Takes a QDate and returns a string in the format YYYYMMDDTHHMMSS.
      @param date is the date to format.
    */
    QString qDateToISO(const QDate &date);

    /**
      Takes a QDateTime and returns a string in format YYYYMMDDTHHMMSS.
      @param date is the date to format.
      @param zulu if true, then shift the date to UTC.
    */
    QString qDateTimeToISO(const QDateTime &date, bool zulu = true);

    /**
      Takes a string in YYYYMMDDTHHMMSS format and returns a valid QDateTime.
      @param dtStr is a QString containing the date to convert. If this value
      is invalid, then QDateTime() is returned.
    */
    QDateTime ISOToQDateTime(const QString &dtStr);

    /**
      Takes a string in the YYYYMMDD format and returns a valid QDate.
      @param dtStr is a QString containing the date to convert. If this value
      is invalid, then QDateTime() is returned.
    */
    QDate ISOToQDate(const QString &dtStr);

    /**
      Parse one of the myriad of ISO8601 timezone offset formats, e.g.
      +- hh : mm
      +- hh mm
      +- hh

      @param s string to be parsed.
      @param result timezone offset in seconds, if parse succeeded.
      @return Whether the parse succeeded or not.
    */
    bool parseTZOffsetISO8601(const QString &s, int &result);

    /**
      Takes a vCalendar tree of VObjects, and puts all of them that have the
      "event" property into the dictionary, todos in the todo-list, etc.
    */
    void populate(VObject *vcal);

    /**
      Converts a two letter representation of the day (i.e. MO, TU, WE, etc) and
      returns a number 0-6 corresponding to that ordinal day of the week.
      @param day is the QString containing the two letter day representation.
    */
    int numFromDay(const QString &day);

    /**
      Converts a status string into an Attendee::PartStat.
      @param s is a null-terminated character string containing the status to convert.

      @return a valid Attendee::PartStat.  If the string provided is empty, null,
      or the contents are unrecognized, then Attendee::NeedsAction is returned.
    */
    Attendee::PartStat readStatus(const char *s) const;

    /**
      Converts an Attendee::PartStat into a QByteArray string.
      @param status is the Attendee::PartStat to convert.

      @return a QByteArray containing the status string.
    */
    QByteArray writeStatus(Attendee::PartStat status) const;

    void readCustomProperties(VObject *o, const Incidence::Ptr &i);
    void writeCustomProperties(VObject *o, const Incidence::Ptr &i);

private:
    //@cond PRIVATE
    Q_DISABLE_COPY(VCalFormat)
    Q_DECLARE_PRIVATE(VCalFormat)
    //@endcond
};

}

#endif
