/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ORG_H
#define ORG_H

#include "kcontacts_export.h"

#include <QMap>
#include <QSharedDataPointer>
#include <QString>

class OrgTest;

namespace KContacts
{
class ParameterMap;

/** @short Class that holds a Organization for a contact.
 *  @since 5.3
 */
class KCONTACTS_EXPORT Org
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const Org &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, Org &);
    friend class VCardTool;
    friend class ::OrgTest;

public:
    Org();
    Org(const Org &other);
    Org(const QString &org);

    ~Org();

    typedef QList<Org> List;

    void setOrganization(const QString &org);
    Q_REQUIRED_RESULT QString organization() const;

    Q_REQUIRED_RESULT bool isValid() const;

    Q_REQUIRED_RESULT bool operator==(const Org &other) const;
    Q_REQUIRED_RESULT bool operator!=(const Org &other) const;

    Org &operator=(const Org &other);

    Q_REQUIRED_RESULT QString toString() const;

private:
    // exported for OrgTest
    void setParams(const ParameterMap &params);
    Q_REQUIRED_RESULT ParameterMap params() const;

    class Private;
    QSharedDataPointer<Private> d;
};
KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const Org &object);

KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, Org &object);
}
Q_DECLARE_TYPEINFO(KContacts::Org, Q_RELOCATABLE_TYPE);
#endif // ORG_H
