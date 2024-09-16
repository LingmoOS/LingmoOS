// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "gtest/gtest.h"

#include "daudiomanager.h"
#include "daudiomanager_p.h"
#include "ut-daudiohelpers.hpp"
#include "daudiomanager_daemon.h"
#include "fakedbus/daemonaudiomanagerservice.h"

#include <QScopedPointer>
#include <QSignalSpy>

DAUDIOMANAGER_USE_NAMESPACE

class ut_DDaemonAudioManager : public testing::Test
{
public:
    void SetUp() override
    {
        m_serivice.reset(new FakeDaemonAudioManagerService());

        m_impl = new DDaemonAudioManager();
        m_target.reset(new DAudioManager(m_impl.data()));
    }
    void TearDown() override;
public:
    QScopedPointer<DAudioManager> m_target;
    QPointer<DDaemonAudioManager> m_impl;
    QScopedPointer<FakeDaemonAudioManagerService> m_serivice;
};

void ut_DDaemonAudioManager::TearDown() {}

TEST_F(ut_DDaemonAudioManager, base)
{
    EXPECT_TRUE(qFuzzyCompare(m_target->maxVolume(), 1.0));
    EXPECT_EQ(m_target->increaseVolume(), false);
    m_target->setIncreaseVolume(true);
    EXPECT_EQ(m_target->increaseVolume(), true);
    EXPECT_EQ(m_target->reduceNoise(), false);
    m_target->setReduceNoise(true);
    EXPECT_EQ(m_target->reduceNoise(), true);
}

TEST_F(ut_DDaemonAudioManager, cards)
{
    auto targetCards = m_target->cards();
    EXPECT_EQ(targetCards.size(), 1);

    auto targetCard1 = m_target->card(TestAudioCardId);
    EXPECT_EQ(targetCard1->name(), "HDA Intel PCH");

    auto ports = targetCard1->ports();
    EXPECT_EQ(ports.size(), 2);
    QStringList portNames;
    for (auto item : ports) {
        portNames << item->name();
    }
    EXPECT_TRUE(portNames.contains("analog-input-front-mic"));
}

TEST_F(ut_DDaemonAudioManager, outputDevices)
{
    auto targetDevices = m_target->outputDevices();
    EXPECT_EQ(targetDevices.size(), 1);

    auto targetDevice1 = m_target->outputDevice("11");
    EXPECT_TRUE(targetDevice1);
    EXPECT_EQ(targetDevice1->name(), "11");
    EXPECT_EQ(targetDevice1->mute(), true);
    EXPECT_TRUE(qFuzzyCompare(targetDevice1->volume(), 1.0));

    auto targetStreams = targetDevice1->streams();
    EXPECT_EQ(targetStreams.size(), 1);
    auto targetStream1 = targetDevice1->stream("111");
    EXPECT_TRUE(targetStream1);
    EXPECT_EQ(targetStream1->name(), "111");
    EXPECT_EQ(targetDevice1->mute(), true);
}
