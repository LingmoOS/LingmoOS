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

#include "GStreamerTest.h"
#include "src/Settings.h"
#include "src/mainwindow.h"
#include "src/photorecordbtn.h"
#include "src/majorimageprocessingthread.h"
#include "src/capplication.h"
#include "src/basepub/datamanager.h"
#include "ac-deepin-camera-define.h"
#include "stub/stub_function.h"
#include "addr_pri.h"


#include <QtTest/qtest.h>
#include <QVideoSurfaceFormat>

ACCESS_PRIVATE_FUN(CMainWindow, void(), initCameraConnection);

ACCESS_PRIVATE_FIELD(CMainWindow, bool, m_bRecording);

GStreamerTest::GStreamerTest()
{
    DataManager::instance()->setEncodeEnv(GStreamer_Env);
}

GStreamerTest::~GStreamerTest()
{
    DataManager::instance()->setEncodeEnv(FFmpeg_Env);
}

void GStreamerTest::SetUp()
{
    m_mainwindow = CamApp->getMainWindow();
    if (m_mainwindow){
        m_processThread = m_mainwindow->findChild<MajorImageProcessingThread *>("MajorThread");
    }

    DataManager::instance()->setEncodeEnv(GStreamer_Env);
}

void GStreamerTest::TearDown()
{
    DataManager::instance()->setEncodeEnv(FFmpeg_Env);
}

/**
 *  @brief 录像功能
 */
TEST_F(GStreamerTest, videorecord)
{
    //切换到录像
    Stub_Function::resetSub(ADDR(DataManager, getdevStatus), ADDR(Stub_Function, getdevStatus));
    Stub_Function::resetSub(ADDR(DataManager, encodeEnv), ADDR(Stub_Function, gstreamerEnv));
    Stub_Function::resetSub(::get_audio_context, ADDR(Stub_Function, get_audio_context));
    Stub_Function::resetSub(::audio_start, ADDR(Stub_Function, audio_start));
    Stub_Function::resetSub(::audio_stop, ADDR(Stub_Function, audio_stop));
    Stub_Function::resetSub(::audio_get_next_buffer, ADDR(Stub_Function, audio_get_next_buffer));
    RollingBox *rollBox = m_mainwindow->findChild<RollingBox *>(MODE_SWITCH_BOX);
    dc::Settings::get().settings()->setOption(QString("photosetting.photosdelay.photodelays"), 0);
    emit rollBox->currentValueChanged(ActType::ActTakeVideo);
    photoRecordBtn *pBtn = m_mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    QTest::mouseMove(pBtn, QPoint(5, 5), 500);

    // 点击录制
    Stub_Function::resetSub(ADDR(videowidget, getCapStatus), ADDR(Stub_Function, isActive));
    emit pBtn->clicked();
    Stub_Function::clearSub(ADDR(videowidget, getCapStatus));
    emit pBtn->clicked();
    Stub_Function::clearSub(ADDR(Stub_Function, video_capture_get_save_video));
    Stub_Function::resetSub(::video_capture_get_save_video, ADDR(Stub_Function, video_capture_get_save_video_no_capture));
    MajorImageProcessingThread *processThread = m_mainwindow->findChild<MajorImageProcessingThread *>("MajorThread");
    if (processThread) {
        // 启动线程，当前选择有滤镜，使用rgb模式显示画面
        processThread->init();
        processThread->start();
    }
    QTest::qWait(500);
    if (processThread) {
        processThread->stop();
        while(processThread->isRunning());
    }
    QTest::qWait(5000);
    Stub_Function::clearSub(ADDR(Stub_Function, video_capture_get_save_video_no_capture));
    Stub_Function::resetSub(::video_capture_get_save_video, ADDR(Stub_Function, video_capture_get_save_video));

    // 点击停止录制
    Stub_Function::resetSub(ADDR(videowidget, getCapStatus), ADDR(Stub_Function, isActive));
    access_private_field::CMainWindowm_bRecording(*m_mainwindow) = true;
    emit pBtn->clicked();
    Stub_Function::clearSub(ADDR(videowidget, getCapStatus));
    access_private_field::CMainWindowm_bRecording(*m_mainwindow) = false;
    Stub_Function::clearSub(ADDR(DataManager, encodeEnv));
    Stub_Function::clearSub(ADDR(Stub_Function, get_audio_context));
    Stub_Function::clearSub(ADDR(Stub_Function, audio_start));
    Stub_Function::clearSub(ADDR(Stub_Function, audio_stop));
    Stub_Function::clearSub(ADDR(Stub_Function, audio_get_next_buffer));

    //切换回拍照模式
    emit rollBox->currentValueChanged(ActType::ActTakePic);
    QTest::qWait(500);
}

/**
 *  @brief 发送一帧图片
 */
TEST_F(GStreamerTest, presentImage)
{
    m_processThread->setHorizontalMirror(false);
    m_processThread->setFilter("warm");
    m_processThread->setExposure(1);
    m_processThread->getStatus();
    m_processThread->m_bTake = true;
    // 发送一帧图片
    //access_private_field::Cameram_videoSurface(*Camera::instance())->present(vf);
    m_processThread->setFilter("");
    m_processThread->setExposure(0);
    m_processThread->m_bTake = false;
}


