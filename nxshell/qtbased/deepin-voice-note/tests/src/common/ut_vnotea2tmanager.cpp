// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnotea2tmanager.h"
#include "vnotea2tmanager.h"

UT_VNoteA2TManager::UT_VNoteA2TManager()
{
}

TEST_F(UT_VNoteA2TManager, UT_VNoteA2TManager_startAsr_001)
{
    VNoteA2TManager vnotea2tmanager;
    QString filepath = "/usr/share/music/bensound-sunny.mp3";
    qint64 fileDuration = 5460;
    vnotea2tmanager.startAsr(filepath, fileDuration, "", "");
    vnotea2tmanager.stopAsr();
}

TEST_F(UT_VNoteA2TManager, UT_VNoteA2TManager_onNotify_001)
{
    VNoteA2TManager vnotea2tmanager;
    QString messg = "{\n    \"code\": \"000000\",\n    \"descInfo\": \"success\",\n    \"failType\": 0,\n    \"status\": 4,\n    \"text\": \"乙醇汽油的保质期呢是15~25天，建议呢在15天之内用完，那乙醇汽油确实是比较容易变质的，但过了一个月还是可以继续使用的。这就好比我买了这个薯片过期了一天，啊那不是说马上它就烂掉了长蘑菇了，呃只是过了这个期限。啊\"\n}\n";
    vnotea2tmanager.onNotify(messg);
    QString messg1 = "{\n    \"code\": \"900003\",\n    \"descInfo\": \"success\",\n    \"failType\": 0,\n    \"status\": 4,\n    \"text\": \"乙醇汽油的保质期呢是15~25天，建议呢在15天之内用完，那乙醇汽油确实是比较容易变质的，但过了一个月还是可以继续使用的。这就好比我买了这个薯片过期了一天，啊那不是说马上它就烂掉了长蘑菇了，呃只是过了这个期限。啊\"\n}\n";
    vnotea2tmanager.onNotify(messg1);
    QString messg2 = "{\n    \"code\": \"000000\",\n    \"descInfo\": \"success\",\n    \"failType\": 0,\n    \"status\": -1,\n    \"text\": \"乙醇汽油的保质期呢是15~25天，建议呢在15天之内用完，那乙醇汽油确实是比较容易变质的，但过了一个月还是可以继续使用的。这就好比我买了这个薯片过期了一天，啊那不是说马上它就烂掉了长蘑菇了，呃只是过了这个期限。啊\"\n}\n";
    vnotea2tmanager.onNotify(messg2);
    QString messg3 = "{\n    \"code\": \"000000\",\n    \"descInfo\": \"success\",\n    \"failType\": 0,\n    \"status\": 0,\n    \"text\": \"乙醇汽油的保质期呢是15~25天，建议呢在15天之内用完，那乙醇汽油确实是比较容易变质的，但过了一个月还是可以继续使用的。这就好比我买了这个薯片过期了一天，啊那不是说马上它就烂掉了长蘑菇了，呃只是过了这个期限。啊\"\n}\n";
    vnotea2tmanager.onNotify(messg3);
}

TEST_F(UT_VNoteA2TManager, UT_VNoteA2TManager_getErrorCode_001)
{
    VNoteA2TManager vnotea2tmanager;
    asrMsg tmpstruct;
    tmpstruct.code = vnotea2tmanager.CODE_SUCCESS;
    tmpstruct.status = vnotea2tmanager.XF_fail;
    tmpstruct.failType = vnotea2tmanager.XF_upload;
    tmpstruct.descInfo = "test";
    tmpstruct.text = "test";
    EXPECT_EQ(VNoteA2TManager::ErrorCode::UploadAudioFileFail, vnotea2tmanager.getErrorCode(tmpstruct))
        << "XF_fail, XF_other";

    tmpstruct.failType = vnotea2tmanager.XF_decode;
    EXPECT_EQ(VNoteA2TManager::ErrorCode::AudioDecodeFail, vnotea2tmanager.getErrorCode(tmpstruct))
        << "XF_fail, XF_decode";

    tmpstruct.failType = vnotea2tmanager.XF_recognize;
    EXPECT_EQ(VNoteA2TManager::ErrorCode::AudioRecognizeFail, vnotea2tmanager.getErrorCode(tmpstruct))
        << "XF_fail, XF_recognize";

    tmpstruct.failType = vnotea2tmanager.XF_limit;
    EXPECT_EQ(VNoteA2TManager::ErrorCode::AudioExceedeLimit, vnotea2tmanager.getErrorCode(tmpstruct))
        << "XF_fail, XF_limit";

    tmpstruct.failType = vnotea2tmanager.XF_verify;
    EXPECT_EQ(VNoteA2TManager::ErrorCode::AudioVerifyFailed, vnotea2tmanager.getErrorCode(tmpstruct))
        << "XF_fail, XF_verify";

    tmpstruct.failType = vnotea2tmanager.XF_mute;
    EXPECT_EQ(VNoteA2TManager::ErrorCode::AudioMuteFile, vnotea2tmanager.getErrorCode(tmpstruct))
        << "XF_fail, XF_mute";

    tmpstruct.failType = vnotea2tmanager.XF_other;
    EXPECT_EQ(VNoteA2TManager::ErrorCode::AudioOther, vnotea2tmanager.getErrorCode(tmpstruct))
        << "XF_fail, XF_other";

    tmpstruct.code = vnotea2tmanager.CODE_NETWORK;
    EXPECT_EQ(VNoteA2TManager::ErrorCode::NetworkError, vnotea2tmanager.getErrorCode(tmpstruct))
        << "CODE_NETWORK, XF_fail";

    tmpstruct.code = vnotea2tmanager.CODE_SUCCESS;
    tmpstruct.status = vnotea2tmanager.XF_finish;
    EXPECT_EQ(VNoteA2TManager::ErrorCode::DontCareError, vnotea2tmanager.getErrorCode(tmpstruct))
        << "CODE_SUCCESS, XF_finish";
}
