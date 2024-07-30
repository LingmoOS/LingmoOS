/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dublincore.h"
#include "dublincorevocab.h"
#include "property.h"
#include "statement.h"

#include <tools.h>

#include <QList>
#include <QString>

namespace Syndication
{
namespace RDF
{
DublinCore::DublinCore(ResourcePtr resource)
    : ResourceWrapper(resource)
{
}

DublinCore::~DublinCore()
{
}

QString DublinCore::contributor() const
{
    return resource()->property(DublinCoreVocab::self()->contributor())->asString();
}

QStringList DublinCore::contributors() const
{
    QStringList res;
    QList<StatementPtr> list = resource()->properties(DublinCoreVocab::self()->contributor());
    QList<StatementPtr>::ConstIterator it = list.constBegin();
    QList<StatementPtr>::ConstIterator end = list.constEnd();
    for (; it != end; ++it) {
        const QString str = (*it)->asString();
        if (!str.isNull()) {
            res.append(str);
        }
    }
    return res;
}

QString DublinCore::coverage() const
{
    return resource()->property(DublinCoreVocab::self()->coverage())->asString();
}

QString DublinCore::creator() const
{
    return resource()->property(DublinCoreVocab::self()->creator())->asString();
}

QStringList DublinCore::creators() const
{
    QStringList res;
    QList<StatementPtr> list = resource()->properties(DublinCoreVocab::self()->creator());
    QList<StatementPtr>::ConstIterator it = list.constBegin();
    QList<StatementPtr>::ConstIterator end = list.constEnd();
    for (; it != end; ++it) {
        const QString str = (*it)->asString();
        if (!str.isNull()) {
            res.append(str);
        }
    }
    return res;
}

time_t DublinCore::date() const
{
    QString str = resource()->property(DublinCoreVocab::self()->date())->asString();
    return parseDate(str, ISODate);
}

QString DublinCore::description() const
{
    return resource()->property(DublinCoreVocab::self()->description())->asString();
}

QString DublinCore::format() const
{
    return resource()->property(DublinCoreVocab::self()->format())->asString();
}

QString DublinCore::identifier() const
{
    return resource()->property(DublinCoreVocab::self()->identifier())->asString();
}

QString DublinCore::language() const
{
    return resource()->property(DublinCoreVocab::self()->language())->asString();
}

QString DublinCore::publisher() const
{
    return resource()->property(DublinCoreVocab::self()->publisher())->asString();
}

QString DublinCore::relation() const
{
    return resource()->property(DublinCoreVocab::self()->relation())->asString();
}

QString DublinCore::rights() const
{
    return resource()->property(DublinCoreVocab::self()->rights())->asString();
}

QString DublinCore::source() const
{
    return resource()->property(DublinCoreVocab::self()->source())->asString();
}

QString DublinCore::subject() const
{
    return resource()->property(DublinCoreVocab::self()->subject())->asString();
}

QStringList DublinCore::subjects() const
{
    QStringList res;
    QList<StatementPtr> list = resource()->properties(DublinCoreVocab::self()->subject());
    QList<StatementPtr>::ConstIterator it = list.constBegin();
    QList<StatementPtr>::ConstIterator end = list.constEnd();
    for (; it != end; ++it) {
        QString str = (*it)->asString();
        if (!str.isNull()) {
            res.append(str);
        }
    }
    return res;
}

QString DublinCore::title() const
{
    return resource()->property(DublinCoreVocab::self()->title())->asString();
}

QString DublinCore::type() const
{
    return resource()->property(DublinCoreVocab::self()->type())->asString();
}

QString DublinCore::debugInfo() const
{
    QString info;
    if (!contributor().isNull()) {
        info += QStringLiteral("dc:contributor: #%1#\n").arg(contributor());
    }
    if (!coverage().isNull()) {
        info += QStringLiteral("dc:coverage: #%1#\n").arg(coverage());
    }
    if (!creator().isNull()) {
        info += QStringLiteral("dc:creator: #%1#\n").arg(creator());
    }

    const QString ddate = dateTimeToString(date());
    if (!ddate.isNull()) {
        info += QStringLiteral("dc:date: #%1#\n").arg(ddate);
    }

    if (!description().isNull()) {
        info += QStringLiteral("dc:description: #%1#\n").arg(description());
    }
    if (!format().isNull()) {
        info += QStringLiteral("dc:format: #%1#\n").arg(format());
    }
    if (!identifier().isNull()) {
        info += QStringLiteral("dc:identifier: #%1#\n").arg(identifier());
    }
    if (!language().isNull()) {
        info += QStringLiteral("dc:language: #%1#\n").arg(language());
    }
    if (!publisher().isNull()) {
        info += QStringLiteral("dc:publisher: #%1#\n").arg(publisher());
    }
    if (!relation().isNull()) {
        info += QStringLiteral("dc:relation: #%1#\n").arg(relation());
    }
    if (!rights().isNull()) {
        info += QStringLiteral("dc:rights: #%1#\n").arg(rights());
    }
    if (!source().isNull()) {
        info += QStringLiteral("dc:source: #%1#\n").arg(source());
    }
    if (!subject().isNull()) {
        info += QStringLiteral("dc:subject: #%1#\n").arg(subject());
    }
    if (!title().isNull()) {
        info += QStringLiteral("dc:title: #%1#\n").arg(title());
    }
    if (!type().isNull()) {
        info += QStringLiteral("dc:type: #%1#\n").arg(type());
    }
    return info;
}

} // namespace RDF
} // namespace Syndication
