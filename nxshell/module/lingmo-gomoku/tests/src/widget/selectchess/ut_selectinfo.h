// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SELECTINFO_H
#define UT_SELECTINFO_H

#include "gtest/gtest.h"
#include "selectinfo.h"

#include <QDebug>

class UT_SelectInfo : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_SelectInfo = new Selectinfo();
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_SelectInfo;
        qInfo() << "TearDown" << endl;
    }
    Selectinfo *m_SelectInfo;
};

#endif // UT_SELECTINFO_H
