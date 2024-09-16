// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_opsstateinterface.h"
#include "opsstateinterface.h"

UT_OpsStateInterface::UT_OpsStateInterface()
{
}

void UT_OpsStateInterface::SetUp()
{
    m_opsstateinterface = new OpsStateInterface();
}

void UT_OpsStateInterface::TearDown()
{
    delete m_opsstateinterface;
}

TEST_F(UT_OpsStateInterface, UT_OpsStateInterface_isSearching_001)
{
    m_opsstateinterface->operState(m_opsstateinterface->StateSearching, true);
    EXPECT_TRUE(m_opsstateinterface->isSearching());
}

TEST_F(UT_OpsStateInterface, UT_OpsStateInterface_isRecording_001)
{
    m_opsstateinterface->operState(m_opsstateinterface->StateRecording, true);
    EXPECT_TRUE(m_opsstateinterface->isRecording());
}

TEST_F(UT_OpsStateInterface, UT_OpsStateInterface_isPlaying_001)
{
    m_opsstateinterface->operState(m_opsstateinterface->StatePlaying, true);
    EXPECT_TRUE(m_opsstateinterface->isPlaying());
}

TEST_F(UT_OpsStateInterface, UT_OpsStateInterface_isVoice2Text_001)
{
    m_opsstateinterface->operState(m_opsstateinterface->StateVoice2Text, true);
    EXPECT_TRUE(m_opsstateinterface->isVoice2Text());
}

TEST_F(UT_OpsStateInterface, UT_OpsStateInterface_isAppQuit_001)
{
    m_opsstateinterface->operState(m_opsstateinterface->StateAppQuit, true);
    EXPECT_TRUE(m_opsstateinterface->isAppQuit());
}

TEST_F(UT_OpsStateInterface, UT_OpsStateInterface_isAiSrvExist_001)
{
    m_opsstateinterface->operState(m_opsstateinterface->StateAISrvAvailable, true);
    EXPECT_TRUE(m_opsstateinterface->isAiSrvExist());
}
