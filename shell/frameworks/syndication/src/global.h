/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_GLOBAL_H
#define SYNDICATION_GLOBAL_H

#include "feed.h"

#include "syndication_export.h"

#include <QString>

namespace Syndication
{
class DocumentSource;
template<class T>
class ParserCollection;

/**
 *
 * The default ParserCollection instance parsing
 * a DocumentSource into a Feed object.
 *
 * Use this to parse a local file or a otherwise
 * manually created DocumentSource object.

* To retrieve a feed from the web, use Loader instead.
 *
 * Example code:
 *
 * @code
 * ...
 * QFile someFile(somePath);
 * ...
 * DocumentSource src(someFile.readAll());
 * someFile.close();
 *
 * FeedPtr feed = parserCollection()->parse(src);
 *
 * if (feed)
 * {
 *     QString title = feed->title();
 *     QList<ItemPtr> items = feed->items();
 *     ...
 * }
 * @endcode
 */
SYNDICATION_EXPORT
ParserCollection<Feed> *parserCollection();

/**
 * parses a document from a source and returns a new Feed object
 * wrapping the feed content.
 * Shortcut for parserCollection()->parse().
 * See ParserCollection::parse() for more details.
 *
 * @param src the document source to parse
 * @param formatHint an optional hint which format to test first
 */
SYNDICATION_EXPORT
FeedPtr parse(const DocumentSource &src, const QString &formatHint = QString());

/**
 * error code indicating fetching or parsing errors
 */
enum ErrorCode {
    Success = 0, /**< No error occurred, feed was fetched and parsed
                  * successfully
                  */
    Aborted = 1, /**< file downloading/parsing was aborted by the user */
    Timeout = 2, /**< file download timed out */

    UnknownHost = 3, /**< The hostname couldn't get resolved to an IP address */

    FileNotFound = 4, /**< the host was contacted successfully, but reported a
                       * 404 error
                       */
    OtherRetrieverError = 5, /**< retriever error not covered by the error codes
                              * above. This is returned if a custom
                              * DataRetriever was used. See the
                              * retriever-specific status byte for more
                              * information on the occurred error. */
    InvalidXml = 6, /**< The XML is invalid. This is returned if no parser
                     * accepts the source and the DOM document can't be parsed.
                     * It is not returned if the source is not valid XML but a
                     * (non-XML) parser accepts it.
                     */
    XmlNotAccepted = 7, /**< The source is valid XML, but no parser accepted
                         * it.
                         */
    InvalidFormat = 8, /**< the source was accepted by a parser, but the actual
                        * parsing failed. As our parser implementations
                        * currently do not validate the source ("parse what you
                        * can get"), this code will be rarely seen.
                        */
};

} // namespace Syndication

#endif // SYNDICATION_GLOBAL_H
