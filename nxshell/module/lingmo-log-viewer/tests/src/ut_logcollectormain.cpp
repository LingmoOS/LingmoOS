// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logcollectormain.h"
#include "DebugTimeManager.h"
#include "ut_stuballthread.h"
#include "logapplication.h"
#include "utils.h"
#include "ut_stubpublic.h"
#include "logapplicationhelper.h"
#include <stub.h>

#include <DTitlebar>
#include <DWindowCloseButton>
#include <DDialog>
#include <QFileDialog>

#include <QKeyEvent>
#include <QDebug>
#include <QList>

#include <gtest/gtest.h>

void Log_beginPointLinux(const QString &point, const QString &status)
{
    Q_UNUSED(point);
    Q_UNUSED(status);
}

void LogCollectorMain_QThreadPool_start(QRunnable *runnable, int priority = 0)
{
    Q_UNUSED(runnable);
    Q_UNUSED(priority);
}

static bool LogCollectorMain_checkAuthorization_false(const QString &actionId, qint64 applicationPid)
{
    Q_UNUSED(actionId);
    Q_UNUSED(applicationPid);
    return false;
}

static bool LogCollectorMain_checkAuthorization_true(const QString &actionId, qint64 applicationPid)
{
    Q_UNUSED(actionId);
    Q_UNUSED(applicationPid);
    return true;
}

static QString LogCollectorMain_QString()
{
    return "true";
}

int LogCollectorMain_int()
{
    return 2;
}

int LogCollectorMain_int_exec()
{
    return 0;
}

bool LogCollectorMain_notify(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    Q_UNUSED(event);
    return false;
}

TEST(LogCollectorMain_Constructor_UT, LogCollectorMain_Constructor_UT)
{
    Stub stub;
    stub.set(ADDR(DebugTimeManager, beginPointLinux), Log_beginPointLinux);
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
    LogCollectorMain *p = new LogCollectorMain(nullptr);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}

TEST(LogCollectorMain_Destructor_UT, LogCollectorMain_Destructor_UT)
{
    Stub stub;
    stub.set(ADDR(DebugTimeManager, beginPointLinux), Log_beginPointLinux);
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
    LogCollectorMain *p = new LogCollectorMain(nullptr);
    EXPECT_NE(p, nullptr);
    p->~LogCollectorMain();
    EXPECT_EQ(p->m_searchEdt, nullptr);
    p->deleteLater();
}
TEST(LogCollectorMain_initUI_UT, LogCollectorMain_initUI_UT)
{
    Stub stub;
    stub.set(ADDR(DebugTimeManager, beginPointLinux), Log_beginPointLinux);
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
    LogCollectorMain *p = new LogCollectorMain(nullptr);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}

TEST(LogCollectorMain_initShortCut_UT, LogCollectorMain_initShortCut_UT_001)
{
    Stub stub;
    stub.set(ADDR(DebugTimeManager, beginPointLinux), Log_beginPointLinux);
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
    LogCollectorMain *p = new LogCollectorMain(nullptr);
    EXPECT_NE(p, nullptr);
    p->initShortCut();
    p->deleteLater();
}

TEST(LogCollectorMain_initShortCut_UT, LogCollectorMain_initShortCut_UT_002)
{
    Stub stub;
    stub.set(ADDR(DebugTimeManager, beginPointLinux), Log_beginPointLinux);
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
    LogCollectorMain *p = new LogCollectorMain(nullptr);
    EXPECT_NE(p, nullptr);
    p->m_scWndReize->deleteLater();
    p->m_scWndReize = nullptr;
    p->m_scFindFont->deleteLater();
    p->m_scFindFont = nullptr;
    p->m_scExport->deleteLater();
    p->m_scExport = nullptr;
    p->initShortCut();
    //    p->m_scWndReize->activated();
    //    p->m_scFindFont->activated();
    //    p->m_scExport->activated();
    p->deleteLater();
}

TEST(LogCollectorMain_initConnection_UT, LogCollectorMain_initConnection_UT)
{
    Stub stub;
    stub.set(ADDR(DebugTimeManager, beginPointLinux), Log_beginPointLinux);
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
    LogCollectorMain *p = new LogCollectorMain(nullptr);
    EXPECT_NE(p, nullptr);

    p->initConnection();
    p->deleteLater();
}
//TEST(LogCollectorMain_resizeWidthByFilterContentWidth_UT, LogCollectorMain_resizeWidthByFilterContentWidth_UT)
//{
//    Stub stub;
//    stub.set(ADDR(DebugTimeManager, beginPointLinux), Log_beginPointLinux);
//    stub.set(ADDR(LogFileParser, parseByJournal), LogFileParser_parseByJournal);
//    stub.set(ADDR(LogFileParser, parseByJournalBoot), LogFileParser_parseByJournalBoot);
//    stub.set(ADDR(LogFileParser, parseByDpkg), LogFileParser_parseByDpkg);
//    stub.set(ADDR(LogFileParser, parseByXlog), LogFileParser_parseByXlog);
//    stub.set(ADDR(LogFileParser, parseByBoot), LogFileParser_parseByBoot);
//    stub.set(ADDR(LogFileParser, parseByKern), LogFileParser_parseByKern);
//    stub.set(ADDR(LogFileParser, parseByApp), LogFileParser_parseByApp);
//    stub.set(ADDR(LogFileParser, parseByNormal), LogFileParser_parseByNormal);
//    stub.set(ADDR(LogFileParser, parseByKwin), LogFileParser_parseByKwin);
//    LogCollectorMain *p = new LogCollectorMain(nullptr);
//    EXPECT_NE(p, nullptr);
//    p->resizeWidthByFilterContentWidth(500);
//    p->deleteLater();
//}

class LogCollectorMain_handleApplicationTabEventNotify_UT_Param
{
public:
    LogCollectorMain_handleApplicationTabEventNotify_UT_Param(bool iKey, const QString &iObjectName, bool iResult, const QString &iFocusObjectName)
        : key(iKey)
        , objectName(iObjectName)
        , result(iResult)
        , focusObjectName(iFocusObjectName)
    {
    }
    int key;
    QString objectName;
    bool result;
    QString focusObjectName;
};

class LogCollectorMain_handleApplicationTabEventNotify_UT : public ::testing::TestWithParam<LogCollectorMain_handleApplicationTabEventNotify_UT_Param>
{
};

INSTANTIATE_TEST_CASE_P(LogCollectorMain, LogCollectorMain_handleApplicationTabEventNotify_UT, ::testing::Values(/*LogCollectorMain_handleApplicationTabEventNotify_UT_Param(Qt::Key_0, "", false, ""),
                                                                                                                  LogCollectorMain_handleApplicationTabEventNotify_UT_Param(Qt::Key_Tab, "", false, ""),
                                                                                                                                                                                                                                    */
                                                                                                                 LogCollectorMain_handleApplicationTabEventNotify_UT_Param(Qt::Key_Tab, "titlebar", true, ""), LogCollectorMain_handleApplicationTabEventNotify_UT_Param(Qt::Key_Tab, "searchChildEdt", false, "titlebar"), LogCollectorMain_handleApplicationTabEventNotify_UT_Param(Qt::Key_Tab, "DTitlebarDWindowCloseButton", false, "logTypeSelectList"), LogCollectorMain_handleApplicationTabEventNotify_UT_Param(Qt::Key_Tab, "mainLogTable", true, "searchChildEdt"), LogCollectorMain_handleApplicationTabEventNotify_UT_Param(Qt::Key_Backtab, "logTypeSelectList", true, "DTitlebarDWindowCloseButton"), LogCollectorMain_handleApplicationTabEventNotify_UT_Param(Qt::Key_Backtab, "DTitlebarDWindowOptionButton", true, "searchChildEdt"), LogCollectorMain_handleApplicationTabEventNotify_UT_Param(Qt::Key_Backtab, "searchChildEdt", true, "")
                                                                                                                 // LogCollectorMain_handleApplicationTabEventNotify_UT_Param(Qt::Key_Backtab, "", false, "")
                                                                                                                 ));
QString stub_ObjectName()
{
    return QString("logTypeSelectList");
}

TEST_P(LogCollectorMain_handleApplicationTabEventNotify_UT, LogCollectorMain_handleApplicationTabEventNotify_UT_001)
{
    Stub stub;
    stub.set(ADDR(DebugTimeManager, beginPointLinux), Log_beginPointLinux);
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
    LogCollectorMain_handleApplicationTabEventNotify_UT_Param param = GetParam();
    LogCollectorMain *p = new LogCollectorMain(nullptr);
    EXPECT_NE(p, nullptr);

    QKeyEvent keyEvent(QEvent::KeyPress, param.key, Qt::NoModifier);
    QWidget *w = nullptr;
    if (!param.objectName.isEmpty())
        w = p->titlebar()->findChild<QWidget *>(param.objectName);
    p->handleApplicationTabEventNotify(w, &keyEvent);
    p->deleteLater();
}

TEST_P(LogCollectorMain_handleApplicationTabEventNotify_UT, LogCollectorMain_handleApplicationTabEventNotify_UT_002)
{
    Stub stub;
    stub.set(ADDR(DebugTimeManager, beginPointLinux), Log_beginPointLinux);
    stub.set(ADDR(LogFileParser, parseByJournal), LogFileParser_parseByJournal);
    stub.set(ADDR(LogFileParser, parseByJournalBoot), LogFileParser_parseByJournalBoot);
    stub.set(ADDR(LogFileParser, parseByDpkg), LogFileParser_parseByDpkg);
    stub.set(ADDR(LogFileParser, parseByXlog), LogFileParser_parseByXlog);
    stub.set(ADDR(LogFileParser, parseByBoot), LogFileParser_parseByBoot);
    stub.set(ADDR(LogFileParser, parseByKern), LogFileParser_parseByKern);
    stub.set(ADDR(LogFileParser, parseByApp), LogFileParser_parseByApp);
    stub.set(ADDR(LogFileParser, parseByNormal), LogFileParser_parseByNormal);
    stub.set(ADDR(LogFileParser, parseByKwin), LogFileParser_parseByKwin);
    stub.set(ADDR(QObject, objectName), stub_ObjectName);
    stub.set(ADDR(LogApplicationHelper, getCustomLogList), LogApplicationHelper_getCustomLogList);
    LogCollectorMain_handleApplicationTabEventNotify_UT_Param param = GetParam();
    LogCollectorMain *p = new LogCollectorMain(nullptr);
    EXPECT_NE(p, nullptr);

    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    QWidget *w = nullptr;
    if (!param.objectName.isEmpty())
        w = p->titlebar()->findChild<QWidget *>(QString("logTypeSelectList"));
    p->handleApplicationTabEventNotify(w, &keyEvent);
    p->deleteLater();
}

TEST(LogCollectorMain_switchRefreshActionTriggered_UT, LogCollectorMain_switchRefreshActionTriggered_UT_001)
{
    Stub stub;
    stub.set(ADDR(DebugTimeManager, beginPointLinux), Log_beginPointLinux);
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
    typedef  bool (*fptr)(LogApplication *);
    fptr test = (fptr)(&LogApplication::notify);
    stub.set(test, LogCollectorMain_notify);
    LogCollectorMain *p = new LogCollectorMain(nullptr);
    ASSERT_TRUE(p);
    QAction *action = new QAction(qApp->translate("titlebar", "5 min"));
    stub.set(ADDR(QList<QAction *>, indexOf), LogCollectorMain_int);
    p->switchRefreshActionTriggered(action);
    if (p->m_refreshTimer && p->m_refreshTimer->isActive()) {
        p->m_refreshTimer->stop();
    }
    delete action;
    p->deleteLater();
}

TEST(LogCollectorMain_exportAllLogs_UT, LogCollectorMain_exportAllLogs_UT_001)
{
    Stub stub;
    stub.set(ADDR(DebugTimeManager, beginPointLinux), Log_beginPointLinux);
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
    typedef  bool (*fptr)(LogApplication *);
    fptr test = (fptr)(&LogApplication::notify);
    stub.set(test, LogCollectorMain_notify);
    LogCollectorMain *p = new LogCollectorMain(nullptr);
    ASSERT_TRUE(p);
    stub.set(ADDR(Utils, checkAuthorization), LogCollectorMain_checkAuthorization_false);
    p->exportAllLogs();
    p->deleteLater();
}

TEST(LogCollectorMain_exportAllLogs_UT, LogCollectorMain_exportAllLogs_UT_002)
{
    Stub stub;
    stub.set(ADDR(DebugTimeManager, beginPointLinux), Log_beginPointLinux);
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
    typedef  bool (*fptr)(LogApplication *);
    fptr test = (fptr)(&LogApplication::notify);
    stub.set(test, LogCollectorMain_notify);
    LogCollectorMain *p = new LogCollectorMain(nullptr);
    ASSERT_TRUE(p);
    stub.set(ADDR(Utils, checkAuthorization), LogCollectorMain_checkAuthorization_true);
    stub.set(ADDR(QFileDialog, getSaveFileName), LogCollectorMain_QString);
//    stub.set(ADDR(QThreadPool, start), LogCollectorMain_QThreadPool_start);
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), LogCollectorMain_QThreadPool_start);

    typedef  int (*fptr2)(DDialog *);
    fptr2 test1 = (fptr2)(&DDialog::exec);
    stub.set(test1, LogCollectorMain_int_exec);
    p->exportAllLogs();
    p->deleteLater();
}

TEST(LogCollectorMain_closeEvent_UT, LogCollectorMain_closeEvent_UT_001)
{
    Stub stub;
    stub.set(ADDR(DebugTimeManager, beginPointLinux), Log_beginPointLinux);
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
    typedef  bool (*fptr)(LogApplication *);
    fptr test = (fptr)(&LogApplication::notify);
    stub.set(test, LogCollectorMain_notify);
    LogCollectorMain *p = new LogCollectorMain(nullptr);
    ASSERT_TRUE(p);
    QCloseEvent *event = new QCloseEvent();
    ASSERT_TRUE(event);
    p->closeEvent(event);
    delete event;
    p->deleteLater();
}


