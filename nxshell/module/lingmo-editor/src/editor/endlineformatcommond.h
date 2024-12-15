// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EndlineFormartCommand_H
#define EndlineFormartCommand_H
#include <QUndoCommand>
#include "../widgets/bottombar.h"
#include "dtextedit.h"


class EndlineFormartCommand:public QUndoCommand
{
public:
    EndlineFormartCommand(TextEdit* edit,BottomBar* bar,BottomBar::EndlineFormat from,BottomBar::EndlineFormat to);
    virtual ~EndlineFormartCommand();

protected:
    virtual void undo();
    virtual void redo();

private:
    BottomBar* m_bar=nullptr;
    BottomBar::EndlineFormat m_from;
    BottomBar::EndlineFormat m_to;
    TextEdit* m_edit=nullptr;

};



#endif // UndoList_H
