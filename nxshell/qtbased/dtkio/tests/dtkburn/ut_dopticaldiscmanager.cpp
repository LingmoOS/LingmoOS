// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <DOpticalDiscManager>
#include <DOpticalDiscInfo>
#include <DOpticalDiscOperator>

#include <QString>

#include <gtest/gtest.h>
#include <stubext.h>

#include "dxorrisoengine.h"

DBURN_USE_NAMESPACE
DCORE_USE_NAMESPACE

class TestDOpticalDiscManager : public testing::Test
{
public:
    const QString m_testDev { "test-device" };

    void SetUp() override {}
    void TearDown() override { m_stub.clear(); }

    stub_ext::StubExt m_stub;
};

TEST_F(TestDOpticalDiscManager, CreateOpticalDiscInfo)
{
    QObject parent;

    // 测试无效情况
    {
        // 设定期望值
        DExpected<DOpticalDiscInfo *> expectedRet;
        expectedRet = DUnexpected<> { DError { -1, "Invalid device" } };
        // 检查结果
        auto ret = DOpticalDiscManager::createOpticalDiscInfo("", &parent);
        EXPECT_EQ(ret, expectedRet);
    }

    // 测试有效情况
    {
        m_stub.set_lamda(ADDR(DXorrisoEngine, acquireDevice), [](DXorrisoEngine *, const QString &) { __DBG_STUB_INVOKE__
                    return true; });
        m_stub.set_lamda(ADDR(DXorrisoEngine, releaseDevice), [] { __DBG_STUB_INVOKE__ });
        // 设定期望值
        DOpticalDiscInfo *expectedRet { new DOpticalDiscInfo(m_testDev, &parent) };
        // 检查结果
        auto ret = DOpticalDiscManager::createOpticalDiscInfo(m_testDev, &parent);
        EXPECT_TRUE(ret.hasValue());
        EXPECT_EQ(ret.value()->device(), expectedRet->device());
    }
}

TEST_F(TestDOpticalDiscManager, CreateOpticalDiscOperator)
{
    QObject parent;

    // 测试无效情况
    {
        // 设定期望值
        DExpected<DOpticalDiscOperator *> expectedRet;
        expectedRet = DUnexpected<> { DError { -1, "Empty device" } };
        // 检查结果
        auto ret = DOpticalDiscManager::createOpticalDiscOperator("", &parent);
        EXPECT_EQ(ret, expectedRet);
    }

    // 测试有效情况
    {
        // 设定期望值
        DOpticalDiscOperator *expectedRet { new DOpticalDiscOperator(m_testDev, &parent) };
        // 检查结果
        auto ret = DOpticalDiscManager::createOpticalDiscOperator(m_testDev, &parent);
        EXPECT_TRUE(ret.hasValue());
    }
}
