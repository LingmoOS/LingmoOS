// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_proexpressionbar.h"

#include "../../src/widgets/proexpressionbar.h"
#include "../../3rdparty/core/settings.h"
#include "../../src/mainwindow.h"
#include <QDir>
#include <QLocale>
#include <QSettings>
#include <QApplication>
#include <QFont>
#include <QtCore/QStandardPaths>
#include <QClipboard>

Ut_ProexpressionBar::Ut_ProexpressionBar()
{
}

TEST_F(Ut_ProexpressionBar, mouseMoveEvent)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar;
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseMove, m_proexpressionBar->pos(), Qt::MouseButton::LeftButton, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    m_proexpressionBar->mouseMoveEvent(m);
    delete m;
    //取消move效果，无assert
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, isnumber)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar;
    QChar a = 'A';
    EXPECT_TRUE(m_proexpressionBar->m_inputEdit->isNumber(a));
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, judgeinput)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar;
    m_proexpressionBar->m_inputEdit->setText("and");
    SSelection select;
    select.curpos = 1;
    select.oldText = "and";
    select.selected = "n";
    m_proexpressionBar->m_inputEdit->setSelection(select);
    EXPECT_FALSE(m_proexpressionBar->judgeinput());
    m_proexpressionBar->m_inputEdit->setText("1and");
    SSelection select0;
    select0.curpos = 0;
    select0.oldText = "1and";
    select0.selected = "1";
    m_proexpressionBar->m_inputEdit->setSelection(select0);
    EXPECT_TRUE(m_proexpressionBar->judgeinput());
    m_proexpressionBar->m_inputEdit->setText("1and");
    SSelection select1;
    select1.curpos = 0;
    select1.oldText = "1and";
    select1.selected = "1a";
    m_proexpressionBar->m_inputEdit->setSelection(select1);
    EXPECT_FALSE(m_proexpressionBar->judgeinput());
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, enterNumberEvent)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar;
    //    Settings::instance()->programmerBase = 16;
    m_proexpressionBar->m_inputNumber = true;
    m_proexpressionBar->m_isResult = true;
    m_proexpressionBar->m_isContinue = false;
    m_proexpressionBar->enterNumberEvent("1");
    EXPECT_FALSE(m_proexpressionBar->m_inputEdit->text().isEmpty());
    m_proexpressionBar->m_inputNumber = false;
    m_proexpressionBar->m_isResult = true;
    m_proexpressionBar->enterNumberEvent("1");
    m_proexpressionBar->enterNumberEvent("8");
    m_proexpressionBar->enterNumberEvent("2");
    m_proexpressionBar->enterNumberEvent("2");
    m_proexpressionBar->enterNumberEvent("2");
    EXPECT_TRUE(m_proexpressionBar->m_inputEdit->text() == "18,222");
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, enterSymbolEvent)
{
    /*
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar;
    m_proexpressionBar->enterSymbolEvent("＋");
    m_proexpressionBar->findChild<InputEdit *>()->clear();
    m_proexpressionBar->enterSymbolEvent("-");
    m_proexpressionBar->enterSymbolEvent("-");
    m_proexpressionBar->enterSymbolEvent("＋");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_proexpressionBar->enterSymbolEvent("-");
    m_proexpressionBar->findChild<InputEdit *>()->setText("1");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_proexpressionBar->enterSymbolEvent("-");
    m_proexpressionBar->findChild<InputEdit *>()->setText("1＋2");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(1);
    m_proexpressionBar->enterSymbolEvent("-");
    m_proexpressionBar->findChild<InputEdit *>()->setText("11");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(1);
    m_proexpressionBar->enterSymbolEvent("-");
    ASSERT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "1－1");
    DSettingsAlt::deleteInstance();
    */
}

TEST_F(Ut_ProexpressionBar, enterBackspaceEvent)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar;
    m_proexpressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_proexpressionBar->allElection();
    m_proexpressionBar->enterBackspaceEvent();
    m_proexpressionBar->enterNotEvent();
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_proexpressionBar->enterBackspaceEvent();
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_proexpressionBar->enterOperatorEvent("and");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_proexpressionBar->enterBackspaceEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "0(0)");
    m_proexpressionBar->findChild<InputEdit *>()->setText("1 and 2");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(6);
    m_proexpressionBar->enterBackspaceEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "1  2");
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, enterClearEvent)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar;
    m_proexpressionBar->m_isAllClear = false;
    m_proexpressionBar->findChild<InputEdit *>()->setText("1＋1");
    m_proexpressionBar->enterClearEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "");
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, enterEqualEvent)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar;
    m_proexpressionBar->enterNumberEvent("0");
    m_proexpressionBar->enterOperatorEvent("and");
    m_proexpressionBar->enterNumberEvent("1");
    m_proexpressionBar->enterEqualEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "0");

    m_proexpressionBar->enterSymbolEvent("-");
    m_proexpressionBar->enterSymbolEvent("+");
    m_proexpressionBar->enterNumberEvent("5");
    m_proexpressionBar->enterEqualEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "5");

    m_proexpressionBar->enterSymbolEvent("/");
    m_proexpressionBar->enterNumberEvent("3");
    m_proexpressionBar->enterOperatorEvent("or");
    m_proexpressionBar->enterNumberEvent("5");
    m_proexpressionBar->enterNumberEvent("6");
    m_proexpressionBar->enterEqualEvent();

    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "57");
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, enterNotEvent)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar;
    m_proexpressionBar->enterNotEvent();
    m_proexpressionBar->enterClearEvent();
    m_proexpressionBar->enterNumberEvent("5");
    m_proexpressionBar->enterNotEvent();
    m_proexpressionBar->enterEqualEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "－6");
    m_proexpressionBar->findChild<InputEdit *>()->setText("1 and (1 and 2)");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(15);
    m_proexpressionBar->enterNotEvent();
    m_proexpressionBar->enterEqualEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "1");
    m_proexpressionBar->findChild<InputEdit *>()->setText("()");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_proexpressionBar->enterNotEvent();
    EXPECT_TRUE(m_proexpressionBar->m_isResult);
    EXPECT_FALSE(m_proexpressionBar->m_isUndo);
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, enterOperatorEvent)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar;
    m_proexpressionBar->enterOperatorEvent("ror");
    m_proexpressionBar->enterClearEvent();
    m_proexpressionBar->enterNumberEvent("5");
    m_proexpressionBar->enterOperatorEvent("sal");
    m_proexpressionBar->enterOperatorEvent("rcl");
    m_proexpressionBar->enterOperatorEvent("ror");
    m_proexpressionBar->enterNumberEvent("4");
    m_proexpressionBar->enterEqualEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "5,764,607,523,034,234,880");
    m_proexpressionBar->enterOperatorEvent("rcl");
    m_proexpressionBar->enterNumberEvent("3");
    m_proexpressionBar->enterEqualEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "－9,223,372,036,854,775,807");
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, enterOppositeEvent)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->enterOppositeEvent();
    EXPECT_TRUE(m_proexpressionBar->findChild<InputEdit *>()->text().isEmpty());
    m_proexpressionBar->enterNumberEvent("0");
    m_proexpressionBar->enterOppositeEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "0");
    m_proexpressionBar->enterClearEvent();
    m_proexpressionBar->enterSymbolEvent("-");
    m_proexpressionBar->enterNumberEvent("1");
    m_proexpressionBar->enterOppositeEvent();
    m_proexpressionBar->enterEqualEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "－1");
    Settings::instance()->programmerBase = 10;
    m_proexpressionBar->findChild<InputEdit *>()->setText("1 and (1 and 2)");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(15);
    m_proexpressionBar->enterOppositeEvent();
    m_proexpressionBar->enterEqualEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "0");
    Settings::instance()->programmerBase = 16;
    m_proexpressionBar->findChild<InputEdit *>()->setText("1");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(1);
    m_proexpressionBar->enterOppositeEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "FFFF FFFF FFFF FFFF");
    Settings::instance()->programmerBase = 0;
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, enterLeftBracketsEvent)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->findChild<InputEdit *>()->setText("3");
    m_proexpressionBar->m_isUndo = true;
    m_proexpressionBar->enterLeftBracketsEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "3(");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_proexpressionBar->enterLeftBracketsEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "(3(");
    m_proexpressionBar->findChild<InputEdit *>()->setText("1111");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_proexpressionBar->enterLeftBracketsEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "1(111");
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, enterRightBracketsEvent)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->findChild<InputEdit *>()->setText("3");
    m_proexpressionBar->m_isUndo = true;
    m_proexpressionBar->enterRightBracketsEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "3)");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_proexpressionBar->enterRightBracketsEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), ")3)");
    m_proexpressionBar->findChild<InputEdit *>()->setText("1111");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_proexpressionBar->enterRightBracketsEvent();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "1)111");
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, moveLeft)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->findChild<InputEdit *>()->setText("1＋2");
    m_proexpressionBar->moveLeft();
    m_proexpressionBar->findChild<InputEdit *>()->setText("and2");
    m_proexpressionBar->moveLeft();
    m_proexpressionBar->moveLeft();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->cursorPosition(), 0);
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, moveRight)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->findChild<InputEdit *>()->setText("1＋2");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_proexpressionBar->moveRight();
    m_proexpressionBar->findChild<InputEdit *>()->setText("and2");
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(0);
    m_proexpressionBar->moveRight();
    m_proexpressionBar->moveRight();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->cursorPosition(), 4);
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, initTheme)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->initTheme(2);
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Text), "#b4b4b4");
    m_proexpressionBar->initTheme(1);
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Text), "#303030");
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, revisionResults)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->m_listModel->updataList(QString("1＋2") + "＝" + "3", -1);
    m_proexpressionBar->revisionResults(m_proexpressionBar->m_listModel->index(0, 0));
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "1＋2");
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, addUndo)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->m_inputEdit->setText("110,911");
    m_proexpressionBar->addUndo();
    EXPECT_EQ(m_proexpressionBar->m_undo.at(0), "110,911");
    EXPECT_TRUE(m_proexpressionBar->m_redo.isEmpty());
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, Undo)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->Undo();
    m_proexpressionBar->findChild<InputEdit *>()->setText("1");
    m_proexpressionBar->m_isResult = true;
    m_proexpressionBar->m_listModel->updataList(QString("1＋2") + "＝" + "3", -1);
    m_proexpressionBar->m_undo.append("1");
    m_proexpressionBar->Undo();
    EXPECT_TRUE(m_proexpressionBar->m_isAllClear);
    EXPECT_TRUE(m_proexpressionBar->m_isUndo);
    EXPECT_FALSE(m_proexpressionBar->m_isResult);
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, Redo)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->Redo();
    m_proexpressionBar->m_listModel->updataList(QString("1＋2") + "＝" + "3", -1);
    m_proexpressionBar->m_redo.append("1");
    m_proexpressionBar->m_redo.append("");
    m_proexpressionBar->Redo();
    EXPECT_TRUE(m_proexpressionBar->m_isAllClear);
    EXPECT_TRUE(m_proexpressionBar->m_inputEdit->m_redo->isEnabled());
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, copyResultToClipboard)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->copyResultToClipboard();
    m_proexpressionBar->findChild<InputEdit *>()->setText("1＋2");
    m_proexpressionBar->allElection();
    m_proexpressionBar->copyResultToClipboard();
    EXPECT_EQ(QApplication::clipboard()->text(), "1＋2");
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, copyClipboard2Result)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->findChild<InputEdit *>()->setText("huhuiandsjoi");
    m_proexpressionBar->allElection();
    m_proexpressionBar->copyResultToClipboard();
    m_proexpressionBar->findChild<InputEdit *>()->setText("112＋334");
    SSelection select;
    select.curpos = 3;
    select.oldText = "112＋334";
    select.selected = "＋";
    m_proexpressionBar->findChild<InputEdit *>()->setSelection(select);
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(3);
    m_proexpressionBar->copyClipboard2Result();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "112and334");

    m_proexpressionBar->enterClearEvent();
    QApplication::clipboard()->setText("11+9");
    Settings::instance()->programmerBase = 16;
    m_proexpressionBar->copyClipboard2Result();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "11＋9");
    m_proexpressionBar->enterClearEvent();

    Settings::instance()->programmerBase = 8;
    m_proexpressionBar->copyClipboard2Result();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "11＋");
    m_proexpressionBar->enterClearEvent();

    Settings::instance()->programmerBase = 2;
    m_proexpressionBar->copyClipboard2Result();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "11＋");
    m_proexpressionBar->enterClearEvent();

    Settings::instance()->programmerBase = 0;
    m_proexpressionBar->copyClipboard2Result();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "11＋9");

    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, allElection)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->findChild<InputEdit *>()->setText("123or321");
    m_proexpressionBar->allElection();
    EXPECT_EQ(m_proexpressionBar->m_inputEdit->getSelection().selected, "123or321");
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, shear)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->findChild<InputEdit *>()->setText("1＋2");
    m_proexpressionBar->allElection();
    m_proexpressionBar->shear();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "");
    EXPECT_FALSE(m_proexpressionBar->m_isResult);
    EXPECT_TRUE(m_proexpressionBar->m_isContinue);
    EXPECT_FALSE(m_proexpressionBar->m_isUndo);
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, deleteText)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->findChild<InputEdit *>()->setText("1＋2");
    m_proexpressionBar->allElection();
    m_proexpressionBar->deleteText();
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "");
    EXPECT_FALSE(m_proexpressionBar->m_isResult);
    EXPECT_TRUE(m_proexpressionBar->m_isContinue);
    EXPECT_FALSE(m_proexpressionBar->m_isUndo);
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, setResultFalse)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->setResultFalse();
    EXPECT_FALSE(m_proexpressionBar->m_isResult);
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, replaceSelection)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->findChild<InputEdit *>()->setText("1111");
    SSelection select;
    select.curpos = 2;
    select.oldText = "1,111";
    select.selected = "1";
    m_proexpressionBar->findChild<InputEdit *>()->setSelection(select);
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(2);
    m_proexpressionBar->replaceSelection("1,111");
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "111");

    m_proexpressionBar->findChild<InputEdit *>()->setText("1%111");
    SSelection select1;
    select1.curpos = 1;
    select1.oldText = "1.111";
    select1.selected = "%";
    m_proexpressionBar->findChild<InputEdit *>()->setSelection(select1);
    m_proexpressionBar->findChild<InputEdit *>()->setCursorPosition(1);
    m_proexpressionBar->replaceSelection("1%111");
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "1,111");
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, isNumberOutOfRange)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    Settings::instance()->programmerBase = 10;
    m_proexpressionBar->findChild<InputEdit *>()->setText("9,223,372,036,854,775,807");
    EXPECT_TRUE(m_proexpressionBar->isNumberOutOfRange("1"));
    Settings::instance()->proBitLength = 32;
    m_proexpressionBar->findChild<InputEdit *>()->setText("2147483647");
    EXPECT_TRUE(m_proexpressionBar->isNumberOutOfRange("1"));
    Settings::instance()->proBitLength = 16;
    m_proexpressionBar->findChild<InputEdit *>()->setText("32767");
    EXPECT_TRUE(m_proexpressionBar->isNumberOutOfRange("1"));
    Settings::instance()->proBitLength = 8;
    m_proexpressionBar->findChild<InputEdit *>()->setText("127");
    EXPECT_TRUE(m_proexpressionBar->isNumberOutOfRange("1"));
    Settings::instance()->proBitLength = 64;
    Settings::instance()->programmerBase = 0;
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, selectedPartDelete)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->findChild<InputEdit *>()->setText("111and22");
    SSelection select1;
    select1.curpos = 2;
    select1.oldText = "111and22";
    select1.selected = "1an";
    m_proexpressionBar->m_inputEdit->setSelection(select1);
    QString sRegNum = "[a-z]";
    QRegExp rx;
    rx.setPattern(sRegNum);
    m_proexpressionBar->selectedPartDelete(rx);
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "1,122");

    m_proexpressionBar->findChild<InputEdit *>()->setText("1and2or3");
    select1.curpos = 3;
    select1.oldText = "1and2or3";
    select1.selected = "d2o";
    m_proexpressionBar->m_inputEdit->setSelection(select1);
    m_proexpressionBar->selectedPartDelete(rx);
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "13");

    m_proexpressionBar->findChild<InputEdit *>()->setText("12or3");
    select1.curpos = 3;
    select1.oldText = "12or3";
    select1.selected = "r3";
    m_proexpressionBar->m_inputEdit->setSelection(select1);
    m_proexpressionBar->selectedPartDelete(rx);
    EXPECT_EQ(m_proexpressionBar->findChild<InputEdit *>()->text(), "12");
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, handleTextChanged)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    m_proexpressionBar->handleTextChanged();
    EXPECT_FALSE(m_proexpressionBar->m_isAllClear);
    EXPECT_TRUE(m_proexpressionBar->m_isContinue);
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, isOperator)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    EXPECT_TRUE(m_proexpressionBar->isOperator(QString::fromUtf8("×")));
    delete m_proexpressionBar;
}

TEST_F(Ut_ProexpressionBar, symbolFaultTolerance)
{
    ProExpressionBar *m_proexpressionBar = new ProExpressionBar();
    QString str = m_proexpressionBar->symbolFaultTolerance("123＋－");
    EXPECT_EQ(str, "123－");
    delete m_proexpressionBar;
}
