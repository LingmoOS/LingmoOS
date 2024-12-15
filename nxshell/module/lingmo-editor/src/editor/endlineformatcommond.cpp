// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "endlineformatcommond.h"
EndlineFormartCommand::EndlineFormartCommand(TextEdit* edit,BottomBar* bar,BottomBar::EndlineFormat from,BottomBar::EndlineFormat to):
    m_edit(edit),
    m_bar(bar),
    m_from(from),
    m_to(to)
{

}
EndlineFormartCommand::~EndlineFormartCommand()
{

}

void EndlineFormartCommand::undo()
{
   m_bar->setEndlineMenuText(m_from);
}
void EndlineFormartCommand::redo()
{
    auto cursor = m_edit->textCursor();
    auto pos = cursor.position();
    cursor.insertText(" ");
    cursor.setPosition(pos);
    cursor.deleteChar();
    m_bar->setEndlineMenuText(m_to);
}
