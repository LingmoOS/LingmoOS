// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logowidget.h"

#include <gtest/gtest.h>

class UT_LogWidget : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    LogoWidget *logoWidget;
};

void UT_LogWidget::SetUp()
{
    logoWidget = new LogoWidget();
}

void UT_LogWidget::TearDown()
{
    delete logoWidget;
}

TEST_F(UT_LogWidget, init)
{
    logoWidget->updateLocale("en_US.UTF-8");
}
