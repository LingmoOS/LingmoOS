// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#define private public
#include "log/LogManager.h"
#undef private

#include "dpathbuf.h"
#include "dstandardpaths.h"
#include "test_helper.hpp"
#include <gtest/gtest.h>

DCORE_USE_NAMESPACE

TEST(ut_DLogManager, testDLogManager)
{
    DPathBuf logPath(QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first());

    logPath = logPath / "tests.log";

    ASSERT_EQ(DLogManager::getlogFilePath(), logPath.toString());
}

TEST(ut_DLogManager, testDefaultLogPath)
{
    EnvGuard guard;
    guard.unset("HOME");

    // unset HOME env will not init default log file path
    ASSERT_TRUE(DLogManager::getlogFilePath().contains(DStandardPaths::homePath()));
}

TEST(ut_DLogManager, testSetInvalidLogPath)
{
    QString tmp = QDir::tempPath();
    DLogManager::setlogFilePath(tmp);
    // set log file path to a dir is not supported
    ASSERT_NE(DLogManager::getlogFilePath(), tmp);
}
