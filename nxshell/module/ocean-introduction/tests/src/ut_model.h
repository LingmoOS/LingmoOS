// Copyright (C) 2019 ~ 2020 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_MODEL_H
#define UT_MODEL_H

#include "gtest/gtest.h"
// #include <QTest>
#include <QObject>

#define protected public
#include "model.h"
#undef protected

class ut_model_test : public QObject, public::testing::Test
{
    Q_OBJECT
public:
    ut_model_test();

    //这里的两个函数都会自动调用
    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

    Model* m_model = nullptr;
};

#endif // UT_MODEL_H
