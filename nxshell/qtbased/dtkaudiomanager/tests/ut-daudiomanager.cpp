// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "gtest/gtest.h"

#include "daudiomanager.h"
#include "daudiomanager_p.h"
#include "ut-daudiohelpers.hpp"

#include <QScopedPointer>
#include <QSignalSpy>

DAUDIOMANAGER_USE_NAMESPACE

class ut_DAudioManager : public testing::Test
{
public:
    void SetUp() override
    {
        m_impl = new TestAudioManager();
        m_target.reset(new DAudioManager(m_impl.data()));
    }
    void TearDown() override;
public:
    QScopedPointer<DAudioManager> m_target;
    QPointer<TestAudioManager> m_impl;
};

void ut_DAudioManager::TearDown() {}

TEST_F(ut_DAudioManager, base)
{
    EXPECT_TRUE(qFuzzyCompare(m_target->maxVolume(), 1.0));
    EXPECT_EQ(m_target->increaseVolume(), false);
    m_target->setIncreaseVolume(true);
    EXPECT_EQ(m_target->increaseVolume(), true);
    EXPECT_EQ(m_target->reduceNoise(), false);
    m_target->setReduceNoise(true);
    EXPECT_EQ(m_target->reduceNoise(), true);
}

TEST_F(ut_DAudioManager, cards)
{
    EXPECT_TRUE(m_target->cards().isEmpty());
    QSignalSpy spy(m_target.data(), &DAudioManager::cardsChanged);
    auto card1 = new TestAudioCard();
    auto port1 = new TestAudioPort(card1);
    Q_UNUSED(port1);
    auto port2 = new TestAudioPort(card1, "test port2");
    Q_UNUSED(port2);
    auto card2  = new TestAudioCard("test card2");
    card2->setId(TestAudioCardId + 1);
    m_impl->addCard(card1);
    m_impl->addCard(card2);

    EXPECT_EQ(spy.count(), 2);

    auto targetCards = m_target->cards();
    EXPECT_EQ(targetCards.size(), 2);

    auto targetCard1 = m_target->card(TestAudioCardId);
    EXPECT_EQ(targetCard1->name(), card1->name());
    EXPECT_EQ(targetCard1->ports().size(), 2);
}

TEST_F(ut_DAudioManager, inputDevices)
{
    EXPECT_TRUE(m_target->inputDevices().isEmpty());
    QSignalSpy spy(m_target.data(), &DAudioManager::deviceAdded);
    auto card1 = new TestAudioCard();
    m_impl->addCard(card1);
    auto device1 = new TestAudioInputDevice(card1);
    auto device2 = new TestAudioInputDevice(card1);
    device2->setName("test inputdevice2");
    m_impl->addInputDevice(device1);
    m_impl->addInputDevice(device2);

    EXPECT_EQ(spy.count(), 2);

    auto targetDevices = m_target->inputDevices();
    EXPECT_EQ(targetDevices.size(), 2);

    auto targetDevice1 = m_target->inputDevice(TestAudioInputDeviceName);
    EXPECT_EQ(targetDevice1->name(), device1->name());
}
