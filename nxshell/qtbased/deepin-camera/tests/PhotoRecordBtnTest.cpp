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

#include "PhotoRecordBtnTest.h"
#include "src/photorecordbtn.h"
#include "src/videowidget.h"
#include "stub.h"
#include "stub_function.h"
#include "src/capplication.h"
#include "ac-deepin-camera-define.h"
#include "stub/addr_pri.h"
#include "datamanager.h"
#include <QtTest/qtest.h>


PhotoRecordBtnTest::PhotoRecordBtnTest()
{
    m_mainwindow = CamApp->getMainWindow();
    if (m_mainwindow){
        m_photoRecordBtn = m_mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    }
}

PhotoRecordBtnTest::~PhotoRecordBtnTest()
{
    m_mainwindow = NULL;
    m_photoRecordBtn = NULL;
}

void PhotoRecordBtnTest::SetUp()
{
   
}

void PhotoRecordBtnTest::TearDown()
{

}

TEST_F(PhotoRecordBtnTest, Photo)
{
    bool state = m_photoRecordBtn->photoState();
    m_photoRecordBtn->setState(!state);
    m_photoRecordBtn->setState(state);
}

TEST_F(PhotoRecordBtnTest, PreRecord)
{
    bool state = m_photoRecordBtn->photoState();
    m_photoRecordBtn->setState(!state);
    m_photoRecordBtn->setRecordState(photoRecordBtn::preRecord);

    m_photoRecordBtn->setState(state);
    m_photoRecordBtn->setRecordState(photoRecordBtn::Normal);
}

TEST_F(PhotoRecordBtnTest, Recording)
{
    bool state = m_photoRecordBtn->photoState();
    m_photoRecordBtn->setState(!state);
    m_photoRecordBtn->setRecordState(photoRecordBtn::Recording);

    m_photoRecordBtn->setState(state);
    m_photoRecordBtn->setRecordState(photoRecordBtn::Normal);
}

TEST_F(PhotoRecordBtnTest, Mouse)
{
    QTest::mouseMove(m_photoRecordBtn, QPoint(5, 5), 500);
    QTest::qWait(100);
    QTest::mousePress(m_photoRecordBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 500);
    QTest::qWait(100);
    QTest::mouseRelease(m_photoRecordBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 500);
    QTest::qWait(1000);
}
