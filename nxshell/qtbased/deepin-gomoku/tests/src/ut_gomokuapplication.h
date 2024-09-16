// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_GOMOKUAPPLICATION_H
#define UT_GOMOKUAPPLICATION_H

#include "gtest/gtest.h"
#include <stub.h>

#include <QDebug>

class UT_GomokuApplication : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        qInfo() << "TearDown" << endl;
    }
};

#endif // UT_GOMOKUAPPLICATION_H
