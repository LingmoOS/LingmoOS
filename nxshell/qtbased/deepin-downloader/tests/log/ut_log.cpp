// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include "log.h"
#include "stub.h"
#include "stubAll.h"
#include <QFile>

class ut_Log : public ::testing::Test
    , public QObject
{
protected:
    ut_Log()
    {
    }

    virtual ~ut_Log()
    {
    }
    virtual void SetUp()
    {
        m_logPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "Log" + QDateTime::currentDateTime().toString();
        m_dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "Log";

        m_dir = QString("%1/%2/%3/Log/")
                    .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                    .arg(qApp->organizationName())
                    .arg(qApp->applicationName());
    }

    virtual void TearDown()
    {
        //   delete m_createTaskWidget;
    }
    QString m_logPath;
    QString m_dir;
};

TEST_F(ut_Log, setLogDir)
{
    setLogDir(m_dir);
    EXPECT_TRUE(true);
}

TEST_F(ut_Log, qInstallMessageHandler)
{
    qInstallMessageHandler(customLogMessageHandler);
    EXPECT_TRUE(true);
}

TEST_F(ut_Log, CheckLogTime)
{
    CheckLogTime();
    EXPECT_TRUE(true);

    Stub stub;
    stub.set(ADDR(QDateTime, daysTo), UrlThreadGet405);

    CheckLogTime();
}

TEST_F(ut_Log, CreateNewLog)
{
   // Stub stub;
   // stub.set((bool (QIODevice::*)(QIODevice::OpenMode))ADDR(QIODevice, open), QIODevice_open);
    CreateNewLog();
    EXPECT_TRUE(true);
}

TEST_F(ut_Log, CheckRotateSize)
{
    CheckRotateSize();
    EXPECT_TRUE(true);

    Stub stub;
    stub.set(ADDR(QDateTime, daysTo), UrlThreadGet405);

    CheckLogTime();
}

TEST_F(ut_Log, setLogLevel)
{
    setLogLevel(0);
    EXPECT_TRUE(true);
}

TEST_F(ut_Log, CheckFreeDisk)
{
    CheckFreeDisk();
    EXPECT_TRUE(true);
}
