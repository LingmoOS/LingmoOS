// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_ddropdownmenu.h"
#include "../stub.h"

#include <QKeyEvent>
#include <QFlags>
#include <QSignalSpy>
#include <QColor>

// 测试函数 DDropdownMenu::setFontEx
TEST_F(test_ddropdownmenu, setFontEx)
{

    DDropdownMenu *dropMenu = new DDropdownMenu();
    dropMenu->setFontEx(QFont());
    EXPECT_EQ(dropMenu->m_font, QFont());

    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();

}

// 测试函数 DDropdownMenu::setCurrentAction
TEST_F(test_ddropdownmenu, setCurrentAction)
{

    // 测试场景1: 非空指针
    DDropdownMenu *dropMenu = new DDropdownMenu();
    QAction *action = new QAction();
    dropMenu->setCurrentAction(action);
    EXPECT_EQ(action->isCheckable(),false);

    EXPECT_NE(action,nullptr);
    EXPECT_NE(dropMenu,nullptr);
    action->deleteLater();
    dropMenu->deleteLater();


    // 测试场景2: 空指针
    dropMenu = new DDropdownMenu();
    dropMenu->setCurrentAction(nullptr);
    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();

}

// 测试函数 DDropdownMenu::setCurrentTextOnly
TEST_F(test_ddropdownmenu, setCurrentTextOnly)
{

    DDropdownMenu *dropMenu = new DDropdownMenu();
    dropMenu->setCurrentTextOnly("test");
    EXPECT_EQ(dropMenu->m_text,"test");

    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();

}

// 测试函数 DDropdownMenu::setText
TEST_F(test_ddropdownmenu, setText)
{

    DDropdownMenu *dropMenu = new DDropdownMenu();
    dropMenu->setText("test");
    EXPECT_EQ(dropMenu->m_text, "test");

    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();


}

// 测试函数 DDropdownMenu::setTheme
TEST_F(test_ddropdownmenu, setTheme)
{

    // 场景1: 正确主题
    DDropdownMenu *dropMenu = new DDropdownMenu();
    dropMenu->setTheme("dark");
    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();




    // 场景2: 错误主题
    dropMenu = new DDropdownMenu();
    dropMenu->setTheme("error");
    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();


}

// 测试函数 DDropdownMenu::setChildrenFocus
TEST_F(test_ddropdownmenu, setChildrenFocus)
{

    // 场景1: true
    DDropdownMenu *dropMenu = new DDropdownMenu();
    dropMenu->setChildrenFocus(true);
    QFlags<Qt::FocusPolicy> flags = QFlags<Qt::FocusPolicy>(dropMenu->m_pToolButton->focusPolicy());
    EXPECT_EQ(flags.testFlag(Qt::StrongFocus), true);
    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();




    // 场景2:false
    dropMenu = new DDropdownMenu();
    dropMenu->setChildrenFocus(false);
    QFlags<Qt::FocusPolicy> flags2 = QFlags<Qt::FocusPolicy>(dropMenu->m_pToolButton->focusPolicy());
    EXPECT_EQ(flags2.testFlag(Qt::NoFocus), true);
    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();


}

// 测试函数 DDropdownMenu::getButton
TEST_F(test_ddropdownmenu, getButton)
{

    DDropdownMenu *dropMenu = new DDropdownMenu();
    DToolButton *button = dropMenu->getButton();
    EXPECT_NE(dropMenu->m_pToolButton, nullptr);
    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();


}

bool createEncodeMenu_isEmpty_stub()
{
    return true;
}

// 测试函数 DDropdownMenu::createEncodeMenu
TEST_F(test_ddropdownmenu, createEncodeMenu)
{
    // 场景2: sm_groupEncodeVec为空
    Stub stub;
    typedef QVector<QPair<QString, QStringList> > VecType;
    stub.set(ADDR(VecType, isEmpty), createEncodeMenu_isEmpty_stub);
    DDropdownMenu *dropMenu = DDropdownMenu::createEncodeMenu();
    ASSERT_NE(dropMenu, nullptr);
    EXPECT_TRUE(dropMenu->m_menu->actions().isEmpty());
    dropMenu->deleteLater();

    // 场景1: sm_groupEncodeVec不为空
    stub.reset(ADDR(VecType, isEmpty));
    dropMenu = DDropdownMenu::createEncodeMenu();
    ASSERT_NE(dropMenu, nullptr);
    EXPECT_FALSE(dropMenu->m_menu->actions().isEmpty());
    dropMenu->deleteLater();
}

// 测试函数 DDropdownMenu::createHighLightMenu
TEST_F(test_ddropdownmenu, createHighLightMenu)
{

    DDropdownMenu *dropMenu = new DDropdownMenu();
    dropMenu->createHighLightMenu();
    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();

}

// 测试函数 DDropdownMenu::createIcon
TEST_F(test_ddropdownmenu, createIcon)
{

    // 场景1: m_bPressed为true
    DDropdownMenu *dropMenu = new DDropdownMenu();
    dropMenu->m_bPressed = true;
    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();


    // 场景2: m_bPressed为false
    dropMenu = new DDropdownMenu();
    dropMenu->m_bPressed = false;
    dropMenu->createIcon();
    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();


}

// 测试函数 DDropdownMenu::OnFontChangedSlot
TEST_F(test_ddropdownmenu, OnFontChangedSlot)
{

    DDropdownMenu *dropMenu = new DDropdownMenu();
    dropMenu->OnFontChangedSlot(QFont());
    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();

}

// 测试函数 DDropdownMenu::eventFilter
TEST_F(test_ddropdownmenu, eventFilter)
{
    //    do {
    //        // 场景1: enter键的keypress事件
    //        DDropdownMenu *dropMenu = new DDropdownMenu();
    //        QSignalBlocker signalBlock(dropMenu);
    //        QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    //        bool result = dropMenu->eventFilter(dropMenu->m_pToolButton, event);
    //        EXPECT_TRUE(result);
    //        delete event;
    //        delete dropMenu;
    //    } while (false);

    //    do {
    //        // 场景2: Key_Left键的keypress事件
    //        DDropdownMenu *dropMenu = new DDropdownMenu();
    //        QSignalBlocker signalBlock(dropMenu);
    //        QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    //        bool result = dropMenu->eventFilter(dropMenu->m_pToolButton, event);
    //        EXPECT_FALSE(result);
    //        delete event;
    //        delete dropMenu;
    //    } while (false);

    //    do {
    //        // 场景3: MouseButtonPress事件
    //        DDropdownMenu *dropMenu = new DDropdownMenu();
    //        QSignalBlocker signalBlock(dropMenu);
    //        QSignalSpy spy(dropMenu, &DDropdownMenu::requestContextMenu);
    //        QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, QPointF(),
    //                                             Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    //        bool result = dropMenu->eventFilter(dropMenu->m_pToolButton, event);
    //        EXPECT_FALSE(result);
    //        delete event;
    //        delete dropMenu;
    //    } while (false);

    //    do {
    //        // 场景4: LeftButton的MouseButtonRelease事件
    //        DDropdownMenu *dropMenu = new DDropdownMenu();
    //        QSignalBlocker signalBlock(dropMenu);
    //        QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonRelease, QPointF(),
    //                                             Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    //        bool result = dropMenu->eventFilter(dropMenu->m_pToolButton, event);
    //        EXPECT_TRUE(result);
    //        delete event;
    //        delete dropMenu;
    //    } while (false);

    //    do {
    //        // 场景5: 非LeftButton的MouseButtonRelease事件
    //        DDropdownMenu *dropMenu = new DDropdownMenu();
    //        QSignalBlocker signalBlock(dropMenu);
    //        QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonRelease, QPointF(),
    //                                             Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    //        bool result = dropMenu->eventFilter(dropMenu->m_pToolButton, event);
    //        EXPECT_TRUE(result);
    //        delete event;
    //        delete dropMenu;
    //    } while (false);
}

// 测试函数 DDropdownMenu::setSvgColor
TEST_F(test_ddropdownmenu, setSvgColor)
{

    DDropdownMenu *dropMenu = new DDropdownMenu();
    EXPECT_NE(dropMenu->setSvgColor("#FF0000").isNull(),true);
    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();

}

// 测试函数 DDropdownMenu::SetSVGBackColor
TEST_F(test_ddropdownmenu, SetSVGBackColor)
{

    QByteArray data = "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"
                      "<text x=\"0\" y=\"15\" fill=\"red\">I love SVG</text>"
                      "</svg>";
    QDomDocument doc;
    doc.setContent(data);
    QDomElement elem = doc.documentElement();
    DDropdownMenu *dropMenu = new DDropdownMenu();
    dropMenu->SetSVGBackColor(elem, "fill", "#FF0000");

    EXPECT_NE(dropMenu,nullptr);
    dropMenu->deleteLater();

}
