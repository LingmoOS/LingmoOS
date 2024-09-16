// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stub.h>
#include "logapplicationhelper.h"
#include <QDebug>

QByteArray stub_readLine(qint64 maxlen = 0)
{
    return "Hidden testX-Deepin-Vendor=deepin";
}

QByteArray stub_readLine001(qint64 maxlen = 0)
{
    return "NotShowIn testX-Deepin-Vendor=deepin";
}

void stub_parseField(QString path, QString name, bool isDeepin, bool isGeneric,
                     bool isName)
{
}

TEST(LogApplicationHelper_Constructor_UT, LogApplicationHelper_Constructor_UT)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}

TEST(LogApplicationHelper_init_UT, LogApplicationHelper_init_UT)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    p->init();
    p->deleteLater();
}

TEST(LogApplicationHelper_initAppLog_UT, LogApplicationHelper_initAppLog_UT)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    p->initAppLog();
    p->deleteLater();
}

TEST(LogApplicationHelper_initOtherLog_UT, LogApplicationHelper_initOtherLog_UT)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    p->initOtherLog();
    p->deleteLater();
}

TEST(LogApplicationHelper_initCustomLog_UT, LogApplicationHelper_initCustomLog_UT)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    p->initCustomLog();
    p->deleteLater();
}

TEST(LogApplicationHelper_createDesktopFiles_UT, LogApplicationHelper_createDesktopFiles_UT)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    p->createDesktopFiles();
    p->deleteLater();
}

TEST(LogApplicationHelper_createDesktopFiles_UT, LogApplicationHelper_createDesktopFiles_UT001)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    Stub stub;
    stub.set((QByteArray(QIODevice::*)(qint64))ADDR(QIODevice, readLine), stub_readLine);
    stub.set(ADDR(LogApplicationHelper, generateTransName), stub_parseField);
    p->createDesktopFiles();
    p->deleteLater();
}

TEST(LogApplicationHelper_createDesktopFiles_UT, LogApplicationHelper_createDesktopFiles_UT002)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    Stub stub;
    stub.set((QByteArray(QIODevice::*)(qint64))ADDR(QIODevice, readLine), stub_readLine001);
    stub.set(ADDR(LogApplicationHelper, generateTransName), stub_parseField);
    p->createDesktopFiles();
    p->deleteLater();
}

TEST(LogApplicationHelper_createLogFiles_UT, LogApplicationHelper_createLogFiles_UT)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    p->createLogFiles();
    p->deleteLater();
}

class LogApplicationHelper_parseField_UT_Param
{
public:
    LogApplicationHelper_parseField_UT_Param(bool iIsPathEmpty, bool iIsDeepin, bool iIsGeneric, bool iIsName)
    {
        isPathEmpty = iIsPathEmpty;
        isDeepin = iIsDeepin;
        isGeneric = iIsGeneric;
        isName = iIsName;
    }
    bool isPathEmpty;
    bool isDeepin;
    bool isGeneric;
    bool isName;
};

class LogApplicationHelper_parseField_UT : public ::testing::TestWithParam<LogApplicationHelper_parseField_UT_Param>
{
};

INSTANTIATE_TEST_CASE_P(LogApplication, LogApplicationHelper_parseField_UT, ::testing::Values(LogApplicationHelper_parseField_UT_Param(false, true, true, true), LogApplicationHelper_parseField_UT_Param(true, true, true, true), LogApplicationHelper_parseField_UT_Param(true, true, false, true), LogApplicationHelper_parseField_UT_Param(true, false, false, true)));

TEST_P(LogApplicationHelper_parseField_UT, LogApplicationHelper_parseField_UT)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    LogApplicationHelper_parseField_UT_Param param = GetParam();
    QString path = param.isPathEmpty ? "" : "../sources/dde-calendar.log";
    p->generateTransName(path, "dde-calendar.log", param.isDeepin, param.isGeneric, param.isName);

    p->deleteLater();
}

TEST(LogApplicationHelper_getLogFile_UT, LogApplicationHelper_getLogFile_UT_001)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    QString path = p->getLogFile("../sources/dde-calendar.log");
    p->deleteLater();
}
TEST(LogApplicationHelper_getLogFile_UT, LogApplicationHelper_getLogFile_UT_002)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    QString path = p->getLogFile("");
    EXPECT_EQ(path.isEmpty(), true);
    p->deleteLater();
}

TEST(LogApplicationHelper_getMap_UT, LogApplicationHelper_getMape_UT)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    p->getMap();
    p->deleteLater();
}

TEST(LogApplicationHelper_transName_UT, LogApplicationHelper_transName_UT)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    p->transName("");
    p->deleteLater();
}

TEST(LogApplicationHelper_getOtherLogList_UT, LogApplicationHelper_getOtherLogList_UT)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    p->getOtherLogList();
    p->deleteLater();
}

TEST(LogApplicationHelper_getCustomLogList_UT, LogApplicationHelper_getCustomLogList_UT)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    p->getCustomLogList();
    p->deleteLater();
}

TEST(LogApplicationHelper_getPathByAppId_UT, LogApplicationHelper_getPathByAppId_UT)
{
    LogApplicationHelper *p = new LogApplicationHelper(nullptr);
    EXPECT_NE(p, nullptr);
    p->getPathByAppId("deepin-log-viewer");
    p->deleteLater();
}

