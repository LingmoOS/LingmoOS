// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INSERTBLOCKBYTEXTCOMMOND_H
#define INSERTBLOCKBYTEXTCOMMOND_H

#include <QUndoCommand>
#include <QTextCursor>
#include <QTextEdit>
#include <qplaintextedit.h>
class TextEdit;
class EditWrapper;

//分块插入文本-撤销重做
class InsertBlockByTextCommand:public QUndoCommand
{
public:
    InsertBlockByTextCommand(const QString& text,TextEdit* edit,EditWrapper* wrapper);
    virtual ~InsertBlockByTextCommand();

    virtual void redo();
    virtual void undo();

private:
    void treat(bool isStart = true);
    void insertByBlock();

private:
    QString m_text;
    TextEdit* m_edit;
    EditWrapper* m_wrapper;
    int m_insertPos;
    int m_delPos {0};
    QString m_selected;
};

#endif // INSERTBLOCKBYTEXTCOMMOND_H
