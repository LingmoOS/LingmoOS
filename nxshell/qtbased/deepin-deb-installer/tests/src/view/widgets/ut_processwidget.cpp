// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DLabel>
#include <DProgressBar>
#include <gtest/gtest.h>

#define private public
#include "../deb-installer/view/widgets/processwidget.h"

class ut_processwidget_TEST : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() { w = new ProcessWidget; }
    void TearDown() { delete w; }

private:
    ProcessWidget *w;
};

TEST_F(ut_processwidget_TEST, Processwidget_UT_setIcon)
{
    QIcon icon;
    w->setIcon(icon);
    ASSERT_EQ(w->mainIcon->pixmap()->width(), 0);
    ASSERT_EQ(w->mainIcon->pixmap()->height(), 0);
}

TEST_F(ut_processwidget_TEST, Processwidget_UT_setMainText)
{
    w->setMainText("123321");
    ASSERT_EQ(w->mainLabel->text() == "123321", true);
}

TEST_F(ut_processwidget_TEST, Processwidget_UT_setProcessText)
{
    w->setProcessText("321123");
    ASSERT_EQ(w->processText == "321123", true);
}

TEST_F(ut_processwidget_TEST, Processwidget_UT_setProgress)
{
    w->setProcessText("321123, %1, %2");
    w->setProgress(1, 10);
    ASSERT_EQ(w->processBar->value(), 1);
    ASSERT_EQ(w->processBar->maximum(), 10);
    ASSERT_EQ(w->processTextLabel->text(), "321123, 1, 10");
}
