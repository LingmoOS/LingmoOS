// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_MIDDLEVIEW_H
#define UT_MIDDLEVIEW_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class MiddleView;

class UT_MiddleView : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_MiddleView();
    virtual void SetUp() override;
    virtual void TearDown() override;
    MiddleView *m_middleView {nullptr};
};

#endif // UT_MIDDLEVIEW_H
