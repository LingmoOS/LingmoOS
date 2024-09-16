// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_splashview.h"
#include "splashview.h"

UT_SplashView::UT_SplashView()
{
}

TEST_F(UT_SplashView, UT_SplashView_testsplshview_001)
{
    SplashView splashview;
    EXPECT_EQ(false, splashview.isVisible());
}
