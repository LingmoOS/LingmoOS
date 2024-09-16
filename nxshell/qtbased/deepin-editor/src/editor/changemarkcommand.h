// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHANGEMARKCOMMAND_H
#define CHANGEMARKCOMMAND_H

#include <QUndoCommand>
#include "dtextedit.h"

// 用于进行颜色标记前后变更撤销恢复
class ChangeMarkCommand : public QUndoCommand
{
public:
    ChangeMarkCommand(QPointer<TextEdit> editPtr,
                      const QList<TextEdit::MarkReplaceInfo> &oldMark,
                      const QList<TextEdit::MarkReplaceInfo> &newMark,
                      QUndoCommand *parent = nullptr);
    virtual ~ChangeMarkCommand();

    virtual void undo();
    virtual void redo();

private:
    QPointer<TextEdit> m_EditPtr;     // 操作的文本编辑器对象指针
    QList<TextEdit::MarkReplaceInfo> m_oldMarkReplace;  // 缓存的标识操作记录
    QList<TextEdit::MarkReplaceInfo> m_newMarkReplace;  // 新的标识操作记录
};

#endif // CHANGEMARKCOMMAND_H
