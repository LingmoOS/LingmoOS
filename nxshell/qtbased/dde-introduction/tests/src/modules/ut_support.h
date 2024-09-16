// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SUPPORT_H
#define UT_SUPPORT_H

#include "gtest/gtest.h"
// #include <QTest>
#include <QObject>

class ut_support_test : public QObject, public::testing::Test
{
    Q_OBJECT
public:
    ut_support_test();
};

#endif // UT_SUPPORT_H
