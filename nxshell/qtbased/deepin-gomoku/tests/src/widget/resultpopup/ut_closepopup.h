// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CLOSEPOPUP_H
#define UT_CLOSEPOPUP_H

#include "gtest/gtest.h"
#include "closepopup.h"
#include <QDebug>

class UT_ClosePopup : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_ClosePopup = new Closepopup();
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_ClosePopup;
        qInfo() << "TearDown" << endl;
    }
    Closepopup *m_ClosePopup;
};

#endif // UT_CLOSEPOPUP_H
