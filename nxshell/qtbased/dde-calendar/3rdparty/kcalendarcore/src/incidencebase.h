/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
  SPDX-FileContributor: Alvaro Manera <alvaro.manera@nokia.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the IncidenceBase class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
  @author Rafal Rzepecki \<divide@users.sourceforge.net\>

  @glossary @anchor incidence @b incidence:
  General term for a calendar component.
  Examples are events, to-dos, and journals.

  @glossary @anchor event @b event:
  An @ref incidence that has a start and end time, typically representing some
  occurrence of social or personal importance. May be recurring.
  Examples are appointments, meetings, or holidays.

  @glossary @anchor to-do @b to-do:
  An @ref incidence that has an optional start time and an optional due time
  typically representing some undertaking to be performed. May be recurring.
  Examples are "fix the bug" or "pay the bills".

  @glossary @anchor todo @b todo:
  See @ref to-do.

  @glossary @anchor journal @b journal:
  An @ref incidence with a start date that represents a diary or daily record
  of one's activities. May @b not be recurring.
*/

#ifndef KCALCORE_INCIDENCEBASE_H
#define KCALCORE_INCIDENCEBASE_H

#include "attendee.h"
#include "customproperties.h"
#include "duration.h"
#include "person.h"

#include <QDataStream>
#include <QDateTime>
#include <QSet>
#include <QSharedPointer>
#include <QUrl>

class QUrl;
class QDate;
class QTimeZone;

namespace KCalendarCore {
/** List of dates */
typedef QList<QDate> DateList;

/** List of times */
typedef QList<QDateTime> DateTimeList;

class Event;
class Todo;
class Journal;
class FreeBusy;
class Visitor;

/**
  @brief
  An abstract class that provides a common base for all calendar incidence
  classes.

  define: organizer (person)
  define: uid (same as the attendee uid?)

  Several properties are not allowed for VFREEBUSY objects (see rfc:2445),
  so they are not in IncidenceBase. The hierarchy is:

  IncidenceBase
  + FreeBusy
  + Incidence
    + Event
    + Todo
    + Journal

  So IncidenceBase contains all properties that are common to all classes,
  and Incidence contains all additional properties that are common to
  Events, Todos and Journals, but are not allowed for FreeBusy entries.
*/
class Q_CORE_EXPORT IncidenceBase : public CustomProperties
{
    Q_GADGET
    Q_PROPERTY(QString uid READ uid WRITE setUid)
    Q_PROPERTY(QDateTime lastModified READ lastModified WRITE setLastModified)
    Q_PROPERTY(QDateTime dtStart READ dtStart WRITE setDtStart)
    Q_PROPERTY(bool allDay READ allDay WRITE setAllDay)
    Q_PROPERTY(KCalendarCore::Person organizer READ organizer WRITE setOrganizer)
    Q_PROPERTY(QVariantList attendees READ attendeesVariant)

public:
    /**
      A shared pointer to an IncidenceBase.
    */
    typedef QSharedPointer<IncidenceBase> Ptr;

    /**
      The different types of incidences, per RFC2445.
      @see type(), typeStr()
    */
    enum IncidenceType {
        TypeEvent = 0, /**< Type is an event */
        TypeTodo, /**< Type is a to-do */
        TypeJournal, /**< Type is a journal */
        TypeFreeBusy, /**< Type is a free/busy */
        TypeUnknown, /**< Type unknown */
    };

    /**
      The different types of incidence date/times roles.
      不同类型的事件日期/时间角色。
      @see dateTime()
    */
    enum DateTimeRole {
        RoleAlarmStartOffset = 0, /**< Role for an incidence alarm's starting offset date/time */
        RoleAlarmEndOffset, /**< Role for an incidence alarm's ending offset date/time */
        RoleSort, /**< Role for an incidence's date/time used when sorting */
        RoleCalendarHashing, /**< Role for looking up an incidence in a Calendar */
        RoleStartTimeZone, /**< Role for determining an incidence's starting timezone */
        RoleEndTimeZone, /**< Role for determining an incidence's ending timezone */
        RoleEndRecurrenceBase,
        RoleEnd, /**< Role for determining an incidence's dtEnd, will return
                    an invalid QDateTime if the incidence does not support dtEnd */
        RoleDisplayEnd, /**< Role used for display purposes, represents the end boundary
                           if an incidence supports dtEnd */
        RoleAlarm, /**< Role for determining the date/time of the first alarm.
                      Returns invalid time if the incidence doesn't have any alarm */
        RoleRecurrenceStart, /**< Role for determining the start of the recurrence.
                                Currently that's DTSTART for an event and DTDUE for a to-do.
                                (NOTE: If the incidence is a to-do, recurrence should be
                                calculated having DTSTART for a reference, not DT-DUE.
                                This is one place KCalendarCore isn't compliant with RFC2445) */
        RoleDisplayStart, /**< Role for display purposes, represents the start boundary of an
                             incidence. To-dos return dtDue here, for historical reasons */
        RoleDnD, /**< Role for determining new start and end dates after a DnD */
    };

    /**
      The different types of incidence fields.
      //不同类型的入射场。
    */
    enum Field {
        FieldDtStart, ///< Field representing the DTSTART component.
        FieldDtEnd, ///< Field representing the DTEND component.
        FieldLastModified, ///< Field representing the LAST-MODIFIED component.
        FieldDescription, ///< Field representing the DESCRIPTION component.
        FieldSummary, ///< Field representing the SUMMARY component.
        FieldLocation, ///< Field representing the LOCATION component.
        FieldCompleted, ///< Field representing the COMPLETED component.
        FieldPercentComplete, ///< Field representing the PERCENT-COMPLETE component.
        FieldDtDue, ///< Field representing the DUE component.
        FieldCategories, ///< Field representing the CATEGORIES component.
        FieldRelatedTo, ///< Field representing the RELATED-TO component.
        FieldRecurrence, ///< Field representing the EXDATE, EXRULE, RDATE, and RRULE components.
        FieldAttachment, ///< Field representing the ATTACH component.
        FieldSecrecy, ///< Field representing the CLASS component.
        FieldStatus, ///< Field representing the STATUS component.
        FieldTransparency, ///< Field representing the TRANSPARENCY component.
        FieldResources, ///< Field representing the RESOURCES component.
        FieldPriority, ///< Field representing the PRIORITY component.
        FieldGeoLatitude, ///< Field representing the latitude part of the GEO component.
        FieldGeoLongitude, ///< Field representing the longitude part of the GEO component.
        FieldRecurrenceId, ///< Field representing the RECURRENCE-ID component.
        FieldAlarms, ///< Field representing the VALARM component.
        FieldSchedulingId, ///< Field representing the X-KDE-LIBKCAL-ID component.
        FieldAttendees, ///< Field representing the ATTENDEE component.
        FieldOrganizer, ///< Field representing the ORGANIZER component.
        FieldCreated, ///< Field representing the CREATED component.
        FieldRevision, ///< Field representing the SEQUENCE component.
        FieldDuration, ///< Field representing the DURATION component.
        FieldContact, ///< Field representing the CONTACT component.
        FieldComment, ///< Field representing the COMMENT component.
        FieldUid, ///< Field representing the UID component.
        FieldUnknown, ///< Something changed. Always set when you use the assignment operator.
        FieldUrl, ///< Field representing the URL component.
        FieldConferences, ///< Field representing the CONFERENCE component.
        FieldColor, ///< Field representing the COLOR component.
    };

    /**
      The IncidenceObserver class.
      事件观察者类。
    */
    class Q_CORE_EXPORT IncidenceObserver
    {
    public:
        /**
          Destroys the IncidenceObserver.
        */
        virtual ~IncidenceObserver();

        /**
          The IncidenceObserver interface.
          This function is called before any changes are made.
          @param uid is the string containing the incidence @ref uid.
          @param recurrenceId is possible recurrenceid of incidence.
        */
        virtual void incidenceUpdate(const QString &uid, const QDateTime &recurrenceId) = 0;

        /**
          The IncidenceObserver interface.
          This function is called after changes are completed.
          @param uid is the string containing the incidence @ref uid.
          @param recurrenceId is possible recurrenceid of incidence.
        */
        virtual void incidenceUpdated(const QString &uid, const QDateTime &recurrenceId) = 0;
    };

    /**
      Constructs an empty IncidenceBase.
    */
    IncidenceBase();

    /**
      Destroys the IncidenceBase.
    */
    ~IncidenceBase() override;

    /**
      Assignment operator.
      All data belonging to derived classes are also copied. @see assign().
      The caller guarantees that both types match.

      @code
      if ( i1.type() == i2.type() ) {
        i1 = i2;
      } else {
        qDebug() << "Invalid assignment!";
      }
      @endcode

      Dirty field FieldUnknown will be set.

      @param other is the IncidenceBase to assign.
     */
    IncidenceBase &operator=(const IncidenceBase &other);

    /**
      Compares this with IncidenceBase @p ib for equality.
      All data belonging to derived classes are also compared. @see equals().
      @param ib is the IncidenceBase to compare against.
      @return true if the incidences are equal; false otherwise.
    */
    bool operator==(const IncidenceBase &ib) const;

    /**
      Compares this with IncidenceBase @p ib for inequality.
      @param ib is the IncidenceBase to compare against.
      @return true if the incidences are /not/ equal; false otherwise.
    */
    bool operator!=(const IncidenceBase &ib) const;

    /**
     Accept IncidenceVisitor. A class taking part in the visitor mechanism
     has to provide this implementation:
     <pre>
       bool accept(Visitor &v) { return v.visit(this); }
     </pre>

     @param v is a reference to a Visitor object.
     @param incidence is a valid IncidenceBase object for visiting.
    */
    virtual bool accept(Visitor &v, const IncidenceBase::Ptr &incidence);

    /**
      Returns the incidence type.
    */
    virtual IncidenceType type() const = 0;

    /**
      Prints the type of incidence as a string.
      将事件类型打印为字符串。
    */
    virtual QByteArray typeStr() const = 0;

    /**
      Sets the unique id for the incidence to @p uid.
      将事件的唯一 id 设置为 uid。
      @param uid is the string containing the incidence @ref uid.
      @see uid()
    */
    void setUid(const QString &uid);

    /**
      Returns the unique id (@ref uid) for the incidence.
      @see setUid()
    */
    Q_REQUIRED_RESULT QString uid() const;

    /**
      Returns the uri for the incidence, of form urn:x-ical:\<uid\>
      返回事件的uri，
    */
    Q_REQUIRED_RESULT QUrl uri() const;

    /**
      Sets the time the incidence was last modified to @p lm.
      It is stored as a UTC date/time.

      @param lm is the QDateTime when the incidence was last modified.

      @see lastModified()
    */
    virtual void setLastModified(const QDateTime &lm);

    /**
      Returns the time the incidence was last modified.
      返回上次修改事件的时间。
      @see setLastModified()
    */
    Q_REQUIRED_RESULT QDateTime lastModified() const;

    /**
      Sets the organizer for the incidence.
      设置事件的组织者。

      @param organizer is a non-null Person to use as the incidence @ref organizer.
      @see organizer(), setOrganizer(const QString &)
    */
    void setOrganizer(const Person &organizer);

    /**
      Sets the incidence organizer to any string @p organizer.

      @param organizer is a string to use as the incidence @ref organizer.
      @see organizer(), setOrganizer(const Person &)
    */
    void setOrganizer(const QString &organizer);

    /**
      Returns the Person associated with this incidence.
      If no Person was set through setOrganizer(), a default Person()
      is returned.
      @see setOrganizer(const QString &), setOrganizer(const Person &)
    */
    Person organizer() const;

    /**
      Sets readonly status.
      设置只读状态。

      @param readOnly if set, the incidence is read-only; else the incidence
      can be modified.
      @see isReadOnly().
    */
    virtual void setReadOnly(bool readOnly);

    /**
      Returns true the object is read-only; false otherwise.
      @see setReadOnly()
    */
    Q_REQUIRED_RESULT bool isReadOnly() const;

    /**
      Sets the incidence's starting date/time with a QDateTime.

      @param dtStart is the incidence start date/time.
      @see dtStart().
    */
    virtual void setDtStart(const QDateTime &dtStart);

    /**
      Returns an incidence's starting date/time as a QDateTime.
      @see setDtStart().
    */
    virtual QDateTime dtStart() const;

    /**
      Sets the incidence duration.

      @param duration the incidence duration

      @see duration()
    */
    virtual void setDuration(const Duration &duration);

    /**
      Returns the length of the incidence duration.
      @see setDuration()
    */
    Q_REQUIRED_RESULT Duration duration() const;

    /**
      Sets if the incidence has a duration.
      @param hasDuration true if the incidence has a duration; false otherwise.
      @see hasDuration()
    */
    void setHasDuration(bool hasDuration);

    /**
      Returns true if the incidence has a duration; false otherwise.
      @see setHasDuration()
    */
    Q_REQUIRED_RESULT bool hasDuration() const;

    /**
      Returns true or false depending on whether the incidence is all-day.
      i.e. has a date but no time attached to it.
      @see setAllDay()
    */
    Q_REQUIRED_RESULT bool allDay() const;

    /**
      Sets whether the incidence is all-day, i.e. has a date but no time
      attached to it.

      @param allDay sets whether the incidence is all-day.

      @see allDay()
    */
    virtual void setAllDay(bool allDay);

    /**
      Shift the times of the incidence so that they appear at the same clock
      time as before but in a new time zone. The shift is done from a viewing
      time zone rather than from the actual incidence time zone.

      For example, shifting an incidence whose start time is 09:00
      America/New York, using an old viewing time zone (@p oldSpec)
      of Europe/London, to a new time zone (@p newSpec) of Europe/Paris,
      will result in the time being shifted from 14:00 (which is the London
      time of the incidence start) to 14:00 Paris time.

      @param oldZone the time zone which provides the clock times
      @param newZone the new time zone
    */
    virtual void shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone);

    /**
      Adds a comment to the incidence. Does not add a linefeed character; simply
      appends the text as specified.
      向事件添加注释。不添加换行字符; 仅按指定附加文本。

      @param comment is the QString containing the comment to add.
      @see removeComment().
    */
    void addComment(const QString &comment);

    /**
      Removes a comment from the incidence. Removes the first comment whose
      string is an exact match for the specified string in @p comment.

      @param comment is the QString containing the comment to remove.
      @return true if match found, false otherwise.
      @see addComment().
     */
    Q_REQUIRED_RESULT bool removeComment(const QString &comment);

    /**
      Deletes all incidence comments.
    */
    void clearComments();

    /**
      Returns all incidence comments as a list of strings.
      将所有关联注释作为字符串列表返回。
    */
    Q_REQUIRED_RESULT QStringList comments() const;

    /**
      Adds a contact to thieincidence. Does not add a linefeed character; simply
      appends the text as specified.

      @param contact is the QString containing the contact to add.
      @see removeContact().
    */
    void addContact(const QString &contact);

    /**
      Removes a contact from the incidence. Removes the first contact whose
      string is an exact match for the specified string in @p contact.

      @param contact is the QString containing the contact to remove.
      @return true if match found, false otherwise.
      @see addContact().
     */
    Q_REQUIRED_RESULT bool removeContact(const QString &contact);

    /**
      Deletes all incidence contacts.
    */
    void clearContacts();

    /**
      Returns all incidence contacts as a list of strings.
    */
    Q_REQUIRED_RESULT QStringList contacts() const;

    /**
      Add Attendee to this incidence.

      @param attendee the attendee to add
      @param doUpdate If true the Observers are notified, if false they are not.
    */
    void addAttendee(const Attendee &attendee, bool doUpdate = true);

    /**
      Removes all attendees from the incidence.
    */
    void clearAttendees();

    /**
       Set the attendees of this incidence.
       This replaces all previously set attendees, unlike addAttendee.

       @param attendees A list of attendees.
       @param doUpdate If true the Observers are notified, if false they are not.
    */
    void setAttendees(const Attendee::List &attendees, bool doUpdate = true);

    /**
      Returns a list of incidence attendees.
      All pointers in the list are valid.
    */
    Q_REQUIRED_RESULT Attendee::List attendees() const;

    /**
      Returns the number of incidence attendees.
    */
    Q_REQUIRED_RESULT int attendeeCount() const;

    /**
      Returns the attendee with the specified email address.

      @param email is a QString containing an email address of the
      form "FirstName LastName <emailaddress>".
      @see attendeeByMails(), attendeesByUid().
    */
    Attendee attendeeByMail(const QString &email) const;

    /**
      Returns the first incidence attendee with one of the specified
      email addresses.

      @param emails is a list of QStrings containing email addresses of the
      form "FirstName LastName <emailaddress>".
      @param email is a QString containing a single email address to search
      in addition to the list specified in @p emails.
      @see attendeeByMail(), attendeesByUid().
    */
    Attendee attendeeByMails(const QStringList &emails, const QString &email = QString()) const;

    /**
      Returns the incidence attendee with the specified attendee @acronym UID.

      @param uid is a QString containing an attendee @acronym UID.
      @see attendeeByMail(), attendeeByMails().
    */
    Attendee attendeeByUid(const QString &uid) const;

    /**
      Sets the incidences url.

      This property can be used to point to a more dynamic rendition of the incidence.
      I.e. a website related to the incidence.

      @param url of the incience.
      @see url()
      @since 4.12
    */
    void setUrl(const QUrl &url);

    /**
      Returns the url.
      @return incidences url value
      @see setUrl()
      @since 4.12
    */
    Q_REQUIRED_RESULT QUrl url() const;

    /**
      Register observer. The observer is notified when the observed object
      changes.

      @param observer is a pointer to an IncidenceObserver object that will be
      watching this incidence.
      @see unRegisterObserver()
    */
    void registerObserver(IncidenceObserver *observer);

    /**
      Unregister observer. It isn't notified anymore about changes.

      @param observer is a pointer to an IncidenceObserver object that will be
      watching this incidence.
      @see registerObserver().
    */
    void unRegisterObserver(IncidenceObserver *observer);

    /**
      Call this to notify the observers after the IncidenceBase object will be
      changed.
      调用此选项可在IncidenceBase对象发生更改后通知观察者。
    */
    void update();

    /**
      Call this to notify the observers after the IncidenceBase object has
      changed.
    */
    void updated();

    /**
      Call this when a group of updates is going to be made. This suppresses
      change notifications until endUpdates() is called, at which point
      updated() will automatically be called.
    */
    void startUpdates();

    /**
      Call this when a group of updates is complete, to notify observers that
      the instance has changed. This should be called in conjunction with
      startUpdates().
    */
    void endUpdates();

    /**
      Returns a date/time corresponding to the specified DateTimeRole.
      @param role is a DateTimeRole.
    */
    virtual QDateTime dateTime(DateTimeRole role) const = 0;

    /**
      Sets the date/time corresponding to the specified DateTimeRole.
      @param dateTime is QDateTime value to set.
      @param role is a DateTimeRole.
    */
    virtual void setDateTime(const QDateTime &dateTime, DateTimeRole role) = 0;

    /**
      Returns the Akonadi specific sub MIME type of a KCalendarCore::IncidenceBase item,
      e.g. getting "application/x-vnd.akonadi.calendar.event" for a KCalendarCore::Event.
    */
    virtual QLatin1String mimeType() const = 0;

    /**
      Returns the incidence recurrenceId.
      @return incidences recurrenceId value
      @see setRecurrenceId().
    */
    virtual QDateTime recurrenceId() const;

    /**
       Returns a QSet with all Fields that were changed since the incidence was created
       or resetDirtyFields() was called.

       @see resetDirtyFields()
    */
    QSet<IncidenceBase::Field> dirtyFields() const;

    /**
       Sets which fields are dirty.
       @see dirtyFields()
       @since 4.8
     */
    void setDirtyFields(const QSet<IncidenceBase::Field> &);

    /**
       Resets dirty fields.
       @see dirtyFields()
    */
    void resetDirtyFields();

    /**
     * Constant that identifies KCalendarCore data in a binary stream.
     *
     * @since 4.12
     */
    Q_REQUIRED_RESULT static quint32 magicSerializationIdentifier();

protected:
    /**
       Marks Field @p field as dirty.
       @param field is the Field type to mark as dirty.
       @see dirtyFields()
    */
    void setFieldDirty(IncidenceBase::Field field);

    /**
      @copydoc
      CustomProperties::customPropertyUpdate()
    */
    void customPropertyUpdate() override;

    /**
      @copydoc
      CustomProperties::customPropertyUpdated()
    */
    void customPropertyUpdated() override;

    /**
      Constructs an IncidenceBase as a copy of another IncidenceBase object.
      @param ib is the IncidenceBase to copy.
    */
    IncidenceBase(const IncidenceBase &ib);

    /**
      Provides polymorfic comparison for equality.
      Only called by IncidenceBase::operator==() which guarantees that
      @p incidenceBase is of the right type.
      @param incidenceBase is the IncidenceBase to compare against.
      @return true if the incidences are equal; false otherwise.
    */
    virtual bool equals(const IncidenceBase &incidenceBase) const;

    /**
      Provides polymorfic assignment.
      @param other is the IncidenceBase to assign.
    */
    virtual IncidenceBase &assign(const IncidenceBase &other);

    /**
     * Sub-type specific serialization.
     */
    virtual void serialize(QDataStream &out) const;
    /**
     * Sub-type specific deserialization.
     */
    virtual void deserialize(QDataStream &in);

    enum VirtualHook {};

    /**
      Standard trick to add virtuals later.

      @param id is any integer unique to this class which we will use to identify the method
             to be called.
      @param data is a pointer to some glob of data, typically a struct.
    */
    virtual void virtual_hook(VirtualHook id, void *data) = 0;

    /**
      Identifies a read-only incidence.
    */
    bool mReadOnly;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_DECL_HIDDEN QVariantList attendeesVariant() const;
    //@endcond

    friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const KCalendarCore::IncidenceBase::Ptr &);

    friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, KCalendarCore::IncidenceBase::Ptr &);
};

/**
 * Incidence serializer.
 *
 * @since 4.12
 */
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &out, const KCalendarCore::IncidenceBase::Ptr &);

/**
 * Incidence deserializer.
 *
 * @since 4.12
 */
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &in, KCalendarCore::IncidenceBase::Ptr &);
} // namespace KCalendarCore

Q_DECLARE_METATYPE(KCalendarCore::IncidenceBase *)
Q_DECLARE_METATYPE(KCalendarCore::IncidenceBase::Ptr)

#endif
