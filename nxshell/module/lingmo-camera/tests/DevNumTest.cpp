/*
* Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     wuzhigang <wuzhigang@uniontech.com>
* Maintainer: wuzhigang <wuzhigang@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DevNumTest.h"
#include "src/mainwindow.h"
#include "src/capplication.h"
#include "stub/stub_function.h"
#include <QtTest/qtest.h>

DevNumberTest::DevNumberTest()
{

}

DevNumberTest::~DevNumberTest()
{

}

void DevNumberTest::SetUp()
{
    m_mainwindow = CamApp->getMainWindow();
    if (m_mainwindow) {
        m_devnumMonitor = m_mainwindow->findChild<DevNumMonitor *>("DevMonitorThread");
    }
}

void DevNumberTest::TearDown()
{
    m_mainwindow = NULL;
    m_devnumMonitor = NULL;
}


TEST_F(DevNumberTest, startCheck)
{
    QTest::qWait(1000);
    Stub_Function::resetSub(::get_device_list, ADDR(Stub_Function, get_device_list_0));
    QTest::qWait(1000);
    Stub_Function::resetSub(::get_device_list, ADDR(Stub_Function, get_device_list_1));
    QTest::qWait(1000);
    Stub_Function::resetSub(::get_device_list, ADDR(Stub_Function, get_device_list_2));
    QTest::qWait(1000);
    Stub_Function::resetSub(::get_device_list, ADDR(Stub_Function, get_device_list_3));
    QTest::qWait(1000);
}
