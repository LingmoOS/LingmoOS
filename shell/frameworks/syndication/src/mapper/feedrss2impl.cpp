/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "feedrss2impl.h"
#include "categoryrss2impl.h"
#include "imagerss2impl.h"
#include "itemrss2impl.h"
#include <personimpl.h>
#include <rss2/category.h>
#include <rss2/item.h>

#include <QDomElement>
#include <QList>
#include <QMultiMap>
#include <QString>

namespace Syndication
{
FeedRSS2Impl::FeedRSS2Impl(Syndication::RSS2::DocumentPtr doc)
    : m_doc(doc)
{
}

Syndication::SpecificDocumentPtr FeedRSS2Impl::specificDocument() const
{
    return m_doc;
}

QList<Syndication::ItemPtr> FeedRSS2Impl::items() const
{
    const QList<Syndication::RSS2::Item> entries = m_doc->items();

    QList<ItemPtr> items;
    items.reserve(entries.count());

    std::transform(entries.cbegin(), entries.cend(), std::back_inserter(items), [](const Syndication::RSS2::Item &entry) {
        return ItemRSS2ImplPtr(new ItemRSS2Impl(entry));
    });

    return items;
}

QList<Syndication::CategoryPtr> FeedRSS2Impl::categories() const
{
    const QList<Syndication::RSS2::Category> entries = m_doc->categories();

    QList<CategoryPtr> categories;
    categories.reserve(entries.count());

    std::transform(entries.cbegin(), entries.cend(), std::back_inserter(categories), [](const Syndication::RSS2::Category &entry) {
        return CategoryRSS2ImplPtr(new CategoryRSS2Impl(entry));
    });

    return categories;
}

QString FeedRSS2Impl::title() const
{
    return m_doc->title();
}

QString FeedRSS2Impl::link() const
{
    return m_doc->link();
}

QString FeedRSS2Impl::description() const
{
    return m_doc->description();
}

QList<PersonPtr> FeedRSS2Impl::authors() const
{
    return QList<PersonPtr>();
}

QString FeedRSS2Impl::language() const
{
    return m_doc->language();
}

QString FeedRSS2Impl::copyright() const
{
    return m_doc->copyright();
}

ImagePtr FeedRSS2Impl::image() const
{
    ImageRSS2ImplPtr ptr(new ImageRSS2Impl(m_doc->image()));
    return ptr;
}

QMultiMap<QString, QDomElement> FeedRSS2Impl::additionalProperties() const
{
    QMultiMap<QString, QDomElement> ret;

    const auto unhandledElements = m_doc->unhandledElements();
    for (const QDomElement &i : unhandledElements) {
        ret.insert(i.namespaceURI() + i.localName(), i);
    }

    return ret;
}

ImagePtr FeedRSS2Impl::icon() const
{
    ImageRSS2ImplPtr ptr(new ImageRSS2Impl({}));
    return ptr;
}

} // namespace Syndication
