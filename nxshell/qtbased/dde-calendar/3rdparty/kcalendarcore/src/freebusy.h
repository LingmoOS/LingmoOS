/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the FreeBusy class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#ifndef KCALCORE_FREEBUSY_H
#define KCALCORE_FREEBUSY_H

#include "event.h"
#include "freebusyperiod.h"
#include "incidencebase.h"
#include "period.h"

#include <QMetaType>

namespace KCalendarCore {
class FreeBusy;

/**
  @brief
  Provides information about the free/busy time of a calendar.

  A free/busy is a collection of Periods.

  @see Period.
*/
class Q_CORE_EXPORT FreeBusy : public IncidenceBase
{
    friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &s, const KCalendarCore::FreeBusy::Ptr &freebusy);
    friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &s, KCalendarCore::FreeBusy::Ptr &freebusy);

public:
    /**
      A shared pointer to a FreeBusy object.
    */
    typedef QSharedPointer<FreeBusy> Ptr;

    /**
      List of FreeBusy objects.
    */
    typedef QVector<Ptr> List;

    /**
      Constructs an free/busy without any periods.
    */
    FreeBusy();

    /**
      Copy constructor.
      @param other is the free/busy to copy.
    */
    FreeBusy(const FreeBusy &other);

    /**
      Constructs a free/busy from a list of periods.
      @param busyPeriods is a list of periods.
    */
    explicit FreeBusy(const Period::List &busyPeriods);

    /**
      Constructs a free/busy from a list of periods.
      @param busyPeriods is a list of periods.
    */
    explicit FreeBusy(const FreeBusyPeriod::List &busyPeriods);

    /**
      Constructs a free/busy from a single period.

      @param start is the start date/time of the period.
      @param end is the end date/time of the period.
    */
    FreeBusy(const QDateTime &start, const QDateTime &end);

    /**
      Constructs a freebusy for a specified list of events given a single period.

      @param events list of events.
      @param start is the start date/time of the period.
      @param end is the end date/time of the period.
    */
    FreeBusy(const Event::List &events, const QDateTime &start, const QDateTime &end);

    /**
      Destroys a free/busy.
    */
    ~FreeBusy() override;

    /**
      @copydoc
      IncidenceBase::type()
    */
    Q_REQUIRED_RESULT IncidenceType type() const override;

    /**
      @copydoc
      IncidenceBase::typeStr()
    */
    Q_REQUIRED_RESULT QByteArray typeStr() const override;

    /**
      Sets the start date/time for the free/busy. Note that this date/time
      may be later or earlier than all periods within the free/busy.

      @param start is a QDateTime specifying an start datetime.
      @see IncidenceBase::dtStart(), setDtEnd().
    */
    void setDtStart(const QDateTime &start) override;

    /**
      Sets the end datetime for the free/busy. Note that this datetime
      may be later or earlier than all periods within the free/busy.

      @param end is a QDateTime specifying an end datetime.
      @see dtEnd(), setDtStart().
    */
    void setDtEnd(const QDateTime &end);

    /**
      Returns the end datetime for the free/busy.
      FIXME: calling addPeriod() does not change mDtEnd. Is that incorrect?
      @see setDtEnd().
    */
    Q_REQUIRED_RESULT virtual QDateTime dtEnd() const;

    /**
      @copydoc
      IncidenceBase::shiftTimes()
    */
    void shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone) override;

    /**
      Returns the list of all periods within the free/busy.
    */
    Q_REQUIRED_RESULT Period::List busyPeriods() const;

    /**
      Returns the list of all periods within the free/busy.
    */
    Q_REQUIRED_RESULT FreeBusyPeriod::List fullBusyPeriods() const;

    /**
      Adds a period to the freebusy list and sorts the list.

      @param start is the start datetime of the period.
      @param end is the end datetime of the period.
    */
    void addPeriod(const QDateTime &start, const QDateTime &end);

    /**
      Adds a period to the freebusy list and sorts the list.

      @param start is the start datetime of the period.
      @param duration is the Duration of the period.
    */
    void addPeriod(const QDateTime &start, const Duration &duration);

    /**
      Adds a list of periods to the freebusy object and then sorts that list.
      Use this if you are adding many items, instead of the addPeriod method,
      to avoid sorting repeatedly.

      @param list is a list of Period objects.
    */
    void addPeriods(const Period::List &list);

    /**
      Adds a list of periods to the freebusy object and then sorts that list.
      Use this if you are adding many items, instead of the addPeriod method,
      to avoid sorting repeatedly.

      @param list is a list of FreeBusyPeriod objects.
    */
    void addPeriods(const FreeBusyPeriod::List &list);

    /**
      Sorts the list of free/busy periods into ascending order.
    */
    void sortList();

    /**
      Merges another free/busy into this free/busy.

      @param freebusy is a pointer to a valid FreeBusy object.
    */
    void merge(const FreeBusy::Ptr &freebusy);

    /**
      @copydoc
      IncidenceBase::dateTime()
    */
    Q_REQUIRED_RESULT QDateTime dateTime(DateTimeRole role) const override;

    /**
      @copydoc
      IncidenceBase::setDateTime()
    */
    void setDateTime(const QDateTime &dateTime, DateTimeRole role) override;

    /**
       @copydoc
       IncidenceBase::mimeType()
    */
    Q_REQUIRED_RESULT QLatin1String mimeType() const override;

    /**
       Returns the Akonadi specific sub MIME type of a KCalendarCore::FreeBusy.
    */
    Q_REQUIRED_RESULT static QLatin1String freeBusyMimeType();

protected:
    /**
      Compare this with @p freebusy for equality.
      @param freebusy is the FreeBusy to compare.
    */
    bool equals(const IncidenceBase &freebusy) const override;

    /**
      @copydoc
      IncidenceBase::assign()
    */
    IncidenceBase &assign(const IncidenceBase &other) override;

    /**
      @copydoc
      IncidenceBase::virtual_hook()
    */
    void virtual_hook(VirtualHook id, void *data) override;

private:
    /**
     @copydoc
     IncidenceBase::accept()
    */
    bool accept(Visitor &v, const IncidenceBase::Ptr &incidence) override;

    /**
      Disabled, otherwise could be dangerous if you subclass FreeBusy.
      Use IncidenceBase::operator= which is safe because it calls
      virtual function assign().
      @param other is another FreeBusy object to assign to this one.
     */
    FreeBusy &operator=(const FreeBusy &other);

    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

/**
  Serializes the @p freebusy object into the @p stream.
*/
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const KCalendarCore::FreeBusy::Ptr &freebusy);
/**
  Initializes the @p freebusy object from the @p stream.
*/
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, KCalendarCore::FreeBusy::Ptr &freebusy);
} // namespace KCalendarCore

//@cond PRIVATE
Q_DECLARE_TYPEINFO(KCalendarCore::FreeBusy::Ptr, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KCalendarCore::FreeBusy::Ptr)
//@endcond

#endif
