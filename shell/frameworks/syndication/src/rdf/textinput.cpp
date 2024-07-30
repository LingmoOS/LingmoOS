/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "textinput.h"
#include "property.h"
#include "resource.h"
#include "rssvocab.h"
#include "statement.h"

namespace Syndication
{
namespace RDF
{
TextInput::TextInput()
    : ResourceWrapper()
{
}

TextInput::TextInput(ResourcePtr resource)
    : ResourceWrapper(resource)
{
}

TextInput::~TextInput()
{
}

QString TextInput::title() const
{
    return resource()->property(RSSVocab::self()->title())->asString();
}

QString TextInput::description() const
{
    return resource()->property(RSSVocab::self()->description())->asString();
}

QString TextInput::link() const
{
    return resource()->property(RSSVocab::self()->link())->asString();
}

QString TextInput::name() const
{
    return resource()->property(RSSVocab::self()->name())->asString();
}

QString TextInput::debugInfo() const
{
    QString info = QLatin1String("### TextInput: ###################\n");
    info += QLatin1String("title: #") + title() + QLatin1String("#\n");
    info += QLatin1String("link: #") + link() + QLatin1String("#\n");
    info += QLatin1String("description: #") + description() + QLatin1String("#\n");
    info += QLatin1String("name: #") + name() + QLatin1String("#\n");
    info += QLatin1String("### TextInput end ################\n");
    return info;
}

} // namespace RDF
} // namespace Syndication
