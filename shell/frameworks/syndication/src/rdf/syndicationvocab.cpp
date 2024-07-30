/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "syndicationvocab.h"
#include "property.h"

#include <QCoreApplication>
#include <QString>

namespace Syndication
{
namespace RDF
{
class SYNDICATION_NO_EXPORT SyndicationVocab::SyndicationVocabPrivate
{
public:
    QString namespaceURI;
    PropertyPtr updatePeriod;
    PropertyPtr updateFrequency;
    PropertyPtr updateBase;

    static SyndicationVocab *sSelf;
    static void cleanupSyndicationVocab()
    {
        delete sSelf;
        sSelf = nullptr;
    }
};
SyndicationVocab *SyndicationVocab::SyndicationVocabPrivate::sSelf = nullptr;

SyndicationVocab::SyndicationVocab()
    : d(new SyndicationVocabPrivate)
{
    QString ns = QStringLiteral("http://purl.org/rss/1.0/modules/syndication/");

    d->namespaceURI = ns;

    d->updatePeriod = PropertyPtr(new Property(ns + QLatin1String("updatePeriod")));
    d->updateFrequency = PropertyPtr(new Property(ns + QLatin1String("updateFrequency")));
    d->updateBase = PropertyPtr(new Property(ns + QLatin1String("updateBase")));
}

SyndicationVocab::~SyndicationVocab() = default;

SyndicationVocab *SyndicationVocab::self()
{
    static SyndicationVocabPrivate p;
    if (!p.sSelf) {
        p.sSelf = new SyndicationVocab;
        qAddPostRoutine(SyndicationVocabPrivate::cleanupSyndicationVocab);
    }
    return p.sSelf;
}

const QString &SyndicationVocab::namespaceURI() const
{
    return d->namespaceURI;
}

PropertyPtr SyndicationVocab::updatePeriod() const
{
    return d->updatePeriod;
}

PropertyPtr SyndicationVocab::updateFrequency() const
{
    return d->updateFrequency;
}

PropertyPtr SyndicationVocab::updateBase() const
{
    return d->updateBase;
}

} // namespace RDF
} // namespace Syndication
