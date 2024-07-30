/*
    SPDX-FileCopyrightText: 2002-2008 The Kopete developers <kopete-devel@kde.org>
    SPDX-FileCopyrightText: 2008 Carlo Segato <brandon.ml@gmail.com>
    SPDX-FileCopyrightText: 2002-2003 Stefan Gehn <metz@gehn.net>
    SPDX-FileCopyrightText: 2005 Engin AYDOGAN <engin@bzzzt.biz>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kemoticonsparser_p.h"

#include <QDebug>
#include <QString>

#include <cstring>

// ### keep sorted by first column and HTML entity-encoded!
struct Emoticon {
    const char *match;
    const char *replacement;
};
// clang-format off
static constexpr const Emoticon emoticons_map[] = {
    {"&gt;-(", "😠"},
    {"&gt;:(", "😠"},
    {"&gt;:)", "😈"},
    {"&gt;:-(", "😠"},
    {"&gt;w&lt;", "😟"},
    {"&lt;-.-&gt;", "😴"},
    {"&lt;3", "♥️"},
    {"&lt;]:o){", "🤡"},
    {"&lt;|:^0|", "🤡"},
    {"()-()", "🤓"},
    {"(-_o)zzZ", "😴"},
    {"(:|", "🥱"},
    {"(@_@)", "😕"},
    {"(c:&gt;*", "🤡"},
    {"({)", "🤗"},
    {"(})", "🤗"},
    {"*&lt;:^)", "🤡"},
    {"*&lt;:o)", "🤡"},
    {"*:o)", "🤡"},
    {"*:oB", "🤡"},
    {"*:oP", "🤡"},
    {"+o(", "🤢"},
    {",':(", "😕"},
    {"-_-", "😴"},
    {"-_-+", "😠"},
    {"-o-o-", "🤓"},
    {"/00\\", "😟"},
    {"0:)", "😇"},
    {"0:-)", "😇"},
    {"0;)", "😇"},
    {"0=)", "😇"},
    {"3:)", "😈"},
    {"8)", "😎"},
    {"8-)", "😎"},
    {"8:::(", "😭"},
    {":\"-(", "😢"},
    {":'(", "😢"},
    {":'-(", "😢"},
    {":'D", "😆"},
    {":(", "🙁"},
    {":((", "😢"},
    {":)", "🙂"},
    {":))", "😆"},
    {":*", "😗"},
    {":*(", "😢"},
    {":*)", "😗"},
    {":-$", "😯"},
    {":-&amp;", "🤢"},
    {":-&gt;", "☺️"},
    {":-&gt;&gt;", "☺️"},
    {":-(", "🙁"},
    {":-)", "🙂"},
    {":-))", "😀"},
    {":-)*", "😗"},
    {":-*", "😗"},
    {":-/", "😕"},
    {":-@", "😠"},
    {":-D", "😀"},
    {":-O", "😮"},
    {":-P", "😛"},
    {":-Q", "😕"},
    {":-S", "😕"},
    {":-X", "🤫"},
    {":-[", "😯"},
    {":-o", "😮"},
    {":-p", "😛"},
    {":-s", "😕"},
    {":-t", "😛"},
    {":-x", "🤫"},
    {":-|", "😐"},
    {":-||", "😠"},
    {":/", "🫤"},
    {":@", "😠"},
    {":C", "☹️"},
    {":D", "😀"},
    {":O", "😮"},
    {":P", "😛"},
    {":S", "😕"},
    {":X", "🤫"},
    {":\\", "🫤"},
    {":_(", "😢"},
    {":c", "☹️"},
    {":o", "😮"},
    {":o)", "🤡"},
    {":p", "😛"},
    {":s", "😕"},
    {":x", "🤫"},
    {":|))", "😀"},
    {";(", "😢"},
    {";)", "😉"},
    {";-(!)", "😗"},
    {";-(", "😢"},
    {";-)", "😉"},
    {";_;", "😢"},
    {"= #", "😗"},
    {"='(", "😢"},
    {"=(", "🙁"},
    {"=[", "🙁"},
    {"=^D", "😆"},
    {"B-)", "😎"},
    {"D:", "🙁"},
    {"D=", "🙁"},
    {"O-)", "😇"},
    {"O.o", "🤔"},
    {"O.o?", "🤔"},
    {"O:)", "😇"},
    {"O:-)", "😇"},
    {"O;", "😇"},
    {"T.T", "🙁"},
    {"T_T", "😭"},
    {"X-(", "😠"},
    {"Y_Y", "🙁"},
    {"Z_Z", "😴"},
    {"\\o-o/", "🤓"},
    {"\\~/", "🤓"},
    {"]:-&gt;", "😈"},
    {"^j^", "😇"},
    {"i_i", "😭"},
    {"t.t", "🙁"},
    {"y_y", "🙁"},
    {"|-O", "🥱"},
    {"}:-)", "😈"},
};
// clang-format on

static const Emoticon *findEmoticon(QStringView s)
{
    auto it = std::lower_bound(std::begin(emoticons_map), std::end(emoticons_map), s, [](const auto &emoticon, auto s) {
        return QLatin1String(emoticon.match) < s;
    });
    if (it != std::end(emoticons_map) && s.startsWith(QLatin1String((*it).match))) {
        return it;
    }
    // if we don't have an exact match but a prefix, that will be in the item before the one returned by lower_bound
    if (it != std::begin(emoticons_map)) {
        it = std::prev(it);
        if (s.startsWith(QLatin1String((*it).match))) {
            return it;
        }
    }
    return nullptr;
}

QString KEmoticonsParser::parseEmoticons(const QString &message)
{
    QString result;

    /* previous char, in the firs iteration assume that it is space since we want
     * to let emoticons at the beginning, the very first previous QChar must be a space. */
    QChar p = QLatin1Char(' ');

    int pos = 0;
    int previousPos = 0;

    bool inHTMLTag = false;
    bool inHTMLLink = false;
    bool inHTMLEntity = false;

    for (; pos < message.length(); ++pos) {
        const QChar c = message[pos];

        if (!inHTMLTag) { // Are we already in an HTML tag ?
            if (c == QLatin1Char('<')) { // If not check if are going into one
                inHTMLTag = true; // If we are, change the state to inHTML
                p = c;
                continue;
            }
        } else { // We are already in a HTML tag
            if (c == QLatin1Char('>')) { // Check if it ends
                inHTMLTag = false; // If so, change the state

                if (p == QLatin1Char('a')) {
                    inHTMLLink = false;
                }
            } else if (c == QLatin1Char('a') && p == QLatin1Char('<')) { // check if we just entered an anchor tag
                inHTMLLink = true; // don't put smileys in urls
            }
            p = c;
            continue;
        }

        if (!inHTMLEntity) { // are we
            if (c == QLatin1Char('&')) {
                inHTMLEntity = true;
            }
        }

        if (inHTMLLink) { // i can't think of any situation where a link address might need emoticons
            p = c;
            continue;
        }

        if (!p.isSpace() && p != QLatin1Char('>')) { // '>' may mark the end of an html tag
            p = c;
            continue;
        } /* strict requires space before the emoticon */

        const auto emoticon = findEmoticon(QStringView(message).mid(pos));
        if (emoticon) {
            bool found = true;
            /* check if the character after this match is space or end of string*/
            const int matchLen = std::strlen(emoticon->match);
            if (message.length() > pos + matchLen) {
                const QChar n = message[pos + matchLen];
                //<br/> marks the end of a line
                if (n != QLatin1Char('<') && !n.isSpace() && !n.isNull() && n != QLatin1Char('&')) {
                    found = false;
                }
            }

            if (found) {
                result += QStringView(message).mid(previousPos, pos - previousPos);
                result += QString::fromUtf8(emoticon->replacement);

                /* Skip the matched emoticon's matchText */
                pos += matchLen - 1;
                previousPos = pos + 1;
            } else {
                if (inHTMLEntity) {
                    // If we are in an HTML entity such as &gt;
                    const int htmlEnd = message.indexOf(QLatin1Char(';'), pos);
                    // Search for where it ends
                    if (htmlEnd == -1) {
                        // Apparently this HTML entity isn't ended, something is wrong, try skip the '&'
                        // and continue
                        // qCDebug(KEMOTICONS_CORE) << "Broken HTML entity, trying to recover.";
                        inHTMLEntity = false;
                        pos++;
                    } else {
                        pos = htmlEnd;
                        inHTMLEntity = false;
                    }
                }
            }
        } /* else no emoticons begin with this character, so don't do anything */
        p = c;
    }

    if (result.isEmpty()) {
        return message;
    }
    if (previousPos < message.length()) {
        result += QStringView(message).mid(previousPos);
    }
    return result;
}
