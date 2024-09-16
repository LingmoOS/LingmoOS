// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnoteplaywidget.h"
#include "vnoteplaywidget.h"
#include "vnoteitem.h"
#include "vnote2siconbutton.h"
#include "utils.h"

#include <stub.h>

static VlcPalyer::VlcState stub_playingState()
{
    return VlcPalyer::Playing;
}

static VlcPalyer::VlcState stub_pausedState()
{
    return VlcPalyer::Paused;
}

static void stub_void()
{
}

UT_VNotePlayWidget::UT_VNotePlayWidget()
{
}

void UT_VNotePlayWidget::SetUp()
{
    m_vnoteplaywidget = new VNotePlayWidget;
}

void UT_VNotePlayWidget::TearDown()
{
    delete m_vnoteplaywidget;
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_onVoicePlayPosChange_001)
{
    m_vnoteplaywidget->onVoicePlayPosChange(24);
    EXPECT_EQ(m_vnoteplaywidget->m_slider->value(), 24);
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_onSliderPressed_001)
{
    m_vnoteplaywidget->onSliderPressed();
    EXPECT_FALSE(m_vnoteplaywidget->m_sliderReleased);
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_onCloseBtnClicked_001)
{
    m_vnoteplaywidget->onCloseBtnClicked();
    EXPECT_EQ(m_vnoteplaywidget->m_slider->value(), 0);
    EXPECT_TRUE(m_vnoteplaywidget->m_sliderReleased);
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_onSliderReleased_001)
{
    Stub stub;
    stub.set(ADDR(VlcPalyer, getState), stub_playingState);

    m_vnoteplaywidget->m_slider->setValue(100);
    m_vnoteplaywidget->m_slider->setMaximum(80);
    m_vnoteplaywidget->onSliderReleased();
    EXPECT_TRUE(m_vnoteplaywidget->m_sliderReleased);
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_onSliderReleased_002)
{
    Stub stub;
    stub.set(ADDR(VlcPalyer, getState), stub_playingState);
    m_vnoteplaywidget->onSliderReleased();
    EXPECT_TRUE(m_vnoteplaywidget->m_sliderReleased);
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_onSliderMove_001)
{
    VNVoiceBlock *voiceBlock = new VNVoiceBlock;
    m_vnoteplaywidget->m_voiceBlock = voiceBlock;
    m_vnoteplaywidget->m_sliderReleased = true;
    m_vnoteplaywidget->onSliderMove(50);
    delete voiceBlock;
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_eventFilter_001)
{
    QKeyEvent *e = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Left, Qt::NoModifier);
    m_vnoteplaywidget->eventFilter(m_vnoteplaywidget->m_slider, e);
    EXPECT_EQ(0, m_vnoteplaywidget->m_slider->value());
    delete e;
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_eventFilter_002)
{
    Stub stub;
    stub.set(ADDR(VlcPalyer, getState), stub_playingState);
    QKeyEvent *e = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Left, Qt::NoModifier);
    m_vnoteplaywidget->eventFilter(m_vnoteplaywidget->m_slider, e);
    EXPECT_EQ(0, m_vnoteplaywidget->m_slider->value());
    delete e;
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_eventFilter_003)
{
    m_vnoteplaywidget->m_slider->setValue(100);
    m_vnoteplaywidget->m_slider->setMaximum(80);
    QKeyEvent *e = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Right, Qt::NoModifier);
    m_vnoteplaywidget->eventFilter(m_vnoteplaywidget->m_slider, e);
    EXPECT_EQ(80, m_vnoteplaywidget->m_slider->value());
    delete e;
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_eventFilter_004)
{
    Stub stub;
    stub.set(ADDR(VlcPalyer, getState), stub_playingState);
    m_vnoteplaywidget->m_slider->setValue(100);
    m_vnoteplaywidget->m_slider->setMaximum(80);
    QKeyEvent *e = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Right, Qt::NoModifier);
    m_vnoteplaywidget->eventFilter(m_vnoteplaywidget->m_slider, e);
    EXPECT_EQ(80, m_vnoteplaywidget->m_slider->value());
    delete e;
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_eventFilter_005)
{
    QKeyEvent *e = new QKeyEvent(QEvent::KeyRelease, Qt::Key_End, Qt::NoModifier);
    m_vnoteplaywidget->eventFilter(m_vnoteplaywidget->m_slider, e);
    delete e;
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_eventFilter_006)
{
    QKeyEvent *e = new QKeyEvent(QEvent::Enter, Qt::Key_End, Qt::NoModifier);
    m_vnoteplaywidget->eventFilter(nullptr, e);
    EXPECT_FALSE(m_vnoteplaywidget->m_nameLab->isVisible());
    delete e;
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_eventFilter_007)
{
    QKeyEvent *e = new QKeyEvent(QEvent::Leave, Qt::Key_End, Qt::NoModifier);
    m_vnoteplaywidget->eventFilter(nullptr, e);
    delete e;
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_getPlayerStatus_005)
{
    EXPECT_EQ(VlcPalyer::None, m_vnoteplaywidget->getPlayerStatus());
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_onDurationChanged_005)
{
    m_vnoteplaywidget->onDurationChanged(100);
    EXPECT_EQ(100, m_vnoteplaywidget->m_slider->maximum());
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_onPlayerBtnClicked_005)
{
    m_vnoteplaywidget->onPlayerBtnClicked();
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_playVoice_001)
{
    Stub stub;
    stub.set(ADDR(VlcPalyer, getState), stub_playingState);
    VNVoiceBlock *voiceBlock = new VNVoiceBlock;
    m_vnoteplaywidget->m_voiceBlock = voiceBlock;
    m_vnoteplaywidget->playVoice(voiceBlock, true);
    delete voiceBlock;
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_playVoice_002)
{
    Stub stub;
    stub.set(ADDR(VlcPalyer, getState), stub_pausedState);
    VNVoiceBlock *voiceBlock = new VNVoiceBlock;
    m_vnoteplaywidget->m_voiceBlock = voiceBlock;
    m_vnoteplaywidget->playVoice(voiceBlock, true);
    delete voiceBlock;
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_playVoice_003)
{
    Stub stub;
    stub.set(ADDR(VlcPalyer, play), stub_void);
    stub.set(ADDR(VlcPalyer, setFilePath), stub_void);
    VNVoiceBlock *voiceBlock = new VNVoiceBlock;
    voiceBlock->voicePath = "/tmp/test";
    m_vnoteplaywidget->playVoice(voiceBlock, false);
    EXPECT_EQ(voiceBlock, m_vnoteplaywidget->m_voiceBlock);
    EXPECT_EQ(0, m_vnoteplaywidget->m_slider->value());
    delete voiceBlock;
}

TEST_F(UT_VNotePlayWidget, UT_VNotePlayWidget_playVoice_004)
{
    m_vnoteplaywidget->playVoice(nullptr, false);
}
