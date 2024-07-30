/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dublincorevocab.h"
#include "property.h"

#include <QCoreApplication>
#include <QString>

namespace Syndication
{
namespace RDF
{
class SYNDICATION_NO_EXPORT DublinCoreVocab::DublinCoreVocabPrivate
{
public:
    QString namespaceURI;
    PropertyPtr contributor;
    PropertyPtr coverage;
    PropertyPtr creator;
    PropertyPtr date;
    PropertyPtr description;
    PropertyPtr format;
    PropertyPtr identifier;
    PropertyPtr language;
    PropertyPtr publisher;
    PropertyPtr relation;
    PropertyPtr rights;
    PropertyPtr source;
    PropertyPtr subject;
    PropertyPtr title;
    PropertyPtr type;

    static DublinCoreVocab *sSelf;
    static void cleanupDublinCoreVocab()
    {
        delete sSelf;
        sSelf = nullptr;
    }
};
DublinCoreVocab *DublinCoreVocab::DublinCoreVocabPrivate::sSelf = nullptr;

DublinCoreVocab::DublinCoreVocab()
    : d(new DublinCoreVocabPrivate)
{
    QString ns = QStringLiteral("http://purl.org/dc/elements/1.1/");

    d->namespaceURI = ns;

    d->contributor = PropertyPtr(new Property(ns + QLatin1String("contributor")));
    d->coverage = PropertyPtr(new Property(ns + QLatin1String("coverage")));
    d->creator = PropertyPtr(new Property(ns + QLatin1String("creator")));
    d->date = PropertyPtr(new Property(ns + QLatin1String("date")));
    d->description = PropertyPtr(new Property(ns + QLatin1String("description")));
    d->format = PropertyPtr(new Property(ns + QLatin1String("format")));
    d->identifier = PropertyPtr(new Property(ns + QLatin1String("identifier")));
    d->language = PropertyPtr(new Property(ns + QLatin1String("language")));
    d->publisher = PropertyPtr(new Property(ns + QLatin1String("publisher")));
    d->relation = PropertyPtr(new Property(ns + QLatin1String("relation")));
    d->rights = PropertyPtr(new Property(ns + QLatin1String("rights")));
    d->source = PropertyPtr(new Property(ns + QLatin1String("source")));
    d->subject = PropertyPtr(new Property(ns + QLatin1String("subject")));
    d->title = PropertyPtr(new Property(ns + QLatin1String("title")));
    d->type = PropertyPtr(new Property(ns + QLatin1String("type")));
}

DublinCoreVocab::~DublinCoreVocab() = default;

DublinCoreVocab *DublinCoreVocab::self()
{
    static DublinCoreVocabPrivate p;
    if (!p.sSelf) {
        p.sSelf = new DublinCoreVocab;
        qAddPostRoutine(DublinCoreVocabPrivate::cleanupDublinCoreVocab);
    }
    return p.sSelf;
}

const QString &DublinCoreVocab::namespaceURI() const
{
    return d->namespaceURI;
}

PropertyPtr DublinCoreVocab::contributor() const
{
    return d->contributor;
}

PropertyPtr DublinCoreVocab::creator() const
{
    return d->creator;
}

PropertyPtr DublinCoreVocab::coverage() const
{
    return d->coverage;
}

PropertyPtr DublinCoreVocab::date() const
{
    return d->date;
}

PropertyPtr DublinCoreVocab::description() const
{
    return d->description;
}

PropertyPtr DublinCoreVocab::format() const
{
    return d->format;
}

PropertyPtr DublinCoreVocab::identifier() const
{
    return d->identifier;
}

PropertyPtr DublinCoreVocab::language() const
{
    return d->language;
}

PropertyPtr DublinCoreVocab::publisher() const
{
    return d->publisher;
}

PropertyPtr DublinCoreVocab::relation() const
{
    return d->relation;
}

PropertyPtr DublinCoreVocab::rights() const
{
    return d->rights;
}

PropertyPtr DublinCoreVocab::source() const
{
    return d->source;
}

PropertyPtr DublinCoreVocab::subject() const
{
    return d->subject;
}

PropertyPtr DublinCoreVocab::title() const
{
    return d->title;
}

PropertyPtr DublinCoreVocab::type() const
{
    return d->type;
}

} // namespace RDF
} // namespace Syndication
