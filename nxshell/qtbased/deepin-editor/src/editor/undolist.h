// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UndoList_H
#define UndoList_H
#include <QUndoCommand>
#include <QTextCursor>
#include <QTextEdit>
#include <qplaintextedit.h>

//combine multiple undo/redo commands in one commond.
//notice:make sure that multiple commands are independent of each other.
class UndoList:public QUndoCommand
{
public:
    UndoList();
    virtual ~UndoList();
    void appendCom(QUndoCommand* com);
protected:
    virtual void undo();
    virtual void redo();

private:
    QList<QUndoCommand*> m_coms;

};



#endif // UndoList_H
