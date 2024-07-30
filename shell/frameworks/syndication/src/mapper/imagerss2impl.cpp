/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imagerss2impl.h"

#include <QString>

namespace Syndication
{
ImageRSS2Impl::ImageRSS2Impl(const Syndication::RSS2::Image &image)
    : m_image(image)
{
}

bool ImageRSS2Impl::isNull() const
{
    return m_image.isNull();
}

QString ImageRSS2Impl::url() const
{
    return m_image.url();
}

QString ImageRSS2Impl::title() const
{
    return m_image.title();
}

QString ImageRSS2Impl::link() const
{
    return m_image.link();
}

QString ImageRSS2Impl::description() const
{
    return m_image.description();
}

uint ImageRSS2Impl::height() const
{
    return m_image.height();
}

uint ImageRSS2Impl::width() const
{
    return m_image.width();
}

} // namespace Syndication
