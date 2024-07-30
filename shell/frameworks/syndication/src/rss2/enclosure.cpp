/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "enclosure.h"

namespace Syndication
{
namespace RSS2
{
Enclosure::Enclosure()
    : ElementWrapper()
{
}

Enclosure::Enclosure(const QDomElement &element)
    : ElementWrapper(element)
{
}

QString Enclosure::url() const
{
    return attribute(QStringLiteral("url"));
}

int Enclosure::length() const
{
    int length = 0;

    if (hasAttribute(QStringLiteral("length"))) {
        bool ok;
        int c = attribute(QStringLiteral("length")).toInt(&ok);
        length = ok ? c : 0;
    }
    return length;
}

QString Enclosure::type() const
{
    return attribute(QStringLiteral("type"));
}

QString Enclosure::debugInfo() const
{
    QString info = QLatin1String("### Enclosure: ###################\n");
    if (!url().isNull()) {
        info += QLatin1String("url: #") + url() + QLatin1String("#\n");
    }
    if (!type().isNull()) {
        info += QLatin1String("type: #") + type() + QLatin1String("#\n");
    }
    if (length() != -1) {
        info += QLatin1String("length: #") + QString::number(length()) + QLatin1String("#\n");
    }
    info += QLatin1String("### Enclosure end ################\n");
    return info;
}

} // namespace RSS2
} // namespace Syndication
