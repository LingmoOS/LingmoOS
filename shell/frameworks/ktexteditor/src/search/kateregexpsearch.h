/*
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2007 Sebastian Pipping <webmaster@hartwork.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KATE_REGEXPSEARCH_H_
#define _KATE_REGEXPSEARCH_H_

#include <QObject>
#include <QRegularExpression>

#include <ktexteditor/range.h>

#include <ktexteditor_export.h>

namespace KTextEditor
{
class Document;
}

/**
 * Object to help to search for regexp.
 * This should be NO QObject, it is created to often!
 * I measured that, if you create it 20k times to replace for example " " in a document, that takes seconds on a modern machine!
 */
class KTEXTEDITOR_EXPORT KateRegExpSearch
{
public:
    explicit KateRegExpSearch(const KTextEditor::Document *document);
    ~KateRegExpSearch() = default;

    //
    // KTextEditor::SearchInterface stuff
    //
public:
    /**
     * Search for the regular expression \p pattern inside the range
     * \p inputRange. If \p backwards is \e true, the search direction will
     * be reversed. \p options is a set of QRegularExpression::PatternOptions
     * OR flags that control certain aspects of the search, e.g. case
     * sensitivity and if the dot "." metacharacter matches any character
     * including a newline.
     *
     * Note: Unicode support is always enabled (QRegularExpression::UseUnicodePropertiesOption).
     * If the pattern is multi-line the QRegularExpression::MultilineOption is enabled.
     *
     * \param pattern text to search for
     * \param inputRange Range to search in
     * \param backwards if \e true, the search will be backwards
     * \param options QRegularExpression pattern options, we will internally add QRegularExpression::UseUnicodePropertiesOption
     * \return Vector of ranges, one for each capture group. The first range (index
     *         zero) spans the whole match. If no matches are found, the vector will
     *         contain one element, an invalid range (see Range::isValid()).
     * \see KTextEditor::Range, QRegularExpression
     */
    QList<KTextEditor::Range> search(const QString &pattern,
                                     KTextEditor::Range inputRange,
                                     bool backwards = false,
                                     QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption);

    /**
     * Returns a modified version of text where escape sequences are resolved, e.g. "\\n" to "\n".
     *
     * \param text text containing escape sequences
     * \return text with resolved escape sequences
     */
    static QString escapePlaintext(const QString &text);

    /**
     * Returns a modified version of text where
     * \li escape sequences are resolved, e.g. "\\n" to "\n",
     * \li references are resolved, e.g. "\\1" to <i>1st entry in capturedTexts</i>, and
     * \li counter sequences are resolved, e.g. "\\#...#" to <i>replacementCounter</i>.
     *
     * \param text text containing escape sequences, references, and counter sequences
     * \param capturedTexts list of substitutes for references
     * \param replacementCounter value for replacement counter
     * \return resolved text
     */
    static QString buildReplacement(const QString &text, const QStringList &capturedTexts, int replacementCounter);

private:
    /**
     * Implementation of escapePlainText() and public buildReplacement().
     *
     * \param text text containing escape sequences and possibly references and counters
     * \param capturedTexts list of substitutes for references
     * \param replacementCounter value for replacement counter (only used when replacementGoodies == true)
     * \param replacementGoodies @c true for buildReplacement(), @c false for escapePlainText()
     * \return resolved text
     */
    KTEXTEDITOR_NO_EXPORT
    static QString buildReplacement(const QString &text, const QStringList &capturedTexts, int replacementCounter, bool replacementGoodies);

    /**
     * Checks the pattern for special characters and escape sequences that can
     * make a match span multiple lines; if any are found, \p stillMultiLine is
     * set to true.
     *
     * "\s" is treated specially so that it doesn't match new line characters; this
     * is achieved by replacing any occurences of "\s" with "[ \t]" in the search
     * \p pattern.
     *
     * \param pattern the regular expression search pattern
     * \param stillMultiLine is set to \c true if the search pattern may still match
     * multiple lines even after replacing "\s" with "[ \t]"; otherwise it's set to false
     * \return the modified pattern
     */
    KTEXTEDITOR_NO_EXPORT
    static QString repairPattern(const QString &pattern, bool &stillMultiLine);

private:
    const KTextEditor::Document *const m_document;
    class ReplacementStream;
};

#endif
