/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "link.h"
#include "constants.h"

#include <QString>

namespace Syndication
{
namespace Atom
{
Link::Link()
    : ElementWrapper()
{
}

Link::Link(const QDomElement &element)
    : ElementWrapper(element)
{
}

QString Link::href() const
{
    return completeURI(attribute(QStringLiteral("href")));
}

QString Link::rel() const
{
    //"alternate" is default
    return attribute(QStringLiteral("rel"), QStringLiteral("alternate"));
}

QString Link::type() const
{
    return attribute(QStringLiteral("type"));
}

QString Link::hrefLanguage() const
{
    return attribute(QStringLiteral("hreflang"));
}

QString Link::title() const
{
    return attribute(QStringLiteral("title"));
}

uint Link::length() const
{
    QString lengthStr = attribute(QStringLiteral("length"));

    bool ok;
    uint c = lengthStr.toUInt(&ok);
    return ok ? c : 0;
}

QString Link::debugInfo() const
{
    QString info = QLatin1String("### Link: ###################\n");
    if (!title().isEmpty()) {
        info += QLatin1String("title: #") + title() + QLatin1String("#\n");
    }
    if (!href().isEmpty()) {
        info += QLatin1String("href: #") + href() + QLatin1String("#\n");
    }
    if (!rel().isEmpty()) {
        info += QLatin1String("rel: #") + rel() + QLatin1String("#\n");
    }
    if (!type().isEmpty()) {
        info += QLatin1String("type: #") + type() + QLatin1String("#\n");
    }
    if (length() != 0) {
        info += QLatin1String("length: #") + QString::number(length()) + QLatin1String("#\n");
    }
    if (!hrefLanguage().isEmpty()) {
        info += QLatin1String("hrefLanguage: #") + hrefLanguage() + QLatin1String("#\n");
    }
    info += QLatin1String("### Link end ################\n");
    return info;
}

} // namespace Atom
} // namespace Syndication
