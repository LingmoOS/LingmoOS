// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wmframe.h"

#include <gtest/gtest.h>

class UT_WMFrame : public testing::Test
{

};

TEST_F(UT_WMFrame, coverageTest)
{
    WMFrame wmFrame;
    wmFrame.setConfigPath("config.ini");
}
