// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_DETERMINEBUTTON_H
#define UT_DETERMINEBUTTON_H

#include "gtest/gtest.h"
#include "determinebutton.h"
#include <QDebug>

class UT_DetermineButton : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_DetermineButton = new Determinebutton();
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_DetermineButton;
        qInfo() << "TearDown" << endl;
    }
    Determinebutton *m_DetermineButton;
};

#endif // UT_DETERMINEBUTTON_H
