/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "textinput.h"
#include "tools.h"

#include <QString>

namespace Syndication
{
namespace RSS2
{
TextInput::TextInput()
    : ElementWrapper()
{
}

TextInput::TextInput(const QDomElement &element)
    : ElementWrapper(element)
{
}

QString TextInput::title() const
{
    return extractElementTextNS(QString(), QStringLiteral("title"));
}

QString TextInput::name() const
{
    return extractElementTextNS(QString(), QStringLiteral("name"));
}

QString TextInput::description() const
{
    return extractElementTextNS(QString(), QStringLiteral("description"));
}

QString TextInput::link() const
{
    return extractElementTextNS(QString(), QStringLiteral("link"));
}

QString TextInput::debugInfo() const
{
    QString info = QLatin1String("### TextInput: ###################\n");
    if (!title().isNull()) {
        info += QLatin1String("title: #") + title() + QLatin1String("#\n");
    }
    if (!link().isNull()) {
        info += QLatin1String("link: #") + link() + QLatin1String("#\n");
    }
    if (!description().isNull()) {
        info += QLatin1String("description: #") + description() + QLatin1String("#\n");
    }
    if (!name().isNull()) {
        info += QLatin1String("name: #") + name() + QLatin1String("#\n");
    }
    info += QLatin1String("### TextInput end ################\n");
    return info;
}

} // namespace RSS2
} // namespace Syndication
