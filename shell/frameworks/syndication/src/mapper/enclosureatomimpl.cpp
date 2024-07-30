/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "enclosureatomimpl.h"

#include <QString>

namespace Syndication
{
EnclosureAtomImpl::EnclosureAtomImpl(const Syndication::Atom::Link &link)
    : m_link(link)
{
}

bool EnclosureAtomImpl::isNull() const
{
    return m_link.isNull();
}

QString EnclosureAtomImpl::url() const
{
    return m_link.href();
}

QString EnclosureAtomImpl::title() const
{
    return m_link.title();
}

QString EnclosureAtomImpl::type() const
{
    return m_link.type();
}

uint EnclosureAtomImpl::length() const
{
    return m_link.length();
}

uint EnclosureAtomImpl::duration() const
{
    return 0;
}

} // namespace Syndication
