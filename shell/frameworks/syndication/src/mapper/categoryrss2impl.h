/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_CATEGORYRSS2IMPL_H
#define SYNDICATION_MAPPER_CATEGORYRSS2IMPL_H

#include <category.h>
#include <rss2/category.h>

namespace Syndication
{
class CategoryRSS2Impl;
typedef QSharedPointer<CategoryRSS2Impl> CategoryRSS2ImplPtr;

/**
 *
 * @internal
 * @author Frank Osterfeld
 */
class CategoryRSS2Impl : public Syndication::Category
{
public:
    explicit CategoryRSS2Impl(const Syndication::RSS2::Category &category);

    Q_REQUIRED_RESULT bool isNull() const override;

    Q_REQUIRED_RESULT QString term() const override;

    Q_REQUIRED_RESULT QString label() const override;

    Q_REQUIRED_RESULT QString scheme() const override;

private:
    Syndication::RSS2::Category m_category;
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_CATEGORYRSS2IMPL_H
