/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "generator.h"
#include "constants.h"

#include <QString>

namespace Syndication
{
namespace Atom
{
Generator::Generator()
    : ElementWrapper()
{
}

Generator::Generator(const QDomElement &element)
    : ElementWrapper(element)
{
}

QString Generator::uri() const
{
    return completeURI(attribute(QStringLiteral("uri")));
}

QString Generator::name() const
{
    return text();
}

QString Generator::version() const
{
    return attribute(QStringLiteral("version"));
}

QString Generator::debugInfo() const
{
    QString info = QLatin1String("### Generator: ###################\n");
    if (!name().isEmpty()) {
        info += QLatin1String("name: #") + name() + QLatin1String("#\n");
    }
    if (!uri().isEmpty()) {
        info += QLatin1String("uri: #") + uri() + QLatin1String("#\n");
    }
    if (!version().isEmpty()) {
        info += QLatin1String("version: #") + version() + QLatin1String("#\n");
    }
    info += QLatin1String("### Generator end ################\n");
    return info;
}

} // namespace Atom
} // namespace Syndication
