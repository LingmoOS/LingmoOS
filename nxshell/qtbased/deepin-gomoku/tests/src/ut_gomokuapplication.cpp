// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_gomokuapplication.h"
#include "gomokuapplication.h"

TEST_F(UT_GomokuApplication, UT_GomokuApplication_handleQuitAction)
{
    int argc = 0;
    char **argv;
    Gomokuapplication *app = new Gomokuapplication(argc, argv);
    app->handleQuitAction();
    EXPECT_NE(app, nullptr) << "check the status after UT_GomokuApplication_handleQuitAction()";
    app->deleteLater();
}
