// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_BUTTONAGAIN_H
#define UT_BUTTONAGAIN_H

#include "gtest/gtest.h"
#include "buttonagain.h"
#include <QDebug>

class UT_ButtonAgain : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_ButtonAgain = new Buttonagain();
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_ButtonAgain;
        qInfo() << "TearDown" << endl;
    }
    Buttonagain *m_ButtonAgain;
};

#endif // UT_BUTTONAGAIN_H
