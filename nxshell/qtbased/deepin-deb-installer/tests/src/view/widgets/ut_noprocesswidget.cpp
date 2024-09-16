// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DLabel>
#include <DSpinner>
#include <gtest/gtest.h>

#define private public
#include "../deb-installer/view/widgets/noprocesswidget.h"

class ut_noprocesswidget_TEST : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() { w = new NoProcessWidget; }
    void TearDown() { delete w; }

private:
    NoProcessWidget *w;
};

TEST_F(ut_noprocesswidget_TEST, Noprocesswidget_UT_setActionTest)
{
    w->setActionText("123321");
    ASSERT_EQ(w->actionTextLabel->text() == "123321", true);
}

TEST_F(ut_noprocesswidget_TEST, Noprocesswidget_UT_startAndStop)
{
    ASSERT_EQ(w->spinner->isPlaying(), false);
    w->start();
    ASSERT_EQ(w->spinner->isPlaying(), true);
    w->stop();
    ASSERT_EQ(w->spinner->isPlaying(), false);
}
