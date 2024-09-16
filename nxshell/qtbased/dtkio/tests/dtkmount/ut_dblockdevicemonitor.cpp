// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmount_global.h"
#include "dblockdevicemonitor.h"

#include <gtest/gtest.h>
#include <stubext.h>

DMOUNT_USE_NAMESPACE

class TestDBlockDeviceMonitor : public testing::Test
{
public:
    void SetUp() override { }
    void TearDown() override { m_stub.clear(); }

    stub_ext::StubExt m_stub;
    DBlockDeviceMonitor m_obj;
};

TEST_F(TestDBlockDeviceMonitor, setWatchChanges)
{
    EXPECT_NO_FATAL_FAILURE(m_obj.setWatchChanges(true));
    EXPECT_NO_FATAL_FAILURE(m_obj.setWatchChanges(false));
}

TEST_F(TestDBlockDeviceMonitor, watchChanges)
{
    EXPECT_NO_FATAL_FAILURE(m_obj.watchChanges());
    EXPECT_NO_FATAL_FAILURE(m_obj.setWatchChanges(true));
    EXPECT_TRUE(m_obj.watchChanges());
}
