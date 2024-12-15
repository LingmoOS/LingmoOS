// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dldbushandler.h"
#include "logfileparser.h"

#include <stub.h>
#include <QDebug>
#include <gtest/gtest.h>


TEST(UT_DLDBusHandler_readLog, UT_DLDBusHandler_readLog_001)
{
    DLDBusHandler *DLDbus = new DLDBusHandler();
    ASSERT_TRUE(DLDbus);
    DLDbus->readLog("test");
    delete DLDbus;
}

TEST(UT_DLDBusHandler_exitCode, UT_DLDBusHandler_exitCode_001)
{
    DLDBusHandler *DLDbus = new DLDBusHandler();
    ASSERT_TRUE(DLDbus);
    DLDbus->exitCode();
    delete DLDbus;
}

TEST(UT_DLDBusHandler_quit, UT_DLDBusHandler_quit_001)
{
    DLDBusHandler *DLDbus = new DLDBusHandler();
    ASSERT_TRUE(DLDbus);
    DLDbus->quit();
    delete DLDbus;
}


TEST(UT_DLDBusHandler_exportLog, UT_DLDBusHandler_exportLog_001)
{
    DLDBusHandler *DLDbus = new DLDBusHandler();
    ASSERT_TRUE(DLDbus);
    DLDbus->exportLog("path", "path1", false);
    delete DLDbus;
}


