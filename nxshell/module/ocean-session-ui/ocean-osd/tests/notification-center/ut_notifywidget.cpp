// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notification-center/notifywidget.h"

#include <gtest/gtest.h>

class UT_NotifyWidget : public testing::Test
{
public:
    void SetUp() override
    {
        obj = new NotifyWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
    }

public:
    NotifyWidget *obj = nullptr;
};

TEST_F(UT_NotifyWidget, coverageTest)
{
    obj->view();
    obj->model();
}
