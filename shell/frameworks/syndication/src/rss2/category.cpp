/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "category.h"
#include "tools.h"

#include <QString>

namespace Syndication
{
namespace RSS2
{
Category::Category()
    : ElementWrapper()
{
}

Category::Category(const QDomElement &element)
    : ElementWrapper(element)
{
}
QString Category::category() const
{
    return text().simplified();
}

QString Category::domain() const
{
    return attribute(QStringLiteral("domain"));
}

QString Category::debugInfo() const
{
    QString info = QLatin1String("### Category: ###################\n");
    if (!category().isNull()) {
        info += QLatin1String("category: #") + category() + QLatin1String("#\n");
    }
    if (!domain().isNull()) {
        info += QLatin1String("domain: #") + domain() + QLatin1String("#\n");
    }
    info += QLatin1String("### Category end ################\n");
    return info;
}

} // namespace RSS2
} // namespace Syndication
