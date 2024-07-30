/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_RDFVOCAB_H
#define SYNDICATION_RDF_RDFVOCAB_H

#include <QSharedPointer>

#include "../syndication_export.h"

#include <memory>

class QString;

namespace Syndication
{
namespace RDF
{
class Property;
typedef QSharedPointer<Property> PropertyPtr;
class Resource;
typedef QSharedPointer<Resource> ResourcePtr;

/**
 * singleton holding RDF vocabulary, expressed as RDF.
 */
class RDFVocab
{
public:
    /**
     * returns the singleton instance
     */
    static RDFVocab *self();

    /**
     * destructor
     */
    ~RDFVocab();

    /**
     * the RDF namespace, which is
     * http://www.w3.org/1999/02/22-rdf-syntax-ns#
     */
    QString namespaceURI();

    /**
     * the sequence type
     */
    ResourcePtr seq();

    /**
     * the rdf:type property (A rdf:type B means A is instance of B)
     */
    PropertyPtr type();

    /**
     * the rdf:li property, used for list items in RDF containers (like
     * rdf:seq)
     */
    PropertyPtr li();

private:
    RDFVocab();
    Q_DISABLE_COPY(RDFVocab)
    class RDFVocabPrivate;
    std::unique_ptr<RDFVocabPrivate> const d;
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_RDFVOCAB_H
