// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <gtest/gtest.h>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>
#include "../../src/utils/dbusutils.h"


using namespace testing;

class DBusUtilsTest:public testing::Test{

public:
    virtual void SetUp() override{
        std::cout << "start DBusUtilsTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DBusUtilsTest" << std::endl;
    }
};

TEST_F(DBusUtilsTest, redDBusProperty)
{
    QVariant v = DBusUtils::redDBusProperty("com.deepin.daemon.Audio", "/com/deepin/daemon/Audio",
                                            "com.deepin.daemon.Audio", "DefaultSource");

    EXPECT_TRUE(v.isValid());
    QString path = v.value<QDBusObjectPath>().path();
    qDebug() << path;
    EXPECT_FALSE(path.isEmpty());
}
TEST_F(DBusUtilsTest, redDBusMethod)
{
    QVariant v = DBusUtils::redDBusMethod("com.deepin.daemon.Display", "/com/deepin/daemon/Display","com.deepin.daemon.Display","CanRotate");
    EXPECT_TRUE(v.isValid());
    EXPECT_FALSE(v.toBool());
}
