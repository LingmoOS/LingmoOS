/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_TOOLS_H
#define SYNDICATION_TOOLS_H

#include "person.h"
#include "syndication_export.h"

#include <QString>

class QByteArray;
class QString;

namespace Syndication
{
/** date formats supported by date parsers */

enum DateFormat {
    ISODate, /**< ISO 8601 extended format.
              * (date: "2003-12-13",datetime: "2003-12-13T18:30:02.25",
              * datetime with timezone: "2003-12-13T18:30:02.25+01:00")
              */
    RFCDate, /**< RFC 822. (e.g. "Sat, 07 Sep 2002 00:00:01 GMT") */
};

/**
 * parses a date string in ISO 8601 extended format.
 * (date: "2003-12-13",datetime: "2003-12-13T18:30:02.25",
 * datetime with timezone: "2003-12-13T18:30:02.25+01:00")
 *
 * @param str a string in ISO 8601 format
 * @return parsed date in seconds since epoch, 0 if no date could
 * be parsed from the string.
 */
SYNDICATION_EXPORT
uint parseISODate(const QString &str);

/**
 * parses a date string as defined in RFC 822.
 * (Sat, 07 Sep 2002 00:00:01 GMT)
 *
 * @param str a string in RFC 822 format
 * @return parsed date in seconds since epoch, 0 if no date could
 * be parsed from the string.
 */
SYNDICATION_EXPORT
uint parseRFCDate(const QString &str);

/**
 * parses a date string in ISO (see parseISODate()) or RFC 822 (see
 * parseRFCDate()) format.
 * It tries both parsers and returns the first valid parsing result found (or 0
 * otherwise).
 * To speed up parsing, you can give a hint which format you expect.
 * The method will try the corresponding parser first then.
 *
 * @param str a date string
 * @param hint the expected format
 * @return parsed date in seconds since epoch, 0 if no date could
 * be parsed from the string.
 */
SYNDICATION_EXPORT
uint parseDate(const QString &str, DateFormat hint = RFCDate);

/**
 * @internal
 * returns a string representation of a datetime.
 * this is used internally to create debugging output.
 *
 * @param date the date to convert
 * @return string representation of the date, or a null string if
 * @c date is 0
 */
SYNDICATION_EXPORT
QString dateTimeToString(uint date);

/**
 * resolves entities to respective unicode chars.
 *
 * @param str a string
 */
SYNDICATION_EXPORT
QString resolveEntities(const QString &str);
/**
 * replaces the characters &lt; >, &, ", '
 * with &amp;lt; &amp;gt; &amp;amp;, &amp;quot; &amp;apos;.
 * @param str the string to escape
 */
SYNDICATION_EXPORT
QString escapeSpecialCharacters(const QString &str);

/**
 * replaces newlines ("\n") by &lt;br/>
 * @param str string to convert
 */
SYNDICATION_EXPORT
QString convertNewlines(const QString &str);

/**
 * converts a plain text string to HTML
 *
 * @param plainText a string in plain text.
 */
SYNDICATION_EXPORT
QString plainTextToHtml(const QString &plainText);

/**
 * converts a HTML string to plain text
 *
 * @param html string in HTML format
 * @return stripped text
 */
SYNDICATION_EXPORT
QString htmlToPlainText(const QString &html);

/**
 * guesses whether a string contains plain text or HTML
 *
 * @param str the string in unknown format
 * @return @c true if the heuristic thinks it's HTML, @c false
 * if thinks it is plain text
 */
SYNDICATION_EXPORT
bool isHtml(const QString &str);

/**
 * guesses whether a string contains (HTML) markup or not. This
 * implements not an exact check for valid HTML markup, but a
 * simple (and relatively fast) heuristic.
 *
 * @param str the string that might or might not contain markup
 * @return @c true if the heuristic thinks it contains markup, @c false
 * if thinks it is markup-free plain text
 */
SYNDICATION_EXPORT
bool stringContainsMarkup(const QString &str);

/**
 * Ensures HTML formatting for a string.
 * guesses via isHtml() if @c str contains HTML or plain text, and returns
 * plainTextToHtml(str) if it thinks it is plain text, or the unmodified @c str
 * otherwise.
 *
 * @param str a string with unknown content
 * @return string as HTML (as long as the heuristics work)
 */
SYNDICATION_EXPORT
QString normalize(const QString &str);

/**
 * normalizes a string based on feed-wide properties of tag content.
 * It is based on the assumption that all items in a feed encode their
 * title/description content in the same way (CDATA or not, plain text
 * vs. HTML). isCDATA and containsMarkup are determined once by the feed,
 * and then passed to this method.
 *
 * The returned string contains HTML, with special characters &lt;, >,
 * &, ", and ' escaped, and all other entities resolved.
 * Whitespace is collapsed, relevant whitespace is replaced by respective
 * HTML tags (&lt;br/>).
 *
 * @param str a string
 * @param isCDATA whether the feed uses CDATA for the tag @c str was read from
 * @param containsMarkup whether the feed uses HTML markup in the
 *        tag @c str was read from.
 * @return string as HTML (as long as the heuristics work)
 */
SYNDICATION_EXPORT
QString normalize(const QString &str, bool isCDATA, bool containsMarkup);

/**
 * Parses a person object from a string by identifying name and email address
 * in the string. Currently detected variants are:
 * "foo@bar.com", "Foo", "Foo &lt;foo@bar.com>", "foo@bar.com (Foo)".
 *
 * @param str the string to parse the person from.
 * @return a Person object containing the parsed information.
 */
SYNDICATION_EXPORT
PersonPtr personFromString(const QString &str);

/**
 * @internal
 * calculates a hash value for a string
 */
unsigned int calcHash(const QString &str);

/**
 * @internal
 * calculates a hash value for a byte array
 */
unsigned int calcHash(const QByteArray &array);

/**
 * @internal
 * calculates a md5 checksum for a string
 */
QString calcMD5Sum(const QString &str);

//@cond PRIVATE
/**
 * @internal
 * used internally to represent element types
 */
struct ElementType {
    ElementType(const QString &localnamep,
                const QString &nsp = QString()); // implicit

    bool operator==(const ElementType &other) const;

    QString ns;
    QString localname;
};
//@endcond

} // namespace Syndication

#endif // SYNDICATION_TOOLS_H
