// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logapplication.h"
#include "logcollectormain.h"
#include "DebugTimeManager.h"
#include "ut_stuballthread.h"
#include "logapplicationhelper.h"
#include "ut_stubpublic.h"
#include <stub.h>

#include <QDebug>
#include <QKeyEvent>

#include <gtest/gtest.h>

TEST(LogApplication_Constructor_UT, LogApplication_Constructor_UT)
{
    int argc = 0;
    char **argv;
    LogApplication *p = new LogApplication(argc, argv);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}
void stub_beginPointLinux(const QString &point, const QString &status)
{
}
TEST(LogApplication_setMainWindow_UT, LogApplication_setMainWindow_UT)
{
    int argc = 0;
    char **argv;
    Stub stub;
    stub.set(ADDR(DebugTimeManager, beginPointLinux), stub_beginPointLinux);
    stub.set(ADDR(LogFileParser, parseByJournal), LogFileParser_parseByJournal);
    stub.set(ADDR(LogFileParser, parseByJournalBoot), LogFileParser_parseByJournalBoot);
    stub.set(ADDR(LogFileParser, parseByDpkg), LogFileParser_parseByDpkg);
    stub.set(ADDR(LogFileParser, parseByXlog), LogFileParser_parseByXlog);
    stub.set(ADDR(LogFileParser, parseByBoot), LogFileParser_parseByBoot);
    stub.set(ADDR(LogFileParser, parseByKern), LogFileParser_parseByKern);
    stub.set(ADDR(LogFileParser, parseByApp), LogFileParser_parseByApp);
    stub.set(ADDR(LogFileParser, parseByNormal), LogFileParser_parseByNormal);
    stub.set(ADDR(LogFileParser, parseByKwin), LogFileParser_parseByKwin);
    stub.set(ADDR(LogApplicationHelper, getCustomLogList), LogApplicationHelper_getCustomLogList);
    LogApplication *p = new LogApplication(argc, argv);
    EXPECT_NE(p, nullptr);
    LogCollectorMain *m = new LogCollectorMain(nullptr);
    p->setMainWindow(m);
    EXPECT_EQ(m, p->m_mainWindow);
    p->deleteLater();
}

class LogApplication_notify_UT_Param
{
public:
    LogApplication_notify_UT_Param(bool iIsKeyPress, bool iIsKeyTab, bool iIsMainWindowb, bool iIsSucces)
    {
        isKeyPress = iIsKeyPress;
        isKeyTab = iIsKeyTab;
        isMainWindow = iIsMainWindowb;
        isSucces = iIsSucces;
    }
    bool isKeyPress;
    bool isKeyTab;
    bool isMainWindow;
    bool isSucces;
};

class LogApplication_notify_UT : public ::testing::TestWithParam<LogApplication_notify_UT_Param>
{
};

static bool LogApplication_notify_mainWindow_handleApplicationTabEventNotify_result = false;
std::vector<LogApplication_notify_UT_Param> vec1 {LogApplication_notify_UT_Param(false, true, true, true),
                                                  LogApplication_notify_UT_Param(true, true, true, true),
                                                  LogApplication_notify_UT_Param(true, false, true, true),
                                                  LogApplication_notify_UT_Param(true, true, false, true),
                                                  LogApplication_notify_UT_Param(true, true, true, false)};

INSTANTIATE_TEST_CASE_P(LogApplication, LogApplication_notify_UT, ::testing::ValuesIn(vec1));
bool LogApplication_notify_mainWindow_handleApplicationTabEventNotify_result_Func(void *obj, QObject *obj1, QKeyEvent *evt)
{
    return LogApplication_notify_mainWindow_handleApplicationTabEventNotify_result;
}

TEST_P(LogApplication_notify_UT, LogApplication_notify_UT)
{
    int argc = 0;
    char **argv;
    Stub stub;
    stub.set(ADDR(LogFileParser, parseByJournal), LogFileParser_parseByJournal);
    stub.set(ADDR(LogFileParser, parseByJournalBoot), LogFileParser_parseByJournalBoot);
    stub.set(ADDR(LogFileParser, parseByDpkg), LogFileParser_parseByDpkg);
    stub.set(ADDR(LogFileParser, parseByXlog), LogFileParser_parseByXlog);
    stub.set(ADDR(LogFileParser, parseByBoot), LogFileParser_parseByBoot);
    stub.set(ADDR(LogFileParser, parseByKern), LogFileParser_parseByKern);
    stub.set(ADDR(LogFileParser, parseByApp), LogFileParser_parseByApp);
    stub.set(ADDR(LogFileParser, parseByNormal), LogFileParser_parseByNormal);
    stub.set(ADDR(LogFileParser, parseByKwin), LogFileParser_parseByKwin);
    stub.set(ADDR(LogCollectorMain, handleApplicationTabEventNotify), LogApplication_notify_mainWindow_handleApplicationTabEventNotify_result_Func);

    LogApplication *p = new LogApplication(argc, argv);
    EXPECT_NE(p, nullptr);
    LogApplication_notify_UT_Param param = GetParam();
    if (param.isKeyPress) {
        int key = param.isKeyTab ? Qt::Key_Tab : Qt::Key_Left;
        QKeyEvent keyEvent(QEvent::KeyPress, key, Qt::NoModifier);
        LogApplication_notify_mainWindow_handleApplicationTabEventNotify_result = param.isSucces;
        p->m_mainWindow = param.isMainWindow ? p->m_mainWindow : nullptr;
        p->notify(nullptr, &keyEvent);
    } else {
        QEvent *e = new QEvent(QEvent::None);
        p->notify(nullptr, e);
        delete e;
    }

    p->deleteLater();
}
