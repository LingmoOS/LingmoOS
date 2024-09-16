/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Event class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/
#ifndef KCALCORE_EVENT_H
#define KCALCORE_EVENT_H

#include "incidence.h"

#include <QTimeZone>

namespace KCalendarCore {
/**
  @brief
  This class provides an Event in the sense of RFC2445.
  此类提供RFC2445意义上的事件。
*/
class Q_CORE_EXPORT Event : public Incidence
{
    Q_GADGET
    Q_PROPERTY(QDateTime dtEnd READ dtEnd WRITE setDtEnd)
    Q_PROPERTY(KCalendarCore::Event::Transparency transparency READ transparency WRITE setTransparency)
public:
    /**
      The different Event transparency types.
    */
    enum Transparency {
        Opaque, /**< Event appears in free/busy time */
        Transparent, /**< Event does @b not appear in free/busy time */
    };
    Q_ENUM(Transparency)

    /**
      A shared pointer to an Event object.
    */
    typedef QSharedPointer<Event> Ptr;

    /**
      List of events.
    */
    typedef QVector<Ptr> List;

    ///@cond PRIVATE
    // needed for Akonadi polymorphic payload support
    typedef Incidence SuperClass;
    ///@endcond

    /**
      Constructs an event.
    */
    Event();

    /**
      Copy constructor.
      @param other is the event to copy.
    */
    Event(const Event &other);

    /**
      Costructs an event out of an incidence
      This constructs allows to make it easy to create an event from a todo.
      @param other is the incidence to copy.
      @since 4.14
    */
    Event(const Incidence &other); // krazy:exclude=explicit (copy ctor)

    /**
      Destroys the event.
    */
    ~Event() override;

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
      Returns an exact copy of this Event. The caller owns the returned object.
    */
    Event *clone() const override;

    /**
      Sets the incidence starting date/time.
      设置事件开始日期/时间。

      @param dt is the starting date/time.
      @see IncidenceBase::dtStart().
    */
    void setDtStart(const QDateTime &dt) override;

    /**
      Sets the event end date and time.
      Important note for all day events: the end date is inclusive,
      the event will still occur during dtEnd(). When serializing to iCalendar
      DTEND will be dtEnd()+1, because the RFC states that DTEND is exclusive.
      @param dtEnd is a QDateTime specifying when the event ends.
      @see dtEnd(), dateEnd().
    */
    void setDtEnd(const QDateTime &dtEnd);

    /**
      Returns the event end date and time.
      Important note for all day events: the returned end date is inclusive,
      the event will still occur during dtEnd(). When serializing to iCalendar
      DTEND will be dtEnd()+1, because the RFC states that DTEND is exclusive.
      @see setDtEnd().
    */
    virtual QDateTime dtEnd() const;

    /**
      Returns the date when the event ends. This might be different from
      dtEnd().date, since the end date/time is non-inclusive. So timed events
      ending at 0:00 have their end date on the day before.
      返回事件结束的日期。这可能与dtEnd（）的日期不同，因为结束日期/时间不包括在内。因此，在0:00结束的定时事件的结束日期为前一天
    */
    Q_REQUIRED_RESULT QDate dateEnd() const;

    /**
      Returns whether the event has an end date/time.
    */
    Q_REQUIRED_RESULT bool hasEndDate() const;

    /**
      Returns true if the event spans multiple days, otherwise return false.
      如果事件跨越多天，则返回true，否则返回false。

      For recurring events, it returns true if the first occurrence spans multiple days,
      otherwise returns false. Other occurrences might have a different span due to day light
      savings changes.

      对于重复发生的事件，如果第一次发生跨越多天，则返回true，否则返回false。由于日光节约的变化，其他事件可能会有不同的跨度

      @param zone If set, looks if the event is multiday for the given zone.
      If not set, looks if event this multiday for its zone.
    */
    Q_REQUIRED_RESULT bool isMultiDay(const QTimeZone &zone = {}) const;

    /**
      @copydoc
      IncidenceBase::shiftTimes()
    */
    void shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone) override;

    /**
      Sets the event's time transparency level.
      设置事件的时间透明度级别。
      @param transparency is the event Transparency level.
    */
    void setTransparency(Transparency transparency);

    /**
      Returns the event's time transparency level.
    */
    Q_REQUIRED_RESULT Transparency transparency() const;

    /**
      Sets the duration of this event.
      设置此事件的持续时间。
      @param duration is the event Duration.
    */
    void setDuration(const Duration &duration) override;

    /**
      @copydoc
      IncidenceBase::setAllDay().
    */
    void setAllDay(bool allDay) override;

    //设置是否为农历日程
    void setLunnar(bool lunnar);
    bool lunnar() const;

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
       @copydoc
       Incidence::iconName()
    */
    Q_REQUIRED_RESULT QLatin1String iconName(const QDateTime &recurrenceId = {}) const override;

    /**
       @copydoc
       Incidence::supportsGroupwareCommunication()
       支持群件通信
    */
    Q_REQUIRED_RESULT bool supportsGroupwareCommunication() const override;

    /**
       Returns the Akonadi specific sub MIME type of a KCalendarCore::Event.
    */
    Q_REQUIRED_RESULT static QLatin1String eventMimeType();

protected:
    /**
      Compares two events for equality.
      比较两个事件是否相等。
      @param event is the event to compare.
    */
    bool equals(const IncidenceBase &event) const override;

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
      Disabled, otherwise could be dangerous if you subclass Event.
      Use IncidenceBase::operator= which is safe because it calls
      virtual function assign().
      @param other is another Event object to assign to this one.
     */
    Event &operator=(const Event &other);

    // For polymorfic serialization
    void serialize(QDataStream &out) const override;
    void deserialize(QDataStream &in) override;

    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

} // namespace KCalendarCore

//@cond PRIVATE
Q_DECLARE_TYPEINFO(KCalendarCore::Event::Ptr, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KCalendarCore::Event::Ptr)
Q_DECLARE_METATYPE(KCalendarCore::Event *)
//@endcond

#endif
