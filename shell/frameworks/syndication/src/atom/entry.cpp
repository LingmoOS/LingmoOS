/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "entry.h"
#include "atomtools.h"
#include "category.h"
#include "constants.h"
#include "content.h"
#include "link.h"
#include "person.h"
#include "source.h"

#include <specificitemvisitor.h>
#include <tools.h>

#include <QDomElement>
#include <QString>

#include <vector>

namespace Syndication
{
namespace Atom
{
Entry::Entry()
    : ElementWrapper()
{
}

Entry::Entry(const QDomElement &element)
    : ElementWrapper(element)
{
}

void Entry::setFeedAuthors(const QList<Person> &feedAuthors)
{
    m_feedAuthors = feedAuthors;
}

QList<Person> Entry::authors() const
{
    const QList<QDomElement> a = elementsByTagNameNS(atom1Namespace(), QStringLiteral("author"));
    QList<Person> list;

    if (!a.isEmpty()) {
        list.reserve(a.count());

        std::transform(a.cbegin(), a.cend(), std::back_inserter(list), [](const QDomElement &element) {
            return Person(element);
        });
    } else {
        list = source().authors();
    }

    return !list.isEmpty() ? list : m_feedAuthors;
}

QList<Person> Entry::contributors() const
{
    const QList<QDomElement> a = elementsByTagNameNS(atom1Namespace(), QStringLiteral("contributor"));
    QList<Person> list;
    list.reserve(a.count());

    std::transform(a.cbegin(), a.cend(), std::back_inserter(list), [](const QDomElement &element) {
        return Person(element);
    });

    return list;
}

QList<Category> Entry::categories() const
{
    const QList<QDomElement> a = elementsByTagNameNS(atom1Namespace(), QStringLiteral("category"));
    QList<Category> list;
    list.reserve(a.count());

    std::transform(a.cbegin(), a.cend(), std::back_inserter(list), [](const QDomElement &element) {
        return Category(element);
    });

    return list;
}

QString Entry::id() const
{
    return extractElementTextNS(atom1Namespace(), QStringLiteral("id"));
}

QList<Link> Entry::links() const
{
    const QList<QDomElement> a = elementsByTagNameNS(atom1Namespace(), QStringLiteral("link"));
    QList<Link> list;
    list.reserve(a.count());

    std::transform(a.cbegin(), a.cend(), std::back_inserter(list), [](const QDomElement &element) {
        return Link(element);
    });

    return list;
}

QString Entry::rights() const
{
    return extractAtomText(*this, QStringLiteral("rights"));
}

Source Entry::source() const
{
    return Source(firstElementByTagNameNS(atom1Namespace(), QStringLiteral("source")));
}

time_t Entry::published() const
{
    QString pub = extractElementTextNS(atom1Namespace(), QStringLiteral("published"));
    return parseDate(pub, ISODate);
}

time_t Entry::updated() const
{
    QString upd = extractElementTextNS(atom1Namespace(), QStringLiteral("updated"));
    return parseDate(upd, ISODate);
}

QString Entry::summary() const
{
    return extractAtomText(*this, QStringLiteral("summary"));
}

QString Entry::title() const
{
    return extractAtomText(*this, QStringLiteral("title"));
}

Content Entry::content() const
{
    return Content(firstElementByTagNameNS(atom1Namespace(), QStringLiteral("content")));
}

QList<QDomElement> Entry::unhandledElements() const
{
    // TODO: do not hardcode this list here
    static std::vector<ElementType> handled; // QVector would require a default ctor, and ElementType is too big for QList
    if (handled.empty()) {
        handled.reserve(12);
        handled.push_back(ElementType(QStringLiteral("author"), atom1Namespace()));
        handled.push_back(ElementType(QStringLiteral("contributor"), atom1Namespace()));
        handled.push_back(ElementType(QStringLiteral("category"), atom1Namespace()));
        handled.push_back(ElementType(QStringLiteral("id"), atom1Namespace()));
        handled.push_back(ElementType(QStringLiteral("link"), atom1Namespace()));
        handled.push_back(ElementType(QStringLiteral("rights"), atom1Namespace()));
        handled.push_back(ElementType(QStringLiteral("source"), atom1Namespace()));
        handled.push_back(ElementType(QStringLiteral("published"), atom1Namespace()));
        handled.push_back(ElementType(QStringLiteral("updated"), atom1Namespace()));
        handled.push_back(ElementType(QStringLiteral("summary"), atom1Namespace()));
        handled.push_back(ElementType(QStringLiteral("title"), atom1Namespace()));
        handled.push_back(ElementType(QStringLiteral("content"), atom1Namespace()));
    }

    QList<QDomElement> notHandled;

    QDomNodeList children = element().childNodes();
    const int numChildren = children.size();
    for (int i = 0; i < numChildren; ++i) {
        QDomElement el = children.at(i).toElement();
        if (!el.isNull() //
            && std::find(handled.cbegin(), handled.cend(), ElementType(el.localName(), el.namespaceURI())) == handled.cend()) {
            notHandled.append(el);
        }
    }

    return notHandled;
}

QString Entry::debugInfo() const
{
    QString info;
    info += QLatin1String("### Entry: ###################\n");
    if (!title().isEmpty()) {
        info += QLatin1String("title: #") + title() + QLatin1String("#\n");
    }
    if (!summary().isEmpty()) {
        info += QLatin1String("summary: #") + summary() + QLatin1String("#\n");
    }
    if (!id().isEmpty()) {
        info += QLatin1String("id: #") + id() + QLatin1String("#\n");
    }
    if (!content().isNull()) {
        info += content().debugInfo();
    }

    if (!rights().isEmpty()) {
        info += QLatin1String("rights: #") + rights() + QLatin1String("#\n");
    }

    QString dupdated = dateTimeToString(updated());
    if (!dupdated.isNull()) {
        info += QLatin1String("updated: #") + dupdated + QLatin1String("#\n");
    }

    QString dpublished = dateTimeToString(published());
    if (!dpublished.isNull()) {
        info += QLatin1String("published: #") + dpublished + QLatin1String("#\n");
    }

    const QList<Link> dlinks = links();
    for (const auto &link : dlinks) {
        info += link.debugInfo();
    }

    const QList<Category> dcats = categories();
    for (const auto &cat : dcats) {
        info += cat.debugInfo();
    }

    info += QLatin1String("### Authors: ###################\n");

    const QList<Person> dauthors = authors();
    for (const auto &author : dauthors) {
        info += author.debugInfo();
    }

    info += QLatin1String("### Contributors: ###################\n");

    const QList<Person> dcontri = contributors();
    for (const auto &person : dcontri) {
        info += person.debugInfo();
    }

    if (!source().isNull()) {
        info += source().debugInfo();
    }

    info += QLatin1String("### Entry end ################\n");

    return info;
}

bool Entry::accept(SpecificItemVisitor *visitor)
{
    return visitor->visitAtomEntry(this);
}

} // namespace Atom
} // namespace Syndication
