/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itemrss2impl.h"
#include "categoryrss2impl.h"
#include "enclosurerss2impl.h"
#include <atom/constants.h>
#include <constants.h>
#include <personimpl.h>
#include <rss2/category.h>
#include <rss2/enclosure.h>
#include <tools.h>

#include <QDomElement>
#include <QList>
#include <QMultiMap>
#include <QString>

namespace Syndication
{
ItemRSS2Impl::ItemRSS2Impl(const Syndication::RSS2::Item &item)
    : m_item(item)
{
}

QString ItemRSS2Impl::title() const
{
    return m_item.title();
}

QString ItemRSS2Impl::link() const
{
    const QString link = m_item.link();
    if (!link.isEmpty()) {
        return link;
    }

    const QString guid = m_item.guid();
    if (m_item.guidIsPermaLink()) {
        return guid;
    }

    return QString();
}

QString ItemRSS2Impl::description() const
{
    return m_item.description();
}

QString ItemRSS2Impl::content() const
{
    return m_item.content();
}

QList<PersonPtr> ItemRSS2Impl::authors() const
{
    QList<PersonPtr> list;

    PersonPtr ptr = personFromString(m_item.author());

    if (!ptr->isNull()) {
        list.append(ptr);
    }

    return list;
}

QString ItemRSS2Impl::language() const
{
    return QString();
}

QString ItemRSS2Impl::id() const
{
    QString guid = m_item.guid();
    if (!guid.isEmpty()) {
        return guid;
    }

    return QStringLiteral("hash:%1").arg(calcMD5Sum(title() + description() + link() + content()));
}

time_t ItemRSS2Impl::datePublished() const
{
    return m_item.pubDate();
}

time_t ItemRSS2Impl::dateUpdated() const
{
    // Some RSS feeds contain atom elements - return atom:dateUpdated if present
    const QString updstr = m_item.extractElementTextNS(Atom::atom1Namespace(), QStringLiteral("updated"));
    if (!updstr.isEmpty()) {
        return parseDate(updstr, ISODate);
    } else {
        return datePublished();
    }
}

QList<Syndication::EnclosurePtr> ItemRSS2Impl::enclosures() const
{
    const QList<Syndication::RSS2::Enclosure> encs = m_item.enclosures();

    QList<Syndication::EnclosurePtr> list;
    list.reserve(encs.size());

    std::transform(encs.cbegin(), encs.cend(), std::back_inserter(list), [this](const Syndication::RSS2::Enclosure &e) {
        return EnclosureRSS2ImplPtr(new EnclosureRSS2Impl(m_item, e));
    });

    return list;
}

QList<Syndication::CategoryPtr> ItemRSS2Impl::categories() const
{
    const QList<Syndication::RSS2::Category> cats = m_item.categories();

    QList<Syndication::CategoryPtr> list;
    list.reserve(cats.size());

    std::transform(cats.cbegin(), cats.cend(), std::back_inserter(list), [](const Syndication::RSS2::Category &c) {
        return CategoryRSS2ImplPtr(new CategoryRSS2Impl(c));
    });

    return list;
}

int ItemRSS2Impl::commentsCount() const
{
    const QString cstr = m_item.extractElementTextNS(slashNamespace(), QStringLiteral("comments"));
    bool ok = false;
    int comments = cstr.toInt(&ok);
    return ok ? comments : -1;
}

QString ItemRSS2Impl::commentsLink() const
{
    return m_item.comments();
}

QString ItemRSS2Impl::commentsFeed() const
{
    QString t = m_item.extractElementTextNS(commentApiNamespace(), QStringLiteral("commentRss"));
    if (t.isNull()) {
        t = m_item.extractElementTextNS(commentApiNamespace(), QStringLiteral("commentRSS"));
    }
    return t;
}

QString ItemRSS2Impl::commentPostUri() const
{
    return m_item.extractElementTextNS(commentApiNamespace(), QStringLiteral("comment"));
}

Syndication::SpecificItemPtr ItemRSS2Impl::specificItem() const
{
    return Syndication::SpecificItemPtr(new Syndication::RSS2::Item(m_item));
}

QMultiMap<QString, QDomElement> ItemRSS2Impl::additionalProperties() const
{
    QMultiMap<QString, QDomElement> ret;

    const auto unhandledElements = m_item.unhandledElements();
    for (const QDomElement &i : unhandledElements) {
        ret.insert(i.namespaceURI() + i.localName(), i);
    }

    return ret;
}

} // namespace Syndication
