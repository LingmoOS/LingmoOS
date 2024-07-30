/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_ITEMRSS2IMPL_H
#define SYNDICATION_MAPPER_ITEMRSS2IMPL_H

#include <item.h>
#include <rss2/item.h>

namespace Syndication
{
class ItemRSS2Impl;
typedef QSharedPointer<ItemRSS2Impl> ItemRSS2ImplPtr;

/**
 * @internal
 */
class ItemRSS2Impl : public Syndication::Item
{
public:
    explicit ItemRSS2Impl(const Syndication::RSS2::Item &item);

    QString title() const override;

    QString link() const override;

    QString description() const override;

    QString content() const override;

    QList<PersonPtr> authors() const override;

    QString language() const override;

    QString id() const override;

    time_t datePublished() const override;

    time_t dateUpdated() const override;

    QList<EnclosurePtr> enclosures() const override;

    QList<CategoryPtr> categories() const override;

    SpecificItemPtr specificItem() const override;

    int commentsCount() const override;

    QString commentsLink() const override;

    QString commentsFeed() const override;

    QString commentPostUri() const override;

    QMultiMap<QString, QDomElement> additionalProperties() const override;

private:
    Syndication::RSS2::Item m_item;
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_ITEMRSS2IMPL_H
