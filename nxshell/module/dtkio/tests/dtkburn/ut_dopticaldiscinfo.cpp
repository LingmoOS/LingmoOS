// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DOpticalDiscInfo>

#include <gtest/gtest.h>
#include <stubext.h>

#include "dxorrisoengine.h"
#include "dopticaldiscinfo_p.h"

DBURN_USE_NAMESPACE
DCORE_USE_NAMESPACE

class TestDOpticalDiscInfo : public testing::Test
{
public:
    void SetUp() override
    {
        m_stub.set_lamda(ADDR(DXorrisoEngine, acquireDevice), [](DXorrisoEngine *, const QString &dev) { __DBG_STUB_INVOKE__
            if (dev == "test_failed")
                return false;
            return true; });
        m_stub.set_lamda(ADDR(DXorrisoEngine, releaseDevice), [] { __DBG_STUB_INVOKE__ });
        m_discInfo = new DOpticalDiscInfo("test");
    }
    void TearDown() override
    {
        m_stub.clear();
        delete m_discInfo;
    }

    stub_ext::StubExt m_stub;
    DOpticalDiscInfo *m_discInfo { nullptr };
};

TEST_F(TestDOpticalDiscInfo, iniData)
{
    DOpticalDiscInfo *info { new DOpticalDiscInfo("test_failed") };
    EXPECT_EQ(info->device(), "");
    delete info;
}

TEST_F(TestDOpticalDiscInfo, operatorEqual)
{
    DOpticalDiscInfo info1(*m_discInfo);
    DOpticalDiscInfo info2(*m_discInfo);
    info1 = info2;
    EXPECT_EQ(info1.device(), info2.device());
}

TEST_F(TestDOpticalDiscInfo, blank)
{
    m_discInfo->d_ptr->formatted = true;
    bool result = m_discInfo->blank();
    EXPECT_TRUE(result);
}

TEST_F(TestDOpticalDiscInfo, device)
{
    EXPECT_EQ(m_discInfo->device(), "test");
    m_discInfo->d_ptr->devid = "test2";
    EXPECT_EQ(m_discInfo->device(), "test2");
}

TEST_F(TestDOpticalDiscInfo, volumeName)
{
    m_discInfo->d_ptr->volid = "123";
    EXPECT_EQ(m_discInfo->volumeName(), "123");
}

TEST_F(TestDOpticalDiscInfo, usedSize)
{
    m_discInfo->d_ptr->data = 1024;
    EXPECT_EQ(m_discInfo->usedSize(), 1024);
}

TEST_F(TestDOpticalDiscInfo, availableSize)
{
    m_discInfo->d_ptr->avail = 2048;
    EXPECT_EQ(m_discInfo->availableSize(), 2048);
}

TEST_F(TestDOpticalDiscInfo, totalSize)
{
    m_discInfo->d_ptr->data = 1024;
    m_discInfo->d_ptr->avail = 2048;

    EXPECT_EQ(m_discInfo->totalSize(), 1024 + 2048);
}

TEST_F(TestDOpticalDiscInfo, dataBlocks)
{
    m_discInfo->d_ptr->datablocks = 100;
    EXPECT_EQ(m_discInfo->dataBlocks(), 100);
}

TEST_F(TestDOpticalDiscInfo, mediaType)
{
    m_discInfo->d_ptr->media = MediaType::CD_R;
    EXPECT_EQ(m_discInfo->mediaType(), MediaType::CD_R);
}

TEST_F(TestDOpticalDiscInfo, writeSpeed)
{
    m_discInfo->d_ptr->writespeed = QStringList("10.0x");
    EXPECT_EQ(m_discInfo->writeSpeed(), QStringList("10.0x"));
}
