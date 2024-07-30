/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itematomimpl.h"
#include "categoryatomimpl.h"
#include "enclosureatomimpl.h"

#include <atom/category.h>
#include <atom/content.h>
#include <atom/link.h>
#include <atom/person.h>
#include <category.h>
#include <constants.h>
#include <enclosure.h>
#include <personimpl.h>
#include <tools.h>

#include <QDomElement>
#include <QList>
#include <QMultiMap>
#include <QString>

using Syndication::Atom::Content;
using Syndication::Atom::Link;
using Syndication::Atom::Person;

namespace Syndication
{
ItemAtomImpl::ItemAtomImpl(const Syndication::Atom::Entry &entry)
    : m_entry(entry)
{
}

QString ItemAtomImpl::title() const
{
    return m_entry.title();
}

QString ItemAtomImpl::link() const
{
    const QList<Syndication::Atom::Link> links = m_entry.links();

    // return first link where rel="alternate"
    auto it = std::find_if(links.cbegin(), links.cend(), [](const Syndication::Atom::Link &link) {
        return link.rel() == QLatin1String("alternate");
    });
    return it != links.cend() ? it->href() : QString{};
}

QString ItemAtomImpl::description() const
{
    return m_entry.summary();
}

QString ItemAtomImpl::content() const
{
    Content content = m_entry.content();
    if (content.isNull()) {
        return QString();
    }

    return content.asString();
}

QList<PersonPtr> ItemAtomImpl::authors() const
{
    const QList<Syndication::Atom::Person> people = m_entry.authors() + m_entry.contributors();

    QList<PersonPtr> list;
    list.reserve(people.size());

    std::transform(people.cbegin(), people.cend(), std::back_inserter(list), [](const Syndication::Atom::Person &person) {
        return PersonImplPtr(new PersonImpl(person.name(), person.uri(), person.email()));
    });

    return list;
}

time_t ItemAtomImpl::datePublished() const
{
    time_t pub = m_entry.published();
    if (pub == 0) {
        return m_entry.updated();
    } else {
        return pub;
    }
}

time_t ItemAtomImpl::dateUpdated() const
{
    time_t upd = m_entry.updated();
    if (upd == 0) {
        return m_entry.published();
    } else {
        return upd;
    }
}

QString ItemAtomImpl::language() const
{
    return m_entry.xmlLang();
}

QString ItemAtomImpl::id() const
{
    const QString id = m_entry.id();
    if (!id.isEmpty()) {
        return id;
    }

    return QStringLiteral("hash:%1").arg(Syndication::calcMD5Sum(title() + description() + link() + content()));
}

QList<Syndication::EnclosurePtr> ItemAtomImpl::enclosures() const
{
    QList<Syndication::EnclosurePtr> list;

    const QList<Syndication::Atom::Link> links = m_entry.links();

    for (const auto &link : links) {
        if (link.rel() == QLatin1String("enclosure")) {
            list.append(EnclosureAtomImplPtr(new EnclosureAtomImpl(link)));
        }
    }

    return list;
}

QList<Syndication::CategoryPtr> ItemAtomImpl::categories() const
{
    const QList<Syndication::Atom::Category> cats = m_entry.categories();

    QList<Syndication::CategoryPtr> list;
    list.reserve(cats.count());

    std::transform(cats.cbegin(), cats.cend(), std::back_inserter(list), [](const Syndication::Atom::Category &c) {
        return CategoryAtomImplPtr(new CategoryAtomImpl(c));
    });

    return list;
}

int ItemAtomImpl::commentsCount() const
{
    QString cstr = m_entry.extractElementTextNS(slashNamespace(), QStringLiteral("comments"));
    bool ok = false;
    int comments = cstr.toInt(&ok);
    return ok ? comments : -1;
}

QString ItemAtomImpl::commentsLink() const
{
    return QString();
}

QString ItemAtomImpl::commentsFeed() const
{
    return m_entry.extractElementTextNS(commentApiNamespace(), QStringLiteral("commentRss"));
}

QString ItemAtomImpl::commentPostUri() const
{
    return m_entry.extractElementTextNS(commentApiNamespace(), QStringLiteral("comment"));
}

Syndication::SpecificItemPtr ItemAtomImpl::specificItem() const
{
    return Syndication::SpecificItemPtr(new Syndication::Atom::Entry(m_entry));
}

QMultiMap<QString, QDomElement> ItemAtomImpl::additionalProperties() const
{
    QMultiMap<QString, QDomElement> ret;
    const auto unhandledElements = m_entry.unhandledElements();
    for (const QDomElement &i : unhandledElements) {
        ret.insert(i.namespaceURI() + i.localName(), i);
    }

    return ret;
}

} // namespace Syndication
