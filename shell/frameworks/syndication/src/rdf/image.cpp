/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "image.h"
#include "rssvocab.h"
#include "statement.h"

namespace Syndication
{
namespace RDF
{
Image::Image()
    : ResourceWrapper()
{
}

Image::Image(ResourcePtr resource)
    : ResourceWrapper(resource)
{
}

Image::~Image()
{
}

QString Image::title() const
{
    return resource()->property(RSSVocab::self()->title())->asString();
}

QString Image::link() const
{
    return resource()->property(RSSVocab::self()->link())->asString();
}

QString Image::url() const
{
    return resource()->property(RSSVocab::self()->url())->asString();
}

QString Image::debugInfo() const
{
    QString info = QLatin1String("### Image: ###################\n");
    info += QLatin1String("url: #") + url() + QLatin1String("#\n");
    info += QLatin1String("title: #") + title() + QLatin1String("#\n");
    info += QLatin1String("link: #") + link() + QLatin1String("#\n");
    info += QLatin1String("### Image end ################\n");
    return info;
}

} // namespace RDF
} // namespace Syndication
