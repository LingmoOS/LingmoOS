/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Person class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#ifndef KCALCORE_PERSON_H
#define KCALCORE_PERSON_H

#include <QHash>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>

namespace KCalendarCore {
/**
  @brief
  Represents a person, by name and email address.

  This class represents a person, with a name and an email address.
  It supports the "FirstName LastName\ <mail@domain\>" format.
*/
class Q_CORE_EXPORT Person
{
    Q_GADGET
    Q_PROPERTY(bool isEmpty READ isEmpty)
    Q_PROPERTY(QString fullName READ fullName)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString email READ email WRITE setEmail)

public:
    /**
      List of persons.
    */
    typedef QVector<Person> List;

    /**
      Constructs a blank person.
    */
    Person();

    /**
      Constructs a person with name and email address taken from @p fullName.

      @param fullName is the name and email of the person in the form
        "FirstName LastName \<mail@domain\>".
      @return A Person object.
    */
    static Person fromFullName(const QString &fullName);

    /**
      Constructs a person with the name @p name and email address @p email.

      @param name is the name of this person.
      @param email is the email address of this person.
    */
    Person(const QString &name, const QString &email);

    /**
       Constructs a person as a copy of another person object.
       @param person is the person to copy.
     */
    Person(const Person &person);

    /**
      Destroys a person.
    */
    virtual ~Person();

    /**
      Returns true if the person name and email address are empty.
    */
    Q_REQUIRED_RESULT bool isEmpty() const;

    /**
      Returns the full name of this person.
      @return A QString containing the person's full name in the form
        "FirstName LastName \<mail@domain\>".
    */
    Q_REQUIRED_RESULT QString fullName() const;

    /**
      Sets the name of the person to @p name.

      @param name is the name of this person.

      @see name()
    */
    void setName(const QString &name);

    /**
      Returns the person name string.

      @see setName()
    */
    Q_REQUIRED_RESULT QString name() const;

    /**
      Sets the email address for this person to @p email.

      @param email is the email address for this person.

      @see email()
    */
    void setEmail(const QString &email);

    /**
      Returns the email address for this person.
      @return A QString containing the person's email address.
      @see setEmail()
    */
    Q_REQUIRED_RESULT QString email() const;

    /**
      Returns true if person's email address is valid.
      Simple email validity check, test that there:
      * is at least one @
      * is at least one character in the local part
      * is at least one dot in the domain part
      * is at least four characters in the domain (assuming that no-one has an address at the tld,
                                                   that the tld is at least 2 chars)

      @param email is the email address to validate
    */
    Q_REQUIRED_RESULT static bool isValidEmail(const QString &email);

    /**
      Compares this with @p person for equality.

      @param person is the person to compare.
    */
    bool operator==(const Person &person) const;

    /**
      Compares this with @p person for non-equality.

      @param person is the person to compare.
    */
    bool operator!=(const Person &person) const;

    /**
      Sets this person equal to @p person.

      @param person is the person to copy.
    */
    Person &operator=(const Person &person);

private:
    //@cond PRIVATE
    class Private;
    QSharedDataPointer<Private> d;
    //@endcond

    friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &s, const KCalendarCore::Person &person);
    friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &s, KCalendarCore::Person &person);
};

/**
  Serializes the @p person object into the @p stream.
*/
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const KCalendarCore::Person &person);

/**
  Initializes the @p person object from the @p stream.
*/
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, KCalendarCore::Person &person);

/**
  Return a hash value for a Person argument.
  @param key is a Person.
*/
Q_CORE_EXPORT uint qHash(const KCalendarCore::Person &key);

} // namespace KCalendarCore

//@cond PRIVATE
Q_DECLARE_TYPEINFO(KCalendarCore::Person, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KCalendarCore::Person)
//@endcond

#endif
