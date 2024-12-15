// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SELECTBUTTON_H
#define UT_SELECTBUTTON_H

#include "gtest/gtest.h"
#include "selectbutton.h"
#include <QDebug>

class UT_SelectButton : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_SelectButton = new Selectbutton();
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_SelectButton;
        qInfo() << "TearDown" << endl;
    }
    Selectbutton *m_SelectButton;
};

#endif // UT_SELECTBUTTON_H
