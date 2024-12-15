// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indenttextcommond.h"
#include "dtextedit.h"

IndentTextCommand::IndentTextCommand(TextEdit* edit,int startpos,int endpos,int startline,int endline):
    m_edit(edit),
    m_startpos(startpos),
    m_endpos(endpos),
    m_startline(startline),
    m_endline(endline)
{
    m_hasselected = m_edit->textCursor().hasSelection();
}
IndentTextCommand::~IndentTextCommand()
{

}

void IndentTextCommand::redo()
{
    auto cursor = m_edit->textCursor();

    //insert "\t" in front of multiple lines.
    cursor.setPosition(m_startpos);
    for(int i=m_startline;i<=m_endline;i++){
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.insertText("\t");

        cursor.movePosition(QTextCursor::NextBlock);
    }

    //reset selection.
    if(m_hasselected){
        if(m_startline == m_endline){
            cursor.setPosition(m_startpos);
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.movePosition(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
            m_edit->setTextCursor(cursor);
        }
        else{
            cursor.setPosition(m_startpos+1);
            cursor.setPosition(m_endpos + m_endline - m_startline +1,QTextCursor::KeepAnchor);
            m_edit->setTextCursor(cursor);
        }

    }
}



void IndentTextCommand::undo()
{
    auto cursor = m_edit->textCursor();

    //delete "\t" in front of multiple lines.
    cursor.setPosition(m_startpos);
    for(int i=m_startline;i<=m_endline;i++){
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.deleteChar();

        cursor.movePosition(QTextCursor::NextBlock);
    }

    //reset selection
    if(m_hasselected){
        cursor.setPosition(m_startpos);
        cursor.setPosition(m_endpos,QTextCursor::KeepAnchor);
        m_edit->setTextCursor(cursor);
    }
}
