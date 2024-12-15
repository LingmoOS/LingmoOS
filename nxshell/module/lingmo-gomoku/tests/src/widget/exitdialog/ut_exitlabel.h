// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_EXITLABEL_H
#define UT_EXITLABEL_H

#include "gtest/gtest.h"
#include "exitlabel.h"
#include <QDebug>

class UT_ExitLabel : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_ExitLabel = new ExitLabel();
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_ExitLabel;
        qInfo() << "TearDown" << endl;
    }
    ExitLabel *m_ExitLabel;
};

#endif // UT_EXITLABEL_H
