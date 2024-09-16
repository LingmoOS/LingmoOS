// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnoterecordbar.h"
#include "vnoterecordbar.h"
#include "globaldef.h"
#include "vnoterecordwidget.h"
#include "vnoteiconbutton.h"
#include "vnoteplaywidget.h"
#include "vnoteitem.h"
#include "audiowatcher.h"
#include "setting.h"
#include "vnotemessagedialog.h"
#include "stub.h"

#include <QDialog>

static bool stub_true()
{
    return true;
}

static bool stub_false()
{
    return false;
}

static int stub_dialog()
{
    return 1;
}

static QString stub_str()
{
    return "abc";
}

static QString stub_emptyStr()
{
    return "";
}

UT_VNoteRecordBar::UT_VNoteRecordBar()
{
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_eventFilter_001)
{
    VNoteRecordBar vnoterecordbar;
    QEvent *event = new QEvent(QEvent::MouseButtonPress);
    QWidget *widget = new QWidget;
    EXPECT_FALSE(vnoterecordbar.eventFilter(widget, event));
    delete event;
    delete widget;
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_onStartRecord_001)
{
    Stub stub;
    stub.set(ADDR(VNoteRecordWidget, startRecord), stub_true);
    stub.set(ADDR(QWidget, isVisible), stub_true);
    stub.set(ADDR(QWidget, isEnabled), stub_true);

    VNoteRecordBar vnoterecordbar;
    vnoterecordbar.m_mainLayout->setCurrentWidget(vnoterecordbar.m_recordBtnHover);
    vnoterecordbar.onStartRecord();
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_onStartRecord_002)
{
    Stub stub;
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    stub.set(A_foo, stub_dialog);
    stub.set(ADDR(VNoteRecordBar, volumeToolow), stub_true);
    stub.set(ADDR(VNoteRecordWidget, startRecord), stub_true);
    stub.set(ADDR(QWidget, isVisible), stub_true);
    stub.set(ADDR(QWidget, isEnabled), stub_true);

    VNoteRecordBar vnoterecordbar;
    vnoterecordbar.m_mainLayout->setCurrentWidget(vnoterecordbar.m_recordBtnHover);
    vnoterecordbar.onStartRecord();
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_startRecord_001)
{
    Stub stub;
    stub.set(ADDR(VNoteRecordWidget, startRecord), stub_true);

    VNoteRecordBar vnoterecordbar;
    vnoterecordbar.startRecord();
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_onFinshRecord_001)
{
    VNoteRecordBar vnoterecordbar;
    QString tmppath = "/usr/share/music/bensound-sunny.mp3";
    vnoterecordbar.onFinshRecord(tmppath, 2650);
    EXPECT_EQ(vnoterecordbar.m_mainLayout->currentWidget(), vnoterecordbar.m_recordBtnHover);
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_stopRecord_001)
{
    VNoteRecordBar vnoterecordbar;
    vnoterecordbar.stopRecord();
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_onClosePlayWidget_001)
{
    VNoteRecordBar vnoterecordbar;
    VNVoiceBlock *vnvoiceblock = new VNVoiceBlock;
    vnvoiceblock->voicePath = "/usr/share/music/bensound-sunny.mp3";
    vnvoiceblock->voiceSize = 2650;
    vnvoiceblock->voiceTitle = "test";
    vnvoiceblock->state = true;
    vnoterecordbar.onClosePlayWidget(vnvoiceblock);
    delete vnvoiceblock;
    EXPECT_EQ(vnoterecordbar.m_mainLayout->currentWidget(), vnoterecordbar.m_recordBtnHover);
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_onAudioVolumeChange_001)
{
    Stub stub;
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    stub.set(A_foo, stub_dialog);
    stub.set(ADDR(VNoteRecordBar, volumeToolow), stub_true);

    VNoteRecordBar vnoterecordbar;
    vnoterecordbar.m_mainLayout->setCurrentWidget(vnoterecordbar.m_recordPanel);
    vnoterecordbar.m_currentMode = AudioWatcher::Internal;
    vnoterecordbar.onAudioVolumeChange(AudioWatcher::Internal);
    EXPECT_TRUE(vnoterecordbar.m_showVolumeWanning);
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_onAudioVolumeChange_002)
{
    Stub stub;
    stub.set(ADDR(VNoteRecordBar, volumeToolow), stub_false);

    VNoteRecordBar vnoterecordbar;
    vnoterecordbar.m_mainLayout->setCurrentWidget(vnoterecordbar.m_recordPanel);
    vnoterecordbar.m_showVolumeWanning = true;
    vnoterecordbar.m_currentMode = AudioWatcher::Internal;
    vnoterecordbar.onAudioVolumeChange(AudioWatcher::Internal);
    EXPECT_FALSE(vnoterecordbar.m_showVolumeWanning);
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_onAudioDeviceChange_001)
{
    Stub stub;
    stub.set(ADDR(AudioWatcher, getDeviceName), stub_emptyStr);
    VNoteRecordBar vnoterecordbar;
    vnoterecordbar.m_mainLayout->setCurrentWidget(vnoterecordbar.m_recordPanel);
    vnoterecordbar.m_showVolumeWanning = true;
    vnoterecordbar.m_currentMode = AudioWatcher::Internal;
    vnoterecordbar.onAudioDeviceChange(AudioWatcher::Internal);
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_onAudioDeviceChange_002)
{
    Stub stub;
    stub.set(ADDR(AudioWatcher, getDeviceName), stub_str);
    VNoteRecordBar vnoterecordbar;
    vnoterecordbar.m_mainLayout->setCurrentWidget(vnoterecordbar.m_recordPanel);
    vnoterecordbar.m_showVolumeWanning = true;
    vnoterecordbar.m_currentMode = AudioWatcher::Internal;
    vnoterecordbar.onAudioDeviceChange(AudioWatcher::Internal);
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_onAudioSelectChange_001)
{
    VNoteRecordBar vnoterecordbar;
    vnoterecordbar.onAudioSelectChange(1);
    EXPECT_EQ(vnoterecordbar.m_currentMode, 1);
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_volumeToolow_001)
{
    VNoteRecordBar vnoterecordbar;
    EXPECT_FALSE(vnoterecordbar.volumeToolow(1.1));
}

TEST_F(UT_VNoteRecordBar, UT_VNoteRecordBar_stopPlay_001)
{
    VNoteRecordBar vnoterecordbar;
    vnoterecordbar.stopPlay();
}
