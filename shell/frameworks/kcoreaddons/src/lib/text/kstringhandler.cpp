/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Ian Zepp <icszepp@islc.net>
    SPDX-FileCopyrightText: 2006 Dominic Battre <dominic@battre.de>
    SPDX-FileCopyrightText: 2006 Martin Pool <mbp@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kstringhandler.h"

#include <stdlib.h> // random()

#include <QList>
#include <QRegularExpression>

//
// Capitalization routines
//
QString KStringHandler::capwords(const QString &text)
{
    if (text.isEmpty()) {
        return text;
    }

    const QString strippedText = text.trimmed();
    const QString space = QString(QLatin1Char(' '));
    const QStringList words = capwords(strippedText.split(space));

    QString result = text;
    result.replace(strippedText, words.join(space));
    return result;
}

QStringList KStringHandler::capwords(const QStringList &list)
{
    QStringList tmp = list;
    for (auto &str : tmp) {
        str[0] = str.at(0).toUpper();
    }
    return tmp;
}

QString KStringHandler::lsqueeze(const QString &str, const int maxlen)
{
    if (str.length() > maxlen) {
        const int part = maxlen - 3;
        return QLatin1String("...") + QStringView(str).right(part);
    } else {
        return str;
    }
}

QString KStringHandler::csqueeze(const QString &str, const int maxlen)
{
    if (str.length() > maxlen && maxlen > 3) {
        const int part = (maxlen - 3) / 2;
        const QStringView strView{str};
        return strView.left(part) + QLatin1String("...") + strView.right(part);
    } else {
        return str;
    }
}

QString KStringHandler::rsqueeze(const QString &str, const int maxlen)
{
    if (str.length() > maxlen) {
        const int part = maxlen - 3;
        return QStringView(str).left(part) + QLatin1String("...");
    } else {
        return str;
    }
}

QStringList KStringHandler::perlSplit(const QStringView sep, const QStringView str, int max)
{
    const bool ignoreMax = max == 0;

    const int sepLength = sep.size();

    QStringList list;
    int searchStart = 0;
    int sepIndex = str.indexOf(sep, searchStart);

    while (sepIndex != -1 && (ignoreMax || list.count() < max - 1)) {
        const auto chunk = str.mid(searchStart, sepIndex - searchStart);
        if (!chunk.isEmpty()) {
            list.append(chunk.toString());
        }

        searchStart = sepIndex + sepLength;
        sepIndex = str.indexOf(sep, searchStart);
    }

    const auto lastChunk = str.mid(searchStart, str.length() - searchStart);
    if (!lastChunk.isEmpty()) {
        list.append(lastChunk.toString());
    }

    return list;
}

QStringList KStringHandler::perlSplit(const QString &sep, const QString &s, int max)
{
    return perlSplit(QStringView(sep), QStringView(s), max);
}

QStringList KStringHandler::perlSplit(const QChar &sep, const QString &str, int max)
{
    return perlSplit(QStringView(&sep, 1), QStringView(str), max);
}

QStringList KStringHandler::perlSplit(const QRegularExpression &sep, const QString &str, int max)
{
    // nothing to split
    if (str.isEmpty()) {
        return QStringList();
    }

    const bool ignoreMax = max == 0;

    QStringList list;

    int start = 0;

    const QStringView strView(str);

    QRegularExpression separator(sep);
    separator.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);

    QRegularExpressionMatchIterator iter = separator.globalMatchView(strView);
    QRegularExpressionMatch match;
    while (iter.hasNext() && (ignoreMax || list.count() < max - 1)) {
        match = iter.next();
        const QStringView chunk = strView.mid(start, match.capturedStart() - start);
        if (!chunk.isEmpty()) {
            list.append(chunk.toString());
        }

        start = match.capturedEnd();
    }

    // catch the remainder
    const QStringView lastChunk = strView.mid(start, strView.size() - start);
    if (!lastChunk.isEmpty()) {
        list.append(lastChunk.toString());
    }

    return list;
}

QString KStringHandler::tagUrls(const QString &text)
{
    QString richText(text);

    static const QRegularExpression urlEx(QStringLiteral(R"((www\.(?!\.)|(fish|ftp|http|https)://[\d\w./,:_~?=&;#@\-+%$()]+))"),
                                          QRegularExpression::UseUnicodePropertiesOption);
    // The reference \1 is going to be replaced by the matched url
    richText.replace(urlEx, QStringLiteral("<a href=\"\\1\">\\1</a>"));
    return richText;
}

QString KStringHandler::obscure(const QString &str)
{
    QString result;
    for (const QChar ch : str) {
        // yes, no typo. can't encode ' ' or '!' because
        // they're the unicode BOM. stupid scrambling. stupid.
        const ushort uc = ch.unicode();
        result += (uc <= 0x21) ? ch : QChar(0x1001F - uc);
    }

    return result;
}

static inline bool containsSpaces(const QString &text)
{
    for (int i = 0; i < text.length(); i++) {
        const QChar c = text[i];
        if (c.isSpace()) {
            return true;
        }
    }
    return false;
}

QString KStringHandler::preProcessWrap(const QString &text)
{
    const QChar zwsp(0x200b);

    QString result;
    result.reserve(text.length());

    const bool containsSpaces = ::containsSpaces(text);

    for (int i = 0; i < text.length(); i++) {
        const QChar c = text[i];

        const bool openingParens = (c == QLatin1Char('(') || c == QLatin1Char('{') || c == QLatin1Char('['));
        const bool singleQuote = (c == QLatin1Char('\''));
        const bool closingParens = (c == QLatin1Char(')') || c == QLatin1Char('}') || c == QLatin1Char(']'));
        const bool breakAfter = (closingParens || c.isPunct() || c.isSymbol());
        const bool isLastChar = i == (text.length() - 1);
        const bool isLower = c.isLower();
        const bool nextIsUpper = !isLastChar && text[i + 1].isUpper(); // false by default
        const bool nextIsSpace = isLastChar || text[i + 1].isSpace(); // true by default
        const bool prevIsSpace = (i == 0 || text[i - 1].isSpace() || result[result.length() - 1] == zwsp);

        // Provide a breaking opportunity before opening parenthesis
        if (openingParens && !prevIsSpace) {
            result += zwsp;
        }

        // Provide a word joiner before the single quote
        if (singleQuote && !prevIsSpace) {
            result += QChar(0x2060);
        }

        result += c;

        // Provide a breaking opportunity between camelCase and PascalCase sub-words;
        // but if source string contains whitespaces, then it should be sufficiently wrappable on its own
        const bool isCamelCase = !containsSpaces && isLower && nextIsUpper;

        if (isCamelCase || (breakAfter && !openingParens && !nextIsSpace && !singleQuote)) {
            result += zwsp;
        }
    }

    return result;
}

int KStringHandler::logicalLength(const QString &text)
{
    int length = 0;
    const auto chrs = text.toUcs4();
    for (const auto chr : chrs) {
        const auto script = QChar::script(chr);
        /* clang-format off */
        if (script == QChar::Script_Han
            || script == QChar::Script_Hangul
            || script == QChar::Script_Hiragana
            || script == QChar::Script_Katakana
            || script == QChar::Script_Yi
            || QChar::isHighSurrogate(chr)) { /* clang-format on */
            length += 2;
        } else {
            length += 1;
        }
    }
    return length;
}
