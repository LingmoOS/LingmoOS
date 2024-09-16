// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LauncherUnitTest_H
#define LauncherUnitTest_H

#include <QObject>

#include <gtest/gtest.h>

class LauncherUnitTest : public QObject, public testing::Test
{
    Q_OBJECT
public:
    LauncherUnitTest() {}
    virtual ~LauncherUnitTest() {}

    virtual void SetUp() {}
    virtual void TearDown() {}
};

#endif // LauncherUnitTest_H
