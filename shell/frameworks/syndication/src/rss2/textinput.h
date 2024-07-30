/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RSS2_TEXTINPUT_H
#define SYNDICATION_RSS2_TEXTINPUT_H

#include <syndication/elementwrapper.h>

class QString;
class QDomElement;

namespace Syndication
{
namespace RSS2
{
/**
 * "The purpose of the &lt;textInput> element is something of a mystery.
 * You can use it to specify a search engine box. Or to allow a
 * reader to provide feedback. Most aggregators ignore it."
 *
 * @author Frank Osterfeld
 */
class TextInput : public ElementWrapper
{
public:
    /**
     * Default constructor, creates a null object, for which isNull() is
     * @c true.
     */
    TextInput();

    /**
     * Creates a TextInput object wrapping a @c &lt;textInput> XML element.
     *
     * @param element The @c &lt;textInput> element to wrap
     */
    explicit TextInput(const QDomElement &element);

    /**
     * The label of the Submit button in the text input area.
     *
     * @return TODO
     */
    QString title() const;

    /**
     * The name of the text object in the text input area.
     *
     * @return TODO
     */
    QString name() const;

    /**
     * Explains the text input area.
     *
     * @return TODO
     */
    QString description() const;

    /**
     * The URL of the CGI script that processes text input requests.
     *
     * @return TODO
     */
    QString link() const;

    /**
     * Returns a description of the object for debugging purposes.
     *
     * @return debug string
     */
    QString debugInfo() const;
};

} // namespace RSS2
} // namespace Syndication

#endif // SYNDICATION_RSS2_TEXTINPUT_H
