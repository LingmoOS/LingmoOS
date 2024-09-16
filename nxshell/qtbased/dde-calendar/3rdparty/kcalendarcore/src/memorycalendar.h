/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 1998 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the MemoryCalendar class.

  Very simple implementation of a Calendar that is only in memory

  @author Preston Brown \<pbrown@kde.org\>
  @author Cornelius Schumacher \<schumacher@kde.org\>
 */
#ifndef KCALCORE_MEMORYCALENDAR_H
#define KCALCORE_MEMORYCALENDAR_H

#include "calendar.h"

namespace KCalendarCore {
/**
  @brief
  This class provides a calendar stored in memory.
  此类提供存储在内存中的日历
*/
class Q_CORE_EXPORT MemoryCalendar : public Calendar
{
    Q_OBJECT
public:
    /**
      A shared pointer to a MemoryCalendar
    */
    typedef QSharedPointer<MemoryCalendar> Ptr;

    /**
      @copydoc Calendar::Calendar(const QTimeZone &)
    */
    explicit MemoryCalendar(const QTimeZone &timeZone);

    /**
      @copydoc Calendar::Calendar(const QString &)
    */
    explicit MemoryCalendar(const QByteArray &timeZoneId);

    /**
      @copydoc Calendar::~Calendar()
    */
    ~MemoryCalendar() override;

    /**
      Clears out the current calendar, freeing all used memory etc. etc.
    */
    void close() override;

    /**
      @copydoc Calendar::doSetTimeZone()
    */
    void doSetTimeZone(const QTimeZone &timeZone) override;

    /**
      @copydoc Calendar::deleteIncidence()
    */
    bool deleteIncidence(const Incidence::Ptr &incidence) override;

    /**
       @copydoc Calendar::deleteIncidenceInstances
    */
    bool deleteIncidenceInstances(const Incidence::Ptr &incidence) override;

    /**
       @copydoc Calendar::addIncidence()
    */
    bool addIncidence(const Incidence::Ptr &incidence) override;

    // Event Specific Methods //

    /**
      @copydoc Calendar::addEvent()
    */
    bool addEvent(const Event::Ptr &event) override;

    /**
      @copydoc Calendar::deleteEvent()
    */
    bool deleteEvent(const Event::Ptr &event) override;

    /**
      @copydoc Calendar::deleteEventInstances()
    */
    bool deleteEventInstances(const Event::Ptr &event) override;

    /**
      @copydoc Calendar::rawEvents(EventSortField, SortDirection)const
    */
    Q_REQUIRED_RESULT Event::List rawEvents(EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending) const override;

    /**
      @copydoc Calendar::rawEvents(const QDate &, const QDate &, const QTimeZone &, bool)const
    */
    Q_REQUIRED_RESULT Event::List rawEvents(const QDate &start, const QDate &end, const QTimeZone &timeZone = {}, bool inclusive = false) const override;

    /**
      Returns an unfiltered list of all Events which occur on the given date.

      @param date request unfiltered Event list for this QDate only.
      @param timeZone time zone to interpret @p date, or the calendar's
                      default time zone if none is specified
      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of unfiltered Events occurring on the specified QDate.
    */
    Q_REQUIRED_RESULT Event::List rawEventsForDate(const QDate &date,
                                                   const QTimeZone &timeZone = {},
                                                   EventSortField sortField = EventSortUnsorted,
                                                   SortDirection sortDirection = SortDirectionAscending) const override;

    /**
      @copydoc Calendar::rawEventsForDate(const QDateTime &)const
    */
    Q_REQUIRED_RESULT Event::List rawEventsForDate(const QDateTime &dt) const override;

    /**
     * Returns an incidence by identifier.
     * @see Incidence::instanceIdentifier()
     * @since 4.11
     */
    Incidence::Ptr instance(const QString &identifier) const;

    /**
      @copydoc Calendar::event()
    */
    Q_REQUIRED_RESULT Event::Ptr event(const QString &uid, const QDateTime &recurrenceId = {}) const override;

    /**
      @copydoc Calendar::deletedEvent()
    */
    Q_REQUIRED_RESULT Event::Ptr deletedEvent(const QString &uid, const QDateTime &recurrenceId = {}) const override;

    /**
      @copydoc Calendar::deletedEvents(EventSortField, SortDirection)const
    */
    Q_REQUIRED_RESULT Event::List deletedEvents(EventSortField sortField = EventSortUnsorted,
                                                SortDirection sortDirection = SortDirectionAscending) const override;

    /**
      @copydoc Calendar::eventInstances(const Incidence::Ptr &, EventSortField, SortDirection)const
    */
    Q_REQUIRED_RESULT Event::List eventInstances(const Incidence::Ptr &event,
                                                 EventSortField sortField = EventSortUnsorted,
                                                 SortDirection sortDirection = SortDirectionAscending) const override;

    // To-do Specific Methods //

    /**
      @copydoc Calendar::addTodo()
    */
    bool addTodo(const Todo::Ptr &todo) override;

    /**
      @copydoc Calendar::deleteTodo()
    */
    bool deleteTodo(const Todo::Ptr &todo) override;

    /**
      @copydoc Calendar::deleteTodoInstances()
    */
    bool deleteTodoInstances(const Todo::Ptr &todo) override;

    /**
      @copydoc Calendar::rawTodos(TodoSortField, SortDirection)const
    */
    Q_REQUIRED_RESULT Todo::List rawTodos(TodoSortField sortField = TodoSortUnsorted, SortDirection sortDirection = SortDirectionAscending) const override;

    /**
       @copydoc Calendar::rawTodos(const QDate &, const QDate &, const QTimeZone &, bool)const
    */
    Q_REQUIRED_RESULT Todo::List rawTodos(const QDate &start, const QDate &end, const QTimeZone &timeZone = {}, bool inclusive = false) const override;

    /**
      @copydoc Calendar::rawTodosForDate()
    */
    Q_REQUIRED_RESULT Todo::List rawTodosForDate(const QDate &date) const override;

    /**
      @copydoc Calendar::todo()
    */
    Q_REQUIRED_RESULT Todo::Ptr todo(const QString &uid, const QDateTime &recurrenceId = {}) const override;

    /**
      @copydoc Calendar::deletedTodo()
    */
    Q_REQUIRED_RESULT Todo::Ptr deletedTodo(const QString &uid, const QDateTime &recurrenceId = {}) const override;

    /**
      @copydoc Calendar::deletedTodos(TodoSortField, SortDirection)const
    */
    Q_REQUIRED_RESULT Todo::List deletedTodos(TodoSortField sortField = TodoSortUnsorted, SortDirection sortDirection = SortDirectionAscending) const override;

    /**
      @copydoc Calendar::todoInstances(const Incidence::Ptr &, TodoSortField, SortDirection)const
    */
    Q_REQUIRED_RESULT Todo::List
    todoInstances(const Incidence::Ptr &todo, TodoSortField sortField = TodoSortUnsorted, SortDirection sortDirection = SortDirectionAscending) const override;

    // Journal Specific Methods //

    /**
      @copydoc Calendar::addJournal()
    */
    bool addJournal(const Journal::Ptr &journal) override;

    /**
      @copydoc Calendar::deleteJournal()
    */
    bool deleteJournal(const Journal::Ptr &journal) override;

    /**
      @copydoc Calendar::deleteJournalInstances()
    */
    bool deleteJournalInstances(const Journal::Ptr &journal) override;

    /**
      @copydoc Calendar::rawJournals()
    */
    Q_REQUIRED_RESULT Journal::List rawJournals(JournalSortField sortField = JournalSortUnsorted,
                                                SortDirection sortDirection = SortDirectionAscending) const override;

    /**
      @copydoc Calendar::rawJournalsForDate()
    */
    Q_REQUIRED_RESULT Journal::List rawJournalsForDate(const QDate &date) const override;

    /**
      @copydoc Calendar::journal()
    */
    Journal::Ptr journal(const QString &uid, const QDateTime &recurrenceId = {}) const override;

    /**
      @copydoc Calendar::deletedJournal()
    */
    Journal::Ptr deletedJournal(const QString &uid, const QDateTime &recurrenceId = {}) const override;

    /**
      @copydoc Calendar::deletedJournals(JournalSortField, SortDirection)const
    */
    Q_REQUIRED_RESULT Journal::List deletedJournals(JournalSortField sortField = JournalSortUnsorted,
                                                    SortDirection sortDirection = SortDirectionAscending) const override;

    /**
      @copydoc Calendar::journalInstances(const Incidence::Ptr &,
                                          JournalSortField, SortDirection)const
    */
    Q_REQUIRED_RESULT Journal::List journalInstances(const Incidence::Ptr &journal,
                                                     JournalSortField sortField = JournalSortUnsorted,
                                                     SortDirection sortDirection = SortDirectionAscending) const override;

    // Alarm Specific Methods //

    /**
      @copydoc Calendar::alarms()
    */
    Q_REQUIRED_RESULT Alarm::List alarms(const QDateTime &from, const QDateTime &to, bool excludeBlockedAlarms = false) const override;

    /**
      Return a list of Alarms that occur before the specified timestamp.

      @param to is the ending timestamp.
      @return the list of Alarms occurring before the specified QDateTime.
    */
    Q_REQUIRED_RESULT Alarm::List alarmsTo(const QDateTime &to) const; // TODO KF6 remove, already defined in Calendar

    /**
      Return true if the memory calendar is updating the lastModified field
      of incidence owned by the calendar on any incidence change.

      @since 5.80
    */
    bool updateLastModifiedOnChange() const;

    /**
      Govern if the memory calendar is changing the lastModified field of incidence
      it owns, on incidence updates.

      @param update, when true, the lastModified field of an incidence owned by the
      calendar is set to the current date time on any change of the incidence.

      @since 5.80
    */
    void setUpdateLastModifiedOnChange(bool update);

    /**
      @copydoc Calendar::incidenceUpdate(const QString &,const QDateTime &)
    */
    void incidenceUpdate(const QString &uid, const QDateTime &recurrenceId) override;

    /**
      @copydoc Calendar::incidenceUpdated(const QString &,const QDateTime &)
    */
    void incidenceUpdated(const QString &uid, const QDateTime &recurrenceId) override;

    using QObject::event; // prevent warning about hidden virtual method

protected:
    /**
      @copydoc IncidenceBase::virtual_hook()
    */
    void virtual_hook(int id, void *data) override;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

    Q_DISABLE_COPY(MemoryCalendar)
};

} // namespace KCalendarCore

#endif
