// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "changemarkcommand.h"

ChangeMarkCommand::ChangeMarkCommand(QPointer<TextEdit> editPtr, const QList<TextEdit::MarkReplaceInfo> &oldMark, const QList<TextEdit::MarkReplaceInfo> &newMark, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_EditPtr(editPtr)
    , m_oldMarkReplace(oldMark)
    , m_newMarkReplace(newMark)
{

}

ChangeMarkCommand::~ChangeMarkCommand()
{

}

void ChangeMarkCommand::undo()
{
    // 优先执行子撤销项，保证在文本操作后执行
    QUndoCommand::undo();

    // 插入文本后，恢复旧的颜色标记状态
    if (m_EditPtr && !m_oldMarkReplace.isEmpty()) {
        auto oldMark = TextEdit::convertReplaceToMark(m_oldMarkReplace);
        m_EditPtr->manualUpdateAllMark(oldMark);
    }
}

void ChangeMarkCommand::redo()
{
    // 优先执行子撤销项，保证在文本操作后执行
    QUndoCommand::redo();

    // 插入文本后，更新颜色标记状态
    if (m_EditPtr && !m_newMarkReplace.isEmpty()) {
        auto newMark = TextEdit::convertReplaceToMark(m_newMarkReplace);
        m_EditPtr->manualUpdateAllMark(newMark);
    }
}
