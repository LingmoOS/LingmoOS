/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcompletionmatches.h"

#include <kcompletion.h>
#include <kcompletion_p.h> // for KCompletionMatchesWrapper

class KCompletionMatchesPrivate
{
public:
    KCompletionMatchesPrivate(bool sort, KCompletionMatches *parent)
        : sorting(sort)
        , q_ptr(parent)
    {
    }

    bool sorting;
    KCompletionMatches *const q_ptr;

    Q_DECLARE_PUBLIC(KCompletionMatches)
};

KCompletionMatches::KCompletionMatches(const KCompletionMatches &o)
    : KSortableList<QString, int>()
    , d_ptr(new KCompletionMatchesPrivate(o.sorting(), this))
{
    *this = KCompletionMatches::operator=(o);
}

KCompletionMatches &KCompletionMatches::operator=(const KCompletionMatches &o)
{
    Q_D(KCompletionMatches);
    if (*this == o) {
        return *this;
    }
    KCompletionMatchesList::operator=(o);
    d->sorting = o.sorting();

    return *this;
}

KCompletionMatches::KCompletionMatches(bool sort_P)
    : d_ptr(new KCompletionMatchesPrivate(sort_P, this))
{
}

KCompletionMatches::KCompletionMatches(const KCompletionMatchesWrapper &matches)
    : d_ptr(new KCompletionMatchesPrivate(matches.sorting(), this))
{
    if (matches.m_sortedListPtr) {
        KCompletionMatchesList::operator=(*matches.m_sortedListPtr);
    } else {
        const QStringList list = matches.list();
        reserve(list.size());
        std::transform(list.crbegin(), list.crend(), std::back_inserter(*this), [](const QString &str) {
            return KSortableItem<QString, int>(1, str);
        });
    }
}

KCompletionMatches::~KCompletionMatches()
{
}

QStringList KCompletionMatches::list(bool sort_P) const
{
    Q_D(const KCompletionMatches);
    if (d->sorting && sort_P) {
        const_cast<KCompletionMatches *>(this)->sort();
    }
    QStringList stringList;
    stringList.reserve(size());
    // high weight == sorted last -> reverse the sorting here
    std::transform(crbegin(), crend(), std::back_inserter(stringList), [](const KSortableItem<QString> &item) {
        return item.value();
    });
    return stringList;
}

bool KCompletionMatches::sorting() const
{
    Q_D(const KCompletionMatches);
    return d->sorting;
}

void KCompletionMatches::removeDuplicates()
{
    for (auto it1 = begin(); it1 != end(); ++it1) {
        auto it2 = it1;
        ++it2;
        while (it2 != end()) {
            if ((*it1).value() == (*it2).value()) {
                // Use the max weight
                (*it1).first = std::max((*it1).key(), (*it2).key());
                it2 = erase(it2);
                continue;
            }
            ++it2;
        }
    }
}
