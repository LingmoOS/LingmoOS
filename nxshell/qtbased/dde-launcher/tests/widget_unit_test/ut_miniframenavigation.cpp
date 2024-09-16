// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "miniframenavigation.h"
#undef private

#include <gtest/gtest.h>

class Tst_Miniframenavigation : public testing::Test
{};

TEST_F(Tst_Miniframenavigation, userButton_test)
{
    UserButton button;
    button.initUser();
    button.setUserPath("");
    button.setUserIconURL("");
}

TEST_F(Tst_Miniframenavigation, navi_test)
{
    MiniFrameNavigation widget;
    widget.openDirectory("");
    widget.openStandardDirectory(QStandardPaths::DesktopLocation);
}
