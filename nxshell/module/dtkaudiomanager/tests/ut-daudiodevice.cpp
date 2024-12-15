// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "gtest/gtest.h"

#include "daudiodevice.h"
#include "daudiodevice_p.h"

#include "ut-daudiohelpers.hpp"

#include <QScopedPointer>
#include <QSignalSpy>

DAUDIOMANAGER_USE_NAMESPACE

class ut_DAudioInputDevice : public testing::Test
{
public:
    void SetUp() override
    {
        m_impl = new TestAudioInputDevice();
        m_target.reset(m_impl->create());
    }
    void TearDown() override;
public:
    QScopedPointer<DAudioDevice> m_target;
    QExplicitlySharedDataPointer<TestAudioInputDevice> m_impl;
};

void ut_DAudioInputDevice::TearDown() {}

TEST_F(ut_DAudioInputDevice, base)
{
    EXPECT_EQ(TestAudioInputDeviceName, m_target->name());
    EXPECT_EQ(TestAudioInputDeviceDescription, m_target->description());

    {
        EXPECT_FALSE(m_target->mute());
        QSignalSpy spy(m_target.data(), &DAudioInputDevice::muteChanged);
        m_target->setMute(true);
        EXPECT_EQ(spy.count(), 1);
        EXPECT_TRUE(m_target->mute());
    }

    {
        EXPECT_TRUE(qFuzzyCompare(m_target->fade(), 1.0));
        QSignalSpy spy(m_target.data(), &DAudioInputDevice::fadeChanged);
        m_target->setFade(0.8);
        EXPECT_EQ(spy.count(), 1);
        EXPECT_TRUE(qFuzzyCompare(m_target->fade(), 0.8));
    }
}

class ut_DAudioOutputDevice : public testing::Test
{
public:
    void SetUp() override
    {
        m_impl = new TestAudioOutputDevice();
        m_target.reset(m_impl->create());
    }
    void TearDown() override;
public:
    QScopedPointer<DAudioDevice> m_target;
    QExplicitlySharedDataPointer<TestAudioOutputDevice> m_impl;
};

void ut_DAudioOutputDevice::TearDown() {}

TEST_F(ut_DAudioOutputDevice, base)
{
    EXPECT_EQ(TestAudioOutputDeviceName, m_target->name());
    EXPECT_EQ(TestAudioOutputDeviceDescription, m_target->description());

    {
        EXPECT_FALSE(m_target->mute());
        QSignalSpy spy(m_target.data(), &DAudioOutputDevice::muteChanged);
        m_target->setMute(true);
        EXPECT_EQ(spy.count(), 1);
        EXPECT_TRUE(m_target->mute());
    }

    {
        EXPECT_TRUE(qFuzzyCompare(m_target->fade(), 1.0));
        QSignalSpy spy(m_target.data(), &DAudioOutputDevice::fadeChanged);
        m_target->setFade(0.8);
        EXPECT_EQ(spy.count(), 1);
        EXPECT_TRUE(qFuzzyCompare(m_target->fade(), 0.8));
    }
}
