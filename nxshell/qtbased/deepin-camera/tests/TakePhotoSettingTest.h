/*
* Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     tanlang <tanlang@uniontech.com>
* Maintainer: tanlang <tanlang@uniontech.com>
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
#ifndef _TAKE_PHOTO_SETTING_TEST_H
#define _TAKE_PHOTO_SETTING_TEST_H

#include <gtest/gtest.h>


class CMainWindow;
class takePhotoSettingAreaWidget;

class TakePhotoSettingTest: public ::testing::Test
{
public:
    TakePhotoSettingTest();
    virtual void SetUp() override;

    virtual void TearDown() override;

protected:
    CMainWindow *m_mainwindow;//主窗口
    takePhotoSettingAreaWidget *m_takePhotoSet;//左侧设置窗口
};
#endif
