// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IndentTextCommand_H
#define IndentTextCommand_H

#include <QUndoCommand>
#include <QTextCursor>
#include <QTextDocument>
#include <QPlainTextEdit>
class TextEdit;
//indent text in front of multiple lines
class IndentTextCommand:public QUndoCommand
{
public:
    IndentTextCommand(TextEdit* edit,int startpos,int endpos,int startline,int endline);
    virtual ~IndentTextCommand();

    virtual void redo();
    virtual void undo();

private:

    TextEdit* m_edit=nullptr;
    //the start postion of selected text.
    int m_startpos=0;
    //the end postion of selected text.
    int m_endpos=0;
    //the start line of selected text.
    int m_startline=0;
    //the end line of selected text.
    int m_endline=0;
    bool m_hasselected=false;

};

#endif // IndentTextCommand_H
