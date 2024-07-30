/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EMAIL_H
#define EMAIL_H

#include "kcontacts_export.h"

#include <QMap>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>

class EmailTest;

namespace KContacts
{
class ParameterMap;

/** @short Class that holds a Email for a contact.
 *  @since 4.14.5
 */
class KCONTACTS_EXPORT Email
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const Email &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, Email &);
    friend class VCardTool;
    friend class ::EmailTest;

    Q_GADGET
    Q_PROPERTY(QString email READ mail WRITE setEmail)
    Q_PROPERTY(bool isValid READ isValid)
    Q_PROPERTY(Type type READ type WRITE setType)
    Q_PROPERTY(bool isPreferred READ isPreferred WRITE setPreferred)

public:
    /**
     * Creates an empty email object.
     */
    Email();
    Email(const Email &other);
    Q_INVOKABLE Email(const QString &mail);

    ~Email();

    typedef QList<Email> List;

    /**
     * Email types.
     * @see Type
     */
    enum TypeFlag {
        Unknown = 0, /**< No or unknown email type is set. */
        Home = 1, /**< Personal email. */
        Work = 2, /**< Work email. */
        Other = 4, /**< Other email. */
    };

    /**
     * Stores a combination of #TypeFlag values.
     */
    Q_DECLARE_FLAGS(Type, TypeFlag)
    Q_FLAG(Type)

    void setEmail(const QString &mail);
    Q_REQUIRED_RESULT QString mail() const;

    Q_REQUIRED_RESULT bool isValid() const;

    /**
     * Returns the type of the email.
     * @since 5.12
     */
    Type type() const;
    /**
     * Sets the email type.
     * @since 5.12
     */
    void setType(Type type);

    /**
     * Returns whether this is the preferred email address.
     * @since 5.12
     */
    bool isPreferred() const;
    /**
     * Sets that this is the preferred email address.
     * @since 5.12
     */
    void setPreferred(bool preferred);

    Q_REQUIRED_RESULT bool operator==(const Email &other) const;
    Q_REQUIRED_RESULT bool operator!=(const Email &other) const;

    Email &operator=(const Email &other);

    Q_REQUIRED_RESULT QString toString() const;

private:
    void setParams(const ParameterMap &params);
    Q_REQUIRED_RESULT ParameterMap params() const;

    class Private;
    QSharedDataPointer<Private> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Email::Type)

KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const Email &object);

KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, Email &object);
}

Q_DECLARE_TYPEINFO(KContacts::Email, Q_RELOCATABLE_TYPE);

#endif // EMAIL_H
