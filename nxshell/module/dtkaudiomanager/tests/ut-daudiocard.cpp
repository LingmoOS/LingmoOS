// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "gtest/gtest.h"

#include "daudioport.h"
#include "daudioport_p.h"

#include "daudiocard.h"
#include "daudiocard_p.h"
#include "ut-daudiohelpers.hpp"

#include <QScopedPointer>
#include <QSignalSpy>

DAUDIOMANAGER_USE_NAMESPACE

class ut_DAudioCard : public testing::Test
{
public:
    void SetUp() override
    {
        m_impl = new TestAudioCard();
        m_target.reset(m_impl->create());
    }
    void TearDown() override;
public:
    QScopedPointer<DAudioCard> m_target;
    QExplicitlySharedDataPointer<TestAudioCard> m_impl;
};

void ut_DAudioCard::TearDown() {}

TEST_F(ut_DAudioCard, base)
{
    EXPECT_EQ(TestAudioCardName, m_target->name());
    EXPECT_FALSE(m_target->enabled());
    QSignalSpy spy(m_target.data(), &DAudioCard::enabledChanged);
    m_impl->setEnabled(true);
    EXPECT_TRUE(m_target->enabled());
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DAudioCard, port)
{
    EXPECT_TRUE(m_target->ports().isEmpty());
    QExplicitlySharedDataPointer<TestAudioPort> port1(new TestAudioPort(m_impl.data()));
    EXPECT_EQ(m_target->ports().count(), 1);
    auto port = m_target->ports()[0];
    EXPECT_EQ(port->name(), port1->name());
}
