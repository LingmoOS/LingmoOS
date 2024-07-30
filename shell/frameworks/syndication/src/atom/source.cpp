/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "source.h"
#include "atomtools.h"
#include "category.h"
#include "constants.h"
#include "generator.h"
#include "link.h"
#include "person.h"

#include <tools.h>

#include <QDomElement>
#include <QList>
#include <QString>

namespace Syndication
{
namespace Atom
{
Source::Source()
    : ElementWrapper()
{
}

Source::Source(const QDomElement &element)
    : ElementWrapper(element)
{
}

QList<Person> Source::authors() const
{
    const QList<QDomElement> a = elementsByTagNameNS(atom1Namespace(), QStringLiteral("author"));

    QList<Person> list;
    list.reserve(a.count());

    std::transform(a.cbegin(), a.cend(), std::back_inserter(list), [](const QDomElement &element) {
        return Person(element);
    });

    return list;
}

QList<Person> Source::contributors() const
{
    const QList<QDomElement> a = elementsByTagNameNS(atom1Namespace(), QStringLiteral("contributor"));
    QList<Person> list;
    list.reserve(a.count());

    std::transform(a.cbegin(), a.cend(), std::back_inserter(list), [](const QDomElement &element) {
        return Person(element);
    });

    return list;
}

QList<Category> Source::categories() const
{
    const QList<QDomElement> a = elementsByTagNameNS(atom1Namespace(), QStringLiteral("category"));
    QList<Category> list;
    list.reserve(a.count());

    std::transform(a.cbegin(), a.cend(), std::back_inserter(list), [](const QDomElement &element) {
        return Category(element);
    });

    return list;
}

Generator Source::generator() const
{
    return Generator(firstElementByTagNameNS(atom1Namespace(), QStringLiteral("generator")));
}

QString Source::icon() const
{
    return extractElementTextNS(atom1Namespace(), QStringLiteral("icon"));
}

QString Source::id() const
{
    return extractElementTextNS(atom1Namespace(), QStringLiteral("id"));
}

QList<Link> Source::links() const
{
    const QList<QDomElement> a = elementsByTagNameNS(atom1Namespace(), QStringLiteral("link"));
    QList<Link> list;
    list.reserve(a.count());

    std::transform(a.cbegin(), a.cend(), std::back_inserter(list), [](const QDomElement &element) {
        return Link(element);
    });

    return list;
}

QString Source::logo() const
{
    return extractElementTextNS(atom1Namespace(), QStringLiteral("logo"));
}

QString Source::rights() const
{
    return extractAtomText(*this, QStringLiteral("rights"));
}

QString Source::subtitle() const
{
    return extractAtomText(*this, QStringLiteral("subtitle"));
}

QString Source::title() const
{
    return extractAtomText(*this, QStringLiteral("title"));
}

time_t Source::updated() const
{
    const QString upd = extractElementTextNS(atom1Namespace(), QStringLiteral("updated"));
    return parseDate(upd, ISODate);
}

QString Source::debugInfo() const
{
    QString info = QLatin1String("### Source: ###################\n");
    if (!title().isEmpty()) {
        info += QLatin1String("title: #") + title() + QLatin1String("#\n");
    }
    if (!subtitle().isEmpty()) {
        info += QLatin1String("subtitle: #") + subtitle() + QLatin1String("#\n");
    }
    if (!id().isEmpty()) {
        info += QLatin1String("id: #") + id() + QLatin1String("#\n");
    }

    if (!rights().isEmpty()) {
        info += QLatin1String("rights: #") + rights() + QLatin1String("#\n");
    }
    if (!icon().isEmpty()) {
        info += QLatin1String("icon: #") + icon() + QLatin1String("#\n");
    }
    if (!logo().isEmpty()) {
        info += QLatin1String("logo: #") + logo() + QLatin1String("#\n");
    }
    if (!generator().isNull()) {
        info += generator().debugInfo();
    }

    QString dupdated = dateTimeToString(updated());
    if (!dupdated.isNull()) {
        info += QLatin1String("updated: #") + dupdated + QLatin1String("#\n");
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

    info += QLatin1String("### Source end ################\n");

    return info;
}

} // namespace Atom
} // namespace Syndication
