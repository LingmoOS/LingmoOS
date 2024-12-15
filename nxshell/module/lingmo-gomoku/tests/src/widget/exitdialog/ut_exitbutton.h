// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_EXITBUTTON_H
#define UT_EXITBUTTON_H

#include "gtest/gtest.h"
#include "exitbutton.h"
#include <QDebug>

class UT_ExitButton : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_ExitButton = new ExitButton();
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_ExitButton;
        qInfo() << "TearDown" << endl;
    }
    ExitButton *m_ExitButton;
};

#endif // UT_EXITBUTTON_H
