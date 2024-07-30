/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_PHONENUMBER_H
#define KCONTACTS_PHONENUMBER_H

#include "kcontacts_export.h"

#include <QList>
#include <QMap>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>

namespace KContacts
{
class ParameterMap;

/**
 * @short Phonenumber information.
 *
 * This class provides phone number information. A phone number is classified by
 * a type. The following types are available, it's possible to use multiple types
 * Types for a number by combining them through a logical or.
 */
class KCONTACTS_EXPORT PhoneNumber
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const PhoneNumber &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, PhoneNumber &);
    friend class VCardTool;

    Q_GADGET
    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(QString number READ number WRITE setNumber)
    Q_PROPERTY(QString normalizedNumber READ normalizedNumber)
    Q_PROPERTY(Type type READ type WRITE setType)
    Q_PROPERTY(QString typeLabel READ typeLabel)
    Q_PROPERTY(bool isEmpty READ isEmpty)
    Q_PROPERTY(bool isPreferred READ isPreferred)
    Q_PROPERTY(bool supportsSms READ supportsSms)

public:
    /**
      Phone number types.
      @see Type
    */
    enum TypeFlag {
        Home = 1, /**< Home number */
        Work = 2, /**< Office number */
        Msg = 4, /**< Messaging */
        Pref = 8, /**< Preferred number */
        Voice = 16, /**< Voice */
        Fax = 32, /**< Fax machine */
        Cell = 64, /**< Cell phone */
        Video = 128, /**< Video phone */
        Bbs = 256, /**< Mailbox */
        Modem = 512, /**< Modem */
        Car = 1024, /**< Car phone */
        Isdn = 2048, /**< ISDN connection */
        Pcs = 4096, /**< Personal Communication Service*/
        Pager = 8192, /**< Pager */
        // TODO add Text and textphone support vcard4
        Undefined = 16384, /** Undefined number type */
    };

    /**
     * Stores a combination of #TypeFlag values.
     */
    Q_DECLARE_FLAGS(Type, TypeFlag)
    Q_FLAG(Type)

    /**
     * List of phone number types.
     */
    typedef QList<TypeFlag> TypeList;

    /**
     * List of phone numbers.
     */
    typedef QList<PhoneNumber> List;

    /**
     * Creates an empty phone number object.
     */
    PhoneNumber();

    /**
     * Creates a phone number object.
     *
     * @param number Number
     * @param type   Type as defined in enum. Multiple types can be
     *               specified by combining them by a logical or.
     */
    PhoneNumber(const QString &number, Type type = Home); // krazy:exclude=explicit

    /**
     * Copy constructor.
     *
     * Fast operation, PhoneNumber's data is implicitly shared.
     *
     * @param other The PhoneNumber object to copy from
     */
    PhoneNumber(const PhoneNumber &other);

    /**
     * Destroys the phone number.
     */
    ~PhoneNumber();

    /**
     * Equality operator.
     *
     * @return @c true if number, type and identifier are equal,
     *         otherwise @c false
     */
    Q_REQUIRED_RESULT bool operator==(const PhoneNumber &other) const;

    /**
     * Not-Equal operator.
     */
    Q_REQUIRED_RESULT bool operator!=(const PhoneNumber &other) const;

    /**
     * Assignment operator.
     *
     * Fast operation, PhoneNumber's data is implicitly shared.
     *
     * @param other The PhoneNumber object to asssign to @c this
     */
    PhoneNumber &operator=(const PhoneNumber &other);

    /**
     * Returns true, if the phone number is empty.
     */
    Q_REQUIRED_RESULT bool isEmpty() const;

    /**
     * Sets the unique @p identifier.
     */
    void setId(const QString &identifier);

    /**
     * Returns the unique identifier.
     */
    Q_REQUIRED_RESULT QString id() const;

    /**
     * Sets the phone @p number.
     */
    void setNumber(const QString &number);

    /**
     * Returns the phone number.
     * This is the number as entered/stored with all formatting preserved. Preferred for display.
     * @see normalizedNumber()
     */
    Q_REQUIRED_RESULT QString number() const;

    /**
     * Returns the phone number normalized for dialing.
     * This has all formatting stripped for passing to dialers or tel: URLs.
     * @see number()
     * @since 5.12
     */
    Q_REQUIRED_RESULT QString normalizedNumber() const;

    /**
     * Sets the @p type.
     * Multiple types can be specified by combining them by a logical or.
     *
     * @param type The #Type of the phone number
     */
    void setType(Type type);

    /**
     * Returns the type. Can be a multiple types combined by a logical or.
     *
     * @see #TypeFlag
     * @see typeLabel()
     */
    Q_REQUIRED_RESULT Type type() const;

    /**
     * Returns a translated string of the address' type.
     */
    Q_REQUIRED_RESULT QString typeLabel() const;

    /**
     * Returns a list of all available types
     */
    Q_REQUIRED_RESULT static TypeList typeList();

    /**
     * Returns whether this phone number is marked as preferred.
     * @since 5.12
     */
    Q_REQUIRED_RESULT bool isPreferred() const;
    /**
     * Returns whether this phone number is expected to support receiving SMS messages.
     * @since 5.12
     */
    Q_REQUIRED_RESULT bool supportsSms() const;

    /**
     * Returns the translated label for phone number @p type.
     *
     * In opposite to typeFlagLabel( TypeFlag type ), it returns all types
     * of the phone number concatenated by '/'.
     *
     * @param type An OR'ed combination of #TypeFlag
     *
     * @see type()
     */
    static QString typeLabel(Type type);

    /**
     * Returns the translated label for phone number @p type.
     *
     * @param type An OR'ed combination of #TypeFlag
     *
     * @see typeLabel()
     * @since 4.5
     */
    Q_REQUIRED_RESULT static QString typeFlagLabel(TypeFlag type);

    /**
     * Returns a string representation of the phone number.
     */
    QString toString() const;

private:
    KCONTACTS_NO_EXPORT void setParams(const ParameterMap &params);
    Q_REQUIRED_RESULT KCONTACTS_NO_EXPORT ParameterMap params() const;

    class Private;
    QSharedDataPointer<Private> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PhoneNumber::Type)

/**
 * Serializes the phone @p number object into the @p stream.
 *
 * @param stream The stream to write into
 * @param number The phone number object to serialize
 */
KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const PhoneNumber &number);

/**
 * Initializes the phone @p number object from the @p stream.
 *
 * @param stream The stream to read from
 * @param number The phone number object to deserialize into
 */
KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, PhoneNumber &number);
}

Q_DECLARE_TYPEINFO(KContacts::PhoneNumber, Q_RELOCATABLE_TYPE);

#endif
