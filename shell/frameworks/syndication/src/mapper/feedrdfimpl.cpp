/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "feedrdfimpl.h"
#include "imagerdfimpl.h"
#include "itemrdfimpl.h"

#include <category.h>
#include <personimpl.h>
#include <rdf/dublincore.h>
#include <rdf/item.h>
#include <tools.h>

#include <QDomElement>
#include <QList>
#include <QMultiMap>
#include <QString>
#include <QStringList>

namespace Syndication
{
FeedRDFImpl::FeedRDFImpl(Syndication::RDF::DocumentPtr doc)
    : m_doc(doc)
{
}

Syndication::SpecificDocumentPtr FeedRDFImpl::specificDocument() const
{
    return m_doc;
}

QList<Syndication::ItemPtr> FeedRDFImpl::items() const
{
    const QList<Syndication::RDF::Item> entries = m_doc->items();

    QList<ItemPtr> items;
    items.reserve(entries.count());

    std::transform(entries.cbegin(), entries.cend(), std::back_inserter(items), [](const Syndication::RDF::Item &entry) {
        return ItemRDFImplPtr(new ItemRDFImpl(entry));
    });

    return items;
}

QList<Syndication::CategoryPtr> FeedRDFImpl::categories() const
{
    // TODO: check if it makes sense to map dc:subject to categories
    return QList<Syndication::CategoryPtr>();
}

QString FeedRDFImpl::title() const
{
    return m_doc->title();
}

QString FeedRDFImpl::link() const
{
    return m_doc->link();
}

QString FeedRDFImpl::description() const
{
    return m_doc->description();
}

QList<PersonPtr> FeedRDFImpl::authors() const
{
    const QStringList people = m_doc->dc().creators() + m_doc->dc().contributors();

    QList<PersonPtr> list;
    list.reserve(people.size());

    for (const auto &person : people) {
        PersonPtr ptr = personFromString(person);
        if (!ptr->isNull()) {
            list.append(ptr);
        }
    }

    return list;
}

QString FeedRDFImpl::language() const
{
    return m_doc->dc().language();
}

QString FeedRDFImpl::copyright() const
{
    return m_doc->dc().rights();
}

ImagePtr FeedRDFImpl::image() const
{
    ImageRDFImplPtr ptr(new ImageRDFImpl(m_doc->image()));
    return ptr;
}

ImagePtr FeedRDFImpl::icon() const
{
    ImageRDFImplPtr ptr(new ImageRDFImpl({}));
    return ptr;
}

QMultiMap<QString, QDomElement> FeedRDFImpl::additionalProperties() const
{
    return QMultiMap<QString, QDomElement>();
}

} // namespace Syndication
