/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "source.h"
#include "tools.h"

#include <QString>

namespace Syndication
{
namespace RSS2
{
Source::Source()
    : ElementWrapper()
{
}

Source::Source(const QDomElement &element)
    : ElementWrapper(element)
{
}

QString Source::source() const
{
    return text();
}

QString Source::url() const
{
    return attribute(QStringLiteral("url"));
}

QString Source::debugInfo() const
{
    QString info = QLatin1String("### Source: ###################\n");
    if (!source().isNull()) {
        info += QLatin1String("source: #") + source() + QLatin1String("#\n");
    }
    if (!url().isNull()) {
        info += QLatin1String("url: #") + url() + QLatin1String("#\n");
    }
    info += QLatin1String("### Source end ################\n");
    return info;
}

} // namespace RSS2
} // namespace Syndication
