// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vtextspeechandtrmanager.h"
#include "vtextspeechandtrmanager.h"

UT_VTextSpeechAndTrManager::UT_VTextSpeechAndTrManager()
{
}

void UT_VTextSpeechAndTrManager::SetUp()
{
    m_vtextspeechandtrmanager = new VTextSpeechAndTrManager;
}

void UT_VTextSpeechAndTrManager::TearDown()
{
    delete m_vtextspeechandtrmanager;
}

TEST_F(UT_VTextSpeechAndTrManager, UT_VTextSpeechAndTrManager_getTextToSpeechEnable_001)
{
    m_vtextspeechandtrmanager->getTextToSpeechEnable();
}

TEST_F(UT_VTextSpeechAndTrManager, UT_VTextSpeechAndTrManager_getSpeechToTextEnable_001)
{
    m_vtextspeechandtrmanager->getSpeechToTextEnable();
}

TEST_F(UT_VTextSpeechAndTrManager, UT_VTextSpeechAndTrManager_getTransEnable_001)
{
    m_vtextspeechandtrmanager->getTransEnable();
}

TEST_F(UT_VTextSpeechAndTrManager, UT_VTextSpeechAndTrManager_onTextToSpeech_001)
{
    m_vtextspeechandtrmanager->onTextToSpeech();
    m_vtextspeechandtrmanager->onStopTextToSpeech();
}

TEST_F(UT_VTextSpeechAndTrManager, UT_VTextSpeechAndTrManager_isTextToSpeechInWorking_001)
{
    m_vtextspeechandtrmanager->onTextToSpeech();
    m_vtextspeechandtrmanager->isTextToSpeechInWorking();
}

TEST_F(UT_VTextSpeechAndTrManager, UT_VTextSpeechAndTrManager_onSpeechToText_001)
{
    m_vtextspeechandtrmanager->onSpeechToText();
}

TEST_F(UT_VTextSpeechAndTrManager, UT_VTextSpeechAndTrManager_onTextTranslate_001)
{
    m_vtextspeechandtrmanager->onTextTranslate();
}
