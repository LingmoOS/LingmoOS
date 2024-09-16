// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontpreviewitemdef.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

namespace {
class TestFontData : public testing::Test
{

protected:

};
}

TEST_F(TestFontData, checkFontData)
{
    FontData fd("aaa", true, false, false, true, FontType::OTF, true);
    EXPECT_EQ(fd.isEnabled(), true);
    EXPECT_EQ(fd.isCollected(), false);
    EXPECT_EQ(fd.isChinese(), false);
    EXPECT_EQ(fd.isMonoSpace(), true);

    FontData fd2("aaa", true, false, false, true, "TrueType", true);
    EXPECT_EQ(fd2.isEnabled(), true);
    EXPECT_EQ(fd2.isCollected(), false);
    EXPECT_EQ(fd2.isChinese(), false);
    EXPECT_EQ(fd2.isMonoSpace(), true);

    fd2.setFontType(FontType::TTF);
    EXPECT_EQ(fd2.getFontType(), FontType::TTF);
    fd2.setFontType(FontType::TTC);
    EXPECT_EQ(fd2.getFontType(), FontType::TTC);
    fd2.setFontType(FontType::UNKNOWN);
    EXPECT_EQ(fd2.getFontType(), FontType::UNKNOWN);

}
