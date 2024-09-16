// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_EXITDIALOG_H
#define UT_EXITDIALOG_H

#include "gtest/gtest.h"
#include "exitdialog.h"
#include <QDebug>

class UT_ExitDialog : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_ExitDialog = new ExitDialog(true);
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_ExitDialog;
        qInfo() << "TearDown" << endl;
    }
    ExitDialog *m_ExitDialog;
};

#endif // UT_EXITDIALOG_H
