// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_about.h"

#define protected public
#include "about.h"
#undef protected

ut_about_test::ut_about_test()
{

}

TEST_F(ut_about_test,about)
{
    About* test_module = new About();
    ASSERT_TRUE(test_module);
    delete test_module;
}
