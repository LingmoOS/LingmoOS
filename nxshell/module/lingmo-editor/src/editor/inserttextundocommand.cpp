// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inserttextundocommand.h"
#include "dtextedit.h"

InsertTextUndoCommand::InsertTextUndoCommand(const QTextCursor &textcursor,
                                             const QString &text,
                                             TextEdit *edit,
                                             QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_pEdit(edit)
    , m_textCursor(textcursor)
    , m_sInsertText(text)
{
    m_sInsertText.replace("\r\n", "\n");
}

InsertTextUndoCommand::InsertTextUndoCommand(QList<QTextEdit::ExtraSelection> &selections,
                                             const QString &text,
                                             TextEdit *edit,
                                             QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_pEdit(edit)
    , m_sInsertText(text)
    , m_columnEditSelections(selections)
{
    m_sInsertText.replace("\r\n", "\n");

    for (const auto &selection : m_columnEditSelections) {
        ColumnReplaceNode replaceNode;
        replaceNode.startPos = selection.cursor.selectionStart();
        replaceNode.endPos = selection.cursor.selectionEnd();
        replaceNode.leftToRight = selection.cursor.anchor() <= selection.cursor.position();
        replaceNode.originText = selection.cursor.selectedText();
        m_replaces.append(replaceNode);
    }
}

void InsertTextUndoCommand::undo()
{
    if (m_columnEditSelections.isEmpty()) {
        // note that some characters appear to occupy more than 1 place
        m_textCursor.setPosition(m_endPostion);
        m_textCursor.setPosition(m_beginPostion, QTextCursor::KeepAnchor);
        m_textCursor.deleteChar();
        if (m_selectText != QString()) {
            m_textCursor.insertText(m_selectText);
            m_textCursor.setPosition(m_textCursor.position() - m_selectText.length(), QTextCursor::KeepAnchor);
        }

        // restore cursor position
        if (m_pEdit) {
            m_pEdit->setTextCursor(m_textCursor);
        }
    } else {
        Q_ASSERT_X(m_columnEditSelections.size() == m_replaces.size(), "Column editing insert undo", "column data size check");

        for (int i = 0; i < m_columnEditSelections.size(); i++) {
            QTextEdit::ExtraSelection &selection = m_columnEditSelections[i];
            const ColumnReplaceNode &replaceNode = m_replaces[i];

            // restore origin text
            QTextCursor cursor = selection.cursor;
            cursor.setPosition(replaceNode.startPos);
            cursor.setPosition(replaceNode.startPos + m_sInsertText.size(), QTextCursor::KeepAnchor);
            cursor.insertText(replaceNode.originText);

            if (replaceNode.leftToRight) {
                cursor.setPosition(replaceNode.startPos);
                cursor.setPosition(replaceNode.endPos, QTextCursor::KeepAnchor);
            } else {
                cursor.setPosition(replaceNode.endPos);
                cursor.setPosition(replaceNode.startPos, QTextCursor::KeepAnchor);
            }

            selection.cursor = cursor;
        }

        if (m_pEdit && !m_columnEditSelections.isEmpty()) {
            m_pEdit->restoreColumnEditSelection(m_columnEditSelections);
            m_pEdit->setTextCursor(m_columnEditSelections.last().cursor);
        }
    }
}

void InsertTextUndoCommand::redo()
{
    if (m_columnEditSelections.isEmpty()) {
        if (m_textCursor.hasSelection()) {
            m_selectText = m_textCursor.selectedText();
            m_textCursor.removeSelectedText();
        }

        m_textCursor.insertText(m_sInsertText);
        m_textCursor.setPosition(m_textCursor.position() - m_sInsertText.length(), QTextCursor::KeepAnchor);
        m_beginPostion = m_textCursor.selectionStart();
        m_endPostion = m_textCursor.selectionEnd();

        // restore cursor position
        if (m_pEdit) {
            QTextCursor curCursor = m_pEdit->textCursor();
            curCursor.setPosition(m_endPostion);
            m_pEdit->setTextCursor(curCursor);
        }
    } else {
        Q_ASSERT_X(m_columnEditSelections.size() == m_replaces.size(), "Column editing insert undo", "column data size check");

        // insert will change all text cursor
        int columnOffset = 0;
        for (int i = 0; i < m_columnEditSelections.size(); i++) {
            QTextEdit::ExtraSelection &selection = m_columnEditSelections[i];
            const ColumnReplaceNode &replaceNode = m_replaces[i];

            // remove origin text with replace text
            QTextCursor cursor = selection.cursor;
            cursor.setPosition(columnOffset + replaceNode.startPos);
            cursor.setPosition(columnOffset + replaceNode.endPos, QTextCursor::KeepAnchor);
            cursor.insertText(m_sInsertText);

            if (replaceNode.leftToRight) {
                cursor.setPosition(columnOffset + replaceNode.startPos);
                cursor.setPosition(columnOffset + replaceNode.startPos + m_sInsertText.size(), QTextCursor::KeepAnchor);
            } else {
                cursor.setPosition(columnOffset + replaceNode.startPos + m_sInsertText.size());
                cursor.setPosition(columnOffset + replaceNode.startPos, QTextCursor::KeepAnchor);
            }

            selection.cursor = cursor;
            columnOffset += m_sInsertText.size() - replaceNode.originText.size();
        }

        if (m_pEdit && !m_columnEditSelections.isEmpty()) {
            QTextCursor curCursor = m_pEdit->textCursor();
            curCursor.setPosition(m_columnEditSelections.last().cursor.selectionEnd());
            m_pEdit->setTextCursor(curCursor);
        }
    }
}

int InsertTextUndoCommand::id() const
{
    return m_columnEditSelections.isEmpty() ? Utils::IdInsert : Utils::IdColumnEditInsert;
}

/**
 * @class MidButtonInsertTextUndoCommand
 * @brief 用于鼠标中键黏贴的插入撤销项，使用 QClipboard::Selection 类型插入选中的数据，
 *      需要注意中键插入不会覆盖被选中的文本。
 */
MidButtonInsertTextUndoCommand::MidButtonInsertTextUndoCommand(const QTextCursor &textcursor,
                                                               const QString &text,
                                                               QPlainTextEdit *edit,
                                                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_pEdit(edit)
    , m_textCursor(textcursor)
    , m_sInsertText(text)
{
    m_sInsertText.replace("\r\n", "\n");
    // 中键黏贴需要构造时计算一次光标位置
    m_beginPostion = m_textCursor.position();
    m_endPostion = m_textCursor.position() + m_sInsertText.length();
}

/**
 * @brief 撤销插入动作，移除已被插入的文本。
 */
void MidButtonInsertTextUndoCommand::undo()
{
    m_textCursor.setPosition(m_endPostion);
    m_textCursor.setPosition(m_beginPostion, QTextCursor::KeepAnchor);
    m_textCursor.deleteChar();

    // 进行撤销/恢复时将光标移动到撤销位置
    if (m_pEdit) {
        m_pEdit->setTextCursor(m_textCursor);
    }
}

/**
 * @brief 重新执行插入文本，用于恢复动作。
 *      当前中键插入使用默认流程，数据已插入至文本后再添加撤销项，
 *      首次 redo() 不执行，通过插入撤销栈后再添加子撤销项规避。
 */
void MidButtonInsertTextUndoCommand::redo()
{
    m_textCursor.insertText(m_sInsertText);
    m_textCursor.setPosition(m_textCursor.position() - m_sInsertText.length(), QTextCursor::KeepAnchor);
    m_beginPostion = m_textCursor.selectionStart();
    m_endPostion = m_textCursor.selectionEnd();

    // 进行撤销/恢复时将光标移动到撤销位置
    if (m_pEdit) {
        QTextCursor curCursor = m_pEdit->textCursor();
        curCursor.setPosition(m_endPostion);
        m_pEdit->setTextCursor(curCursor);
    }
}

/**
   @brief 用于拖拽 `Drag` 插入的文本，仅插入不会覆盖数据。由于 `Drag` 操作时会先执行删除操作，
        QTextCuesor可能变更，调整为使用固定偏移量。
 */
DragInsertTextUndoCommand::DragInsertTextUndoCommand(const QTextCursor &textcursor,
                                                     const QString &text,
                                                     QPlainTextEdit *edit,
                                                     QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_pEdit(edit)
    , m_textCursor(textcursor)
    , m_sInsertText(text)
{
    m_beginPostion = m_textCursor.selectionStart();
}

/**
   @brief 复位插入操作
 */
void DragInsertTextUndoCommand::undo()
{
    m_textCursor.setPosition(m_beginPostion);
    m_textCursor.setPosition(m_beginPostion + m_sInsertText.size(), QTextCursor::KeepAnchor);
    m_textCursor.deleteChar();

    if (m_pEdit) {
        m_pEdit->setTextCursor(m_textCursor);
    }
}

/**
   @brief 重做插入操作，使用固定偏移量
 */
void DragInsertTextUndoCommand::redo()
{
    if (m_beginPostion != m_textCursor.position()) {
        m_textCursor.setPosition(m_beginPostion);
    }

    m_textCursor.insertText(m_sInsertText);

    if (m_pEdit) {
        QTextCursor curCursor = m_pEdit->textCursor();
        curCursor.setPosition(m_beginPostion + m_sInsertText.size());
        m_pEdit->setTextCursor(curCursor);
    }
}
