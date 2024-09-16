// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_NORMALMODULE_H
#define UT_NORMALMODULE_H

#include "gtest/gtest.h"
// #include <QTest>
#include <QObject>

class ut_normalmodule_test : public QObject, public::testing::Test
{
    Q_OBJECT
public:
    ut_normalmodule_test();
};

#endif // UT_NORMALMODULE_H
