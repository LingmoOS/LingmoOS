// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_RESULTPOPUP_H
#define UT_RESULTPOPUP_H

#include "gtest/gtest.h"
#include "resultpopup.h"
#include <QDebug>

class UT_ResultPopup : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_ResulePopup = new Resultpopup(true);
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_ResulePopup;
        qInfo() << "TearDown" << endl;
    }
    Resultpopup *m_ResulePopup;
};

#endif // UT_RESULTPOPUP_H
