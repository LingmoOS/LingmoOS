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


#ifndef _IMAGE_ITEM_TEST_H
#define _IMAGE_ITEM_TEST_H

#include <gtest/gtest.h>
#include <QString>


class CMainWindow;
class ImageItem;
class ImageItemTest: public ::testing::Test
{
public:
    ImageItemTest();
    ~ImageItemTest();
    virtual void SetUp() override;

    virtual void TearDown() override;

    QString getImageFileName();
    QString getVideoFileName();

protected:
    CMainWindow* m_mainwindow;
    ImageItem*  m_imageItem;

};




#endif
