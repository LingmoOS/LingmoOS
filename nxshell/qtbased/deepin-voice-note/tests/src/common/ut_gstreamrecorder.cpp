// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_gstreamrecorder.h"
#include "gstreamrecorder.h"
#include "vnoterecordbar.h"

UT_GstreamRecorder::UT_GstreamRecorder()
{
}

TEST_F(UT_GstreamRecorder, UT_GstreamRecorder_GetGstState_001)
{
    int state = -1;
    int pending = -1;
    GstreamRecorder gstreamrecorder;
    gstreamrecorder.createPipe();
    gstreamrecorder.GetGstState(&state, &pending);
    EXPECT_NE(-1, state) << "state";
    EXPECT_NE(-1, pending) << "pending";
}

TEST_F(UT_GstreamRecorder, UT_GstreamRecorder_pauseRecord_001)
{
    GstreamRecorder gstreamrecorder;
    gstreamrecorder.createPipe();
    gstreamrecorder.pauseRecord();
}

TEST_F(UT_GstreamRecorder, UT_GstreamRecorder_stopRecord_001)
{
    GstreamRecorder gstreamrecorder;
    gstreamrecorder.createPipe();
    gstreamrecorder.stopRecord();
}

TEST_F(UT_GstreamRecorder, UT_GstreamRecorder_setDevice_001)
{
    GstreamRecorder gstreamrecorder;
    gstreamrecorder.createPipe();
    gstreamrecorder.m_currentDevice = "123";
    gstreamrecorder.setDevice(QString("test"));
    EXPECT_EQ("test", gstreamrecorder.m_currentDevice);
}

TEST_F(UT_GstreamRecorder, UT_GstreamRecorder_setStateToNull_001)
{
    GstreamRecorder gstreamrecorder;
    gstreamrecorder.createPipe();
    gstreamrecorder.setStateToNull();
}

TEST_F(UT_GstreamRecorder, UT_GstreamRecorder_initFormat)
{
    GstreamRecorder gstreamrecorder;
    gstreamrecorder.initFormat();
    EXPECT_EQ(44100, gstreamrecorder.m_format.sampleRate()) << "code";
    EXPECT_EQ(QAudioFormat::LittleEndian, gstreamrecorder.m_format.byteOrder()) << "byteOrder";
    EXPECT_EQ(QAudioFormat::SignedInt, gstreamrecorder.m_format.sampleType()) << "sampleType";
    EXPECT_EQ(16, gstreamrecorder.m_format.sampleSize()) << "sampleSize";
}
