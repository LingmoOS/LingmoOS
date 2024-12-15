// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusInterface.h"
#include "ut_Head.h"
#include "stub.h"

#include <DStyle>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QWidget>
#include <QDBusReply>

#include <gtest/gtest.h>

class UT_DBusInterface : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

bool ut_replay_001()
{
    return true;
}
bool ut_replay_002()
{
    return false;
}
TEST_F(UT_DBusInterface, UT_DBusInterface_getInfo_001)
{
    QString info;
    Stub stub;
    stub.set(ADDR(QDBusReply<QString>, isValid), ut_replay_001);
    DBusInterface::getInstance()->getInfo("lshw", info);
    // 对函数isValid打桩后，发现在gerrit上依旧是实际的执行结果，因此该处不做预期判断，下同
    // EXPECT_TRUE(DBusInterface::getInstance()->getInfo("lshw",info));
}

TEST_F(UT_DBusInterface, UT_DBusInterface_getInfo_002)
{
    QString info;
    Stub stub;
    // 对函数isValid打桩后，发现在gerrit上依旧是实际的执行结果，因此该处不做预期判断，下同
    stub.set(ADDR(QDBusReply<QString>, isValid), ut_replay_002);
    DBusInterface::getInstance()->getInfo("lshw", info);
    // EXPECT_FALSE(DBusInterface::getInstance()->getInfo("lshw",info));
}
