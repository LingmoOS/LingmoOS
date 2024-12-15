// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CANCELBUTTON_H
#define UT_CANCELBUTTON_H

#include "gtest/gtest.h"
#include "cancelbutton.h"
#include <QDebug>

class UT_CancelButton : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_CalcelButton = new CancelButton();
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_CalcelButton;
        qInfo() << "TearDown" << endl;
    }
    CancelButton *m_CalcelButton;
};

#endif // UT_CANCELBUTTON_H
