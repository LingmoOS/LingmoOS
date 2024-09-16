// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dipconflictcheck.h"
#include "fakeDBus/ipconfliccheckservice.h"
#include <QDebug>

DNETWORKMANAGER_USE_NAMESPACE

class TestDIPConflictCheckConfig : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_fakeService = new FakeIpConflictCheckService();
        m_icc = new DIPConflictCheck();
    }

    static void TearDownTestCase()
    {
        delete m_icc;
        delete m_fakeService;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DIPConflictCheck *m_icc{nullptr};
    static inline FakeIpConflictCheckService *m_fakeService{nullptr};
};

TEST_F(TestDIPConflictCheckConfig, requestIPConflictCheck)
{
    m_icc->requestIPConflictCheck({}, {});
    EXPECT_EQ(true, m_fakeService->m_requestIPConflictCheckTrigger);
}
