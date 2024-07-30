/*
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2007 Sebastian Pipping <webmaster@hartwork.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_MATCH_H
#define KATE_MATCH_H

#include <memory>

#include <ktexteditor/document.h>
#include <ktexteditor/movingrange.h>

namespace KTextEditor
{
class DocumentPrivate;
}

class KateMatch
{
public:
    KateMatch(KTextEditor::DocumentPrivate *document, KTextEditor::SearchOptions options);
    KTextEditor::Range searchText(KTextEditor::Range range, const QString &pattern);
    KTextEditor::Range replace(const QString &replacement, bool blockMode, int replacementCounter = 1);
    bool isValid() const;
    bool isEmpty() const;
    KTextEditor::Range range() const;

private:
    /**
     * Resolve references and escape sequences.
     */
    QString buildReplacement(const QString &replacement, bool blockMode, int replacementCounter) const;

private:
    KTextEditor::DocumentPrivate *const m_document;
    const KTextEditor::SearchOptions m_options;
    QList<KTextEditor::Range> m_resultRanges;

    /**
     * moving range to track replace changes
     * kept for later reuse
     */
    std::unique_ptr<KTextEditor::MovingRange> m_afterReplaceRange;
};

#endif // KATE_MATCH_H
