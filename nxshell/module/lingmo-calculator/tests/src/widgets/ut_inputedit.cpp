// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_inputedit.h"

#include "../../src/widgets/inputedit.h"
#include <QAction>
#include "../../3rdparty/math/cmath.h"

Ut_InputEdit::Ut_InputEdit()
{

}

TEST_F(Ut_InputEdit, expressionText)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setText("3.333333333333333E－4");
    m_inputEdit->m_evaluator = Evaluator::instance();
    m_inputEdit->m_evaluator->setExpression("1/3000");
    m_inputEdit->m_ans = m_inputEdit->m_evaluator->evalUpdateAns();
    m_inputEdit->m_ansVaild = true;
    m_inputEdit->m_strans = QString("3.333333333333333E－4");
    m_inputEdit->m_ansStartPos = 0;
    m_inputEdit->m_ansLength = 20;
    EXPECT_EQ(m_inputEdit->expressionText(), "lastans");
    delete  m_inputEdit;
}

TEST_F(Ut_InputEdit, setAnswer)
{
    InputEdit *m_inputEdit = new InputEdit;
    Evaluator *m_evaluator = Evaluator::instance();
    m_evaluator->setExpression("1/3000");
    Quantity ans = m_evaluator->evalUpdateAns();
    m_inputEdit->setAnswer("3.333333333333333E－4", ans);
    EXPECT_EQ(m_inputEdit->m_ansLength, 20);
    EXPECT_EQ(m_inputEdit->m_ans, ans);
    EXPECT_EQ(m_inputEdit->m_strans, "3.333333333333333E－4");
    delete m_inputEdit;
}

bool stub_focus_input()
{
    return true;
}

QAction *stub_exec_input(const QPoint &pos, QAction *at = nullptr)
{
    Q_UNUSED(pos)
    Q_UNUSED(at)

    return nullptr;
}

TEST_F(Ut_InputEdit, keyPressEvent)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setFocus();
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_M, Qt::AltModifier);

    Stub stub;
    stub.set(ADDR(InputEdit, hasFocus), stub_focus_input);

    Stub stub1;
    stub1.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), stub_exec_input);

    m_inputEdit->setText("1");
    m_inputEdit->selectAll();
    m_inputEdit->keyPressEvent(k);
    EXPECT_TRUE(m_inputEdit->m_cut->isEnabled());
    EXPECT_TRUE(m_inputEdit->m_copy->isEnabled());
    EXPECT_TRUE(m_inputEdit->m_delete->isEnabled());
    EXPECT_TRUE(m_inputEdit->m_select->isEnabled());

    m_inputEdit->clear();
    m_inputEdit->keyPressEvent(k);
    EXPECT_FALSE(m_inputEdit->m_cut->isEnabled());
    EXPECT_FALSE(m_inputEdit->m_copy->isEnabled());
    EXPECT_FALSE(m_inputEdit->m_delete->isEnabled());
    EXPECT_FALSE(m_inputEdit->m_select->isEnabled());

    delete k;
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, mouseDoubleClickEvent)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setText("12345");
    QMouseEvent *m = new QMouseEvent(QEvent::MouseButtonDblClick, m_inputEdit->rect().center(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_inputEdit->mouseDoubleClickEvent(m);
    EXPECT_EQ(m_inputEdit->m_selected.selected, "12,345");
    delete m;
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, mousePressEvent)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setText("12345");
//    m_inputEdit->mouseDoubleClickEvent(new QMouseEvent(QEvent::MouseButtonDblClick, m_inputEdit->rect().center(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
    QMouseEvent *m = new QMouseEvent(QEvent::MouseButtonPress, m_inputEdit->rect().center(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_inputEdit->mousePressEvent(m);
    EXPECT_EQ(m_inputEdit->m_selected.selected, "");
    delete m;
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, handleTextChanged)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->handleTextChanged("1=");
    EXPECT_EQ(m_inputEdit->text(), "1");
    EXPECT_FALSE(m_inputEdit->m_ansVaild);
    EXPECT_EQ(m_inputEdit->m_oldText, "1");
    EXPECT_EQ(m_inputEdit->m_selected.selected, "");
    EXPECT_EQ(m_inputEdit->m_selected.curpos, -1);
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, pointFaultTolerance)
{
    InputEdit *m_inputEdit = new InputEdit;

    EXPECT_EQ(m_inputEdit->pointFaultTolerance("1"), "1");
    EXPECT_EQ(m_inputEdit->pointFaultTolerance(".1"), ".1");
    EXPECT_EQ(m_inputEdit->pointFaultTolerance("%.1"), "%1");
    EXPECT_EQ(m_inputEdit->pointFaultTolerance("0.1.1"), "0.11");
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, symbolFaultTolerance)
{
    InputEdit *m_inputEdit = new InputEdit;
    EXPECT_EQ(m_inputEdit->symbolFaultTolerance("－1"), "－1");
    EXPECT_EQ(m_inputEdit->symbolFaultTolerance("E(－"), "E(－");
    EXPECT_EQ(m_inputEdit->symbolFaultTolerance("1E－.1－1"), "1E－1－1");
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, isSymbol)
{
    InputEdit *m_inputEdit = new InputEdit;
    EXPECT_TRUE(m_inputEdit->isSymbol("＋"));
    EXPECT_TRUE(m_inputEdit->isSymbol("－"));
    EXPECT_TRUE(m_inputEdit->isSymbol("÷"));
    EXPECT_FALSE(m_inputEdit->isSymbol("*"));
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, handleCursorPositionChanged)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->m_ansStartPos = 0;
    m_inputEdit->m_ansLength = 3;
    m_inputEdit->m_selected.curpos = 1;
    m_inputEdit->m_selected.selected = "2";
    m_inputEdit->handleCursorPositionChanged(0, 0);
    EXPECT_TRUE(m_inputEdit->m_currentInAns);
    EXPECT_EQ(m_inputEdit->m_lastPos, 0);
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, BracketCompletion)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setText("(()");
    QKeyEvent *k = new QKeyEvent(QEvent::None, Qt::Key_1, Qt::NoModifier);
    m_inputEdit->BracketCompletion(k);
    EXPECT_EQ(m_inputEdit->text(), "(())");
    m_inputEdit->setText("())");
    m_inputEdit->BracketCompletion(k);
    EXPECT_EQ(m_inputEdit->text(), "())()");
    m_inputEdit->setText("()");
    m_inputEdit->BracketCompletion(k);
    EXPECT_EQ(m_inputEdit->text(), "()()");
    delete k;
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, multipleArithmetic)
{
    InputEdit *m_inputEdit = new InputEdit;
    QString s = "\n)\n";
    m_inputEdit->multipleArithmetic(s);
    EXPECT_EQ(s, "\n)×");
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, getMemoryAnswer)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setText("1log0");
    EXPECT_FALSE(m_inputEdit->getMemoryAnswer().first);
    m_inputEdit->setText("2log2");
    EXPECT_TRUE(m_inputEdit->getMemoryAnswer().first);
    EXPECT_EQ(m_inputEdit->getMemoryAnswer().second, Quantity(1));
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, hisexpression)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->m_ansLength = 1;
    m_inputEdit->hisexpression();
    EXPECT_EQ(m_inputEdit->m_ansLength, 0);
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, symbolComplement)
{
    InputEdit *m_inputEdit = new InputEdit;
    EXPECT_EQ(m_inputEdit->symbolComplement("1(2)"), "1×(2)");
    EXPECT_EQ(m_inputEdit->symbolComplement("(1)2"), "(1)×2");
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, focusInEvent)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setText("1111111");
    m_inputEdit->setCursorPosition(5);
    QFocusEvent *f = new QFocusEvent(QEvent::Type::FocusIn, Qt::TabFocusReason);
    m_inputEdit->focusInEvent(f);
    EXPECT_EQ(m_inputEdit->cursorPosition(), 5);
    delete f;
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, clear)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->clear();
    EXPECT_EQ(m_inputEdit->m_ansLength, 0);
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, setUndoAction)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setUndoAction(false);
    EXPECT_FALSE(m_inputEdit->m_undo->isEnabled());
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, setRedoAction)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setRedoAction(true);
    EXPECT_TRUE(m_inputEdit->m_redo->isEnabled());
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, autoZoomFontSize)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setText("0");
    m_inputEdit->autoZoomFontSize();
    EXPECT_EQ(m_inputEdit->font().pixelSize(), 30);
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, themetypechanged)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->themetypechanged(2);
    EXPECT_EQ(m_inputEdit->palette().text(), QBrush("#b4b4b4"));
    m_inputEdit->themetypechanged(1);
    EXPECT_EQ(m_inputEdit->palette().text(), QBrush("#303030"));
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, valueChangeFromProSyskeypad)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setText("");
    m_inputEdit->setCursorPosition(0);
    m_inputEdit->valueChangeFromProSyskeypad("11100011");
    EXPECT_EQ(m_inputEdit->text(), "227");
    EXPECT_EQ(m_inputEdit->cursorPosition(), 3);
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, radixChanged)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setText("120+323+456");
    m_inputEdit->radixChanged(10, 16);
    EXPECT_EQ(m_inputEdit->text(), "78＋143＋1C8");
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, formatAns)
{
    InputEdit *m_inputEdit = new InputEdit;
    QString ans = m_inputEdit->formatAns("11100101");
    EXPECT_EQ(ans, "229");
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, getCurrentAns)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setText("0x1C8");
    QPair<bool, Quantity> pair = m_inputEdit->getCurrentAns();
    int ans = pair.second.numericValue().toInt();
    EXPECT_TRUE(pair.first);
    EXPECT_EQ(ans, 456);
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, CurrentCursorPositionNumber)
{
    InputEdit *m_inputEdit = new InputEdit;
    m_inputEdit->setText("6666+2333");
    m_inputEdit->setCursorPosition(9);
    QString str = m_inputEdit->CurrentCursorPositionNumber(m_inputEdit->cursorPosition());
    EXPECT_EQ(str, "2333");
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, isNumber)
{
    InputEdit *m_inputEdit = new InputEdit;
    EXPECT_FALSE(m_inputEdit->isNumber('a'));
    EXPECT_TRUE(m_inputEdit->isNumber('A'));
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, formatBinaryNumber)
{
    InputEdit *m_inputEdit = new InputEdit;
    EXPECT_EQ(m_inputEdit->formatBinaryNumber("0000000000"), "0");
    QString str = m_inputEdit->formatBinaryNumber("00000000111001");
    EXPECT_EQ(str, "57");
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, formatExpression)
{
    InputEdit *m_inputEdit = new InputEdit;
    EXPECT_EQ(m_inputEdit->formatExpression(16, "4435"), "0x4435");
    EXPECT_EQ(m_inputEdit->formatExpression(10, "4435"), "4435");
    EXPECT_EQ(m_inputEdit->formatExpression(2, "1111"), "0b1111");
    EXPECT_EQ(m_inputEdit->formatExpression(8, "4435"), "0o4435");
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, showTextEditMenu)
{
    InputEdit *m_inputEdit = new InputEdit;
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), stub_exec_input);
    m_inputEdit->setText("1");
    m_inputEdit->selectAll();
    m_inputEdit->showTextEditMenu();
    EXPECT_TRUE(m_inputEdit->m_cut->isEnabled());
    EXPECT_TRUE(m_inputEdit->m_copy->isEnabled());
    EXPECT_TRUE(m_inputEdit->m_delete->isEnabled());
    EXPECT_TRUE(m_inputEdit->m_select->isEnabled());

    m_inputEdit->clear();
    m_inputEdit->showTextEditMenu();
    EXPECT_FALSE(m_inputEdit->m_cut->isEnabled());
    EXPECT_FALSE(m_inputEdit->m_copy->isEnabled());
    EXPECT_FALSE(m_inputEdit->m_delete->isEnabled());
    EXPECT_FALSE(m_inputEdit->m_select->isEnabled());
    delete m_inputEdit;
}

TEST_F(Ut_InputEdit, showTextEditMenuByAltM)
{
    InputEdit *m_inputEdit = new InputEdit;
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), stub_exec_input);
    m_inputEdit->setText("1");
    m_inputEdit->selectAll();
    m_inputEdit->showTextEditMenuByAltM();
    EXPECT_TRUE(m_inputEdit->m_cut->isEnabled());
    EXPECT_TRUE(m_inputEdit->m_copy->isEnabled());
    EXPECT_TRUE(m_inputEdit->m_delete->isEnabled());
    EXPECT_TRUE(m_inputEdit->m_select->isEnabled());

    m_inputEdit->clear();
    m_inputEdit->showTextEditMenuByAltM();
    EXPECT_FALSE(m_inputEdit->m_cut->isEnabled());
    EXPECT_FALSE(m_inputEdit->m_copy->isEnabled());
    EXPECT_FALSE(m_inputEdit->m_delete->isEnabled());
    EXPECT_FALSE(m_inputEdit->m_select->isEnabled());
    delete m_inputEdit;
}
