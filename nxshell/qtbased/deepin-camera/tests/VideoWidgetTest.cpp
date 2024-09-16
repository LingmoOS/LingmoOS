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

#include "VideoWidgetTest.h"
#include "src/videowidget.h"
#include "stub.h"
#include "stub_function.h"
#include "src/capplication.h"
#include "ac-deepin-camera-define.h"
#include "stub/addr_pri.h"
#include "datamanager.h"
#include <QtTest/QTest>


ACCESS_PRIVATE_FUN(videowidget, void(), showNocam);
ACCESS_PRIVATE_FUN(videowidget, void(), showCamUsed);
ACCESS_PRIVATE_FUN(videowidget, void(), startTakeVideo);
ACCESS_PRIVATE_FUN(videowidget, void(), itemPosChange);
ACCESS_PRIVATE_FUN(videowidget, void(), stopEverything);
ACCESS_PRIVATE_FUN(videowidget, void(), onReachMaxDelayedFrames);
ACCESS_PRIVATE_FUN(videowidget, void(), flash);
ACCESS_PRIVATE_FUN(videowidget, void(QGraphicsView *view), forbidScrollBar);
ACCESS_PRIVATE_FUN(videowidget, void(PRIVIEW_ENUM_STATE state), showCountDownLabel);
ACCESS_PRIVATE_FUN(videowidget, void(const QString &resolution), slotresolutionchanged);

ACCESS_PRIVATE_FIELD(videowidget, QGraphicsTextItem *, m_pCamErrItem);
ACCESS_PRIVATE_FIELD(videowidget, DLabel *, m_dLabel);
ACCESS_PRIVATE_FIELD(videowidget, DLabel *, m_flashLabel);
ACCESS_PRIVATE_FIELD(videowidget, PreviewOpenglWidget *, m_openglwidget);
//ACCESS_PRIVATE_FIELD(videowidget, ThumbnailsBar *, m_thumbnail);
ACCESS_PRIVATE_FIELD(videowidget, QGraphicsViewEx *, m_pNormalView);
ACCESS_PRIVATE_FIELD(videowidget, int, m_nInterval);
ACCESS_PRIVATE_FIELD(videowidget, int, m_Maxinterval);
ACCESS_PRIVATE_FIELD(videowidget, int, m_nCount);
ACCESS_PRIVATE_FIELD(videowidget, int, m_curTakePicTime);

ACCESS_PRIVATE_FIELD(MajorImageProcessingThread, QAtomicInt, m_stopped);

VideoWidgetTest::VideoWidgetTest()
{

}

VideoWidgetTest::~VideoWidgetTest()
{

}

void VideoWidgetTest::SetUp()
{
    m_mainwindow = CamApp->getMainWindow();
    if (m_mainwindow) {
        m_videoWidget = m_mainwindow->findChild<videowidget *>(VIDEO_PREVIEW_WIDGET);
    }
}

void VideoWidgetTest::TearDown()
{
    m_mainwindow = NULL;
    m_videoWidget = NULL;
}

TEST_F(VideoWidgetTest, setFocus)
{
    m_videoWidget->setFocus();
}

TEST_F(VideoWidgetTest, TakePhoto)
{
    m_videoWidget->onTakePic(true);
    QTest::qWait(2000);
    m_videoWidget->onTakePic(false);
}

TEST_F(VideoWidgetTest, Record)
{
    m_videoWidget->onTakeVideo();
    QTest::qWait(2000);
    m_videoWidget->onEndBtnClicked();
}

/**
 *  @brief 删除快捷键
 */
TEST_F(VideoWidgetTest, DelShortCut)
{
    //点击鼠标右键
    QTest::qWait(1000);
    QTest::mouseMove(m_videoWidget, QPoint(0, 0), 500);
    for (int i = 0; i < 3; i++) {
        QTest::keyClick(m_videoWidget, Qt::Key_Delete, Qt::NoModifier, 0);
        QTest::qWait(500);
    }
}

/**
 *  @brief 阻止关机
 */
TEST_F(VideoWidgetTest, updateBlockSystem)
{
    emit m_videoWidget->updateBlockSystem(false);
    emit m_videoWidget->updateBlockSystem(true);
    m_mainwindow->setWayland(true);
    emit m_videoWidget->updateBlockSystem(false);
    emit m_videoWidget->updateBlockSystem(true);
    m_mainwindow->setWayland(false);
}

///**
// *  @brief 快捷方式测试
// */
//TEST_F(VideoWidgetTest, deleteFile)
//{
////    dc::Settings::get().setOption(QString("photosetting.mirrorMode.mirrorMode"), false);
////    QTest::qWait(5000);
////    dc::Settings::get().setOption(QString("photosetting.mirrorMode.mirrorMode"), true);
//}



