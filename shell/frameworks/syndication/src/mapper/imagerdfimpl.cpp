/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imagerdfimpl.h"

namespace Syndication
{
ImageRDFImpl::ImageRDFImpl(const Syndication::RDF::Image &image)
    : m_image(image)
{
}

bool ImageRDFImpl::isNull() const
{
    return m_image.isNull();
}

QString ImageRDFImpl::url() const
{
    return m_image.url();
}

QString ImageRDFImpl::title() const
{
    return m_image.title();
}

QString ImageRDFImpl::link() const
{
    return m_image.link();
}

QString ImageRDFImpl::description() const
{
    return QString();
}

uint ImageRDFImpl::height() const
{
    return 0;
}

uint ImageRDFImpl::width() const
{
    return 0;
}

} // namespace Syndication
