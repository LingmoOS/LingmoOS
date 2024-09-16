// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/utils/voicevolumewatcher.h"
using namespace testing;

class voiceVolumeWatcherTest: public testing::Test
{

public:
    Stub stub;
    voiceVolumeWatcher *m_voiceVolumeWatcher;
    virtual void SetUp() override
    {
        std::cout << "start voiceVolumeWatcherTest" << std::endl;
        m_voiceVolumeWatcher = new voiceVolumeWatcher();
    }

    virtual void TearDown() override
    {
        delete m_voiceVolumeWatcher;
        std::cout << "end voiceVolumeWatcherTest" << std::endl;
    }
};

ACCESS_PRIVATE_FIELD(voiceVolumeWatcher,    QScopedPointer<com::deepin::daemon::audio::Source>, m_defaultSource);
TEST_F(voiceVolumeWatcherTest, getystemAudioState)
{
    m_voiceVolumeWatcher->getystemAudioState();
}

ACCESS_PRIVATE_FUN(voiceVolumeWatcher, void(const QString &), onCardsChanged);
ACCESS_PRIVATE_FUN(voiceVolumeWatcher, void(const QString &), initAvailInputPorts);
void initAvailInputPorts_stub(const QString &cards)
{
    Q_UNUSED(cards);
}
TEST_F(voiceVolumeWatcherTest, onCardsChanged)
{
    stub.set(get_private_fun::voiceVolumeWatcherinitAvailInputPorts(), initAvailInputPorts_stub);
    call_private_fun::voiceVolumeWatcheronCardsChanged(*m_voiceVolumeWatcher, "default");
    stub.reset(get_private_fun::voiceVolumeWatcherinitAvailInputPorts());
}

TEST_F(voiceVolumeWatcherTest, initAvailInputPorts)
{
    QString cards =  "[{\"Id\":0,\"Name\":\"HDA ATI HDMI\",\"Ports\":[{\"Name\":\"hdmi-output-0\",\"Description\":\"HDMI / DisplayPort\",\"Direction\":1}]},{\"Id\":1,\"Name\":\"HDA Intel\",\"Ports\":[{\"Name\":\"analog-input-front-mic\",\"Description\":\"前麦克风\",\"Direction\":2},{\"Name\":\"analog-input-rear-mic\",\"Description\":\"后麦克风\",\"Direction\":2},{\"Name\":\"analog-input-linein\",\"Description\":\"输入插孔\",\"Direction\":2},{\"Name\":\"analog-output-lineout\",\"Description\":\"线缆输出\",\"Direction\":1},{\"Name\":\"analog-output-headphones\",\"Description\":\"模拟耳机\",\"Direction\":1}]}]";
    call_private_fun::voiceVolumeWatcherinitAvailInputPorts(*m_voiceVolumeWatcher, cards);
}

