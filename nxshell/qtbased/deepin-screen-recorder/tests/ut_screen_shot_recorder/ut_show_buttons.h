// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QPoint>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/screenshot.h"
#include "../../src/keydefine.h"


using namespace testing;

class ShowButtonsTest: public testing::Test
{

public:
    Stub stub;
    ShowButtons showButtons;
    virtual void SetUp() override
    {
        std::cout << "start ShowButtonsTest" << std::endl;
    }

    virtual void TearDown() override
    {
        std::cout << "end ShowButtonsTest" << std::endl;
    }
};

TEST_F(ShowButtonsTest, releaseContentButtons)
{

}


TEST_F(ShowButtonsTest, showContentButtons)
{
    showButtons.showContentButtons(KEY_1);
    showButtons.releaseContentButtons(KEY_1);

    showButtons.showContentButtons(KEY_2);
    showButtons.showContentButtons(KEY_2);
    showButtons.showContentButtons(KEY_2);
    showButtons.releaseContentButtons(KEY_2);
}

TEST_F(ShowButtonsTest, getKeyCodeFromEvent)
{
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUMENTER).compare(QString("NUM =")) == 0);

}

TEST_F(ShowButtonsTest, getKeyCodeFromEventWayland)
{
    EXPECT_TRUE(showButtons.getKeyCodeFromEventWayland(KEY_ENTER).compare(QString("ENTER")) == 0);

}

TEST_F(ShowButtonsTest, showContentButtons1)
{
    showButtons.showContentButtons('K');
}
TEST_F(ShowButtonsTest, showContentButtons2)
{
    showButtons.showContentButtons(' ');
}

TEST_F(ShowButtonsTest, releaseContentButtons1)
{
    showButtons.releaseContentButtons('K');
}
