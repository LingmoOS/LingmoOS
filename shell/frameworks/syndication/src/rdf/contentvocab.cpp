/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "contentvocab.h"
#include "property.h"

#include <QCoreApplication>
#include <QString>

namespace Syndication
{
namespace RDF
{
class SYNDICATION_NO_EXPORT ContentVocab::ContentVocabPrivate
{
public:
    QString namespaceURI;
    PropertyPtr encoded;

    static ContentVocab *sSelf;
    static void cleanupContentVocab()
    {
        delete sSelf;
        sSelf = nullptr;
    }
};
ContentVocab *ContentVocab::ContentVocabPrivate::sSelf = nullptr;

ContentVocab::ContentVocab()
    : d(new ContentVocabPrivate)
{
    QString ns = QStringLiteral("http://purl.org/rss/1.0/modules/content/");

    d->namespaceURI = ns;

    d->encoded = PropertyPtr(new Property(ns + QLatin1String("encoded")));
}

ContentVocab::~ContentVocab() = default;

ContentVocab *ContentVocab::self()
{
    static ContentVocabPrivate p;
    if (!p.sSelf) {
        p.sSelf = new ContentVocab;
        qAddPostRoutine(ContentVocabPrivate::cleanupContentVocab);
    }
    return p.sSelf;
}

const QString &ContentVocab::namespaceURI() const
{
    return d->namespaceURI;
}

PropertyPtr ContentVocab::encoded() const
{
    return d->encoded;
}

} // namespace RDF
} // namespace Syndication
