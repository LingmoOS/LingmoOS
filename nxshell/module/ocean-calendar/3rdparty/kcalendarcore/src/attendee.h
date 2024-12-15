/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Attendee class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#ifndef KCALCORE_ATTENDEE_H
#define KCALCORE_ATTENDEE_H

#include <QMetaType>
#include <QSharedDataPointer>

#include "customproperties.h"

namespace KCalendarCore {
/**
  @brief
  Represents information related to an attendee of an Calendar Incidence,
  typically a meeting or task (to-do).

  Attendees are people with a name and (optional) email address who are
  invited to participate in some way in a meeting or task.  This class
  also tracks that status of the invitation: accepted; tentatively accepted;
  declined; delegated to another person; in-progress; completed.

  Attendees may optionally be asked to @acronym RSVP ("Respond Please") to
  the invitation.

  Note that each attendee be can optionally associated with a @acronym UID
  (unique identifier) derived from a Calendar Incidence, Email Message,
  or any other thing you want.
*/
class Q_CORE_EXPORT Attendee
{
    Q_GADGET
    Q_PROPERTY(bool isNull READ isNull)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString fullName READ fullName)
    Q_PROPERTY(QString email READ email WRITE setEmail)
    Q_PROPERTY(Role role READ role WRITE setRole)
    Q_PROPERTY(QString uid READ uid WRITE setUid)
    Q_PROPERTY(PartStat status READ status WRITE setStatus)
    Q_PROPERTY(CuType cuType READ cuType WRITE setCuType)
    Q_PROPERTY(bool rsvp READ RSVP WRITE setRSVP)
    Q_PROPERTY(QString delegate READ delegate WRITE setDelegate)
    Q_PROPERTY(QString delegator READ delegator WRITE setDelegator)

public:
    /**
      The different types of participant status.
      The meaning is specific to the incidence type in context.
    */
    enum PartStat {
        NeedsAction, /**< Event, to-do or journal needs action (default) */
        Accepted, /**< Event, to-do or journal accepted */
        Declined, /**< Event, to-do or journal declined */
        Tentative, /**< Event or to-do tentatively accepted */
        Delegated, /**< Event or to-do delegated */
        Completed, /**< To-do completed */
        InProcess, /**< To-do in process of being completed */
        None,
    };
    Q_ENUM(PartStat)

    /**
      The different types of participation roles.
    */
    enum Role {
        ReqParticipant, /**< Participation is required (default) */
        OptParticipant, /**< Participation is optional */
        NonParticipant, /**< Non-Participant; copied for information purposes */
        Chair, /**< Chairperson */
    };
    Q_ENUM(Role)

    /**
     * The different types of a participant.
     *
     * @since 4.14
     */
    enum CuType {
        Individual, /**< An individual (default) */
        Group, /**< A group of individuals */
        Resource, /**< A physical resource */
        Room, /**< A room resource */
        Unknown, /**< Otherwise not known */
        /**
         * Parameters that have to set via the QString variant of @setCuType() and @cuType()
         * x-name         ; Experimental cuType
         * iana-token     ; Other IANA-registered
         */
    };
    Q_ENUM(CuType)

    /**
      List of attendees.
    */
    typedef QVector<Attendee> List;

    /** Create a null Attendee. */
    Attendee();

    /**
      Constructs an attendee consisting of a person name (@p name) and
      email address (@p email); invitation status and #Role;
      an optional @acronym RSVP flag and @acronym UID.

      @param name is person name of the attendee.
      @param email is person email address of the attendee.
      @param rsvp if true, the attendee is requested to reply to invitations.
      @param status is the #PartStat status of the attendee.
      @param role is the #Role of the attendee.
      @param uid is the @acronym UID of the attendee.
    */
    Attendee(const QString &name, const QString &email, bool rsvp = false, PartStat status = None, Role role = ReqParticipant, const QString &uid = QString());

    /**
      Constructs an attendee by copying another attendee.

      @param attendee is the attendee to be copied.
    */
    Attendee(const Attendee &attendee);

    /**
      Destroys the attendee.
    */
    ~Attendee();

    /**
     * Returns @c true if this is a default-constructed Attendee instance.
     */
    bool isNull() const;

    /**
      Returns the name of the attendee.
    */
    Q_REQUIRED_RESULT QString name() const;
    /**
      Sets the name of the attendee to @p name.
    */
    void setName(const QString &name);

    /**
      Returns the full name and email address of this attendee
      @return A QString containing the person's full name in the form
        "FirstName LastName \<mail@domain\>".
    */
    Q_REQUIRED_RESULT QString fullName() const;

    /**
      Returns the email address for this attendee.
    */
    Q_REQUIRED_RESULT QString email() const;
    /**
      Sets the email address for this attendee to @p email.
    */
    void setEmail(const QString &email);

    /**
      Sets the Role of the attendee to @p role.

      @param role is the Role to use for the attendee.

      @see role()
    */
    void setRole(Role role);

    /**
      Returns the Role of the attendee.

      @see setRole()
    */
    Q_REQUIRED_RESULT Role role() const;

    /**
      Sets the @acronym UID of the attendee to @p uid.

      @param uid is the @acronym UID to use for the attendee.

      @see uid()
    */
    void setUid(const QString &uid);

    /**
      Returns the @acronym UID of the attendee.

      @see setUid()
    */
    Q_REQUIRED_RESULT QString uid() const;

    /**
      Sets the #PartStat of the attendee to @p status.

      @param status is the #PartStat to use for the attendee.

      @see status()
    */
    void setStatus(PartStat status);

    /**
      Returns the #PartStat of the attendee.

      @see setStatus()
    */
    Q_REQUIRED_RESULT PartStat status() const;

    /**
      Sets the #CuType of the attendee to @p cuType.

      @param cuType is the #CuType to use for the attendee.

      @see cuType()

      @since 4.14
    */
    void setCuType(CuType cuType);

    /**
      Sets the #CuType of the attendee to @p cuType.

      @param cuType is the #CuType to use for the attendee.

      @see cuType()

      @since 4.14
    */
    void setCuType(const QString &cuType);

    /**
      Returns the #CuType of the attendee.

      @see setCuType()

      @since 4.14
    */
    Q_REQUIRED_RESULT CuType cuType() const;

    /**
      Returns the #CuType of the attendee.

      @see setCuType()

      @since 4.14
    */
    Q_REQUIRED_RESULT QString cuTypeStr() const;

    /**
      Sets the @acronym RSVP flag of the attendee to @p rsvp.

      @param rsvp if set (true), the attendee is requested to reply to
      invitations.

      @see RSVP()
    */
    void setRSVP(bool rsvp);

    /**
      Returns the attendee @acronym RSVP flag.

      @see setRSVP()
    */
    Q_REQUIRED_RESULT bool RSVP() const;

    /**
      Compares this with @p attendee for equality.

      @param attendee the attendee to compare.
    */
    bool operator==(const Attendee &attendee) const;

    /**
      Compares this with @p attendee for inequality.

      @param attendee the attendee to compare.
    */
    bool operator!=(const Attendee &attendee) const;

    /**
      Sets the delegate.
      @param delegate is a string containing a MAILTO URI of those delegated
      to attend the meeting.
      @see delegate(), setDelegator().
    */
    void setDelegate(const QString &delegate);

    /**
      Returns the delegate.
      @see setDelegate().
    */
    Q_REQUIRED_RESULT QString delegate() const;

    /**
      Sets the delegator.
      @param delegator is a string containing a MAILTO URI of those who
      have delegated their meeting attendance.
      @see delegator(), setDelegate().
    */
    void setDelegator(const QString &delegator);

    /**
      Returns the delegator.
      @see setDelegator().
    */
    Q_REQUIRED_RESULT QString delegator() const;

    /**
      Adds a custom property. If the property already exists it will be overwritten.
      @param xname is the name of the property.
      @param xvalue is its value.
    */
    void setCustomProperty(const QByteArray &xname, const QString &xvalue);

    /**
      Returns a reference to the CustomProperties object
    */
    Q_REQUIRED_RESULT CustomProperties &customProperties();

    /**
      Returns a const reference to the CustomProperties object
    */
    const CustomProperties &customProperties() const;

    /**
      Sets this attendee equal to @p attendee.

      @param attendee is the attendee to copy.
    */
    Attendee &operator=(const Attendee &attendee);

private:
    //@cond PRIVATE
    class Private;
    QSharedDataPointer<Private> d;
    //@endcond

    friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &s, const KCalendarCore::Attendee &attendee);
    friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &s, KCalendarCore::Attendee &attendee);
};

/**
  Serializes an Attendee object into a data stream.
  @param stream is a QDataStream.
  @param attendee is a pointer to a Attendee object to be serialized.
*/
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const KCalendarCore::Attendee &attendee);

/**
  Initializes an Attendee object from a data stream.
  @param stream is a QDataStream.
  @param attendee is a pointer to a Attendee object to be initialized.
*/
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, KCalendarCore::Attendee &attendee);
} // namespace KCalendarCore

//@cond PRIVATE
Q_DECLARE_TYPEINFO(KCalendarCore::Attendee, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KCalendarCore::Attendee)
//@endcond

#endif
