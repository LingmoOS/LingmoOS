/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and defines
  classes for managing compatibility between different calendar formats.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#ifndef KCALCORE_COMPAT_P_H
#define KCALCORE_COMPAT_P_H

#include "incidence.h"

#include <QtGlobal> // for Q_DISABLE_COPY()

#include <memory>

class QDate;
class QString;

namespace KCalendarCore
{
class Compat;

/**
  @brief
  Factory for creating the right Compat object.

  @internal
*/
class CompatFactory
{
public:
    /**
      Creates the appropriate Compat class as determined by the Product ID.

      @param productId is a string containing a valid Product ID from
      a supported calendar format.
      @return A pointer to a Compat object which is owned by the caller.
    */
    static Compat *createCompat(const QString &productId, const QString &implementationVersion);
};

/**
  @brief
  This class provides compatibility to older or broken calendar files.

  @internal
*/
class Compat
{
public:
    /**
      Destructor.
    */
    virtual ~Compat();

    /**
      Fixes the recurrence rule for an incidence.
      @param incidence is a pointer to an Incidence object that may
      need its recurrence rule fixed.
    */
    virtual void fixRecurrence(const Incidence::Ptr &incidence);

    /**
      Fixes an empty summary for an incidence.
      @param incidence is a pointer to an Incidence object that may need
      its summary fixed.
    */
    virtual void fixEmptySummary(const Incidence::Ptr &incidence);

    /**
      Fixes the alarms list an incidence.
      @param incidence is a pointer to an Incidence object that may need
      its alarms fixed.
    */
    virtual void fixAlarms(const Incidence::Ptr &incidence);

    /**
      Fixes the end date for floating events.
      @param date is the end date to fix.
    */
    virtual void fixFloatingEnd(QDate &date);

    /**
      Fixes the priority.
      @param priority is the priority value to fix.
      @return an integer representing a valid priority value.
    */
    virtual int fixPriority(int priority);

    /**
      Returns true if a timezone shift should be used; false otherwise.
    */
    virtual bool useTimeZoneShift() const;

    /**
      Sets the created and dtstamp.
    */
    virtual void setCreatedToDtStamp(const Incidence::Ptr &incidence, const QDateTime &dtstamp);
};

/**
  @brief
  Decorator so multiple compatibility classes can be stacked.
*/
class CompatDecorator : public Compat
{
public:
    explicit CompatDecorator(Compat *decoratedCompat);
    ~CompatDecorator() override;

    /**
      @copydoc
      Compat::fixRecurrence()
    */
    void fixRecurrence(const Incidence::Ptr &incidence) override;

    /**
      @copydoc
      Compat::fixEmptySummary()
    */
    void fixEmptySummary(const Incidence::Ptr &incidence) override;

    /**
      @copydoc
      Compat::fixAlarms()
    */
    void fixAlarms(const Incidence::Ptr &incidence) override;

    /**
      @copydoc
      Compat::fixFloatingEnd()
    */
    void fixFloatingEnd(QDate &date) override;

    /**
      @copydoc
      Compat::fixPriority()
    */
    int fixPriority(int priority) override;

    /**
      @copydoc
      Compat::useTimeZoneShift()
    */
    bool useTimeZoneShift() const override;

    /**
      @copydoc
      Compat::setCreatedToDtStamp()
    */
    void setCreatedToDtStamp(const Incidence::Ptr &incidence, const QDateTime &dtstamp) override;

private:
    Q_DISABLE_COPY(CompatDecorator)
    std::unique_ptr<Compat> m_compat;
};

/**
  @brief
  Compatibility class for KOrganizer pre-3.5 calendar files.

  Before kde 3.5, the start date was not automatically a recurring date.
  So, if the start date doesn't match the recurrence rule, we need to add
  an ex-date for the date start. If a duration was given, the DTSTART was
  only counted if it matched, so by accident this was already the correct
  behavior, so we don't need to adjust the duration.
*/
class CompatPre35 : public Compat
{
public:
    /**
      @copydoc
      Compat::fixRecurrence()
    */
    void fixRecurrence(const Incidence::Ptr &incidence) override;
};

/**
  @brief
  Compatibility class for KOrganizer pre-3.4 calendar files.
*/
class CompatPre34 : public CompatPre35
{
public:
    /**
      @copydoc
      Compat::fixPriority()
    */
    int fixPriority(int priority) override;
};

/**
  @brief
  Compatibility class for KOrganizer pre-3.2 calendar files.

  The recurrence has a specified number of repetitions.
  Pre-3.2, this was extended by the number of exception dates.
  This is also rfc 2445-compliant. The duration of an RRULE also counts
  events that are later excluded via EXDATE or EXRULE.
*/
class CompatPre32 : public CompatPre34
{
public:
    /**
      @copydoc
      Compat::fixRecurrence()
    */
    void fixRecurrence(const Incidence::Ptr &incidence) override;
};

/**
  @brief
  Compatibility class for KOrganizer pre-3.1 calendar files.

  Before kde 3.1, floating events (events without a date) had 0:00 of their
  last day as the end date. E.g. 28.5.2005  0:00 until 28.5.2005 0:00 for an
  event that lasted the whole day on May 28, 2005. According to RFC 2445, the
  end date for such an event needs to be 29.5.2005 0:00.

  Update: We misunderstood rfc 2445 in this regard. For all-day events, the
  DTEND is the last day of the event. See a mail from the Author or rfc 2445:
    http://www.imc.org/ietf-calendar/archive1/msg03648.html
  However, as all other applications also got this wrong, we'll just leave it
  as it is and use the wrong interpretation (was also discussed on ietf-calsify)
*/
class CompatPre31 : public CompatPre32
{
public:
    /**
      @copydoc
      Compat::fixFloatingEnd()
    */
    void fixFloatingEnd(QDate &date) override;

    /**
      @copydoc
      Compat::fixRecurrence()
    */
    void fixRecurrence(const Incidence::Ptr &incidence) override;
};

/**
  @brief
  Compatibility class for KOrganizer prerelease 3.2 calendar files.
*/
class Compat32PrereleaseVersions : public Compat
{
public:
    /**
      @copydoc
      Compat::useTimeZoneShift()
    */
    bool useTimeZoneShift() const override;
};

/**
  @brief
  Compatibility class for Outlook 9 calendar files.

  In Outlook 9, alarms have the wrong sign. I.e. RFC 2445 says that negative
  values for the trigger are before the event's start. Outlook/exchange,
  however used positive values.
*/
class CompatOutlook9 : public Compat
{
public:
    /**
      @copydoc
      Compat::fixAlarms()
    */
    void fixAlarms(const Incidence::Ptr &incidence) override;
};

/**
  @brief
  Compatibility class for Kontact < 4.10 calendar files.
*/
class CompatPre410 : public CompatDecorator
{
public:
    explicit CompatPre410(Compat *decoratedCompat);
    /**
      @copydoc
      Compat::setCreatedToDtStamp()
    */
    void setCreatedToDtStamp(const Incidence::Ptr &incidence, const QDateTime &dtstamp) override;
};

}

#endif
