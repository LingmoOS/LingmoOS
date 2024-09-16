// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_GOMOKUMAINWINDOW_H
#define UT_GOMOKUMAINWINDOW_H

#include "gomokumainwindow.h"
#include "gtest/gtest.h"
#include <QtTest/QTest>
#include <stub.h>

#include <QDebug>

void stub_initCompositingStatus()
{
}

class UT_GomokuMainWindow : public ::testing::Test
{

public:
    GomokuMainWindow *m_mainwindow = nullptr;
    void SetUp() //TEST跑之前会执行SetUp
    {
        Stub stub;
        stub.set(ADDR(GomokuMainWindow, initCompositingStatus), stub_initCompositingStatus);
        m_mainwindow = new GomokuMainWindow();
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        if (m_mainwindow != nullptr) {
            delete m_mainwindow;
            m_mainwindow = nullptr;
        }
        qInfo() << "TearDown" << endl;
    }
};

#endif // UT_GOMOKUMAINWINDOW_H
