// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "undolist.h"
UndoList::UndoList()
{

}
UndoList::~UndoList()
{
    for (auto& it : m_coms) {
        if(nullptr != it){
            delete it;
            it = nullptr;
        }
    }

    m_coms.clear();
}
void UndoList::appendCom(QUndoCommand* com)
{
    if(nullptr != com){
        m_coms.push_back(com);
    }
}
void UndoList::undo()
{
    //do the undo operation in the reverse order.
    for(auto it = m_coms.rbegin();it!=m_coms.rend();it++){
        (*it)->undo();
    }
}
void UndoList::redo()
{
    foreach (auto it, m_coms) {
        it->redo();
    }
}
