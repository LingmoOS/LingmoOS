// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_bottombar.h"

#include <QPushButton>
#include <QPaintEvent>

// 测试函数 BottomBar::updatePosition
TEST_F(TestBottomBar, checkUpdatePosition)
{
    auto bottomBar = new BottomBar;
    bottomBar->updatePosition(1, 1);

    EXPECT_EQ(bottomBar->m_pPositionLabel->text().contains("1"),true);

    EXPECT_NE(bottomBar,nullptr);
    bottomBar->deleteLater();
}

// 测试函数 BottomBar::updateWordCount
TEST_F(TestBottomBar, checkUpdateWordCount)
{
    auto bottomBar = new BottomBar;
    bottomBar->updateWordCount(1);

    EXPECT_EQ(bottomBar->m_pCharCountLabel->text().contains("0"),true);

    EXPECT_NE(bottomBar,nullptr);
    bottomBar->deleteLater();
}

// 测试函数 BottomBar::setEncodeName
TEST_F(TestBottomBar, checkSetEncodeName)
{
    auto bottomBar = new BottomBar;
    bottomBar->setEncodeName("UTF-8");


    EXPECT_NE(bottomBar,nullptr);
    bottomBar->deleteLater();
}

// 测试函数 BottomBar::setCursorStatus
TEST_F(TestBottomBar, checkSetCursorStatus)
{
    auto bottomBar = new BottomBar;
    bottomBar->setCursorStatus("INSERT");
    EXPECT_EQ(bottomBar->m_pCursorStatus->text(),"INSERT");


    EXPECT_NE(bottomBar,nullptr);
    bottomBar->deleteLater();
}

// 测试函数 BottomBar::setPalette
TEST_F(TestBottomBar, checkSetPalette)
{
    auto bottomBar = new BottomBar;

    QString backgroundColor = "#f8f8f8";
    QString textColor = "#1f1c1b";
    QPalette palette = bottomBar->palette();
    palette.setColor(QPalette::Background, backgroundColor);
    palette.setColor(QPalette::Text, textColor);
    bottomBar->setPalette(palette);

    EXPECT_NE(bottomBar,nullptr);
    bottomBar->deleteLater();
}

// 测试函数 BottomBar::updateSize
TEST_F(TestBottomBar, checkUpdateSize)
{
    auto bottomBar = new BottomBar;
    bottomBar->updateSize(32, false);
    EXPECT_EQ(bottomBar->height(),32);


    EXPECT_NE(bottomBar,nullptr);
    bottomBar->deleteLater();
}

// 测试函数 BottomBar::setChildrenFocus
TEST_F(TestBottomBar, checkSetChildrenFocus)
{

    // 场景1: ok = false, preOrderWidget = nullptr
    auto bottomBar = new BottomBar;
    bottomBar->setChildrenFocus(false, nullptr);
    EXPECT_NE(bottomBar->m_pEncodeMenu->hasFocus(),true);
    EXPECT_NE(bottomBar,nullptr);
    bottomBar->deleteLater();




    // 场景2: ok = true, preOrderWidget = nullptr
    bottomBar = new BottomBar;
    bottomBar->setChildrenFocus(true, nullptr);
    EXPECT_NE(bottomBar->m_pEncodeMenu->hasFocus(),true);
    EXPECT_NE(bottomBar,nullptr);
    bottomBar->deleteLater();



    // 场景3: ok = true, preOrderWidget = new QWidget()
    bottomBar = new BottomBar;
    QPushButton *button = new QPushButton();
    bottomBar->setChildrenFocus(true, button);
    EXPECT_NE(button->hasFocus(),true);
    EXPECT_NE(bottomBar,nullptr);
    EXPECT_NE(button,nullptr);
    bottomBar->deleteLater();
    button->deleteLater();



    // 场景4: ok = false, preOrderWidget = new QWidget()
    bottomBar = new BottomBar;
    button = new QPushButton();
    bottomBar->setChildrenFocus(false, button);
    EXPECT_NE(button->hasFocus(),true);
    EXPECT_NE(bottomBar,nullptr);
    EXPECT_NE(button,nullptr);
    bottomBar->deleteLater();
    button->deleteLater();

}

// 测试函数 BottomBar::getEncodeMenu
TEST_F(TestBottomBar, checkGetEncodeMenu)
{

    auto bottomBar = new BottomBar;
    DDropdownMenu *menu = bottomBar->getEncodeMenu();
    EXPECT_NE(menu, nullptr);
    EXPECT_NE(bottomBar,nullptr);
    bottomBar->deleteLater();

}

// 测试函数 BottomBar::getHighlightMenu
TEST_F(TestBottomBar, checkGetHighlightMenu)
{
    auto bottomBar = new BottomBar;
    DDropdownMenu *menu = bottomBar->getHighlightMenu();
    EXPECT_NE(menu, nullptr);
    EXPECT_NE(bottomBar,nullptr);
    bottomBar->deleteLater();

}

// 测试函数 BottomBar::paintEvent
TEST_F(TestBottomBar, checkPaintEvent)
{
    auto bottomBar = new BottomBar;
    QPaintEvent event(bottomBar->rect());
    bottomBar->paintEvent(&event);
    EXPECT_NE(bottomBar,nullptr);
    bottomBar->deleteLater();

}
