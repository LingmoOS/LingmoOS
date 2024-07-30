/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "personimpl.h"

namespace Syndication
{
PersonImpl::PersonImpl()
    : m_null(true)
{
}

PersonImpl::PersonImpl(const QString &name, const QString &uri, const QString &email)
    : m_null(false)
    , m_name(name)
    , m_uri(uri)
    , m_email(email)
{
}

} // namespace Syndication
