/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "category.h"
#include "constants.h"

#include <QDomElement>
#include <QString>

namespace Syndication
{
namespace Atom
{
Category::Category()
    : ElementWrapper()
{
}

Category::Category(const QDomElement &element)
    : ElementWrapper(element)
{
}

QString Category::term() const
{
    return attribute(QStringLiteral("term"));
}

QString Category::scheme() const
{
    // NOTE: The scheme IRI is not completed by purpose.
    // According to Atom spec, it must be an absolute IRI.
    // If this is a problem with real-world feeds, it might be changed.
    return attribute(QStringLiteral("scheme"));
}

QString Category::label() const
{
    return attribute(QStringLiteral("label"));
}

QString Category::debugInfo() const
{
    QString info = QLatin1String("### Category: ###################\n");
    info += QLatin1String("term: #") + term() + QLatin1String("#\n");
    if (!scheme().isEmpty()) {
        info += QLatin1String("scheme: #") + scheme() + QLatin1String("#\n");
    }
    if (!label().isEmpty()) {
        info += QLatin1String("label: #") + label() + QLatin1String("#\n");
    }
    info += QLatin1String("### Category end ################\n");

    return info;
}

} // namespace Atom
} // namespace Syndication
