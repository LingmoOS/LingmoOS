/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Todo class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Allen Winter \<winter@kde.org\>
*/

#ifndef KCALCORE_TODO_H
#define KCALCORE_TODO_H

#include "incidence.h"

namespace KCalendarCore {
/**
  @brief
  Provides a To-do in the sense of RFC2445.
*/
class Q_CORE_EXPORT Todo : public Incidence
{
public:
    /**
      A shared pointer to a Todo object.
    */
    typedef QSharedPointer<Todo> Ptr;

    /**
      List of to-dos.
    */
    typedef QVector<Ptr> List;

    ///@cond PRIVATE
    // needed for Akonadi polymorphic payload support
    typedef Incidence SuperClass;
    ///@endcond

    /**
      Constructs an empty to-do.
    */
    Todo();

    /**
      Copy constructor.
      @param other is the to-do to copy.
    */
    Todo(const Todo &other);

    /**
      Costructs a todo out of an incidence
      This constructs allows to make it easy to create a todo from an event.
      @param other is the incidence to copy.
      @since 4.14
     */
    Todo(const Incidence &other); // krazy:exclude=explicit (copy ctor)

    /**
      Destroys a to-do.
    */
    ~Todo() override;

    /**
      @copydoc IncidenceBase::type()
    */
    Q_REQUIRED_RESULT IncidenceType type() const override;

    /**
      @copydoc IncidenceBase::typeStr()
    */
    Q_REQUIRED_RESULT QByteArray typeStr() const override;

    /**
      Returns an exact copy of this todo. The returned object is owned by the caller.
      @return A pointer to a Todo containing an exact copy of this object.
    */
    Todo *clone() const override;

    /**
      Sets due date and time.

      @param dtDue The due date/time.
      @param first If true and the todo recurs, the due date of the first
      occurrence will be returned. If false and recurrent, the date of the
      current occurrence will be returned. If non-recurrent, the normal due
      date will be returned.
    */
    void setDtDue(const QDateTime &dtDue, bool first = false);

    /**
      Returns the todo due datetime.

      @param first If true and the todo recurs, the due datetime of the first
      occurrence will be returned. If false and recurrent, the datetime of the
      current occurrence will be returned. If non-recurrent, the normal due
      datetime will be returned.
      @return A QDateTime containing the todo due datetime.
    */
    Q_REQUIRED_RESULT QDateTime dtDue(bool first = false) const;

    /**
      Returns if the todo has a due datetime.
      @return true if the todo has a due datetime; false otherwise.
    */
    Q_REQUIRED_RESULT bool hasDueDate() const;

    /**
      Returns if the todo has a start datetime.
      @return true if the todo has a start datetime; false otherwise.
    */
    Q_REQUIRED_RESULT bool hasStartDate() const;

    /**
      @copydoc IncidenceBase::dtStart()
    */
    Q_REQUIRED_RESULT QDateTime dtStart() const override;

    /**
      Returns the start datetime of the todo.

      @param first If true, the start datetime of the todo will be returned;
      also, if the todo recurs, the start datetime of the first occurrence
      will be returned.
      If false and the todo recurs, the relative start datetime will be returned,
      based on the datetime returned by dtRecurrence().
      @return A QDateTime for the start datetime of the todo.
    */
    Q_REQUIRED_RESULT QDateTime dtStart(bool first) const;

    /**
      Returns whether the todo is completed or not.
      @return true if the todo is 100% completed, has status @c StatusCompleted,
      or has a completed date; false otherwise.

      @see isOverdue, isInProgress(), isOpenEnded(), isNotStarted(bool),
      setCompleted(), percentComplete()
    */
    Q_REQUIRED_RESULT bool isCompleted() const;

    /**
      Sets completion percentage and status.

      @param completed If  @c true, percentage complete is set to 100%, and
      status is set to @c StatusCompleted;  the completion date is @b not set or cleared.
      If @c false, percentage complete is set to 0%,
      status is set to @c StatusNone, and the completion date is cleared.


      @see isCompleted(), percentComplete(), hasCompletedDate()
    */
    void setCompleted(bool completed);

    /**
      Returns what percentage of the to-do is completed.
      @return The percentage complete of the to-do as an integer between 0 and 100, inclusive.
      @see setPercentComplete(), isCompleted()
    */
    Q_REQUIRED_RESULT int percentComplete() const;

    /**
      Sets what percentage of the to-do is completed.

      To prevent inconsistency, if @p percent is not 100, completed() is cleared,
      and if status() is StatusCompleted it is reset to StatusNone.

      @param percent is the completion percentage.  Values greater than 100 are
      treated as 100; values less than p are treated as 0.

      @see isCompleted(), setCompleted()
    */
    void setPercentComplete(int percent);

    /**
      Returns the to-do was completion datetime.

      @return A QDateTime for the completion datetime of the to-do.
      @see hasCompletedDate()
    */
    Q_REQUIRED_RESULT QDateTime completed() const;

    /**
      Marks this Todo, or its current recurrence, as completed.

      If the todo does not recur, its completion percentage is set to 100%,
      and its completion date is set to @p completeDate.  If its status is not
      StatusNone, it is set to StatusCompleted.

      @note
      If @p completeDate is invalid, the completion date is cleared, but the
      todo is still "complete".

      If the todo recurs, the first incomplete recurrence is marked complete.

      @param completeDate is the to-do completion date.
      @see completed(), hasCompletedDate()
    */
    void setCompleted(const QDateTime &completeDate);

    /**
      Returns if the to-do has a completion datetime.

      @return true if the to-do has a date associated with completion; false otherwise.
      @see setCompleted(), completed()
    */
    bool hasCompletedDate() const;

    /**
      Returns true, if the to-do is in-progress (started, or >0% completed);
      otherwise return false. If the to-do is overdue, then it is not
      considered to be in-progress.

      @param first If true, the start and due dates of the todo will be used;
      also, if the todo recurs, the start date and due date of the first
      occurrence will be used.
      If false and the todo recurs, the relative start and due dates will be
      used, based on the date returned by dtRecurrence().
      @see isOverdue(), isCompleted(), isOpenEnded(), isNotStarted(bool)
    */
    Q_REQUIRED_RESULT bool isInProgress(bool first) const;

    /**
      Returns true, if the to-do is open-ended (no due date); false otherwise.
      @see isOverdue(), isCompleted(), isInProgress(), isNotStarted(bool)
    */
    Q_REQUIRED_RESULT bool isOpenEnded() const;

    /**
      Returns true, if the to-do has yet to be started (no start date and 0%
      completed); otherwise return false.

      @param first If true, the start date of the todo will be used;
      also, if the todo recurs, the start date of the first occurrence
      will be used.
      If false and the todo recurs, the relative start date will be used,
      based on the date returned by dtRecurrence().
      @see isOverdue(), isCompleted(), isInProgress(), isOpenEnded()
    */
    Q_REQUIRED_RESULT bool isNotStarted(bool first) const;

    /**
      @copydoc IncidenceBase::shiftTimes()
    */
    void shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone) override;

    /**
      @copydoc IncidenceBase::setAllDay().
    */
    void setAllDay(bool allDay) override;

    /**
      Sets the due date/time of the current occurrence if recurrent.

      @param dt is the
    */
    void setDtRecurrence(const QDateTime &dt);

    /**
      Returns the due date/time of the current occurrence if recurrent.
    */
    Q_REQUIRED_RESULT QDateTime dtRecurrence() const;

    /**
      Returns true if the @p date specified is one on which the to-do will
      recur. Todos are a special case, hence the overload. It adds an extra
      check, which make it return false if there's an occurrence between
      the recur start and today.

      @param date is the date to check.
      @param timeZone is the time zone
    */
    bool recursOn(const QDate &date, const QTimeZone &timeZone) const override;

    /**
      Returns true if this todo is overdue (e.g. due date is lower than today
      and not completed), else false.
      @see isCompleted(), isInProgress(), isOpenEnded(), isNotStarted(bool)
     */
    bool isOverdue() const;

    /**
      @copydoc IncidenceBase::dateTime()
    */
    Q_REQUIRED_RESULT QDateTime dateTime(DateTimeRole role) const override;

    /**
      @copydoc IncidenceBase::setDateTime()
    */
    void setDateTime(const QDateTime &dateTime, DateTimeRole role) override;

    /**
       @copydoc IncidenceBase::mimeType()
    */
    Q_REQUIRED_RESULT QLatin1String mimeType() const override;

    /**
       @copydoc Incidence::iconName()
    */
    Q_REQUIRED_RESULT QLatin1String iconName(const QDateTime &recurrenceId = {}) const override;

    /**
       @copydoc
       Incidence::supportsGroupwareCommunication()
    */
    bool supportsGroupwareCommunication() const override;

    /**
       Returns the Akonadi specific sub MIME type of a KCalendarCore::Todo.
    */
    Q_REQUIRED_RESULT static QLatin1String todoMimeType();

protected:
    /**
      Compare this with @p todo for equality.
      @param todo is the to-do to compare.
    */
    bool equals(const IncidenceBase &todo) const override;

    /**
      @copydoc IncidenceBase::assign()
    */
    IncidenceBase &assign(const IncidenceBase &other) override;

    /**
      @copydoc IncidenceBase::virtual_hook()
    */
    void virtual_hook(VirtualHook id, void *data) override;

private:
    /**
      @copydoc IncidenceBase::accept()
    */
    bool accept(Visitor &v, const IncidenceBase::Ptr &incidence) override;

    /**
      Disabled, otherwise could be dangerous if you subclass Todo.
      Use IncidenceBase::operator= which is safe because it calls
      virtual function assign().
      @param other is another Todo object to assign to this one.
     */
    Todo &operator=(const Todo &other);

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
Q_DECLARE_TYPEINFO(KCalendarCore::Todo::Ptr, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KCalendarCore::Todo::Ptr)
Q_DECLARE_METATYPE(KCalendarCore::Todo *)
//@endcond

#endif
