// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logfileparser.h"
#include "ut_stuballthread.h"
#include "logauththread.h"
#include "structdef.h"
#include "sharedmemorymanager.h"
#include "wtmpparse.h"

#include <stub.h>

#include <QDebug>
#include <QMessageBox>
#include <QThreadPool>
#include <QDateTime>

#include <gtest/gtest.h>

bool stub_isAttached001()
{
    return true;
}

bool stub_Logexists001()
{
    return true;
}

void stub_Logstart001(const QString &program, const QStringList &arguments, QIODevice::OpenMode mode)
{
}

bool stub_LogwaitForFinished001(int msecs)
{
    return true;
}

void stub_LogsetRunnableTag001(ShareMemoryInfo iShareInfo)
{
}

QByteArray stub_LogreadAllStandardOutput001()
{
    return "2020-11-24 01:57:24 startup archives install \n2020-11-24 01:57:24 install base-passwd:amd64 <none> 3.5.46\n            2021-01-09, 17:04:10.721 [Debug  ] [                                                         0] onTermGetFocus 2";
}

QByteArray stub_LogreadAllStandardError001()
{
    return "noerror";
}

void stub_wtmp_close001(void)
{
}

QString stub_toString001(QStringView format)
{
    Q_UNUSED(format);
    return "20190120";
}

void stub_start001(QRunnable *runnable, int priority = 0)
{
    Q_UNUSED(runnable);
    Q_UNUSED(priority);
}

TEST(LogFileParser_Constructor_UT, LogFileParser_Constructor_UT)
{
    Stub stub;
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_start001);
    LogFileParser *p = new LogFileParser(nullptr);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}

TEST(LogFileParser_Destructor_UT, LogFileParser_Destructor_UT)
{
    Stub stub;
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_start001);
    LogFileParser *p = new LogFileParser(nullptr);
    EXPECT_NE(p, nullptr);
    p->~LogFileParser();
    p->deleteLater();
}

//TEST(LogFileParser_parseByJournalBoot_UT, LogFileParser_parseByJournalBoot_UT)
//{
//    Stub stub;
//    stub.set(ADDR(QThreadPool, start), QThreadPool_start);
//    stub.set(ADDR(QThread, start), QThread_start);
//    LogFileParser *p = new LogFileParser(nullptr);
//    EXPECT_NE(p, nullptr);
//    p->parseByJournalBoot(QStringList() << "test");
//    p->deleteLater();
//}


TEST(LogFileParser_stopAllLoad_UT, LogFileParser_stopAllLoad_UT)
{
    Stub stub;
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_start001);
    LogFileParser *p = new LogFileParser(nullptr);
    EXPECT_NE(p, nullptr);
    p->stopAllLoad();
    p->deleteLater();
}

TEST(LogFileParser_quitLogAuththread_UT, LogFileParser_quitLogAuththread_UT_001)
{
    Stub stub;
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_start001);
    LogFileParser *p = new LogFileParser(nullptr);
    EXPECT_NE(p, nullptr);
    p->quitLogAuththread(nullptr);
    p->deleteLater();
}

TEST(LogFileParser_quitLogAuththread_UT, LogFileParser_quitLogAuththread_UT_002)
{
    Stub stub;
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_start001);
    LogFileParser *p = new LogFileParser(nullptr);
    EXPECT_NE(p, nullptr);
    QThread *thread = new QThread;
    thread->start();
    p->quitLogAuththread(thread);
    EXPECT_EQ(thread->isRunning(), false);
    thread->deleteLater();
    p->deleteLater();
}

QMessageBox::StandardButton LogFileParser_slog_proccessError_UT_DMessageBox_information(QWidget *parent, const QString &title,
                                                                                        const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                                                                        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton)
{
    qDebug() << "info-------";
    return QMessageBox::NoButton;
}
TEST(LogFileParser_slog_proccessError_UT, LogFileParser_slog_proccessError_UT)
{
    LogFileParser *p = new LogFileParser(nullptr);
    EXPECT_NE(p, nullptr);
    typedef QMessageBox::StandardButton (*fptr)(QWidget *,
                                                const QString &, const QString &, QMessageBox::StandardButtons,
                                                QMessageBox::StandardButton);
    fptr p_func = (fptr)(&QMessageBox::information);
    Stub stub;
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_start001);
    stub.set(p_func, LogFileParser_slog_proccessError_UT_DMessageBox_information);
    p->slog_proccessError(QString("testError"));
    p->deleteLater();
}

void stubfileparser_setProcessChannelMode(QProcess::ProcessChannelMode mode)
{
}

int stubfileparser_exitCode()
{
    return 0;
}

QStringList stubfileparser_getFileInfo(const QString &flag)
{
    return QStringList() << "kern.log";
}

QString stub_FilereadLog(const QString &filePath)
{
    Q_UNUSED(filePath);
    return "2021-04-06 13:29:32 install code:amd64 <none> 1.55.0-1617120720";
}

class LogFileParser_UT : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_parser = new LogFileParser();
        qDebug() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_parser;
    }
    LogFileParser *m_parser;
};

//TEST_F(LogFileParser_UT, sLogFileParser_UT001)
//{
//    Stub stub;
//    typedef bool (QFile::*fptr)() const;
//    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
//    stub.set(A_foo, stub_Logexists001);
//    stub.set(ADDR(QProcess, setProcessChannelMode), stubfileparser_setProcessChannelMode);
//    stub.set(ADDR(QProcess, exitCode), stubfileparser_exitCode);
//    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached001);
//    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart001);
//    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString001);
//    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished001);
//    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput001);
//    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError001);
//    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag001);
//    stub.set(ADDR(DLDBusHandler, getFileInfo), stubfileparser_getFileInfo);
//    stub.set(wtmp_close, stub_wtmp_close001);
//    stub.set(ADDR(DLDBusHandler, readLog), stub_FilereadLog);
//    struct KWIN_FILTERS fitler = {"test"};
//    stub.set(ADDR(QThreadPool, start), stub_start001);
//    int index=m_parser->parseByKwin(fitler);
//    EXPECT_EQ(index,2)<<"check the status after LogFileParser()";
//    struct KERN_FILTERS kern_fitler;
//    int index1=m_parser->parseByKern(kern_fitler);
//    EXPECT_EQ(index1,3)<<"check the status after LogFileParser()";
//    struct DKPG_FILTERS dkpg_fitler;
//    int index2=m_parser->parseByDpkg(dkpg_fitler);
//    EXPECT_EQ(index2,4)<<"check the status after LogFileParser()";    EXPECT_EQ(index,2)<<"check the status after LogFileParser()";
//    struct BOOT_FILTERS boot_fitler;
//    int index3=m_parser->parseByBoot();
//    EXPECT_EQ(index3,5)<<"check the status after LogFileParser()";
//}

//TEST_F(LogFileParser_UT, UT_ParseByKwin){
//    Stub stub;
//    typedef bool (QFile::*fptr)() const;
//    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
//    stub.set(A_foo, stub_Logexists001);
//    stub.set(ADDR(QProcess, setProcessChannelMode), stubfileparser_setProcessChannelMode);
//    stub.set(ADDR(QProcess, exitCode), stubfileparser_exitCode);
//    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached001);
//    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart001);
//    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString001);
//    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished001);
//    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput001);
//    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError001);
//    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag001);
//    stub.set(wtmp_close, stub_wtmp_close001);
//    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_start001);
//    struct KWIN_FILTERS fitler = {"test"};
//    int index=m_parser->parseByKwin(fitler);
//    EXPECT_EQ(index,2)<<"check the status after LogFileParser()";
//}

//TEST_F(LogFileParser_UT, UT_ParseByKern){
//    Stub stub;
//    typedef bool (QFile::*fptr)() const;
//    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
//    stub.set(A_foo, stub_Logexists001);
//    stub.set(ADDR(QProcess, setProcessChannelMode), stubfileparser_setProcessChannelMode);
//    stub.set(ADDR(QProcess, exitCode), stubfileparser_exitCode);
//    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached001);
//    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart001);
//    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString001);
//    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished001);
//    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput001);
//    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError001);
//    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag001);
//    stub.set(ADDR(DLDBusHandler, getFileInfo), stubfileparser_getFileInfo);
//    stub.set(wtmp_close, stub_wtmp_close001);
//    stub.set(ADDR(DLDBusHandler, readLog), stub_FilereadLog);
//    stub.set(ADDR(QThreadPool, start), stub_start001);
//    struct DKPG_FILTERS dkpg_fitler;
//    int index2=m_parser->parseByDpkg(dkpg_fitler);
//    EXPECT_EQ(index2,3)<<"check the status after LogFileParser()";
//}


//TEST_F(LogFileParser_UT, UT_ParseByBoot){
//    Stub stub;
//    typedef bool (QFile::*fptr)() const;
//    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
//    stub.set(A_foo, stub_Logexists001);
//    stub.set(ADDR(QProcess, setProcessChannelMode), stubfileparser_setProcessChannelMode);
//    stub.set(ADDR(QProcess, exitCode), stubfileparser_exitCode);
//    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached001);
//    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart001);
//    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString001);
//    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished001);
//    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput001);
//    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError001);
//    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag001);
//    stub.set(ADDR(DLDBusHandler, getFileInfo), stubfileparser_getFileInfo);
//    stub.set(wtmp_close, stub_wtmp_close001);
//    stub.set(ADDR(DLDBusHandler, readLog), stub_FilereadLog);
//    stub.set(ADDR(QThreadPool, start), stub_start001);
//    struct BOOT_FILTERS boot_fitler;
//    int index3=m_parser->parseByBoot();
//    EXPECT_EQ(index3,4)<<"check the status after LogFileParser()";
//}

//TEST_F(LogFileParser_UT, UT_ParseByDpkg){
//    Stub stub;
//    typedef bool (QFile::*fptr)() const;
//    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
//    stub.set(A_foo, stub_Logexists001);
//    stub.set(ADDR(QProcess, setProcessChannelMode), stubfileparser_setProcessChannelMode);
//    stub.set(ADDR(QProcess, exitCode), stubfileparser_exitCode);
//    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached001);
//    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart001);
//    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString001);
//    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished001);
//    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput001);
//    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError001);
//    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag001);
//    stub.set(wtmp_close, stub_wtmp_close001);
//    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_start001);
//    struct DKPG_FILTERS dpkg_fitler;
//    int index1=m_parser->parseByDpkg(dpkg_fitler);
////    EXPECT_EQ(index1,5)<<"check the status after LogFileParser()";
//}


//TEST_F(LogFileParser_UT, UT_ParseByDnf)
//{
//    Stub stub;
//    typedef bool (QFile::*fptr)() const;
//    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
//    stub.set(A_foo, stub_Logexists001);
//    stub.set(ADDR(QProcess, setProcessChannelMode), stubfileparser_setProcessChannelMode);
//    stub.set(ADDR(QProcess, exitCode), stubfileparser_exitCode);
//    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_start001);
//    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached001);
//    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart001);
//    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString001);
//    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished001);
//    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput001);
//    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError001);
//    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag001);
//    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_start001);
//    stub.set(wtmp_close, stub_wtmp_close001);
//    DNF_FILTERS dnfFilter = {10000, DNFPRIORITY::DEBUG};
//    m_parser->parseByDnf(dnfFilter);
//    EXPECT_NE(m_parser,nullptr);
//}

//TEST_F(LogFileParser_UT, UT_ParseByXorg)
//{
//    Stub stub;
//    typedef bool (QFile::*fptr)() const;
//    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
//    stub.set(A_foo, stub_Logexists001);
//    stub.set(ADDR(QProcess, setProcessChannelMode), stubfileparser_setProcessChannelMode);
//    stub.set(ADDR(QProcess, exitCode), stubfileparser_exitCode);

//    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached001);
//    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart001);
//    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString001);
//    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished001);
//    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput001);
//    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError001);
//    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag001);
//    stub.set(ADDR(DLDBusHandler, getFileInfo), stubfileparser_getFileInfo);
//    stub.set(ADDR(QThreadPool, start), stub_start001);
//    stub.set(wtmp_close, stub_wtmp_close001);
//    stub.set(ADDR(DLDBusHandler, readLog), stub_FilereadLog);
//    XORG_FILTERS xorgFilter;
//    m_parser->parseByXlog(xorgFilter);
//    EXPECT_NE(m_parser,nullptr);
//}

//TEST_F(LogFileParser_UT, UT_ParseByNormal)
//{
//    Stub stub;
//    typedef bool (QFile::*fptr)() const;
//    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
//    stub.set(A_foo, stub_Logexists001);
//    stub.set(ADDR(QProcess, setProcessChannelMode), stubfileparser_setProcessChannelMode);
//    stub.set(ADDR(QProcess, exitCode), stubfileparser_exitCode);

//    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached001);
//    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart001);
//    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString001);
//    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished001);
//    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput001);
//    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError001);
//    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag001);
//    stub.set(ADDR(DLDBusHandler, getFileInfo), stubfileparser_getFileInfo);
//    stub.set(ADDR(QThreadPool, start), stub_start001);
//    stub.set(wtmp_close, stub_wtmp_close001);
//    stub.set(ADDR(DLDBusHandler, readLog), stub_FilereadLog);
//    NORMAL_FILTERS normalFilter;
//    m_parser->parseByNormal(normalFilter);
//    EXPECT_NE(m_parser,nullptr);
//}

//TEST_F(LogFileParser_UT, UT_ParseByApp)
//{
//    Stub stub;
//    typedef bool (QFile::*fptr)() const;
//    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
//    stub.set(A_foo, stub_Logexists001);
//    stub.set(ADDR(QProcess, setProcessChannelMode), stubfileparser_setProcessChannelMode);
//    stub.set(ADDR(QProcess, exitCode), stubfileparser_exitCode);

//    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached001);
//    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart001);
//    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString001);
//    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished001);
//    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput001);
//    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError001);
//    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag001);
//    stub.set(ADDR(DLDBusHandler, getFileInfo), stubfileparser_getFileInfo);
//    stub.set(ADDR(QThreadPool, start), stub_start001);
//    stub.set(wtmp_close, stub_wtmp_close001);
//    stub.set(ADDR(DLDBusHandler, readLog), stub_FilereadLog);
//    APP_FILTERS appFilter;
//    appFilter.path="test";
//    appFilter.lvlFilter=1;
//    m_parser->parseByApp(appFilter);
//    EXPECT_NE(m_parser,nullptr);
//}

TEST_F(LogFileParser_UT, UT_ParseByJournal_001)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_Logexists001);
    stub.set(ADDR(QProcess, setProcessChannelMode), stubfileparser_setProcessChannelMode);
    stub.set(ADDR(QProcess, exitCode), stubfileparser_exitCode);
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_start001);
    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached001);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart001);
    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString001);
    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished001);
    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput001);
    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError001);
    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag001);
    stub.set(wtmp_close, stub_wtmp_close001);
    m_parser->parseByJournal();
}

TEST_F(LogFileParser_UT, UT_parseByJournalBoot_001)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_Logexists001);
    stub.set(ADDR(QProcess, setProcessChannelMode), stubfileparser_setProcessChannelMode);
    stub.set(ADDR(QProcess, exitCode), stubfileparser_exitCode);
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_start001);
    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached001);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart001);
    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString001);
    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished001);
    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput001);
    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError001);
    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag001);
    stub.set(wtmp_close, stub_wtmp_close001);
    int index=m_parser->parseByJournalBoot();
    qInfo()<<index;
}

TEST_F(LogFileParser_UT, UT_parseByOOC_001)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_Logexists001);
    stub.set(ADDR(QProcess, setProcessChannelMode), stubfileparser_setProcessChannelMode);
    stub.set(ADDR(QProcess, exitCode), stubfileparser_exitCode);
    stub.set(ADDR(QThread, start), QThread_start);
    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached001);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart001);
    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString001);
    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished001);
    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput001);
    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError001);
    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag001);
    stub.set(wtmp_close, stub_wtmp_close001);
    QString path("test");
    m_parser->parseByOOC(path);
}
