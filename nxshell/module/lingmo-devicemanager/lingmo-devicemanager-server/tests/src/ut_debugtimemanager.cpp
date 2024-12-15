// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#define private public // hack complier
#define protected public

#include "debugtimemanager.h"

#undef private
#undef protected
#include "stub.h"

TEST(DebugTimeManager_Test, DebugTimeManager_UT_001)
{
    DebugTimeManager *dtm = new DebugTimeManager();
    dtm->beginPointLinux("001", "");
    ASSERT_FALSE(dtm->m_MapPoint.isEmpty());
    delete dtm;
}

TEST(DebugTimeManager_Test, DebugTimeManager_UT_002)
{
    DebugTimeManager *dtm = new DebugTimeManager();
    dtm->beginPointLinux("001", "");
    dtm->endPointLinux("001");
    ASSERT_TRUE(dtm->m_MapPoint["001"].time >= 0);
    delete dtm;
}

TEST(DebugTimeManager_Test, DebugTimeManager_UT_003)
{
    DebugTimeManager *dtm = new DebugTimeManager();
    dtm->clear();

    ASSERT_TRUE(dtm->m_MapPoint.isEmpty());
    delete dtm;
}
