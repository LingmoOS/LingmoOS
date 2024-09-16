// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dldbusinterface.h"

#include <stub.h>

#include <QDebug>

#include <gtest/gtest.h>

class DlDbusinterface_UT : public testing::Test
{
public:
    //添加日志
    static void SetUpTestCase()
    {
        qDebug() << "SetUpTestCase" << endl;
    }
    static void TearDownTestCase()
    {
        qDebug() << "TearDownTestCase" << endl;
    }
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_interface = new DeepinLogviewerInterface("test", "test", QDBusConnection::systemBus());
        qDebug() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_interface;
    }
    DeepinLogviewerInterface *m_interface;
};

TEST_F(DlDbusinterface_UT, Compare_UT)
{
    m_interface->exitCode();
    m_interface->exportLog("path", "path1", false);
    m_interface->getFileInfo("path", false);
    m_interface->quit();
    m_interface->readLog("Test");
}
