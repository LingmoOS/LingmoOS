/*
    SPDX-FileCopyrightText: 2009-2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2007 Sebastian Pipping <webmaster@hartwork.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KATE_PLAINTEXTSEARCH_H_
#define _KATE_PLAINTEXTSEARCH_H_

#include <QObject>

#include <ktexteditor/range.h>

#include <ktexteditor_export.h>

namespace KTextEditor
{
class Document;
}

/**
 * Object to help to search for plain text.
 * This should be NO QObject, it is created too often!
 * I measured that, if you create it 20k times to replace for example " " in a document, that takes seconds on a modern machine!
 */
class KTEXTEDITOR_EXPORT KatePlainTextSearch
{
public:
    explicit KatePlainTextSearch(const KTextEditor::Document *document, Qt::CaseSensitivity caseSensitivity, bool wholeWords);
    ~KatePlainTextSearch() = default;

public:
    /**
     * Search for the given \p text inside the range \p inputRange taking
     * into account whether to search \p casesensitive and \p backwards.
     *
     * \param text text to search for
     * \param inputRange Range to search in
     * \param backwards if \e true, the search will be backwards
     * \return The valid range of the matched text if \p text was found. If
     *        the \p text was not found, the returned range is not valid
     *        (see Range::isValid()).
     * \see KTextEditor::Range
     */
    KTextEditor::Range search(const QString &text, KTextEditor::Range inputRange, bool backwards = false);

private:
    const KTextEditor::Document *m_document;
    Qt::CaseSensitivity m_caseSensitivity;
    bool m_wholeWords;
};

#endif
