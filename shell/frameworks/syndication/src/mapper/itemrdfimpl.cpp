/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "itemrdfimpl.h"

#include <category.h>
#include <constants.h>
#include <enclosure.h>
#include <personimpl.h>
#include <rdf/dublincore.h>
#include <rdf/property.h>
#include <rdf/resource.h>
#include <rdf/resourcewrapper.h>
#include <rdf/statement.h>
#include <tools.h>

#include <QDomElement>
#include <QList>
#include <QMultiMap>
#include <QStringList>

using Syndication::RDF::Property;
using Syndication::RDF::PropertyPtr;

namespace Syndication
{
ItemRDFImpl::ItemRDFImpl(const Syndication::RDF::Item &item)
    : m_item(item)
{
}

QString ItemRDFImpl::title() const
{
    return m_item.title();
}

QString ItemRDFImpl::link() const
{
    return m_item.link();
}

QString ItemRDFImpl::description() const
{
    return m_item.description();
}

QString ItemRDFImpl::content() const
{
    return m_item.encodedContent();
}

QList<PersonPtr> ItemRDFImpl::authors() const
{
    QList<PersonPtr> list;

    const QStringList people = m_item.dc().creators() + m_item.dc().contributors();

    for (const auto &person : people) {
        PersonPtr ptr = personFromString(person);
        if (!ptr->isNull()) {
            list.append(ptr);
        }
    }

    return list;
}

QString ItemRDFImpl::language() const
{
    return m_item.dc().language();
}

QString ItemRDFImpl::id() const
{
    if (!m_item.resource()->isAnon()) {
        return m_item.resource()->uri();
    } else {
        return QLatin1String("hash:") + calcMD5Sum(title() + description() + link() + content());
    }
}

time_t ItemRDFImpl::datePublished() const
{
    return m_item.dc().date();
}

time_t ItemRDFImpl::dateUpdated() const
{
    return m_item.dc().date();
}

QList<Syndication::EnclosurePtr> ItemRDFImpl::enclosures() const
{
    // return empty list
    return QList<Syndication::EnclosurePtr>();
}

QList<Syndication::CategoryPtr> ItemRDFImpl::categories() const
{
    // return empty list
    return QList<Syndication::CategoryPtr>();
}

int ItemRDFImpl::commentsCount() const
{
    PropertyPtr prop(new Property(slashNamespace() + QLatin1String("comments")));
    QString cstr = m_item.resource()->property(prop)->asString();
    bool ok = false;
    int comments = cstr.toInt(&ok);
    return ok ? comments : -1;
    return -1;
}

QString ItemRDFImpl::commentsLink() const
{
    return QString();
}

QString ItemRDFImpl::commentsFeed() const
{
    PropertyPtr prop(new Property(commentApiNamespace() + QLatin1String("commentRss")));
    return m_item.resource()->property(prop)->asString();
}

QString ItemRDFImpl::commentPostUri() const
{
    PropertyPtr prop(new Property(commentApiNamespace() + QLatin1String("comment")));
    return m_item.resource()->property(prop)->asString();
}

Syndication::SpecificItemPtr ItemRDFImpl::specificItem() const
{
    return Syndication::SpecificItemPtr(new Syndication::RDF::Item(m_item));
}

QMultiMap<QString, QDomElement> ItemRDFImpl::additionalProperties() const
{
    return QMultiMap<QString, QDomElement>();
}

} // namespace Syndication
