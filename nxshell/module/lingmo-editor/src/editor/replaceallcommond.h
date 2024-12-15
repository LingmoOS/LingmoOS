// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPLACEALLCOMMOND_H
#define REPLACEALLCOMMOND_H

#include <QUndoCommand>
#include <QTextCursor>
#include <QTextDocument>
#include <QPlainTextEdit>
#include <QPointer>
#include "dtextedit.h"

// 全部替换撤销-重做
class ReplaceAllCommand: public QUndoCommand
{
public:
    ReplaceAllCommand(QString &oldText, QString &newText, QTextCursor cursor, QUndoCommand *parent = nullptr);
    virtual ~ReplaceAllCommand();

    virtual void redo();
    virtual void undo();

private:
    QString m_oldText;
    QString m_newText;
    QTextCursor m_cursor;
};

#endif // REPLACEALLCOMMOND_H
