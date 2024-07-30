/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2010 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "regexpcache.h"
#include "baloodebug.h"

#include <QStringList>

RegExpCache::RegExpCache()
{
}

RegExpCache::~RegExpCache()
{
}

bool RegExpCache::exactMatch(const QString& s) const
{
    if (m_exactMatches.contains(s)) {
        return true;
    }
    for (const QRegularExpression& filter : std::as_const(m_regexpCache)) {
        if (filter.match(s).hasMatch()) {
            return true;
        }
    }
    return false;
}

void RegExpCache::rebuildCacheFromFilterList(const QStringList& filters)
{
    m_regexpCache.clear();
    m_exactMatches.clear();

    // RE matching "*.foo" style patterns
    QRegularExpression suffixOnlyRe(QStringLiteral("^\\*\\.([^.\\*\\?]+)$"));
    QStringList suffixes;

    for (const QString& filter : filters) {
        QString f = filter;
        if (!f.contains(QLatin1Char('*')) && !f.contains(QLatin1Char('?'))) {
            m_exactMatches += f;
            continue;
        }
        auto m = suffixOnlyRe.match(f);
        if (m.hasMatch()) {
            // qCDebug(BALOO) << "filter is suffix match:" << m;
            suffixes += m.captured(1);
            continue;
        }
        f.replace(QLatin1Char('.'), QStringLiteral("\\."));
        f.replace(QLatin1Char('?'), QLatin1Char('.'));
        f.replace(QStringLiteral("*"), QStringLiteral(".*"));
        f = QLatin1String("^") + f + QLatin1String("$");

        m_regexpCache.append(QRegularExpression(f));
    }

    // Combine all suffixes into one large RE: "^.*(foo|bar|baz)$"
    QString suffixMatch = QStringLiteral("^.*\\.(")
        + suffixes.join(QLatin1Char('|'))
        + QStringLiteral(")$");
    // qCDebug(BALOO) << suffixMatch;
    m_regexpCache.prepend(QRegularExpression(suffixMatch));
}
