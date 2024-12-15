// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INSERTTEXTUNDOCOMMAND_H
#define INSERTTEXTUNDOCOMMAND_H

#include <QObject>
#include <QUndoCommand>
#include <QTextCursor>
#include <QTextEdit>
#include <QPlainTextEdit>

class TextEdit;

class InsertTextUndoCommand : public QUndoCommand
{
public:
    explicit InsertTextUndoCommand(const QTextCursor &textcursor,
                                   const QString &text,
                                   TextEdit *edit,
                                   QUndoCommand *parent = nullptr);
    explicit InsertTextUndoCommand(QList<QTextEdit::ExtraSelection> &selections,
                                   const QString &text,
                                   TextEdit *edit,
                                   QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

    int id() const override;

private:
    struct ColumnReplaceNode
    {
        int startPos{false};
        int endPos{false};
        bool leftToRight{true};
        QString originText;  // replaced text before insert.
    };

    TextEdit *m_pEdit = nullptr;
    QTextCursor m_textCursor;
    int m_beginPostion{0};
    int m_endPostion{0};
    QString m_sInsertText;
    QList<QTextEdit::ExtraSelection> m_columnEditSelections;
    QList<ColumnReplaceNode> m_replaces;
    QString m_selectText = QString();
};

/**
 * @brief 鼠标中键插入字符时使用的文本插入撤销项, 需要注意中键插入不会覆盖被选中的文本。
 */
class MidButtonInsertTextUndoCommand : public QUndoCommand
{
public:
    explicit MidButtonInsertTextUndoCommand(const QTextCursor &textcursor,
                                            const QString &text,
                                            QPlainTextEdit *edit,
                                            QUndoCommand *parent = nullptr);

    virtual void undo();
    virtual void redo();

private:
    QPlainTextEdit *m_pEdit = nullptr;  // 关联的文本编辑控件
    QTextCursor m_textCursor;           // 插入前的光标
    QString m_sInsertText;              // 插入文本
    int m_beginPostion = 0;             // 维护插入位置的标记
    int m_endPostion = 0;
};

/**
   @brief 拖拽插入文本处理
 */
class DragInsertTextUndoCommand : public QUndoCommand
{
public:
    explicit DragInsertTextUndoCommand(const QTextCursor &textcursor,
                                       const QString &text,
                                       QPlainTextEdit *edit,
                                       QUndoCommand *parent = nullptr);
    virtual void undo() override;
    virtual void redo() override;

private:
    QPlainTextEdit *m_pEdit = nullptr;
    QTextCursor m_textCursor;
    QString m_sInsertText;
    int m_beginPostion{0};
};

#endif  // INSERTTEXTUNDOCOMMAND_H
