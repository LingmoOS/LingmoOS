// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "content.h"
#undef private

#include <QPushButton>

#include <gtest/gtest.h>

class UT_Content : public testing::Test
{
public:
    void SetUp() override;


    void TearDown() override;

public:
    Content *widget = nullptr;
};

void UT_Content::SetUp()
{
    widget = new Content();
}

void UT_Content::TearDown()
{
    widget->deleteLater();
    widget = nullptr;
}

TEST_F(UT_Content, coverageTest)
{
    widget->setCnSource("/usr/share/deepin-deepinid-client/privacy/deepinid-CN-zh_CN.md");
    widget->setEnSource("/usr/share/deepin-deepinid-client/privacy/deepinid-US-en_US.md");
    widget->setHideBottom(false);
    Q_EMIT widget->m_languageBtn->buttonClicked(nullptr);
    EXPECT_TRUE(widget->calWidgetWidth() > 0);
    //发送关闭信号
    widget->m_cancelBtn->clicked();
}
