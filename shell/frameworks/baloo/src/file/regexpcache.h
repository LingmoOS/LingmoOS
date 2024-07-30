/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef REGEXP_CACHE_H_
#define REGEXP_CACHE_H_

#include <QList>
#include <QSet>
#include <QRegularExpression>

class RegExpCache
{
public:
    RegExpCache();
    ~RegExpCache();

    bool exactMatch(const QString& s) const;

    void rebuildCacheFromFilterList(const QStringList& filters);

private:
    QList<QRegularExpression> m_regexpCache;
    QSet<QString> m_exactMatches;
};

#endif
