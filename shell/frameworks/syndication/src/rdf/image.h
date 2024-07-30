/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_IMAGE_H
#define SYNDICATION_RDF_IMAGE_H

#include <QString>
#include <syndication/rdf/resourcewrapper.h>

namespace Syndication
{
namespace RDF
{
class Resource;
//@cond PRIVATE
typedef QSharedPointer<Resource> ResourcePtr;
//@endcond

/**
 * An image to be associated with an HTML rendering of the channel.
 */
class Image : public ResourceWrapper
{
public:
    /**
     * creates a wrapper wrapping a null resource
     */
    Image();

    /**
     * creates an image object by wrapping an image resource
     *
     * @param resource the image resource to wrap
     */
    explicit Image(ResourcePtr resource);

    /**
     * destructor
     */
    ~Image() override;

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
     * the URL of the image file
     */
    QString url() const;

    /**
     * Returns a description of the image for debugging purposes.
     *
     * @return debug string
     */
    QString debugInfo() const;
};

} // namespace RDF
} // namespace Syndication

#endif //  SYNDICATION_RDF_IMAGE_H
