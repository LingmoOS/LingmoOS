/*
    SPDX-FileCopyrightText: 2015 Michal Humpula <michal.humpula@hudrydum.cz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "wordcounter.h"
#include "katedocument.h"
#include "kateview.h"

WordCounter::WordCounter(KTextEditor::ViewPrivate *view)
    : QObject(view)
    , m_wordsInDocument(0)
    , m_wordsInSelection(0)
    , m_charsInDocument(0)
    , m_charsInSelection(0)
    , m_startRecalculationFrom(0)
    , m_document(view->document())
{
    connect(view->doc(), &KTextEditor::DocumentPrivate::textInsertedRange, this, &WordCounter::textInserted);
    connect(view->doc(), &KTextEditor::DocumentPrivate::textRemoved, this, &WordCounter::textRemoved);
    connect(view->doc(), &KTextEditor::DocumentPrivate::loaded, this, &WordCounter::recalculate);
    connect(view, &KTextEditor::View::selectionChanged, this, &WordCounter::selectionChanged);

    m_timer.setInterval(500);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &WordCounter::recalculateLines);

    recalculate(m_document);
}

void WordCounter::textInserted(KTextEditor::Document *, KTextEditor::Range range)
{
    auto startLine = m_countByLine.begin() + range.start().line();
    auto endLine = m_countByLine.begin() + range.end().line();
    size_t newLines = std::distance(startLine, endLine);

    if (m_countByLine.empty()) { // was empty document before insert
        newLines++;
    }

    if (newLines > 0) {
        m_countByLine.insert(startLine, newLines, -1);
    }

    m_countByLine[range.end().line()] = -1;
    m_timer.start();
}

void WordCounter::textRemoved(KTextEditor::Document *, KTextEditor::Range range, const QString &)
{
    const auto startLine = m_countByLine.begin() + range.start().line();
    const auto endLine = m_countByLine.begin() + range.end().line();
    const int removedLines = endLine - startLine;

    if (removedLines > 0) {
        m_countByLine.erase(startLine, endLine);
    }

    if (!m_countByLine.empty()) {
        m_countByLine[range.start().line()] = -1;
        m_timer.start();
    } else {
        Q_EMIT changed(0, 0, 0, 0);
    }
}

void WordCounter::recalculate(KTextEditor::Document *)
{
    m_countByLine = std::vector<int>(m_document->lines(), -1);
    m_timer.start();
}

static int countWords(const QString &text)
{
    int count = 0;
    bool inWord = false;

    for (const QChar c : text) {
        if (c.isLetterOrNumber()) {
            if (!inWord) {
                inWord = true;
            }
        } else {
            if (inWord) {
                inWord = false;
                count++;
            }
        }
    }

    return inWord ? count + 1 : count;
}

void WordCounter::selectionChanged(KTextEditor::View *view)
{
    if (view->selectionRange().isEmpty()) {
        m_wordsInSelection = m_charsInSelection = 0;
        Q_EMIT changed(m_wordsInDocument, 0, m_charsInDocument, 0);
        return;
    }

    const int firstLine = view->selectionRange().start().line();
    const int lastLine = view->selectionRange().end().line();

    if (firstLine == lastLine || view->blockSelection()) {
        const QString text = view->selectionText();
        m_wordsInSelection = countWords(text);
        m_charsInSelection = text.size();
    } else {
        m_wordsInSelection = m_charsInSelection = 0;

        const KTextEditor::Range firstLineRange(view->selectionRange().start(), firstLine, view->document()->lineLength(firstLine));
        const QString firstLineText = view->document()->text(firstLineRange);
        m_wordsInSelection += countWords(firstLineText);
        m_charsInSelection += firstLineText.size();

        // whole lines
        for (int i = firstLine + 1; i < lastLine; i++) {
            m_wordsInSelection += m_countByLine[i];
            m_charsInSelection += m_document->lineLength(i);
        }

        const KTextEditor::Range lastLineRange(KTextEditor::Cursor(lastLine, 0), view->selectionRange().end());
        const QString lastLineText = view->document()->text(lastLineRange);
        m_wordsInSelection += countWords(lastLineText);
        m_charsInSelection += lastLineText.size();
    }

    Q_EMIT changed(m_wordsInDocument, m_wordsInSelection, m_charsInDocument, m_charsInSelection);
}

void WordCounter::recalculateLines()
{
    if ((size_t)m_startRecalculationFrom >= m_countByLine.size()) {
        m_startRecalculationFrom = 0;
    }

    int wordsCount = 0;
    int charsCount = 0;
    int calculated = 0;
    size_t i = m_startRecalculationFrom;
    constexpr int MaximumLinesToRecalculate = 100;

    // stay in bounds, vector might be empty, even 0 is too large then
    while (i < m_countByLine.size()) {
        if (m_countByLine[i] == -1) {
            m_countByLine[i] = countWords(m_document->line(i));
            if (++calculated > MaximumLinesToRecalculate) {
                m_startRecalculationFrom = i;
                m_timer.start();
                return;
            }
        }

        wordsCount += m_countByLine[i];
        charsCount += m_document->lineLength(i);

        if (++i == m_countByLine.size()) { // array cycle
            i = 0;
        }

        if (i == (size_t)m_startRecalculationFrom) {
            break;
        }
    }

    m_wordsInDocument = wordsCount;
    m_charsInDocument = charsCount;
    Q_EMIT changed(m_wordsInDocument, m_wordsInSelection, m_charsInDocument, m_charsInSelection);
}

#include "moc_wordcounter.cpp"
