// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "gtest/gtest.h"

#include "daudiostream.h"
#include "daudiostream_p.h"

#include "ut-daudiohelpers.hpp"

#include <QScopedPointer>
#include <QSignalSpy>

DAUDIOMANAGER_USE_NAMESPACE

class ut_DAudioInputStream : public testing::Test
{
public:
    void SetUp() override
    {
        m_impl = new TestAudioInputStream();
        m_target.reset(m_impl->create());
    }
    void TearDown() override;
public:
    QScopedPointer<DAudioStream> m_target;
    QExplicitlySharedDataPointer<TestAudioInputStream> m_impl;
};

void ut_DAudioInputStream::TearDown() {}

TEST_F(ut_DAudioInputStream, base)
{
    {
        EXPECT_FALSE(m_target->mute());
        QSignalSpy spy(m_target.data(), &DAudioInputStream::muteChanged);
        m_target->setMute(true);
        EXPECT_EQ(spy.count(), 1);
        EXPECT_TRUE(m_target->mute());
    }

    {
        EXPECT_TRUE(qFuzzyCompare(m_target->fade(), 1.0));
        QSignalSpy spy(m_target.data(), &DAudioInputStream::fadeChanged);
        m_target->setFade(0.8);
        EXPECT_EQ(spy.count(), 1);
        EXPECT_TRUE(qFuzzyCompare(m_target->fade(), 0.8));
    }
}

class ut_DAudioOutputStream : public testing::Test
{
public:
    void SetUp() override
    {
        m_impl = new TestAudioOutputStream();
        m_target.reset(m_impl->create());
    }
    void TearDown() override;
public:
    QScopedPointer<DAudioStream> m_target;
    QExplicitlySharedDataPointer<TestAudioOutputStream> m_impl;
};

void ut_DAudioOutputStream::TearDown() {}

TEST_F(ut_DAudioOutputStream, base)
{
    {
        EXPECT_FALSE(m_target->mute());
        QSignalSpy spy(m_target.data(), &DAudioOutputStream::muteChanged);
        m_target->setMute(true);
        EXPECT_EQ(spy.count(), 1);
        EXPECT_TRUE(m_target->mute());
    }

    {
        EXPECT_TRUE(qFuzzyCompare(m_target->fade(), 1.0));
        QSignalSpy spy(m_target.data(), &DAudioOutputStream::fadeChanged);
        m_target->setFade(0.8);
        EXPECT_EQ(spy.count(), 1);
        EXPECT_TRUE(qFuzzyCompare(m_target->fade(), 0.8));
    }
}
