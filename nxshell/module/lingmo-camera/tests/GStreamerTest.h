/*
* Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co.,Ltd.
*
* Author:     houchengqiu <houchengqiu@uniontech.com>
* Maintainer: houchengqiu <houchengqiu@uniontech.com>
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

#ifndef _DEV_NUM_TEST_H
#define _DEV_NUM_TEST_H
#include <gtest/gtest.h>

class CMainWindow;
class MajorImageProcessingThread;
class GStreamerTest: public ::testing::Test
{
public:
    GStreamerTest();
    ~GStreamerTest();
    virtual void SetUp() override;

    virtual void TearDown() override;

protected:
    CMainWindow*                m_mainwindow;
    MajorImageProcessingThread* m_processThread;
};


#endif
