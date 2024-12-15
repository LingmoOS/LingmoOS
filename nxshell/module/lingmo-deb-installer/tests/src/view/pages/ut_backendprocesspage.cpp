// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DLabel>
#include <DSpinner>
#include <DProgressBar>
#include <QStackedLayout>
#include <gtest/gtest.h>

#define private public
#include "../deb-installer/view/pages/backendprocesspage.h"
#include "../deb-installer/view/widgets/noprocesswidget.h"
#include "../deb-installer/view/widgets/processwidget.h"

class ut_backendProcessPage_TEST : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() { w = new BackendProcessPage; }
    void TearDown() { delete w; }

private:
    BackendProcessPage *w;
};

TEST_F(ut_backendProcessPage_TEST, BackendProcessPage_UT_setDisplayPage_0)
{
    w->setDisplayPage(BackendProcessPage::APT_INIT);
    ASSERT_EQ(w->allLayout->currentWidget(), w->noProcessWidget);
}

TEST_F(ut_backendProcessPage_TEST, BackendProcessPage_UT_setDisplayPage_1)
{
    w->setDisplayPage(BackendProcessPage::READ_PKG);
    ASSERT_EQ(w->allLayout->currentWidget(), w->processWidget);
}

TEST_F(ut_backendProcessPage_TEST, BackendProcessPage_UT_setDisplayPage_2)
{
    w->setDisplayPage(BackendProcessPage::PROCESS_FIN);
    ASSERT_EQ(w->noProcessWidget->spinner->isPlaying(), false);
}

TEST_F(ut_backendProcessPage_TEST, BackendProcessPage_UT_setPkgProcessRate)
{
    w->setPkgProcessRate(1, 10);
    ASSERT_EQ(w->processWidget->processBar->value(), 1);
    ASSERT_EQ(w->processWidget->processBar->maximum(), 10);
}
