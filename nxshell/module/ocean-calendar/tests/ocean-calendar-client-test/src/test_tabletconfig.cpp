// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_tabletconfig.h"

test_TabletConfig::test_TabletConfig()
{
}

TEST(istableTest, TabletConfig)
{
    TabletConfig tabletconfig;
    tabletconfig.setIsTablet(true);
    ASSERT_TRUE(tabletconfig.isTablet());
    tabletconfig.setIsTablet(false);
    ASSERT_FALSE(tabletconfig.isTablet());
}
