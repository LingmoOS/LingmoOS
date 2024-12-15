// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "iteminfo.h"

#include <QDebug>

class TstItemInfo : public testing::Test
{
public:
    void SetUp() override
    {
        info = new ItemInfo();
    }

    void TearDown() override
    {
       delete info;
       info = nullptr;
    }

public:
    ItemInfo *info = nullptr;
};

TEST_F(TstItemInfo, coverageTest)
{
    
}
