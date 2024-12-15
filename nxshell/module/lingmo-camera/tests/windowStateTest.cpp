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

#include "windowStateTest.h"
#include "src/capplication.h"
#include "src/mainwindow.h"
#include "src/windowstatethread.h"
#include "stub/stub_function.h"
#include <QtTest/qtest.h>
#include "addr_pri.h"
ACCESS_PRIVATE_FUN(windowStateThread, void(), run);

WindowStateTest::WindowStateTest()
{

}

WindowStateTest::~WindowStateTest()
{

}

void WindowStateTest::SetUp()
{
    m_mainwindow = CamApp->getMainWindow();
    if (m_mainwindow){
        m_pThread = m_mainwindow->findChild<windowStateThread *>("windowStateThread");
    }
}

void WindowStateTest::TearDown()
{
    m_mainwindow = NULL;
    m_pThread= nullptr;
}

TEST_F(WindowStateTest, Thread)
{
    call_private_fun::windowStateThreadrun(*m_pThread);
}
