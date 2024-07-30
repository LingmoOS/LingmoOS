/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "enclosurerss2impl.h"
#include <constants.h>

#include <QString>
#include <QStringList>

namespace Syndication
{
EnclosureRSS2Impl::EnclosureRSS2Impl(const Syndication::RSS2::Item &item, const Syndication::RSS2::Enclosure &enc)
    : m_item(item)
    , m_enclosure(enc)
{
}

bool EnclosureRSS2Impl::isNull() const
{
    return m_enclosure.isNull();
}

QString EnclosureRSS2Impl::url() const
{
    return m_enclosure.url();
}

QString EnclosureRSS2Impl::title() const
{
    // RSS2 enclosures have no title
    return QString();
}

QString EnclosureRSS2Impl::type() const
{
    return m_enclosure.type();
}

uint EnclosureRSS2Impl::length() const
{
    return m_enclosure.length();
}

uint EnclosureRSS2Impl::duration() const
{
    QString durStr = m_item.extractElementTextNS(itunesNamespace(), QStringLiteral("duration"));

    if (durStr.isEmpty()) {
        return 0;
    }

    const QStringList strTokens = durStr.split(QLatin1Char(':'));
    QList<int> intTokens;

    const int count = strTokens.count();
    bool ok;

    for (int i = 0; i < count; ++i) {
        int intVal = strTokens.at(i).toInt(&ok);
        if (ok) {
            intTokens.append(intVal >= 0 ? intVal : 0); // do not accept negative values
        } else {
            return 0;
        }
    }

    if (count == 3) {
        return intTokens.at(0) * 3600 + intTokens.at(1) * 60 + intTokens.at(2);
    } else if (count == 2) {
        return intTokens.at(0) * 60 + intTokens.at(1);
    } else if (count == 1) {
        return intTokens.at(0);
    }

    return 0;
}

} // namespace Syndication
