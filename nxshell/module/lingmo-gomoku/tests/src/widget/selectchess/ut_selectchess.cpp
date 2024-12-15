// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_selectchess.h"

#include <DHiDPIHelper>
#include <QPaintEvent>
#include "constants.h"

TEST_F(UT_SelectChess, UT_SelectChess_initUI)
{
    m_SelectChess->initUI();
}

TEST_F(UT_SelectChess, UT_SelectChess_initBackgroundPix1)
{
    m_SelectChess->compositingStatus = true;
    m_SelectChess->initBackgroundPix();
    EXPECT_EQ(m_SelectChess->backgroundPixmap, DHiDPIHelper::loadNxPixmap(":/resources/chessselected/selectbase.svg"))
            << "check seleceChess initBackgroundPix";
}

TEST_F(UT_SelectChess, UT_SelectChess_initBackgroundPix2)
{
    m_SelectChess->compositingStatus = false;
    m_SelectChess->initBackgroundPix();
    EXPECT_EQ(m_SelectChess->backgroundPixmap, DHiDPIHelper::loadNxPixmap(":/resources/chessselected/selectbase_nshadow.svg"))
            << "check seleceChess initBackgroundPix";
}

TEST_F(UT_SelectChess, UT_SelectChess_slotButtonOKClicked)
{
    m_SelectChess->slotButtonOKClicked();
}

TEST_F(UT_SelectChess, UT_SelectChess_setSelectChess1)
{
    m_SelectChess->setSelectChess(chess_black);
    EXPECT_EQ(m_SelectChess->selectChessColor, chess_black)
            << "check selectChess setSelectChess";
}

TEST_F(UT_SelectChess, UT_SelectChess_setSelectChess2)
{
    m_SelectChess->setSelectChess(chess_white);
    EXPECT_EQ(m_SelectChess->selectChessColor, chess_white)
            << "check selectChess setSelectChess";
}

TEST_F(UT_SelectChess, UT_SelectChess_getSelectChess1)
{
    m_SelectChess->selectLButton->setChecked(true);
    int res = m_SelectChess->getSelsectChess();
    EXPECT_EQ(res, chess_white)
            << "check selectChess setSelectChess";
}

TEST_F(UT_SelectChess, UT_SelectChess_getSelectChess2)
{
    m_SelectChess->selectLButton->setChecked(false);
    int res = m_SelectChess->getSelsectChess();
    EXPECT_EQ(res, chess_black)
            << "check selectChess setSelectChess";
}

TEST_F(UT_SelectChess, UT_SelectChess_selectClose)
{
    m_SelectChess->selectClose();
}

TEST_F(UT_SelectChess, UT_SelectChess_slotCloseSelectPopup)
{
    m_SelectChess->slotCloseSelectPopup();
}

TEST_F(UT_SelectChess, UT_SelectChess_slotCompositingChanged)
{
    m_SelectChess->slotCompositingChanged(true);
    EXPECT_EQ(m_SelectChess->compositingStatus, true)
            << "check selectChess compositingChanged";
}

TEST_F(UT_SelectChess, UT_SelectChess_paintEvent)
{
    QRect rect;
    QPaintEvent event(rect);
    m_SelectChess->paintEvent(&event);
}
