/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RSS2_ENCLOSURE_H
#define SYNDICATION_RSS2_ENCLOSURE_H

#include <syndication/elementwrapper.h>

class QDomElement;
class QString;

namespace Syndication
{
namespace RSS2
{
/**
 * Describes a media object that is "attached" to the item.
 * The most common use case for enclosures are podcasts:
 * An audio file is attached to the feed and can be
 * automatically downloaded by a podcast client.
 *
 * @author Frank Osterfeld
 */
class Enclosure : public ElementWrapper
{
public:
    /**
     * Default constructor, creates a null object, for which isNull() is
     * @c true.
     */
    Enclosure();

    /**
     * Creates an Enclosure object wrapping an @c &lt;enclosure> XML element.
     *
     * @param element The @c &lt;enclosure> element to wrap
     */
    explicit Enclosure(const QDomElement &element);

    /**
     * returns the URL of the enclosure
     */
    QString url() const;

    /**
     * returns the size of the enclosure in bytes
     */
    int length() const;

    /**
     * returns the mime type of the enclosure
     * (e.g. "audio/mpeg")
     */
    QString type() const;

    /**
     * Returns a description of the object for debugging purposes.
     *
     * @return debug string
     */
    QString debugInfo() const;
};

} // namespace RSS2
} // namespace Syndication

#endif // SYNDICATION_RSS2_ENCLOSURE_H
