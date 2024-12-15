// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_sciexpressionbar.h"

#include <QClipboard>

#include "../../src/widgets/sciexpressionbar.h"

Ut_SciexpressionBar::Ut_SciexpressionBar()
{
}

TEST_F(Ut_SciexpressionBar, mouseMoveEvent)
{
    SciExpressionBar *m_sciexpressionBar = new SciExpressionBar;
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseMove, m_sciexpressionBar->pos(), Qt::MouseButton::LeftButton, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    m_sciexpressionBar->mouseMoveEvent(m);
    delete m;
    //取消mousemove的事件，无assert
    delete m_sciexpressionBar;
}

TEST_F(Ut_SciexpressionBar, getanswer)
{
    SciExpressionBar *m_sciexpressionBar = new SciExpressionBar;
    m_sciexpressionBar->m_pair.first = false;
    EXPECT_TRUE(m_sciexpressionBar->getanswer().isZero());
    delete m_sciexpressionBar;
}

TEST_F(Ut_SciexpressionBar, enterNumberEvent)
{
    SciExpressionBar *m_sciexpressionBar = new SciExpressionBar;
    m_sciexpressionBar->m_inputNumber = true;
    m_sciexpressionBar->m_isResult = true;
    m_sciexpressionBar->m_isContinue = false;
    m_sciexpressionBar->enterNumberEvent("1");
    EXPECT_EQ(m_sciexpressionBar->m_inputEdit->text(), "1");
    m_sciexpressionBar->m_inputNumber = false;
    m_sciexpressionBar->m_isResult = true;
    m_sciexpressionBar->enterNumberEvent("1");
    EXPECT_EQ(m_sciexpressionBar->m_inputEdit->text(), "1");
    m_sciexpressionBar->m_inputEdit->setText("π");
    m_sciexpressionBar->enterNumberEvent("1");
    EXPECT_EQ(m_sciexpressionBar->m_inputEdit->text(), "π×1");
    EXPECT_FALSE(m_sciexpressionBar->m_inputNumber);
    EXPECT_FALSE(m_sciexpressionBar->m_isUndo);
    EXPECT_FALSE(m_sciexpressionBar->m_isResult);

    delete m_sciexpressionBar;
}

TEST_F(Ut_SciexpressionBar, enterSymbolEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    //    m_expressionBar->m_listModel->updataList(QString("1＋2") + "＝" + "3", -1);
    //    m_expressionBar->findChild<InputEdit *>()->setText("3");
    //    m_expressionBar->m_isUndo = true;
    m_expressionBar->enterSymbolEvent("＋");
    m_expressionBar->findChild<InputEdit *>()->clear();
    m_expressionBar->enterSymbolEvent("-");
    m_expressionBar->enterSymbolEvent("-");
    m_expressionBar->enterSymbolEvent("＋");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "－");

    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_expressionBar->enterSymbolEvent("-");
    m_expressionBar->findChild<InputEdit *>()->setText("1");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_expressionBar->enterSymbolEvent("-");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "－1");
    m_expressionBar->findChild<InputEdit *>()->setText("1＋2");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(1);
    m_expressionBar->enterSymbolEvent("-");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1＋2");
    m_expressionBar->findChild<InputEdit *>()->setText("11");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(1);
    m_expressionBar->enterSymbolEvent("-");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1－1");
    EXPECT_FALSE(m_expressionBar->m_isUndo);
    EXPECT_FALSE(m_expressionBar->m_isResult);
    EXPECT_TRUE(m_expressionBar->m_isContinue);
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enterPercentEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->enterPercentEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "");
    m_expressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_expressionBar->enterPercentEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1＋1%");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(1);
    m_expressionBar->enterPercentEvent();
    m_expressionBar->findChild<InputEdit *>()->setText("1111");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(1);
    m_expressionBar->enterPercentEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1%111");
    m_expressionBar->findChild<InputEdit *>()->setText("1111");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_expressionBar->enterPercentEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1%111");
    EXPECT_FALSE(m_expressionBar->m_isUndo);
    EXPECT_FALSE(m_expressionBar->m_isResult);
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enterPointEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->enterPointEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "0.");
    m_expressionBar->findChild<InputEdit *>()->setText("1");
    m_expressionBar->enterPointEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1.");
    m_expressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_expressionBar->enterPointEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1＋0.1");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(4);
    m_expressionBar->enterPointEvent();
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(5);
    m_expressionBar->enterPointEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1＋0.1");
    EXPECT_FALSE(m_expressionBar->m_isUndo);
    EXPECT_FALSE(m_expressionBar->m_isResult);
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enterBackspaceEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_expressionBar->allElection();
    m_expressionBar->enterBackspaceEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "");
    m_expressionBar->findChild<InputEdit *>()->setText("1.＋1");
    m_expressionBar->enterBackspaceEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1.＋");
    m_expressionBar->enterBackspaceEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1.");
    m_expressionBar->enterBackspaceEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1");
    m_expressionBar->findChild<InputEdit *>()->clear();
    m_expressionBar->enterFunctionEvent("sin");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_expressionBar->enterBackspaceEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "()");
    m_expressionBar->enterFunctionEvent("sin");
    SSelection select;
    select.curpos = 1;
    select.oldText = "sin()";
    select.selected = "i";
    m_expressionBar->findChild<InputEdit *>()->setSelection(select);
    m_expressionBar->enterBackspaceEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "()()");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enterClearEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->m_isAllClear = false;
    m_expressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_expressionBar->enterClearEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "");
    EXPECT_FALSE(m_expressionBar->m_FEisdown);
    EXPECT_FALSE(m_expressionBar->m_isResult);
    EXPECT_FALSE(m_expressionBar->m_isUndo);
    EXPECT_TRUE(m_expressionBar->m_isAllClear);
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enterEqualEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->enterEqualEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "");
    m_expressionBar->findChild<InputEdit *>()->setText("2log2");
    m_expressionBar->enterEqualEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1");
    m_expressionBar->findChild<InputEdit *>()->setText("1");
    m_expressionBar->enterEqualEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1");
    m_expressionBar->findChild<InputEdit *>()->setText("1÷3000");
    m_expressionBar->enterEqualEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "3.3333333333333333333333333333333E－4");
    m_expressionBar->enterEqualEvent();
    m_expressionBar->findChild<InputEdit *>()->setText("1＋");
    m_expressionBar->enterEqualEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1＋");
    m_expressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_expressionBar->m_FEisdown = true;
    m_expressionBar->enterEqualEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "2E＋0");
    m_expressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_expressionBar->m_FEisdown = false;
    m_expressionBar->enterEqualEvent();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "2");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enterLeftBracketsEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->findChild<InputEdit *>()->setText("3");
    m_expressionBar->m_isUndo = true;
    m_expressionBar->enterBracketEvent(1);
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "3(");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_expressionBar->enterBracketEvent(1);
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "(3(");
    m_expressionBar->findChild<InputEdit *>()->setText("1111");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_expressionBar->enterBracketEvent(1);
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1(111");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enterRightBracketsEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->findChild<InputEdit *>()->setText("3");
    m_expressionBar->m_isUndo = true;
    m_expressionBar->enterBracketEvent(2);
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "3)");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_expressionBar->enterBracketEvent(2);
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), ")3)");
    m_expressionBar->findChild<InputEdit *>()->setText("1111");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_expressionBar->enterBracketEvent(2);
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1)111");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enterFEEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->enterFEEvent(true);
    EXPECT_FALSE(m_expressionBar->m_FEisdown);
    m_expressionBar->enterFEEvent(false);
    EXPECT_TRUE(m_expressionBar->m_FEisdown);
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enterModEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->enterOperatorEvent("mod");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "0mod");
    m_expressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_expressionBar->enterOperatorEvent("mod");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "0mod1＋1");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(6);
    m_expressionBar->enterOperatorEvent("mod");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "0mod1＋0mod1");
    m_expressionBar->findChild<InputEdit *>()->setText("1^2");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_expressionBar->enterOperatorEvent("mod");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1^(0mod2");
    m_expressionBar->findChild<InputEdit *>()->setText("1111");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_expressionBar->enterOperatorEvent("mod");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1mod111");
    EXPECT_FALSE(m_expressionBar->m_isResult);
    EXPECT_FALSE(m_expressionBar->m_isUndo);
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enterx2Event)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->enterOperatorEvent("^2");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "0^2");
    m_expressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_expressionBar->enterOperatorEvent("^2");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "0^2×1＋1");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(6);
    m_expressionBar->enterOperatorEvent("^2");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "0^2×1＋0^2×1");
    m_expressionBar->findChild<InputEdit *>()->setText("1^2");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_expressionBar->enterOperatorEvent("^2");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1^(0^2×2");
    m_expressionBar->findChild<InputEdit *>()->setText("1111");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_expressionBar->enterOperatorEvent("^2");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1^2×111");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enterDerivativeEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->enterSpecialFuncEvent("1/(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "");
    m_expressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_expressionBar->allElection();
    m_expressionBar->enterSpecialFuncEvent("1/(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1/(");
    m_expressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_expressionBar->enterSpecialFuncEvent("1/(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1＋1");
    SSelection select;
    select.curpos = 0;
    select.oldText = "1＋1";
    select.selected = "1";
    m_expressionBar->findChild<InputEdit *>()->setSelection(select);
    m_expressionBar->enterSpecialFuncEvent("1/(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1＋1");
    m_expressionBar->findChild<InputEdit *>()->setText("1E-2");
    m_expressionBar->enterSpecialFuncEvent("1/(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1/(1E－2)");
    m_expressionBar->findChild<InputEdit *>()->setText("－");
    m_expressionBar->enterSpecialFuncEvent("1/(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "－");
    m_expressionBar->findChild<InputEdit *>()->setText("()");
    m_expressionBar->enterSpecialFuncEvent("1/(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "()");
    m_expressionBar->findChild<InputEdit *>()->setText("((1))");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(4);
    m_expressionBar->enterSpecialFuncEvent("1/(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "(1/((1)))");
    m_expressionBar->findChild<InputEdit *>()->setText("sin(2)");
    m_expressionBar->enterSpecialFuncEvent("1/(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1/(sin(2))");
    m_expressionBar->findChild<InputEdit *>()->setText("1/(2)");
    m_expressionBar->enterSpecialFuncEvent("1/(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1/(1/(2))");
    m_expressionBar->findChild<InputEdit *>()->setText("2(2)");
    m_expressionBar->enterSpecialFuncEvent("1/(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "2×1/((2))");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enterCosEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->enterFunctionEvent("cos");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "cos()");
    m_expressionBar->findChild<InputEdit *>()->setText("1111");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_expressionBar->enterFunctionEvent("cos");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1cos()111");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enter10xEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->findChild<InputEdit *>()->setText("1");
    m_expressionBar->enterConstantEvent("10^");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1×10^");
    m_expressionBar->findChild<InputEdit *>()->setText("11");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(1);
    m_expressionBar->enterConstantEvent("10^");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1×10^1");
    m_expressionBar->findChild<InputEdit *>()->setText("1111");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_expressionBar->enterConstantEvent("10^");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1×10^111");
    m_expressionBar->findChild<InputEdit *>()->setText("1＋");
    m_expressionBar->enterConstantEvent("10^");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1＋10^");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, enterModulusEvent)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->enterSpecialFuncEvent("abs(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "abs(0)");
    m_expressionBar->findChild<InputEdit *>()->setText("E");
    m_expressionBar->enterSpecialFuncEvent("abs(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "E");
    m_expressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_expressionBar->allElection();
    m_expressionBar->enterSpecialFuncEvent("abs(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "abs(");
    m_expressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_expressionBar->enterSpecialFuncEvent("abs(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1＋1");
    SSelection select;
    select.curpos = 0;
    select.oldText = "1＋1";
    select.selected = "1";
    m_expressionBar->findChild<InputEdit *>()->setSelection(select);
    m_expressionBar->enterSpecialFuncEvent("abs(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1＋1");
    m_expressionBar->findChild<InputEdit *>()->setText("1E-2");
    m_expressionBar->enterSpecialFuncEvent("abs(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "abs(1E－2)");
    m_expressionBar->findChild<InputEdit *>()->setText("－");
    m_expressionBar->enterSpecialFuncEvent("abs(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "－");
    m_expressionBar->findChild<InputEdit *>()->setText("()");
    m_expressionBar->enterSpecialFuncEvent("abs(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "()");
    m_expressionBar->findChild<InputEdit *>()->setText("((1))");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(4);
    m_expressionBar->enterSpecialFuncEvent("abs(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "(abs((1)))");
    m_expressionBar->findChild<InputEdit *>()->setText("sin(2)");
    m_expressionBar->enterSpecialFuncEvent("abs(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "abs(sin(2))");
    m_expressionBar->findChild<InputEdit *>()->setText("1/(2)");
    m_expressionBar->enterSpecialFuncEvent("abs(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "abs(1/(2))");
    m_expressionBar->findChild<InputEdit *>()->setText("2(2)");
    m_expressionBar->enterSpecialFuncEvent("abs(");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "2abs((2))");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, copyResultToClipboard)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->copyResultToClipboard();
    m_expressionBar->findChild<InputEdit *>()->setText("1＋2");
    m_expressionBar->allElection();
    m_expressionBar->copyResultToClipboard();
    EXPECT_EQ(QApplication::clipboard()->text(), "1＋2");
    m_expressionBar->copyClipboard2Result();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1＋2");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, copyClipboard2Result)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->findChild<InputEdit *>()->setText("huhuisinsjoi");
    m_expressionBar->allElection();
    m_expressionBar->copyResultToClipboard();
    m_expressionBar->copyClipboard2Result();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "sin");
    m_expressionBar->findChild<InputEdit *>()->setText("1.2＋3.4");
    SSelection select;
    select.curpos = 3;
    select.oldText = "1.2＋3.4";
    select.selected = "＋";
    m_expressionBar->findChild<InputEdit *>()->setSelection(select);
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(3);
    m_expressionBar->copyClipboard2Result();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1.2sin34");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, shear)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->findChild<InputEdit *>()->setText("1＋2");
    m_expressionBar->allElection();
    m_expressionBar->shear();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "");
    EXPECT_FALSE(m_expressionBar->m_isResult);
    EXPECT_TRUE(m_expressionBar->m_isContinue);
    EXPECT_FALSE(m_expressionBar->m_isUndo);
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, revisionResults)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->m_listModel->updataList(QString("1＋2") + "＝" + "3", -1);
    m_expressionBar->revisionResults(m_expressionBar->m_listModel->index(0, 0));
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1＋2");
    EXPECT_FALSE(m_expressionBar->m_isResult);
    EXPECT_FALSE(m_expressionBar->m_isUndo);
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, hisRevisionResults)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->m_listModel->updataList(QString("1＋2") + "＝" + "3", -1);
    m_expressionBar->hisRevisionResults(m_expressionBar->m_listModel->index(0, 0), Quantity(3));
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "3");
    EXPECT_FALSE(m_expressionBar->m_isResult);
    EXPECT_FALSE(m_expressionBar->m_isUndo);
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, judgeinput)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->findChild<InputEdit *>()->setText("sin");
    SSelection select;
    select.curpos = 1;
    select.oldText = "sin";
    select.selected = "i";
    m_expressionBar->findChild<InputEdit *>()->setSelection(select);
    EXPECT_FALSE(m_expressionBar->judgeinput());
    m_expressionBar->findChild<InputEdit *>()->setText("1sin");
    SSelection select0;
    select0.curpos = 0;
    select0.oldText = "1sin";
    select0.selected = "1";
    m_expressionBar->findChild<InputEdit *>()->setSelection(select0);
    EXPECT_TRUE(m_expressionBar->judgeinput());
    m_expressionBar->findChild<InputEdit *>()->setText("1sin");
    SSelection select1;
    select1.curpos = 0;
    select1.oldText = "1sin";
    select1.selected = "1s";
    m_expressionBar->findChild<InputEdit *>()->setSelection(select1);
    EXPECT_FALSE(m_expressionBar->judgeinput());
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, symbolComplement)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    EXPECT_EQ(m_expressionBar->symbolComplement("1("), "1×(");
    EXPECT_EQ(m_expressionBar->symbolComplement(")1"), ")×1");
    EXPECT_EQ(m_expressionBar->symbolComplement("e"), "(e)");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, pointFaultTolerance)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    EXPECT_EQ(m_expressionBar->pointFaultTolerance(")."), ")0.");
    EXPECT_EQ(m_expressionBar->pointFaultTolerance(".1"), "0.1");
    EXPECT_EQ(m_expressionBar->pointFaultTolerance("1+.2"), "1＋0.2");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, expressionCheck)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->findChild<InputEdit *>()->setText(".1");
    m_expressionBar->expressionCheck();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "0.1");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, Undo)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->Undo();
    m_expressionBar->findChild<InputEdit *>()->setText("1");
    m_expressionBar->m_isResult = true;
    m_expressionBar->m_listModel->updataList(QString("1＋2") + "＝" + "3", -1);
    m_expressionBar->m_undo.append("1");
    m_expressionBar->Undo();
    EXPECT_TRUE(m_expressionBar->m_isAllClear);
    EXPECT_TRUE(m_expressionBar->m_isUndo);
    EXPECT_FALSE(m_expressionBar->m_isResult);
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, Redo)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->Redo();
    m_expressionBar->m_listModel->updataList(QString("1＋2") + "＝" + "3", -1);
    m_expressionBar->m_redo.append("1");
    m_expressionBar->m_redo.append("");
    m_expressionBar->Redo();
    EXPECT_TRUE(m_expressionBar->m_isAllClear);
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, initTheme)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->initTheme(1);
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Text), "#303030");
    m_expressionBar->initTheme(2);
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Text), "#b4b4b4");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, setResultFalse)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->setResultFalse();
    EXPECT_FALSE(m_expressionBar->m_isResult);
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, replaceSelection)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->findChild<InputEdit *>()->setText("1111");
    SSelection select;
    select.curpos = 2;
    select.oldText = "1,111";
    select.selected = "1";
    m_expressionBar->findChild<InputEdit *>()->setSelection(select);
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_expressionBar->replaceSelection("1,111");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "111");
    m_expressionBar->findChild<InputEdit *>()->setText("1.111");
    SSelection select1;
    select1.curpos = 1;
    select1.oldText = "1.111";
    select1.selected = ".";
    m_expressionBar->findChild<InputEdit *>()->setSelection(select1);
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(1);
    m_expressionBar->replaceSelection("1.111");
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "1,111");
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, moveLeft)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->findChild<InputEdit *>()->setText("1＋2");
    m_expressionBar->moveLeft();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->cursorPosition(), 2);
    m_expressionBar->findChild<InputEdit *>()->setText("sin2");
    m_expressionBar->moveLeft();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->cursorPosition(), 3);
    m_expressionBar->moveLeft();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->cursorPosition(), 0);
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, moveRight)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->findChild<InputEdit *>()->setText("1＋2");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_expressionBar->moveRight();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->cursorPosition(), 1);
    m_expressionBar->findChild<InputEdit *>()->setText("sin2");
    m_expressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_expressionBar->moveRight();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->cursorPosition(), 3);
    m_expressionBar->moveRight();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->cursorPosition(), 4);
    delete m_expressionBar;
}

TEST_F(Ut_SciexpressionBar, deleteText)
{
    SciExpressionBar *m_expressionBar = new SciExpressionBar;
    m_expressionBar->findChild<InputEdit *>()->setText("1＋2");
    m_expressionBar->allElection();
    m_expressionBar->deleteText();
    EXPECT_EQ(m_expressionBar->findChild<InputEdit *>()->text(), "");
    EXPECT_TRUE(m_expressionBar->m_isContinue);
    EXPECT_FALSE(m_expressionBar->m_isUndo);
    EXPECT_FALSE(m_expressionBar->m_isResult);
    delete m_expressionBar;
}
