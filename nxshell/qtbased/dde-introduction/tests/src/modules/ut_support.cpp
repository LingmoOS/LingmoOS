// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_support.h"

#define protected public
#include "support.h"
#undef protected

ut_support_test::ut_support_test()
{

}

TEST_F(ut_support_test, Support)
{
    Support* test_module = new Support();
    ASSERT_TRUE(test_module);
    ASSERT_TRUE(test_module->m_label);
    test_module->deleteLater();
}
