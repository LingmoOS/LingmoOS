/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "rdfvocab.h"
#include "model.h"
#include "property.h"

#include <QCoreApplication>
#include <QString>

namespace Syndication
{
namespace RDF
{
class SYNDICATION_NO_EXPORT RDFVocab::RDFVocabPrivate
{
public:
    QString namespaceURI;
    ResourcePtr seq;
    PropertyPtr type;
    PropertyPtr li;

    static RDFVocab *sSelf;
    static void cleanupRDFVocab()
    {
        delete sSelf;
        sSelf = nullptr;
    }
};
RDFVocab *RDFVocab::RDFVocabPrivate::sSelf = nullptr;

RDFVocab *RDFVocab::self()
{
    static RDFVocabPrivate p;
    if (!p.sSelf) {
        p.sSelf = new RDFVocab;
        qAddPostRoutine(RDFVocabPrivate::cleanupRDFVocab);
    }
    return p.sSelf;
}

RDFVocab::RDFVocab()
    : d(new RDFVocabPrivate)
{
    QString ns = QStringLiteral("http://www.w3.org/1999/02/22-rdf-syntax-ns#");

    d->namespaceURI = ns;

    d->seq = ResourcePtr(new Resource(ns + QLatin1String("Seq")));
    d->type = PropertyPtr(new Property(ns + QLatin1String("type")));
    d->li = PropertyPtr(new Property(ns + QLatin1String("li")));
}

RDFVocab::~RDFVocab() = default;

ResourcePtr RDFVocab::seq()
{
    return d->seq;
}

PropertyPtr RDFVocab::type()
{
    return d->type;
}

PropertyPtr RDFVocab::li()
{
    return d->li;
}

QString RDFVocab::namespaceURI()
{
    return d->namespaceURI;
}

} // namespace RDF
} // namespace Syndication
