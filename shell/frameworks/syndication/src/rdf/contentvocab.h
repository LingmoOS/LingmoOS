/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_CONTENTVOCAB_H
#define SYNDICATION_RDF_CONTENTVOCAB_H

#include <QSharedPointer>

#include "../syndication_export.h"

#include <memory>

class QString;

namespace Syndication
{
namespace RDF
{
class Property;
//@cond PRIVATE
typedef QSharedPointer<Property> PropertyPtr;
//@endcond
/**
 * Singleton holding RDF class and property constants of the RSS 1.0 content
 * module. See http://web.resource.org/rss/1.0/modules/content/ for a
 * specification.
 *
 * @author Frank Osterfeld
 */
class ContentVocab
{
public:
    /**
     * returns the singleton instance
     */
    static ContentVocab *self();

    /**
     * destructor
     */
    ~ContentVocab();

    /**
     * Namespace of the Content module
     *
     * http://purl.org/rss/1.0/modules/content/
     **/
    const QString &namespaceURI() const;

    /**
     * content:encoded property
     */
    PropertyPtr encoded() const;

private:
    ContentVocab();
    Q_DISABLE_COPY(ContentVocab)

    class ContentVocabPrivate;
    std::unique_ptr<ContentVocabPrivate> const d;
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_CONTENTVOCAB_H
