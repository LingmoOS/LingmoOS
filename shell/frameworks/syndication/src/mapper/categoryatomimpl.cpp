/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "categoryatomimpl.h"

#include <QString>

namespace Syndication
{
CategoryAtomImpl::CategoryAtomImpl(const Syndication::Atom::Category &category)
    : m_category(category)
{
}

bool CategoryAtomImpl::isNull() const
{
    return m_category.isNull();
}

QString CategoryAtomImpl::term() const
{
    return m_category.term();
}

QString CategoryAtomImpl::scheme() const
{
    return m_category.scheme();
}

QString CategoryAtomImpl::label() const
{
    return m_category.label();
}

} // namespace Syndication
