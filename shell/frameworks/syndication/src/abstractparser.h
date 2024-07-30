/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_ABSTRACTPARSER_H
#define SYNDICATION_ABSTRACTPARSER_H

#include "specificdocument.h"

#include "syndication_export.h"

class QString;

namespace Syndication
{
class DocumentSource;

/**
 * Interface for all parsers. The parsers for the various formats must
 * implement this interface and register themselves at the ParserRegistry.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT AbstractParser
{
public:
    /**
     * virtual destructor
     */
    virtual ~AbstractParser();

    /**
     * Lets the parser check if it can parse the passed source.
     * Parser implementations should do a _quick_ check for the file
     * format (i.e. check for feed format and version number in the root
     * element) to find out if the source is in a supported format. They
     * should _not_ completely parse the document to test for full
     * compliance to the format specification.
     *
     * @param source the document source to be checked
     * @return whether @c source seems to be in a format supported by the
     * parser
     */
    virtual bool accept(const DocumentSource &source) const = 0;

    /**
     * Lets the parser parse a document source. The parser returns a
     * valid document instance if successful, or an invalid one if
     * not.
     *
     * @see SpecificDocument::isValid()
     * @param source The document source to be parsed
     * @return a newly created document parsed from @c source
     */
    virtual SpecificDocumentPtr parse(const DocumentSource &source) const = 0;

    /**
     * Returns the name of the format supported by this
     * parser.
     *
     * @return a string like "rss2", "atom" or "rdf"
     */
    virtual QString format() const = 0;
};

} // namespace Syndication

#endif // SYNDICATION_ABSTRACTPARSER_H
