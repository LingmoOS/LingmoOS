/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "image.h"

#include <QString>

namespace Syndication
{
namespace RSS2
{
Image::Image()
    : ElementWrapper()
{
}

Image::Image(const QDomElement &element)
    : ElementWrapper(element)
{
}

QString Image::url() const
{
    return extractElementTextNS(QString(), QStringLiteral("url"));
}

QString Image::title() const
{
    return extractElementTextNS(QString(), QStringLiteral("title"));
}

QString Image::link() const
{
    return extractElementTextNS(QString(), QStringLiteral("link"));
}

uint Image::width() const
{
    QString text;
    bool ok;
    uint c;

    text = extractElementTextNS(QString(), QStringLiteral("width"));
    c = text.toUInt(&ok);
    return ok ? c : 88; // set to default if not parsable
}

uint Image::height() const
{
    QString text;
    bool ok;
    uint c;

    text = extractElementTextNS(QString(), QStringLiteral("height"));
    c = text.toUInt(&ok);
    return ok ? c : 31; // set to default if not parsable
}

QString Image::description() const
{
    return extractElementTextNS(QString(), QStringLiteral("description"));
}

QString Image::debugInfo() const
{
    QString info = QLatin1String("### Image: ###################\n");
    if (!title().isNull()) {
        info += QLatin1String("title: #") + title() + QLatin1String("#\n");
    }
    if (!link().isNull()) {
        info += QLatin1String("link: #") + link() + QLatin1String("#\n");
    }
    if (!description().isNull()) {
        info += QLatin1String("description: #") + description() + QLatin1String("#\n");
    }
    if (!url().isNull()) {
        info += QLatin1String("url: #") + url() + QLatin1String("#\n");
    }
    info += QLatin1String("width: #") + QString::number(width()) + QLatin1String("#\n");
    info += QLatin1String("height: #") + QString::number(height()) + QLatin1String("#\n");
    info += QLatin1String("### Image end ################\n");
    return info;
}

} // namespace RSS2
} // namespace Syndication
