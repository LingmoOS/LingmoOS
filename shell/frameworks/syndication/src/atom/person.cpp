/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "person.h"
#include "constants.h"

#include <QString>

namespace Syndication
{
namespace Atom
{
Person::Person()
    : ElementWrapper()
{
}

Person::Person(const QDomElement &element)
    : ElementWrapper(element)
{
}

QString Person::name() const
{
    return extractElementTextNS(atom1Namespace(), QStringLiteral("name"));
}

QString Person::uri() const
{
    return completeURI(extractElementTextNS(atom1Namespace(), QStringLiteral("uri")));
}

QString Person::email() const
{
    return extractElementTextNS(atom1Namespace(), QStringLiteral("email"));
}

QString Person::debugInfo() const
{
    QString info = QLatin1String("### Person: ###################\n");
    info += QLatin1String("name: #") + name() + QLatin1String("#\n");
    info += QLatin1String("email: #") + email() + QLatin1String("#\n");
    info += QLatin1String("uri: #") + uri() + QLatin1String("#\n");
    info += QLatin1String("### Person end ################\n");

    return info;
}

} // namespace Atom
} // namespace Syndication
