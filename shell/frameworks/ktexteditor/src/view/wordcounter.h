/*
    SPDX-FileCopyrightText: 2015 Michal Humpula <michal.humpula@hudrydum.cz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WORDCOUNTER_H
#define WORDCOUNTER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <vector>

namespace KTextEditor
{
class Document;
class View;
class ViewPrivate;
class Range;
}

class WordCounter : public QObject
{
    Q_OBJECT

public:
    explicit WordCounter(KTextEditor::ViewPrivate *view);

Q_SIGNALS:
    void changed(int wordsInDocument, int wordsInSelection, int charsInDocument, int charsInSelection);

private Q_SLOTS:
    void textInserted(KTextEditor::Document *document, KTextEditor::Range range);
    void textRemoved(KTextEditor::Document *document, KTextEditor::Range range, const QString &oldText);
    void recalculate(KTextEditor::Document *document);
    void selectionChanged(KTextEditor::View *view);
    void recalculateLines();

private:
    std::vector<int> m_countByLine;
    int m_wordsInDocument, m_wordsInSelection;
    int m_charsInDocument, m_charsInSelection;
    QTimer m_timer;
    int m_startRecalculationFrom;
    KTextEditor::Document *m_document;
};

#endif
