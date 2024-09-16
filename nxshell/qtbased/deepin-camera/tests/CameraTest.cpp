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

#include "CameraTest.h"
#include "src/Settings.h"
#include "src/mainwindow.h"
#include "src/photorecordbtn.h"
#include "src/majorimageprocessingthread.h"
#include "src/capplication.h"
#include "src/camera.h"
#include "src/videosurface.h"
#include "src/basepub/datamanager.h"
#include "ac-deepin-camera-define.h"
#include "stub/stub_function.h"
#include "addr_pri.h"


#include <QtTest/qtest.h>
#include <QVideoSurfaceFormat>

ACCESS_PRIVATE_FUN(CMainWindow, void(), initCameraConnection);

ACCESS_PRIVATE_FIELD(CMainWindow, bool, m_bRecording);
ACCESS_PRIVATE_FIELD(Camera, QString, m_curDevName);
ACCESS_PRIVATE_FIELD(Camera, VideoSurface*, m_videoSurface);

CameraTest::CameraTest()
{
    DataManager::instance()->setEncodeEnv(QCamera_Env);
}

CameraTest::~CameraTest()
{
    DataManager::instance()->setEncodeEnv(FFmpeg_Env);
}

void CameraTest::SetUp()
{
    m_mainwindow = CamApp->getMainWindow();
    if (m_mainwindow){
        // 建立设备监听线程与QCamera的信号连接
        call_private_fun::CMainWindowinitCameraConnection(*m_mainwindow);

        m_processThread = m_mainwindow->findChild<MajorImageProcessingThread *>("MajorThread");
        m_devnumMonitor = m_mainwindow->findChild<DevNumMonitor *>("DevMonitorThread");
        if (m_processThread)
            QObject::connect(Camera::instance(), SIGNAL(presentImage(QImage &)), m_processThread, SLOT(processingImage(QImage &)));
    }

    DataManager::instance()->setEncodeEnv(QCamera_Env);
}

void CameraTest::TearDown()
{
    if (m_processThread)
        QObject::disconnect(Camera::instance(), SIGNAL(presentImage(QImage &)), m_processThread, SLOT(processingImage(QImage &)));

    Camera::release();

    DataManager::instance()->setEncodeEnv(FFmpeg_Env);
}

/**
 *  @brief 刷新相机状态
 */
TEST_F(CameraTest, refreshCamera)
{
    // 重启摄像头
    Stub_Function::resetSub(ADDR(Camera, getSupportResolutionsSize), ADDR(Stub_Function, getSupportResolutionsSize));
    Stub_Function::resetSub(ADDR(QCamera, status), ADDR(Stub_Function, cameraStatus));
    Stub_Function::resetSub(ADDR(DataManager, getdevStatus), ADDR(Stub_Function, getNoDevStatus));
    m_devnumMonitor->existDevice();
    Stub_Function::clearSub(ADDR(Camera, getSupportResolutionsSize));
    Stub_Function::clearSub(ADDR(QCamera, status));
    Stub_Function::clearSub(ADDR(DataManager, getdevStatus));

    // 置为有设备名，显示设备断开提示
    access_private_field::Cameram_curDevName(*Camera::instance()) = "/dev/video4";
    m_devnumMonitor->existDevice();
    access_private_field::Cameram_curDevName(*Camera::instance()) = "";
}

/**
 *  @brief 获取分辨率
 */
TEST_F(CameraTest, setNewResolutionList)
{
    Stub_Function::resetSub(ADDR(Camera, getSupportResolutionsSize), ADDR(Stub_Function, getSupportResolutionsSize));
    Camera::instance()->getSupportResolutions();
    Stub_Function::clearSub(ADDR(Camera, getSupportResolutionsSize));
}

/**
 *  @brief 录像功能
 */
TEST_F(CameraTest, recordFunction)
{
    //切换到录像
    Stub_Function::resetSub(ADDR(DataManager, getdevStatus), ADDR(Stub_Function, getdevStatus));
    Stub_Function::resetSub(ADDR(DataManager, encodeEnv), ADDR(Stub_Function, qCameraEnv));
    RollingBox *rollBox = m_mainwindow->findChild<RollingBox *>(MODE_SWITCH_BOX);
    dc::Settings::get().settings()->setOption(QString("photosetting.photosdelay.photodelays"), 0);
    emit rollBox->currentValueChanged(ActType::ActTakeVideo);
    photoRecordBtn *pBtn = m_mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    QTest::mouseMove(pBtn, QPoint(5, 5), 500);

    // 点击录制
    emit pBtn->clicked();
    Camera::instance()->isReadyRecord();
    Camera::instance()->getRecoderState();
    Camera::instance()->isRecording();
    Camera::instance()->getRecoderTime();

    QTest::qWait(7000);

    // 点击停止录制
    Stub_Function::resetSub(ADDR(videowidget, getCapStatus), ADDR(Stub_Function, isActive));
    Stub_Function::resetSub(ADDR(Camera, getRecoderState), ADDR(Stub_Function, recordingState));
    access_private_field::CMainWindowm_bRecording(*m_mainwindow) = true;
    emit pBtn->clicked();
    Stub_Function::clearSub(ADDR(videowidget, getCapStatus));
    Stub_Function::clearSub(ADDR(Camera, getRecoderState));
    access_private_field::CMainWindowm_bRecording(*m_mainwindow) = false;
    Stub_Function::clearSub(ADDR(DataManager, getdevStatus));
    Stub_Function::clearSub(ADDR(DataManager, encodeEnv));

    //切换回拍照模式
    emit rollBox->currentValueChanged(ActType::ActTakePic);
    QTest::qWait(500);
}

/**
 *  @brief videoSurface功能
 */
TEST_F(CameraTest, videoSurfaceFunction)
{
    access_private_field::Cameram_videoSurface(*Camera::instance())->supportedPixelFormats();
    QVideoSurfaceFormat vdf;
    access_private_field::Cameram_videoSurface(*Camera::instance())->isFormatSupported(vdf);
    access_private_field::Cameram_videoSurface(*Camera::instance())->start(vdf);
    access_private_field::Cameram_videoSurface(*Camera::instance())->stop();
}

/**
 *  @brief 发送一帧图片
 */
TEST_F(CameraTest, presentImage)
{
    m_processThread->setHorizontalMirror(false);
    m_processThread->setFilter("warm");
    m_processThread->setExposure(1);
    m_processThread->getStatus();
    m_processThread->m_bTake = true;
    Stub_Function::resetSub(ADDR(QVideoFrame, map), ADDR(Stub_Function, videoFrameMapReadOnly));
    QVideoFrame vf;
    access_private_field::Cameram_videoSurface(*Camera::instance())->present(vf);
    m_processThread->setFilter("");
    m_processThread->setExposure(0);
    m_processThread->m_bTake = false;
    Stub_Function::clearSub(ADDR(QVideoFrame, map));
}


