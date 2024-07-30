/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_ATOM_LINK_H
#define SYNDICATION_ATOM_LINK_H

#include <syndication/elementwrapper.h>

class QDomElement;
class QString;

namespace Syndication
{
namespace Atom
{
/**
 * A link, pointing to webpages, media files on the web ("podcast"),
 * related content, etc. See rel() for details.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Link : public Syndication::ElementWrapper
{
public:
    /**
     * creates a null link object.
     */
    Link();

    /**
     * creates a Link object wrapping an atom:link element.
     * @param element a DOM element, should be a atom:link element
     * (although not enforced), otherwise this object will not parse
     * anything useful
     */
    explicit Link(const QDomElement &element);

    /**
     * URL of the referenced resource (required)
     */
    Q_REQUIRED_RESULT QString href() const;

    /**
     * the relation between the feed/entry and the linked resource.
     *
     * The value of rel() is usually one of the following:
     *
     * @c "alternate": The URL points to an alternate version of the
     * feed/entry. In practice, this is the article described in an entry,
     * or the homepage of the feed.
     *
     * @c "enclosure": The link describes an Enclosure. See
     * Syndication::Enclosure for more information.
     *
     * @c "related": links to web resources with related content. E.g., an
     * article discussing KDE might link to the KDE homepage.
     *
     * @c "self": "identifies a resource equivalent to the containing
     * element". This is usually the URL of the feed source itself.
     *
     * @c "via": The link points to the source of the information contained
     * in the feed/entry
     *
     * @return the rel value specified in the feed. Default value is
     * @c "alternate"
     */
    Q_REQUIRED_RESULT QString rel() const;

    /**
     * MIME type of the linked resource. (optional)
     *
     * @return MIME type following (e.g., "text/html", "audio/mpeg"),
     * or a null string if not set
     */
    Q_REQUIRED_RESULT QString type() const;

    /**
     * the language of the linked resource. (optional)
     * If used together with a rel() value of "alternate", it
     * implies a translated version of the entry.
     *
     * @return a language tag as defined in RFC 3066,
     * or a null string if not specified
     */
    Q_REQUIRED_RESULT QString hrefLanguage() const;

    /**
     * human-readable information about the link. (optional)
     *
     * @return the link title as plain text ("<", "&" are text, not
     * markup!), or a null string if not specified
     */
    Q_REQUIRED_RESULT QString title() const;

    /**
     * size of the linked resource in bytes. (optional)
     *
     * @return file size in bytes, or 0 if not specified
     */
    Q_REQUIRED_RESULT uint length() const;

    /**
     * description of the link object for debugging purposes
     *
     * @return debug string
     */
    Q_REQUIRED_RESULT QString debugInfo() const;
};

} // namespace Atom
} // namespace Syndication

#endif // SYNDICATION_ATOM_LINK_H
