// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_MODULEINTERFACE_H
#define UT_MODULEINTERFACE_H

#include "gtest/gtest.h"
// #include <QTest>
#include <QObject>

class ut_moduleinterface_test : public QObject, public::testing::Test
{
    Q_OBJECT
public:
    ut_moduleinterface_test();
};

#endif // UT_MODULEINTERFACE_H
