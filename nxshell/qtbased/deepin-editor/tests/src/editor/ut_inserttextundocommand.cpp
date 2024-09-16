// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_inserttextundocommand.h"
#include "../../src/editor/inserttextundocommand.h"

test_InsertTextUndoCommand::test_InsertTextUndoCommand()
{
}

void test_InsertTextUndoCommand::SetUp()
{
    QTextCursor textcursor;
    QString test;
    ituc = new InsertTextUndoCommand(textcursor, test, nullptr);
}

void test_InsertTextUndoCommand::TearDown()
{
    delete ituc;
}

TEST_F(test_InsertTextUndoCommand, undo)
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::yellow).lighter(160);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    QString text = "aaa";
    InsertTextUndoCommand *command = new InsertTextUndoCommand(extraSelections, text, nullptr);
    command->undo();

    int iRet = command->m_textCursor.position();
    ASSERT_TRUE(iRet != 0);

}

TEST_F(test_InsertTextUndoCommand, undo2)
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    QString text = "aaa";
    InsertTextUndoCommand *command = new InsertTextUndoCommand(extraSelections, text, nullptr);
    command->undo();

    int iRet = command->m_textCursor.position();
    ASSERT_TRUE(iRet != 0);
}

TEST_F(test_InsertTextUndoCommand, redo)
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    QString text = "aaa";
    InsertTextUndoCommand *command = new InsertTextUndoCommand(extraSelections, text, nullptr);
    ituc->redo();

    int iRet = command->m_textCursor.position();
    ASSERT_TRUE(iRet != 0);
}

TEST_F(test_InsertTextUndoCommand, redo2)
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::yellow).lighter(160);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor.clearSelection();
    extraSelections.append(selection);
    QString text = "aaa";
    InsertTextUndoCommand *command = new InsertTextUndoCommand(extraSelections, text, nullptr);
    command->redo();

    int iRet = command->m_textCursor.position();
    ASSERT_TRUE(iRet != 0);
}

TEST_F(test_InsertTextUndoCommand, redo_withTextCursor_restoreCursor)
{
    // 重做恢复光标位置
    QPlainTextEdit *edit = new QPlainTextEdit;
    edit->setPlainText("123789");

    QTextCursor cursor = edit->textCursor();
    cursor.setPosition(3);
    InsertTextUndoCommand *command = new InsertTextUndoCommand(cursor, QString("456"), edit);
    command->redo();

    EXPECT_EQ(QString("123456789"), edit->toPlainText());
    EXPECT_EQ(6, edit->textCursor().position());

    edit->setPlainText("123789");
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::yellow).lighter(160);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = edit->textCursor();
    selection.cursor.setPosition(3);
    extraSelections.append(selection);
    InsertTextUndoCommand *command2 = new InsertTextUndoCommand(extraSelections, QString("456"), edit);
    command2->redo();

    EXPECT_EQ(QString("123456789"), edit->toPlainText());
    EXPECT_EQ(6, edit->textCursor().position());

    delete command;
    delete command2;
    delete edit;
}

TEST_F(test_InsertTextUndoCommand, undo_withTextCursor_restoreCursor)
{
    // 撤销后恢复光标位置
    QPlainTextEdit *edit = new QPlainTextEdit;
    edit->setPlainText("123456789");

    QTextCursor cursor = edit->textCursor();
    cursor.setPosition(6);
    InsertTextUndoCommand *command = new InsertTextUndoCommand(cursor, QString("456"), edit);
    command->m_endPostion = 6;
    command->m_beginPostion = 3;
    command->undo();

    EXPECT_EQ(QString("123789"), edit->toPlainText());
    EXPECT_EQ(3, edit->textCursor().position());

    edit->setPlainText("123456789");
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::yellow).lighter(160);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = edit->textCursor();
    selection.cursor.setPosition(3);
    selection.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 3);
    extraSelections.append(selection);
    InsertTextUndoCommand *command2 = new InsertTextUndoCommand(extraSelections, QString("456"), edit);
    command2->undo();

    EXPECT_EQ(QString("123789"), edit->toPlainText());
    EXPECT_EQ(3, edit->textCursor().position());

    delete command;
    delete command2;
    delete edit;
}

TEST(test_MidButtonInsertTextUndoCommand, redo_withTextCursor_restoreCursor)
{
    // 重做恢复光标位置
    QPlainTextEdit *edit = new QPlainTextEdit;
    edit->setPlainText("123789");

    QTextCursor cursor = edit->textCursor();
    cursor.setPosition(3);
    MidButtonInsertTextUndoCommand *command = new MidButtonInsertTextUndoCommand(cursor, QString("456"), edit);
    EXPECT_EQ(command->m_beginPostion, 3);
    EXPECT_EQ(command->m_endPostion, 6);
    command->redo();

    EXPECT_EQ(QString("123456789"), edit->toPlainText());
    EXPECT_EQ(6, edit->textCursor().position());

    delete command;
    delete edit;
}

TEST(test_MidButtonInsertTextUndoCommand, undo_withTextCursor_restoreCursor)
{
    // 撤销后恢复光标位置
    QPlainTextEdit *edit = new QPlainTextEdit;
    edit->setPlainText("123456789");

    QTextCursor cursor = edit->textCursor();
    cursor.setPosition(6);
    MidButtonInsertTextUndoCommand *command = new MidButtonInsertTextUndoCommand(cursor, QString("456"), edit);
    command->m_endPostion = 6;
    command->m_beginPostion = 3;
    command->undo();

    EXPECT_EQ(QString("123789"), edit->toPlainText());
    EXPECT_EQ(3, edit->textCursor().position());

    delete command;
    delete edit;
}
