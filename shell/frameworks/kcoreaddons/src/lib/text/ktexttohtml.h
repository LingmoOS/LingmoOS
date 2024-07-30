/*
    SPDX-FileCopyrightText: 2002 Dave Corrie <kde@davecorrie.com>
    SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOREADDONS_KTEXTTOHTML_H
#define KCOREADDONS_KTEXTTOHTML_H

#include <kcoreaddons_export.h>

#include <QString>

/**
 * @author Dave Corrie \<kde@davecorrie.com\>
 */
namespace KTextToHTML
{
/**
 * @see Options
 * @since 5.5.0
 */
enum Option {
    /**
     * Preserve white-space formatting of the text
     */
    PreserveSpaces = 1 << 1,

    /**
     * Replace text emoticons smileys by emoticons images.
     */
    ReplaceSmileys = 1 << 2,

    /**
     * Don't parse and replace any URLs.
     */
    IgnoreUrls = 1 << 3,

    /**
     * Interpret text highlighting markup, like *bold*, _underline_ and /italic/,
     * and wrap them in corresponding HTML entities.
     */
    HighlightText = 1 << 4,

    /**
     * Replace phone numbers with tel: links.
     * @since 5.56.0
     */
    ConvertPhoneNumbers = 1 << 5,
};
/**
 * Stores a combination of #Option values.
 */
Q_DECLARE_FLAGS(Options, Option)
Q_DECLARE_OPERATORS_FOR_FLAGS(Options)

/**
 * Converts plaintext into html. The following characters are converted
 * to HTML entities: & " < >. Newlines are also preserved.
 *
 * @param  plainText  The text to be converted into HTML.
 * @param  options    The options to use when processing @p plainText.
 * @param  maxUrlLen  The maximum length of permitted URLs. The reason for
 *                    this limit is that there may be possible security
 *                    implications in handling URLs of unlimited length.
 * @param  maxAddressLen  The maximum length of permitted email addresses.
 *                    The reason for this limit is that there may be possible
 *                    security implications in handling addresses of unlimited
 *                    length.
 *
 * @return An HTML version of the text supplied in the 'plainText'
 * parameter, suitable for inclusion in the BODY of an HTML document.
 *
 * @since 5.5.0
 */
KCOREADDONS_EXPORT QString convertToHtml(const QString &plainText, const KTextToHTML::Options &options, int maxUrlLen = 4096, int maxAddressLen = 255);

}

#endif
