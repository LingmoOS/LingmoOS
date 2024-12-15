// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DELETEBACKCOMMOND_H
#define DELETEBACKCOMMOND_H

#include <QUndoCommand>
#include <QTextCursor>
#include <QTextEdit>
#include <qplaintextedit.h>

class TextEdit;

// Delete selected text or charactor (if not selected) backward
class DeleteBackCommand : public QUndoCommand
{
public:
    DeleteBackCommand(QTextCursor cursor, QPlainTextEdit *edit);
    ~DeleteBackCommand() override;
    void undo() override;
    void redo() override;

private:
    QTextCursor m_cursor;
    QString m_delText{QString()};
    int m_delPos{0};
    int m_insertPos{0};

    QPlainTextEdit *m_edit;
};

// Delete redo / undo on column editing
class DeleteBackAltCommand : public QUndoCommand
{
public:
    DeleteBackAltCommand(const QList<QTextEdit::ExtraSelection> &selections, TextEdit *edit, bool backward = false);
    ~DeleteBackAltCommand() override;
    void undo() override;
    void redo() override;

    int id() const override;

public:
    struct DelNode
    {
        QString delText;
        int delPos;
        int insertPos;
        int idInColumn;
        QTextCursor cursor;
        bool leftToRight;  // select orientation
    };

private:
    QList<QTextEdit::ExtraSelection> m_columnEditSelections;
    QList<DelNode> m_deletions;
    TextEdit *m_edit;
};

#endif  // DELETEBACKCOMMOND_H
