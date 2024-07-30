/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RSS2_IMAGE_H
#define SYNDICATION_RSS2_IMAGE_H

#include <syndication/elementwrapper.h>

class QDomElement;
class QString;

namespace Syndication
{
namespace RSS2
{
/**
 * An RSS2 image, used to describe feed logos.
 */
class Image : public ElementWrapper
{
public:
    /**
     * Default constructor, creates a null object, for which isNull() is
     * @c true.
     */
    Image();

    /**
     * Creates an Image object wrapping an @c &lt;image> XML element.
     *
     * @param element The @c &lt;image> element to wrap
     */
    explicit Image(const QDomElement &element);

    /**
     * the URL of a GIF, JPEG or PNG image
     */
    QString url() const;

    /**
     * Describes the image, can be used in the ALT attribute of the
     * HTML @c &lt;img> tag when the channel is rendered in HTML.
     *
     * @return TODO: specify format
     */
    QString title() const;

    /**
     * The URL of the site, when the channel is rendered, the image should
     * be a link to the site.
     */
    QString link() const;

    /**
     * The width of the image. If the feed itself doesn't specify a width,
     * this method returns 88, the default value.
     *
     * @return image width in pixels.
     */
    uint width() const;

    /**
     * The height of the image. If the feed itself doesn't specify a height,
     * this method returns 31, the default value.
     *
     * @return image height in pixels.
     */
    uint height() const;

    /**
     * optional text that can be included in the TITLE attribute of the link
     * formed around the image in HTML rendering.
     *
     * @return TODO: specify format (HTML etc.)
     */
    QString description() const;

    /**
     * Returns a description of the object for debugging purposes.
     *
     * @return debug string
     */
    QString debugInfo() const;
};

} // namespace RSS2
} // namespace Syndication

#endif // SYNDICATION_RSS2_IMAGE_H
