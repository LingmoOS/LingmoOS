// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QTimer>
#include <QTest>
#include <QMainWindow>
#include <QHBoxLayout>
#include  <QFont>

#include "stub.h"
#include "addr_pri.h"

#include "waylandrecord/recordadmin.h"

using namespace testing;

class RecordAdminTest: public testing::Test
{

public:
    Stub stub;
    RecordAdmin *m_recordAdmin;
    WaylandIntegration::WaylandIntegrationPrivate *m_context ;
    virtual void SetUp() override
    {
        QStringList arguments;
        arguments << QString("%1").arg(3);
        arguments << QString("%1").arg(1000) << QString("%1").arg(500);
        arguments << QString("%1").arg(0) << QString("%1").arg(0);
        arguments << QString("%1").arg(24);
        arguments << QString("%1").arg("tmp/test.mkv");
        arguments << QString("%1").arg(4);
        std::cout << "start UtilsTest" << std::endl;
        avlibInterface::initFunctions();
        m_context = new  WaylandIntegration::WaylandIntegrationPrivate();
        m_recordAdmin = new RecordAdmin(arguments, m_context);
    }

    virtual void TearDown() override
    {
        delete m_recordAdmin;
        delete m_context;
        avlibInterface::unloadFunctions();
        std::cout << "end UtilsTest" << std::endl;
    }
};

ACCESS_PRIVATE_FUN(RecordAdmin, void(int), setRecordAudioType);
void setRecordAudioType_stub()
{

}
int pthread_create_stub1()
{

    return 1;
}
int pthread_detach_stub()
{
    return 1;
}
TEST_F(RecordAdminTest, init)
{
    auto RecordAdmin_setRecordAudioType = get_private_fun::RecordAdminsetRecordAudioType();
    stub.set(RecordAdmin_setRecordAudioType, setRecordAudioType_stub);
    stub.set(pthread_create, pthread_create_stub1);
    stub.set(pthread_detach, pthread_detach_stub);
    m_recordAdmin->init(1920, 1080);
    stub.reset(RecordAdmin_setRecordAudioType);
    stub.reset(pthread_create);
    stub.reset(pthread_detach);
}

void setBGetFrame_stub(bool bGetFrame)
{

}
void setBWriteFrame_stub(bool bWriteFrame)
{

}
void setbWriteAmix_stub(bool bWriteMix)
{

}
void setIsWriteFrame_stub(bool isWriteFrame)
{

}
void setbRunThread_stub(bool bRunThread)
{

}
void close_stub()
{

}
void lock_stub()
{

}
void unlock_stub()
{

}

TEST_F(RecordAdminTest, stopStream)
{
    if (QSysInfo::currentCpuArchitecture().startsWith("arm"))
        return;
    stub.set(ADDR(WaylandIntegration::WaylandIntegrationPrivate, setBGetFrame), setBGetFrame_stub);
    stub.set(ADDR(WriteFrameThread, setBWriteFrame), setBWriteFrame_stub);
    stub.set(ADDR(CAVInputStream, setbWriteAmix), setbWriteAmix_stub);
    stub.set(ADDR(CAVOutputStream, setIsWriteFrame), setIsWriteFrame_stub);
    stub.set(ADDR(CAVInputStream, setbRunThread), setbRunThread_stub);
    stub.set(ADDR(CAVOutputStream, close), close_stub);
    stub.set(ADDR(QMutex, lock), lock_stub);
    stub.set(ADDR(QMutex, unlock), unlock_stub);

    m_recordAdmin->stopStream();

    stub.reset(ADDR(WaylandIntegration::WaylandIntegrationPrivate, setBGetFrame));
    stub.reset(ADDR(WriteFrameThread, setBWriteFrame));
    stub.reset(ADDR(CAVInputStream, setbWriteAmix));
    stub.reset(ADDR(CAVOutputStream, setIsWriteFrame));
    stub.reset(ADDR(CAVInputStream, setbRunThread));
    stub.reset(ADDR(CAVOutputStream, close));
    stub.reset(ADDR(QMutex, lock));
    stub.reset(ADDR(QMutex, unlock));

}

bool setMicAudioRecord_stub(bool flag)
{
    return flag;
}
bool setSysAudioRecord_stub(bool flag)
{
    return flag;

}
ACCESS_PRIVATE_FUN(RecordAdmin, void(bool), setMicAudioRecord);
ACCESS_PRIVATE_FUN(RecordAdmin,  void(bool), setSysAudioRecord);
TEST_F(RecordAdminTest, setRecordAudioType)
{
    int audioType = 4;
    auto RecordAdmin_setMicAudioRecord = get_private_fun::RecordAdminsetMicAudioRecord();
    auto RecordAdmin_setSysAudioRecord = get_private_fun::RecordAdminsetSysAudioRecord();
    stub.set(RecordAdmin_setMicAudioRecord, setMicAudioRecord_stub);
    stub.set(RecordAdmin_setSysAudioRecord, setSysAudioRecord_stub);

    call_private_fun::RecordAdminsetRecordAudioType(*m_recordAdmin, audioType);
    stub.reset(RecordAdmin_setMicAudioRecord);
    stub.reset(RecordAdmin_setSysAudioRecord);

}

TEST_F(RecordAdminTest, setMicAudioRecord)
{
    int bRecord = true;
    stub.set(ADDR(CAVInputStream, setMicAudioRecord), setMicAudioRecord_stub);
    call_private_fun::RecordAdminsetMicAudioRecord(*m_recordAdmin, bRecord);
    stub.reset(ADDR(CAVInputStream, setMicAudioRecord));

}
TEST_F(RecordAdminTest, setSysAudioRecord)
{
    int bRecord = true;
    stub.set(ADDR(CAVInputStream, setSysAudioRecord), setSysAudioRecord_stub);
    call_private_fun::RecordAdminsetSysAudioRecord(*m_recordAdmin, bRecord);
    stub.reset(ADDR(CAVInputStream, setSysAudioRecord));
}

bool openInputStream_stub()
{
    return true;
}
bool GetVideoInputInfo_stub(int &width, int &height, int &frame_rate, AVPixelFormat &pixFmt)
{
    width = 1920;
    height = 1080;
    pixFmt = AVPixelFormat::AV_PIX_FMT_NB;
    frame_rate = 24;
    return true;
}
bool open_stub(QString path)
{
    return true;

}
bool  GetAudioInputInfo_stub(AVSampleFormat &sample_fmt, int &sample_rate, int &channels, int &layout)
{
    sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_NB;
    sample_rate = 48000;
    channels = 2;
    layout = 4;
    return true;
}
bool  GetAudioSCardInputInfo_stub(AVSampleFormat &sample_fmt, int &sample_rate, int &channels, int &layout)
{
    sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_NB;
    sample_rate = 48000;
    channels = 2;
    layout = 4;
    return true;
}

void start_stub1(QThread::Priority p)
{
    qDebug() << "模拟写视频帧线程启动！";
}
bool audioCapture_stub()
{
    qDebug() << "模拟写音频帧启动！";
    return true;
}
ACCESS_PRIVATE_FUN(RecordAdmin, int(), startStream);
TEST_F(RecordAdminTest, startStream)
{
    stub.set(ADDR(CAVInputStream, openInputStream), openInputStream_stub);
    stub.set(ADDR(CAVInputStream, GetVideoInputInfo), GetVideoInputInfo_stub);
    stub.set(ADDR(CAVInputStream, GetAudioInputInfo), GetAudioInputInfo_stub);
    stub.set(ADDR(CAVInputStream, GetAudioSCardInputInfo), GetAudioSCardInputInfo_stub);
    stub.set(ADDR(CAVOutputStream, open), open_stub);
    stub.set((void(QThread::*)(QThread::Priority))ADDR(QThread, start), start_stub1);
    stub.set(ADDR(CAVInputStream, audioCapture), audioCapture_stub);

    int flag = call_private_fun::RecordAdminstartStream(*m_recordAdmin);

    EXPECT_EQ(flag, 0);

    stub.reset(ADDR(CAVInputStream, openInputStream));
    stub.reset(ADDR(CAVInputStream, GetVideoInputInfo));
    stub.reset(ADDR(CAVInputStream, GetAudioInputInfo));
    stub.reset(ADDR(CAVInputStream, GetAudioSCardInputInfo));
    stub.reset(ADDR(CAVOutputStream, open));
    stub.reset((void(QThread::*)(QThread::Priority))ADDR(QThread, start));
    stub.reset(ADDR(CAVInputStream, audioCapture));
}

int startStream_stub()
{
    return 0;
}
ACCESS_PRIVATE_STATIC_FUN(RecordAdmin, void *(void *), stream);
TEST_F(RecordAdminTest, stream)
{
    auto RecordAdmin_startStream = get_private_fun::RecordAdminstartStream();
    stub.set(RecordAdmin_startStream, startStream_stub);
    call_private_static_fun::RecordAdmin::RecordAdminstream(m_recordAdmin);
    stub.reset(RecordAdmin_startStream);

}
