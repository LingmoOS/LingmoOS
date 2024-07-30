/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_SYNDICATIONVOCAB_H
#define SYNDICATION_RDF_SYNDICATIONVOCAB_H

#include <QSharedPointer>

#include "../syndication_export.h"

#include <memory>

class QString;

namespace Syndication
{
namespace RDF
{
//@cond PRIVATE
class Property;
typedef QSharedPointer<Property> PropertyPtr;
//@endcond

/**
 * Singleton providing Property constants for
 * the Syndication module.
 * For a specification, see
 * http://web.resource.org/rss/1.0/modules/syndication/
 *
 * @author Frank Osterfeld
 */
class SyndicationVocab
{
public:
    /**
     * destructor
     */
    ~SyndicationVocab();

    /**
     * returns the singleton instance
     */
    static SyndicationVocab *self();

    /**
     * namespace URI of the syndication vocabulary,
     * @p "http://purl.org/rss/1.0/modules/syndication/"
     */
    const QString &namespaceURI() const;

    /**
     * updatePeriod property, see Syndication::updatePeriod() for more
     * information.
     */
    PropertyPtr updatePeriod() const;

    /**
     * updateFrequency property, see Syndication::updateFrequency() for more
     * information.
     */
    PropertyPtr updateFrequency() const;

    /**
     * updateBase property, see Syndication::updateBase() for more
     * information.
     */
    PropertyPtr updateBase() const;

private:
    SyndicationVocab();
    Q_DISABLE_COPY(SyndicationVocab)
    class SyndicationVocabPrivate;
    std::unique_ptr<SyndicationVocabPrivate> const d;
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_SYNDICATIONVOCAB_H
