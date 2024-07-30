/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RSS2_SOURCE_H
#define SYNDICATION_RSS2_SOURCE_H

#include <syndication/elementwrapper.h>

class QDomElement;
class QString;

namespace Syndication
{
namespace RSS2
{
/**
 * Class representing the @c &lt;source&gt; element in RSS2.
 * "The purpose of this element is to propagate credit for links, to publicize
 * the sources of news items. It's used in the post command in the Radio
 * UserLand aggregator. It should be generated automatically when forwarding an
 * item from an aggregator to a weblog authoring tool."
 *
 * @author Frank Osterfeld
 */
class Source : public ElementWrapper
{
public:
    /**
     * Default constructor, creates a null object, for which isNull() is
     * @c true.
     */
    Source();

    /**
     * Creates a source object wrapping a @c &lt;source&gt; XML element.
     *
     * @param element The @c &lt;source&gt; element to wrap
     */
    explicit Source(const QDomElement &element);

    /**
     * The name of the RSS channel that the item came from, derived from
     * its &lt;title>. Example: "Tomalak's Realm"
     *
     * @return A string containing the source, or a null string if not set
     * (and for null objects)
     */
    QString source() const;

    /**
     * Required attribute, links to the XMLization of the source.
     *
     * @return A URL, or a null string if not set (though required from
     * the spec), and for null objects
     */
    QString url() const;

    /**
     * Returns a description of the object for debugging purposes.
     *
     * @return debug string
     */
    QString debugInfo() const;
};

} // namespace RSS2
} // namespace Syndication

#endif // SYNDICATION_RSS2_SOURCE_H
