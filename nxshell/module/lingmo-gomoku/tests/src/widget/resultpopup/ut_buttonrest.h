// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_BUTTONREST_H
#define UT_BUTTONREST_H

#include "gtest/gtest.h"
#include "buttonrest.h"
#include <QDebug>

class UT_ButtonRest : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_ButtonRest = new Buttonrest();
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_ButtonRest;
        qInfo() << "TearDown" << endl;
    }
    Buttonrest *m_ButtonRest;
};

#endif // UT_BUTTONREST_H
