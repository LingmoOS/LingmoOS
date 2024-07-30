/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "feedatomimpl.h"
#include "categoryatomimpl.h"
#include "imageatomimpl.h"
#include "itematomimpl.h"
#include <atom/category.h>
#include <atom/entry.h>
#include <atom/link.h>
#include <atom/person.h>
#include <personimpl.h>

#include <QDomElement>
#include <QList>
#include <QMultiMap>
#include <QString>

namespace Syndication
{
FeedAtomImpl::FeedAtomImpl(Syndication::Atom::FeedDocumentPtr doc)
    : m_doc(doc)
{
}

Syndication::SpecificDocumentPtr FeedAtomImpl::specificDocument() const
{
    return m_doc;
}

QList<Syndication::ItemPtr> FeedAtomImpl::items() const
{
    const QList<Syndication::Atom::Entry> entries = m_doc->entries();

    QList<ItemPtr> items;
    items.reserve(entries.count());

    std::transform(entries.cbegin(), entries.cend(), std::back_inserter(items), [](const Syndication::Atom::Entry &entry) {
        return ItemAtomImplPtr(new ItemAtomImpl(entry));
    });

    return items;
}

QList<Syndication::CategoryPtr> FeedAtomImpl::categories() const
{
    const QList<Syndication::Atom::Category> entries = m_doc->categories();
    QList<CategoryPtr> categories;
    categories.reserve(entries.count());

    std::transform(entries.cbegin(), entries.cend(), std::back_inserter(categories), [](const Syndication::Atom::Category &entry) {
        return CategoryAtomImplPtr(new CategoryAtomImpl(entry));
    });

    return categories;
}

QString FeedAtomImpl::title() const
{
    return m_doc->title();
}

QString FeedAtomImpl::link() const
{
    const QList<Syndication::Atom::Link> links = m_doc->links();
    // return first link where rel="alternate"
    // TODO: if there are multiple "alternate" links, find other criteria to choose one of them
    auto it = std::find_if(links.cbegin(), links.cend(), [](const Syndication::Atom::Link &link) {
        return link.rel() == QLatin1String("alternate");
    });
    return it != links.cend() ? it->href() : QString{};
}

QString FeedAtomImpl::description() const
{
    return m_doc->subtitle();
}

QList<PersonPtr> FeedAtomImpl::authors() const
{
    const QList<Syndication::Atom::Person> people = m_doc->authors() + m_doc->contributors();

    QList<PersonPtr> list;
    list.reserve(people.size());

    std::transform(people.cbegin(), people.cend(), std::back_inserter(list), [](const Syndication::Atom::Person &person) {
        return PersonImplPtr(new PersonImpl(person.name(), person.uri(), person.email()));
    });

    return list;
}

QString FeedAtomImpl::language() const
{
    return m_doc->xmlLang();
}

QString FeedAtomImpl::copyright() const
{
    return m_doc->rights();
}

ImagePtr FeedAtomImpl::image() const
{
    return ImageAtomImplPtr(new ImageAtomImpl(m_doc->logo()));
}

ImagePtr FeedAtomImpl::icon() const
{
    return ImageAtomImplPtr(new ImageAtomImpl(m_doc->icon()));
}

QMultiMap<QString, QDomElement> FeedAtomImpl::additionalProperties() const
{
    QMultiMap<QString, QDomElement> ret;
    const auto unhandledElements = m_doc->unhandledElements();
    for (const QDomElement &i : unhandledElements) {
        ret.insert(i.namespaceURI() + i.localName(), i);
    }

    return ret;
}

} // namespace Syndication
