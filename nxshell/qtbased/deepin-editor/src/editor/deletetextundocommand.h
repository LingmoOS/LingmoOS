// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DELETETEXTUNDOCOMMAND_H
#define DELETETEXTUNDOCOMMAND_H

#include <QObject>
#include <QUndoCommand>
#include <QTextCursor>
#include <QTextEdit>
#include <qplaintextedit.h>

class DeleteTextUndoCommand : public QUndoCommand
{
public:
    explicit DeleteTextUndoCommand(QTextCursor textcursor, QPlainTextEdit* edit, QUndoCommand *parent = nullptr);
    explicit DeleteTextUndoCommand(QList<QTextEdit::ExtraSelection> &selections, QPlainTextEdit* edit, QUndoCommand *parent = nullptr);
    virtual void undo();
    virtual void redo();

private:
    QPlainTextEdit* m_edit;
    QTextCursor m_textCursor;
    QString m_sInsertText;
    QList<QString> m_selectTextList;
    QList<QTextEdit::ExtraSelection> m_ColumnEditSelections;
    int m_beginPos;     // 记录光标删除前位置
};

//重写ctrl + k 和Ctrl +shift +K 逻辑的删除和撤销功能 ut002764
class DeleteTextUndoCommand2 : public QUndoCommand
{
public:
    explicit DeleteTextUndoCommand2(QTextCursor textcursor,QString text,QPlainTextEdit* edit,bool currLine);
    explicit DeleteTextUndoCommand2(QList<QTextEdit::ExtraSelection> &selections,QString text,QPlainTextEdit* edit,bool m_iscurrLine);
    virtual void undo();
    virtual void redo();

private:
    QTextCursor m_textCursor;
    QString m_sInsertText;
    QList<QString> m_selectTextList;
    QList<QTextEdit::ExtraSelection> m_ColumnEditSelections;
    QPlainTextEdit* m_edit;
    int m_beginPostion {0};
    bool m_iscurrLine {false};
};

#endif // INSERTTEXTUNDOCOMMAND_H
