/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_CATEGORYATOMIMPL_H
#define SYNDICATION_MAPPER_CATEGORYATOMIMPL_H

#include <atom/category.h>
#include <category.h>

namespace Syndication
{
class CategoryAtomImpl;
typedef QSharedPointer<CategoryAtomImpl> CategoryAtomImplPtr;

/**
 *
 * @internal
 * @author Frank Osterfeld
 */
class CategoryAtomImpl : public Syndication::Category
{
public:
    explicit CategoryAtomImpl(const Syndication::Atom::Category &category);

    Q_REQUIRED_RESULT bool isNull() const override;

    Q_REQUIRED_RESULT QString term() const override;

    Q_REQUIRED_RESULT QString scheme() const override;

    Q_REQUIRED_RESULT QString label() const override;

private:
    Syndication::Atom::Category m_category;
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_CATEGORYATOMIMPL_H
