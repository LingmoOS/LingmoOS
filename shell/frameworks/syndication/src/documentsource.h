/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_DOCUMENTSOURCE_H
#define SYNDICATION_DOCUMENTSOURCE_H

#include <QSharedPointer>
#include <QString>

#include "syndication_export.h"

class QByteArray;
class QDomDocument;

namespace Syndication
{
/**
 * Represents the source of a syndication document, as read from the
 * downloaded file.
 *
 * It provides a (cached) DOM representation of the document, but keeps
 * the raw data available (for (rarely used) non-XML formats like Okay!
 * News).
 *
 * This way the document can be passed to all available parsers (to find the
 * right one for the source), regardless whether they parse XML formats or
 * non-XML formats, without having every parser to do the XML parsing again.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT DocumentSource
{
public:
    /**
     * Creates an empty document source. The raw representation is empty and
     * the DOM representation will be invalid.
     */
    DocumentSource();

    /**
     * Creates a DocumentSource object from a raw byte array
     *
     * @param source the raw source (of the downloaded feed file usually)
     * @param url the URL/path the source was read from
     */
    DocumentSource(const QByteArray &source, const QString &url);

    /**
     * Copy constructor. The d pointer is shared, so this is a cheap
     * operation.
     *
     * @param other DocumentSource to copy
     */
    DocumentSource(const DocumentSource &other);

    /**
     * destructor
     */
    ~DocumentSource();

    /**
     * Assignment operator. The d pointer is shared, so this is a cheap
     * operation.
     *
     * @param other DocumentSource to assign to this instance
     * @return reference to this instance
     */
    DocumentSource &operator=(const DocumentSource &other);

    /**
     * Returns the feed source as byte array.
     *
     * @return the feed source as raw byte array.
     */
    Q_REQUIRED_RESULT QByteArray asByteArray() const;

    /**
     * returns the size the source array in bytes.
     *
     * @return the size of the byte array in bytes.
     * See also QByteArray::size()
     */
    Q_REQUIRED_RESULT unsigned int size() const;

    /**
     * calculates a hash value for the source array.
     * This can be used to decide whether the feed has changed since
     * the last fetch. If the hash hasn't changed since the last fetch,
     * the feed wasn't modified with high probability.
     *
     * @return the hash calculated from the source, 0 if the
     * source is empty
     */
    Q_REQUIRED_RESULT unsigned int hash() const;

    /**
     * Returns the feed source as DOM document.
     * The document is parsed only on the first call of this method
     * and then cached.
     *
     * If the feed source cannot be parsed successfully then the
     * returned DOM node will be null (eg. asDomDocument().isNull()
     * will return true)
     *
     * @return XML representation parsed from the raw source
     */
    Q_REQUIRED_RESULT QDomDocument asDomDocument() const;

    /**
     * returns the URL the document source was loaded from
     */
    Q_REQUIRED_RESULT QString url() const;

private:
    class DocumentSourcePrivate;
    QSharedPointer<DocumentSourcePrivate> d;
};

} // namespace Syndication

#endif // SYNDICATION_DOCUMENTSOURCE_H
