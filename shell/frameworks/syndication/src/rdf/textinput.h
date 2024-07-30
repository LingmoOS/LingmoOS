/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_TEXTINPUT_H
#define SYNDICATION_RDF_TEXTINPUT_H

#include <QString>
#include <syndication/rdf/resourcewrapper.h>

namespace Syndication
{
namespace RDF
{
/**
 * "The textinput element affords a method for submitting form data to an
 * arbitrary URL - usually located at the parent website.
 * The field is typically used as a search box or subscription form"
 *
 * @author Frank Osterfeld
 */
class TextInput : public ResourceWrapper
{
public:
    /**
     * creates an object wrapping a null resource.
     * isNull() is @p true.
     */
    TextInput();

    /**
     * creates a text input object wrapping a rss:textinput resource
     *
     * @param resource resource to wrap, must be of type
     * rss:textinput, otherwise this object will not return
     * useful information.
     */
    explicit TextInput(ResourcePtr resource);

    /**
     * destructor
     */
    ~TextInput() override;

    /**
     * A descriptive title for the textinput field. For example: "Subscribe"
     * or "Search!"
     *
     * @return title of the text input, or a null string if not specified
     */
    QString title() const;

    /**
     * A brief description of the textinput field's purpose. For example:
     * "Subscribe to our newsletter for..." or "Search our site's archive
     * of..."
     *
     *
     * @return description TODO: specify format (HTML/plain text), or
     * a null string if not specified.
     */
    QString description() const;

    /**
     * The URL to which a textinput submission will be directed (using GET).
     *
     * @return URL, or a null string if not specified
     */
    QString link() const;

    /**
     * The text input field's (variable) name.
     *
     * @return name, or a null string if not specified
     */
    QString name() const;

    /**
     * Returns a description of the text input for debugging purposes.
     *
     * @return debug string
     */
    QString debugInfo() const;
};

} // namespace RDF
} // namespace Syndication

#endif //  SYNDICATION_RDF_TEXTINPUT_H
