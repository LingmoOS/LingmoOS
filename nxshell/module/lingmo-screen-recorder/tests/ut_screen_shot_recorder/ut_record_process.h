// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QRect>
#include <QTimer>
#include "../../src/record_process.h"
#include "stub.h"
#include "addr_pri.h"

using namespace testing;

class RecordProcessTest:public testing::Test{

public:

    Stub stub;
    virtual void SetUp() override{
        std::cout << "start RecordProcessTest" << std::endl;

    }

    virtual void TearDown() override{
        std::cout << "end RecordProcessTest" << std::endl;
        stub.reset(ADDR(ConfigSettings, value));
        stub.reset(ADDR(QSysInfo, currentCpuArchitecture));
    }
};


QVariant getOption_gif_stub(void* obj, const QString &group, const QString &key)
{
    if(key == "save_as_gif") {
        return true;
    }else if(key == "save_directory"){
        return  "";
    }else if(key == "lossless_recording") {
        return false;
    }
    return true;
}

QVariant getOption_mp4_stub(void* obj, const QString &group, const QString &key)
{
    if(key == "save_as_gif") {
        return false;
    }else if(key == "save_directory"){
        return  "";
    }else if(key == "mkv_framerate"){
        return 24;
    }
    return true;
}

QVariant getOption_mkv_stub(void* obj, const QString &group, const QString &key)
{
    if(key == "save_as_gif") {
        return false;
    }else if(key == "mkv_framerate"){
        return 24;
    }else if(key == "save_directory"){
        return  "";
    }
    return  false;
}

static QString currentCpuArchitecture_stub(void* obj)
{
    return  "mips";
}

//TEST_F(RecordProcessTest, recordGIF)
//{
//    stub.set(ADDR(ConfigSettings, value), getOption_gif_stub);
//    RecordProcess *m_process;
//    m_process = new RecordProcess;
//    m_process->setRecordAudioInputType(RecordProcess::RECORD_AUDIO_INPUT_SYSTEMAUDIO);
//    m_process->setRecordInfo(QRect(0, 0, 1920, 1080), "gif");
//    m_process->startRecord();
//    QEventLoop loop;
//    QTimer::singleShot(5000, &loop, SLOT(quit()));
//    loop.exec();
//    m_process->stopRecord();
//    delete  m_process;
//}
void emitRecording_stub(){

}
TEST_F(RecordProcessTest, recordVideoMp4)
{
    stub.set(ADDR(ConfigSettings, value), getOption_mp4_stub);
    stub.set(ADDR(RecordProcess,emitRecording),emitRecording_stub);
    RecordProcess *m_process;
    m_process = new RecordProcess;
    //m_process->setRecordAudioInputType(RecordProcess::RECORD_AUDIO_INPUT_SYSTEMAUDIO);
    m_process->setRecordInfo(QRect(0, 0, 1920, 1080), "mp4");
    m_process->startRecord();
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
    m_process->stopRecord();
    delete  m_process;
    stub.reset(ADDR(RecordProcess,emitRecording));
}

TEST_F(RecordProcessTest, recordVideoMKV)
{
    stub.set(ADDR(ConfigSettings, value), getOption_mkv_stub);
    stub.set(ADDR(RecordProcess,emitRecording),emitRecording_stub);
    RecordProcess *m_process;
    m_process = new RecordProcess;
    //m_process->setRecordAudioInputType(RecordProcess::RECORD_AUDIO_INPUT_SYSTEMAUDIO);
    m_process->setRecordInfo(QRect(0, 0, 1920, 1080), "mkv");
    m_process->startRecord();
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
    m_process->stopRecord();
    delete  m_process;
    sleep(1);
    stub.reset(ADDR(RecordProcess,emitRecording));
}

TEST_F(RecordProcessTest, mpisRecordVideoMp4)
{
    stub.set(ADDR(QSysInfo, currentCpuArchitecture), currentCpuArchitecture_stub);
    stub.set(ADDR(ConfigSettings, value), getOption_mp4_stub);
    stub.set(ADDR(RecordProcess,emitRecording),emitRecording_stub);
    RecordProcess *m_process;
    m_process = new RecordProcess;
    //m_process->setRecordAudioInputType(RecordProcess::RECORD_AUDIO_INPUT_SYSTEMAUDIO);
    m_process->setRecordInfo(QRect(0, 0, 1920, 1080), "mips_mp4");
    m_process->startRecord();
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
    m_process->stopRecord();
    delete  m_process;
    stub.reset(ADDR(RecordProcess,emitRecording));
}

TEST_F(RecordProcessTest, mpisRecordVideoMKV)
{
    stub.set(ADDR(ConfigSettings, value), getOption_mkv_stub);
    stub.set(ADDR(QSysInfo, currentCpuArchitecture), currentCpuArchitecture_stub);
    stub.set(ADDR(RecordProcess,emitRecording),emitRecording_stub);
    RecordProcess *m_process;
    m_process = new RecordProcess;
    //m_process->setRecordAudioInputType(RecordProcess::RECORD_AUDIO_INPUT_SYSTEMAUDIO);
    m_process->setRecordInfo(QRect(0, 0, 1920, 1080), "mips_mkv");
    m_process->startRecord();
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
    m_process->stopRecord();
    delete  m_process;
    sleep(1);
    stub.reset(ADDR(RecordProcess,emitRecording));
}


