/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GENDER_H
#define GENDER_H
#include "kcontacts_export.h"
#include <QSharedDataPointer>
#include <QString>

namespace KContacts
{
/** @short Class that holds a Gender for a contact.
 *  @since 4.14.5
 */
class KCONTACTS_EXPORT Gender
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const Gender &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, Gender &);

public:
    /**
     * Creates an empty Gender object.
     */
    Gender();
    Gender(const Gender &other);
    Gender(const QString &gender);

    ~Gender();

    void setGender(const QString &gender);
    Q_REQUIRED_RESULT QString gender() const;

    void setComment(const QString &comment);
    Q_REQUIRED_RESULT QString comment() const;

    Q_REQUIRED_RESULT bool isValid() const;

    Q_REQUIRED_RESULT bool operator==(const Gender &other) const;
    Q_REQUIRED_RESULT bool operator!=(const Gender &other) const;

    Gender &operator=(const Gender &other);

    Q_REQUIRED_RESULT QString toString() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const Gender &object);

KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, Gender &object);
}
Q_DECLARE_TYPEINFO(KContacts::Gender, Q_RELOCATABLE_TYPE);
#endif // GENDER_H
