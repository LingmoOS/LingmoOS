// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deletebackcommond.h"

#include <QTextBlock>

#include "dtextedit.h"

DeleteBackCommand::DeleteBackCommand(QTextCursor cursor, QPlainTextEdit *edit)
    : m_cursor(cursor)
    , m_edit(edit)
{
    m_delText = m_cursor.selectedText();
    m_delPos = std::min(m_cursor.position(), m_cursor.anchor());
    m_insertPos = m_delPos;
}

DeleteBackCommand::~DeleteBackCommand() {}

void DeleteBackCommand::undo()
{
    m_cursor.setPosition(m_insertPos);
    m_cursor.insertText(m_delText);

    QTextCursor cursor = m_edit->textCursor();
    cursor.setPosition(m_insertPos);
    cursor.setPosition(m_insertPos + m_delText.size(), QTextCursor::KeepAnchor);
    m_edit->setTextCursor(cursor);
}

void DeleteBackCommand::redo()
{
    m_cursor.setPosition(m_delPos);
    m_cursor.setPosition(m_delPos + m_delText.size(), QTextCursor::KeepAnchor);
    m_cursor.deleteChar();

    // restore cursor position
    m_edit->setTextCursor(m_cursor);
}

/**
   @brief Delete column selection with 'Backsapce' or 'Delete'
        The \a backward is default delete character direction (if not selected).
        The 'Backspace' button is forward, and the 'Delete' button is backward.
 */
DeleteBackAltCommand::DeleteBackAltCommand(const QList<QTextEdit::ExtraSelection> &selections,
                                                 TextEdit *edit,
                                                 bool backward)
    : m_columnEditSelections(selections)
    , m_edit(edit)
{
    int sum = 0;
    for (int i = 0; i < m_columnEditSelections.size(); i++) {
        QString text;
        auto cursor = m_columnEditSelections[i].cursor;

        if (!cursor.hasSelection()) {
            // keyboard 'delete' button
            if (backward) {
                if (!cursor.atEnd()) {
                    cursor.setPosition(cursor.position() + 1, QTextCursor::KeepAnchor);
                }

            } else {
                // keyboard 'backspace' button
                if (!cursor.atStart()) {
                    cursor.setPosition(cursor.position() - 1);
                    cursor.setPosition(cursor.position() + 1, QTextCursor::KeepAnchor);
                }
            }
        }

        text = cursor.selectedText();
        if (!text.isEmpty()) {
            int pos = std::min(cursor.anchor(), cursor.position());
            DelNode node;
            node.cursor = cursor;
            node.insertPos = pos;
            node.delPos = pos - sum;
            node.delText = text;
            node.idInColumn = i;
            node.leftToRight = cursor.anchor() < cursor.position();
            m_deletions.push_back(node);

            sum += text.size();
        }
    }
}

DeleteBackAltCommand::~DeleteBackAltCommand() {}

void DeleteBackAltCommand::undo()
{
    for (const DelNode &node : m_deletions) {
        auto cursor = node.cursor;
        const int pos = node.insertPos;
        const QString text = node.delText;
        cursor.setPosition(pos, QTextCursor::MoveAnchor);
        cursor.insertText(text);

        if (0 <= node.idInColumn && node.idInColumn < m_columnEditSelections.size()) {
            const int endPos = pos + text.size();
            if (node.leftToRight) {
                cursor.setPosition(pos);
                cursor.setPosition(endPos, QTextCursor::KeepAnchor);
            } else {
                cursor.setPosition(endPos);
                cursor.setPosition(pos, QTextCursor::KeepAnchor);
            }
            m_columnEditSelections[node.idInColumn].cursor = cursor;
        }
    }

    if (m_edit && !m_columnEditSelections.isEmpty()) {
        m_edit->restoreColumnEditSelection(m_columnEditSelections);
        m_edit->setTextCursor(m_columnEditSelections.last().cursor);
    }
}

void DeleteBackAltCommand::redo()
{
    for (int i = 0; i < m_deletions.size(); i++) {
        auto cursor = m_deletions[i].cursor;
        const int pos = m_deletions[i].delPos;
        const QString text = m_deletions[i].delText;
        cursor.setPosition(pos, QTextCursor::MoveAnchor);

        for (int j = 0; j < text.size(); j++) {
            cursor.deleteChar();
        }
    }

    if (m_edit && !m_columnEditSelections.isEmpty()) {
        m_edit->restoreColumnEditSelection(m_columnEditSelections);
        m_edit->setTextCursor(m_columnEditSelections.last().cursor);
    }
}

int DeleteBackAltCommand::id() const
{
    return Utils::IdColumnEditDelete;
}
