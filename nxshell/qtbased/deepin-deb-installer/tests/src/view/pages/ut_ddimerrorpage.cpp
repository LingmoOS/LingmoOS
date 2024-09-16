// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <DLabel>
#include <QPushButton>

#define private public
#include "../deb-installer/view/pages/ddimerrorpage.h"

class ut_ddimErrorPage_TEST : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() { w = new DdimErrorPage; }
    void TearDown() { delete w; }

private:
    DdimErrorPage *w;
};

TEST_F(ut_ddimErrorPage_TEST, DdimErrorPage_UT_setErrorMessage)
{
    w->setErrorMessage("123321");
    ASSERT_EQ(w->errorMessageLabel->text() == "123321", true);
}
