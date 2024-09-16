// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include "window/imwindow.h"
//#include <QObject>
class ut_imwindow : public ::testing::Test
{
protected:
    ut_imwindow()
    {
    }

    virtual ~ut_imwindow()
    {
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ut_imwindow, initUI)
{
    IMWindow imwindow;
    imwindow.initUI();
}

TEST_F(ut_imwindow, initConnect)
{
    IMWindow imwindow;
    imwindow.initConnect();
}

//TEST_F(ut_imwindow, initFcitxInterface)
//{
//    IMWindow imwindow;
//    imwindow.initFcitxInterface();
//}
