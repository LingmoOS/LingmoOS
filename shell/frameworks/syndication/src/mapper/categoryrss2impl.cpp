/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "categoryrss2impl.h"

#include <QString>

namespace Syndication
{
CategoryRSS2Impl::CategoryRSS2Impl(const Syndication::RSS2::Category &category)
    : m_category(category)
{
}

bool CategoryRSS2Impl::isNull() const
{
    return m_category.isNull();
}

QString CategoryRSS2Impl::term() const
{
    return m_category.category();
}

QString CategoryRSS2Impl::scheme() const
{
    return m_category.domain();
}

QString CategoryRSS2Impl::label() const
{
    return QString();
}

} // namespace Syndication
