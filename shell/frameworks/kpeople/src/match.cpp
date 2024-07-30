/*
    KPeople - Duplicates
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "match_p.h"
#include <KLocalizedString>

using namespace KPeople;

Match::Match(const QList<MatchReason> &reasons, const QPersistentModelIndex &a, const QPersistentModelIndex &b)
    : reasons(reasons)
    , indexA(a)
    , indexB(b)
{
    if (indexB < indexA) {
        qSwap(indexA, indexB);
    }
}

bool Match::operator==(const Match &m) const
{
    /* clang-format off */
    return reasons == m.reasons
        && indexA == m.indexA
        && indexB == m.indexB;
    /* clang-format on */
}

bool Match::operator<(const Match &m) const
{
    /* clang-format off */
    return indexA < m.indexA
        || (indexA == m.indexA && indexB < m.indexB);
    /* clang-format on */
}

QStringList Match::matchReasons() const
{
    QStringList ret;
    for (MatchReason r : reasons) {
        switch (r) {
        case NameMatch:
            ret += i18nc("@title:column", "Name");
            break;
        case EmailMatch:
            ret += i18nc("@title:column", "E-mail");
            break;
        }
    }
    return ret;
}

QString Match::matchValue(MatchReason r, const AbstractContact::Ptr &addr)
{
    switch (r) {
    case NameMatch:
        return addr->customProperty(AbstractContact::NameProperty).toString();
    case EmailMatch:
        return addr->customProperty(AbstractContact::EmailProperty).toString();
    }
    Q_UNREACHABLE();
}

QList<Match::MatchReason> Match::matchAt(const AbstractContact::Ptr &value, const AbstractContact::Ptr &toCompare)
{
    QList<Match::MatchReason> ret;

    QVariant name = value->customProperty(AbstractContact::NameProperty);
    if (name.isValid() && name == toCompare->customProperty(AbstractContact::NameProperty)) {
        ret.append(Match::NameMatch);
    }

    return ret;
}

#include "moc_match_p.cpp"
