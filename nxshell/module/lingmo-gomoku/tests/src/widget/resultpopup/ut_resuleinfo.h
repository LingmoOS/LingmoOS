// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_RESULEINFO_H
#define UT_RESULEINFO_H

#include "gtest/gtest.h"
#include "resultinfo.h"
#include <QDebug>

class UT_ResuleInfo : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_ResuleInof = new Resultinfo();
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_ResuleInof;
        qInfo() << "TearDown" << endl;
    }
    Resultinfo *m_ResuleInof;
};

#endif // UT_RESULEINFO_H
