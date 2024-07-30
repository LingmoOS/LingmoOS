/*
    KPeople - Duplicates
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef MATCH_H
#define MATCH_H

#include <QList>
#include <QPersistentModelIndex>
#include <qnamespace.h>

#include "backends/abstractcontact.h"
#include "kpeople/kpeople_export.h"

namespace KContacts
{
class Addressee;
}

namespace KPeople
{
class KPEOPLE_EXPORT Match
{
    Q_GADGET
public:
    enum MatchReason {
        NameMatch,
        EmailMatch,
    };
    Q_ENUM(MatchReason)

    Match()
    {
    }
    Match(const QList<MatchReason> &roles, const QPersistentModelIndex &a, const QPersistentModelIndex &b);
    bool operator==(const Match &m) const;
    bool operator<(const Match &m) const;
    QStringList matchReasons() const;
    static QString matchValue(MatchReason r, const AbstractContact::Ptr &addr);
    static QList<Match::MatchReason> matchAt(const AbstractContact::Ptr &value, const AbstractContact::Ptr &toCompare);

    QList<MatchReason> reasons;
    QPersistentModelIndex indexA, indexB;
};

}

Q_DECLARE_METATYPE(KPeople::Match)

#endif // MATCH_H
