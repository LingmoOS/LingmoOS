/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ROLE_H
#define ROLE_H

#include "kcontacts_export.h"

#include <QMap>
#include <QSharedDataPointer>
#include <QString>

class RoleTest;

namespace KContacts
{
class ParameterMap;

/** @short Class that holds a Role for a contact.
 *  @since 5.3
 */
class KCONTACTS_EXPORT Role
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const Role &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, Role &);
    friend class VCardTool;
    friend class ::RoleTest;

public:
    Role();
    Role(const Role &other);
    Role(const QString &role);

    ~Role();

    typedef QList<Role> List;

    void setRole(const QString &role);
    Q_REQUIRED_RESULT QString role() const;

    Q_REQUIRED_RESULT bool isValid() const;

    Q_REQUIRED_RESULT bool operator==(const Role &other) const;
    Q_REQUIRED_RESULT bool operator!=(const Role &other) const;

    Role &operator=(const Role &other);

    Q_REQUIRED_RESULT QString toString() const;

private:
    // exported for RoleTest
    void setParams(const ParameterMap &params);
    Q_REQUIRED_RESULT ParameterMap params() const;

    class Private;
    QSharedDataPointer<Private> d;
};
KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const Role &object);

KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, Role &object);
}
Q_DECLARE_TYPEINFO(KContacts::Role, Q_RELOCATABLE_TYPE);
#endif // ROLE_H
