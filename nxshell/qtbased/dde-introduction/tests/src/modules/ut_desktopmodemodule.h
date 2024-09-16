// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_DESKTOPMODEMODULE_H
#define UT_DESKTOPMODEMODULE_H

#include "gtest/gtest.h"
// #include <QTest>
#include <QObject>

class ut_desktopmodemodule_test : public QObject, public::testing::Test
{
    Q_OBJECT
public:
    ut_desktopmodemodule_test();
};

#endif // UT_DESKTOPMODEMODULE_H
