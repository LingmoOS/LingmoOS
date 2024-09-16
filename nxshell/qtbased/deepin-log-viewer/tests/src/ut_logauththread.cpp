// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logauththread.h"
#include "structdef.h"
#include "sharedmemorymanager.h"
#include "wtmpparse.h"
#include "dldbushandler.h"

#include <stub.h>

#include <QDebug>
#include <QDateTime>

#include <gtest/gtest.h>
bool stub_isAttached()
{
    return true;
}

bool stub_Logexists()
{
    return true;
}

void stub_Logstart(const QString &program, const QStringList &arguments, QIODevice::OpenMode mode)
{
    Q_UNUSED(program);
    Q_UNUSED(arguments);
    Q_UNUSED(mode);
}

bool stub_LogwaitForFinished(int msecs)
{
    Q_UNUSED(msecs);
    return true;
}

void stub_LogsetRunnableTag(ShareMemoryInfo iShareInfo)
{
    Q_UNUSED(iShareInfo);
}

QByteArray stub_LogreadAllStandardOutput()
{
    return "2020-11-24 01:57:24 startup archives install \n2020-11-24 01:57:24 install base-passwd:amd64 <none> 3.5.46\n            2021-01-09, 17:04:10.721 [Debug  ] [                                                         0] onTermGetFocus 2";
}

QByteArray dmesgLogreadAllStandardOutput()
{
    return "[101619.805280] snd_hda_codec_hdmi hdaudioC1D0: hda_codec_cleanup_stream: NID=0x8";
}

QByteArray stub_LogreadAllStandardError()
{
    return "noerror";
}

void stub_wtmp_close(void)
{
}

QString stub_toString(QStringView format)
{
    Q_UNUSED(format);
    return "20190120";
}

void stub_setProcessChannelMode(QProcess::ProcessChannelMode mode)
{
    Q_UNUSED(mode);
}

int stub_exitCode()
{
    return 0;
}

QString stub_readLog(const QString &filePath)
{
    Q_UNUSED(filePath);
    return "2021-04-06 13:29:32 install code:amd64 <none> 1.55.0-1617120720";
}

QString stub_xorgReadLog(const QString &filePath)
{
    Q_UNUSED(filePath);
    return "[  9.576 ] (II) Loading /usr/lib/xorg/modules/drivers/fbdev_drv.so";
}

QString stub_dnfReadLog(const QString &filePath)
{
    Q_UNUSED(filePath);
    return "2021-05-21T02:08:36Z DEBUG 加载插件：builddep, changelog, \nconfig-manager, copr, debug, debuginfo-install, download, \ngenerate_completion_cache, needs-restarting, playground, repoclosure, repodiff, repograph, repomanage, reposync";
}

QString stub_dnfReadStream(const QString &filePath)
{
    Q_UNUSED(filePath);
    static int i = 0;
    QString result;
    if(i % 2 == 0) {
        result = "2021-05-21T02:08:36Z DEBUG 加载插件：builddep, changelog, \nconfig-manager, copr, debug, debuginfo-install, download, \ngenerate_completion_cache, needs-restarting, playground, repoclosure, repodiff, repograph, repomanage, reposync";
    } else {
        result = "";
    }
    ++i;
    return result;
}

QString stub_BootReadLog(const QString &filePath)
{
    Q_UNUSED(filePath);
    return "[   10.336791] rc.local[1655]: net.ipv4.tcp_max_syn_backlog = 2048";
}

QString stub_KernReadLog(const QString &filePath)
{
    Q_UNUSED(filePath);
    return "Aug 21 13:23:33 uos-PC deepin-deepinid-daemon[2946]: #033[33m[WAR] manager.go:218:#033[0m skip doSync";
}

QByteArray fileReadLine(qint64 maxlen = 0)
{
    Q_UNUSED(maxlen);
    return "211111";
}

qint64 dnfToMSecsSinceEpoch()
{
    return 1999999999999999999;
}

int dmesgIndexIn(const QString &str, int offset = 0, QRegExp::CaretMode caretMode = QRegExp::CaretAtZero)
{
    Q_UNUSED(str);
    Q_UNUSED(offset);
    Q_UNUSED(caretMode);
    return 1;
}

QString &dmesgReplace(const QRegExp &rx, const QString &after)
{
    Q_UNUSED(rx);
    Q_UNUSED(after);
    static QString str = "[ 101619.805280 ] snd_hda_codec_hdmi hdaudioC1D0: hda_codec_cleanup_stream: NID=0x8";
    return str;
}

QStringList dmesgCapturedTexts()
{
    return QStringList() << "[ 101619.805280"
                         << "snd_hda_codec_"
                         << "ddd"
                         << "5555555"
                         << "test"
                         << "NID=0x8";
}

void handleFile()
{
}

class LogAuthThread_UT : public testing::Test
{
public:
    //添加日志
    static void SetUpTestCase()
    {
        qDebug() << "SetUpTestCase" << endl;
    }
    static void TearDownTestCase()
    {
        qDebug() << "TearDownTestCase" << endl;
    }
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_logAuthThread = new LogAuthThread();
        qDebug() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
    }
    LogAuthThread *m_logAuthThread;
};

//TEST_F(LogAuthThread_UT, LogAuthThread_UT001)
//{
//    Stub stub;
//    typedef bool (QFile::*fptr)() const;
//    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
//    stub.set(A_foo, stub_Logexists);

//    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached);
//    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart);
//    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString);
//    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished);
//    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput);
//    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError);
//    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag);
//    stub.set(wtmp_close, stub_wtmp_close);
//    stub.set(ADDR(QProcess, setProcessChannelMode), stub_setProcessChannelMode);
//    stub.set(ADDR(QProcess, exitCode), stub_exitCode);
//    stub.set(ADDR(DLDBusHandler, readLog), stub_readLog);

//    m_logAuthThread->m_process.reset(new QProcess);
//    m_logAuthThread->m_isStopProccess = true;
//    m_logAuthThread->m_type = LOG_FLAG::KERN;
//    m_logAuthThread->m_FilePath = QStringList() << "/var/log/kern.log";
//    m_logAuthThread->run();
//    m_logAuthThread->m_canRun = true;
//    m_logAuthThread->m_type = LOG_FLAG::BOOT;
//    m_logAuthThread->m_FilePath = QStringList() << "/var/log/boot.log";
//    m_logAuthThread->handleBoot();
//    m_logAuthThread->m_FilePath = QStringList() << "/test";
//    m_logAuthThread->handleBoot();
//    m_logAuthThread->m_type = LOG_FLAG::DPKG;
//    m_logAuthThread->m_FilePath = QStringList() << "/var/log/dpkg.log";
//    m_logAuthThread->handleDkpg();
//    m_logAuthThread->m_FilePath = QStringList() << "/test";
//    m_logAuthThread->handleDkpg();
//    m_logAuthThread->m_type = LOG_FLAG::Normal;
//    m_logAuthThread->handleNormal();
//    m_logAuthThread->m_type = LOG_FLAG::Kwin;
//    m_logAuthThread->handleKwin();




//    m_logAuthThread->thread_count = 1;
//    EXPECT_EQ(, 1);
//}

TEST_F(LogAuthThread_UT, UT_GetIndex_001){
     int index=m_logAuthThread->getIndex();
     EXPECT_EQ(index,1);
}

TEST_F(LogAuthThread_UT, UT_FormatDateTime_001){
   qint64 timeMesc= m_logAuthThread->formatDateTime("2021", "08-30", "14:25");
   qint64 timeMesc1= m_logAuthThread->formatDateTime("Aug 30" ,"19:04:43");
   EXPECT_NE(timeMesc,-1);
   EXPECT_NE(timeMesc1,-1);
}

TEST_F(LogAuthThread_UT, UT_SetFileterParam_001){
    KWIN_FILTERS kwin;
    m_logAuthThread->setFileterParam(kwin);
    EXPECT_EQ(m_logAuthThread->m_kwinFilters.msg,kwin.msg);
    KERN_FILTERS kern;
    m_logAuthThread->setFileterParam(kern);
    EXPECT_EQ(m_logAuthThread->m_kernFilters.timeFilterEnd,kern.timeFilterEnd);
    DKPG_FILTERS dpkg;
    m_logAuthThread->setFileterParam(dpkg);
    EXPECT_EQ(m_logAuthThread->m_dkpgFilters.timeFilterEnd,dpkg.timeFilterEnd);
    XORG_FILTERS xorg;
    m_logAuthThread->setFileterParam(xorg);
    EXPECT_EQ(m_logAuthThread->m_xorgFilters.timeFilterEnd,xorg.timeFilterEnd);
    NORMAL_FILTERS normal;
    m_logAuthThread->setFileterParam(normal);
    EXPECT_EQ(m_logAuthThread->m_normalFilters.timeFilterEnd,normal.timeFilterEnd);
}

TEST_F(LogAuthThread_UT, UT_handleXorg_001)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_Logexists);

    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart);
    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString);
    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput);
    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError);
    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag);
    stub.set(wtmp_close, stub_wtmp_close);
    stub.set(ADDR(QProcess, setProcessChannelMode), stub_setProcessChannelMode);
    stub.set(ADDR(QProcess, exitCode), stub_exitCode);
    stub.set(ADDR(DLDBusHandler, readLog), stub_xorgReadLog);
    stub.set((QByteArray(QIODevice::*)(qint64))ADDR(QIODevice, readLine), fileReadLine);
    m_logAuthThread->m_FilePath = QStringList() << "/test" << "/xorg.old";
    m_logAuthThread->m_canRun=true;
    m_logAuthThread->handleXorg();
    EXPECT_NE(m_logAuthThread,nullptr);
    EXPECT_EQ(m_logAuthThread->m_canRun,true);
}

TEST_F(LogAuthThread_UT, UT_HandleDnf_001)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_Logexists);

    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart);
    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString);
    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput);
    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError);
    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag);
    stub.set(wtmp_close, stub_wtmp_close);
    stub.set(ADDR(QProcess, setProcessChannelMode), stub_setProcessChannelMode);
    stub.set(ADDR(QProcess, exitCode), stub_exitCode);
    stub.set(ADDR(DLDBusHandler, readLog), stub_dnfReadLog);
    stub.set((QByteArray(QIODevice::*)(qint64))ADDR(QIODevice, readLine), fileReadLine);
    stub.set(ADDR(QDateTime, toMSecsSinceEpoch), dnfToMSecsSinceEpoch);
    m_logAuthThread->m_FilePath = QStringList() << "/test"
                                                << "/xorg.old";
    m_logAuthThread->m_dnfFilters.levelfilter = DNFPRIORITY::DNFLVALL;
    m_logAuthThread->m_canRun=true;
    m_logAuthThread->handleDnf();
    EXPECT_NE(m_logAuthThread,nullptr);
    EXPECT_EQ(m_logAuthThread->m_canRun,true);
}

TEST_F(LogAuthThread_UT, UT_HandleKern_001)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_Logexists);

    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart);
    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString);
    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), dmesgLogreadAllStandardOutput);
    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError);
    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag);
    stub.set(wtmp_close, stub_wtmp_close);
    stub.set(ADDR(QProcess, setProcessChannelMode), stub_setProcessChannelMode);
    stub.set(ADDR(QProcess, exitCode), stub_exitCode);
    stub.set(ADDR(DLDBusHandler, openLogStream), stub_dnfReadLog);
    stub.set(ADDR(DLDBusHandler, readLogInStream), stub_dnfReadStream);
    stub.set((QByteArray(QIODevice::*)(qint64))ADDR(QIODevice, readLine), fileReadLine);
    stub.set(ADDR(QDateTime, toMSecsSinceEpoch), dnfToMSecsSinceEpoch);
    stub.set(ADDR(QRegExp, indexIn), dmesgIndexIn);
    stub.set((QString & (QString::*)(const QRegExp &, const QString &)) ADDR(QString, replace), dmesgReplace);
    stub.set((QStringList(QRegExp::*)())ADDR(QRegExp, capturedTexts), dmesgCapturedTexts);
    m_logAuthThread->m_FilePath = QStringList() << "/test" << "/xorg.old";
    m_logAuthThread->m_canRun=true;
    m_logAuthThread->handleKern();
    EXPECT_NE(m_logAuthThread,nullptr);
    EXPECT_EQ(m_logAuthThread->m_canRun,true);
}



TEST_F(LogAuthThread_UT, UT_HandleDmesg_001)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_Logexists);

    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart);
    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString);
    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), dmesgLogreadAllStandardOutput);
    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError);
    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag);
    stub.set(wtmp_close, stub_wtmp_close);
    stub.set(ADDR(QProcess, setProcessChannelMode), stub_setProcessChannelMode);
    stub.set(ADDR(QProcess, exitCode), stub_exitCode);
    stub.set(ADDR(DLDBusHandler, readLog), stub_KernReadLog);
    stub.set((QByteArray(QIODevice::*)(qint64))ADDR(QIODevice, readLine), fileReadLine);
    stub.set(ADDR(QDateTime, toMSecsSinceEpoch), dnfToMSecsSinceEpoch);
    stub.set(ADDR(QRegExp, indexIn), dmesgIndexIn);
    stub.set((QString & (QString::*)(const QRegExp &, const QString &)) ADDR(QString, replace), dmesgReplace);
    stub.set((QStringList(QRegExp::*)())ADDR(QRegExp, capturedTexts), dmesgCapturedTexts);
    m_logAuthThread->m_FilePath = QStringList() << "/test"<< "/xorg.old";
    m_logAuthThread->m_canRun=true;
    m_logAuthThread->handleDmesg();
    EXPECT_NE(m_logAuthThread,nullptr);
    EXPECT_EQ(m_logAuthThread->m_canRun,true);
}

TEST_F(LogAuthThread_UT, handleBoot_UT_001)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_Logexists);

    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart);
    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString);
    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), dmesgLogreadAllStandardOutput);
    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError);
    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag);
    stub.set(wtmp_close, stub_wtmp_close);
    stub.set(ADDR(QProcess, setProcessChannelMode), stub_setProcessChannelMode);
    stub.set(ADDR(QProcess, exitCode), stub_exitCode);
    stub.set(ADDR(DLDBusHandler, readLog), stub_BootReadLog);
    stub.set((QByteArray(QIODevice::*)(qint64))ADDR(QIODevice, readLine), fileReadLine);
    stub.set(ADDR(QDateTime, toMSecsSinceEpoch), dnfToMSecsSinceEpoch);
    stub.set(ADDR(QRegExp, indexIn), dmesgIndexIn);
    stub.set((QString & (QString::*)(const QRegExp &, const QString &)) ADDR(QString, replace), dmesgReplace);
    stub.set((QStringList(QRegExp::*)())ADDR(QRegExp, capturedTexts), dmesgCapturedTexts);
    m_logAuthThread->m_FilePath = QStringList() << "/test"<< "/xorg.old";
    m_logAuthThread->m_canRun=true;
    m_logAuthThread->handleBoot();
    EXPECT_NE(m_logAuthThread,nullptr);
    EXPECT_EQ(m_logAuthThread->m_canRun,true);
}

TEST_F(LogAuthThread_UT, handleKwin_UT_001)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_Logexists);

    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart);
    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString);
    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), dmesgLogreadAllStandardOutput);
    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError);
    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag);
    stub.set(wtmp_close, stub_wtmp_close);
    stub.set(ADDR(QProcess, setProcessChannelMode), stub_setProcessChannelMode);
    stub.set(ADDR(QProcess, exitCode), stub_exitCode);
    stub.set(ADDR(DLDBusHandler, readLog), stub_BootReadLog);
    stub.set((QByteArray(QIODevice::*)(qint64))ADDR(QIODevice, readLine), fileReadLine);
    stub.set(ADDR(QDateTime, toMSecsSinceEpoch), dnfToMSecsSinceEpoch);
    stub.set(ADDR(QRegExp, indexIn), dmesgIndexIn);
    stub.set((QString & (QString::*)(const QRegExp &, const QString &)) ADDR(QString, replace), dmesgReplace);
    stub.set((QStringList(QRegExp::*)())ADDR(QRegExp, capturedTexts), dmesgCapturedTexts);
    m_logAuthThread->m_FilePath = QStringList() << "/test"<< "/xorg.old";
    m_logAuthThread->m_canRun=true;
    m_logAuthThread->handleKwin();
    EXPECT_NE(m_logAuthThread,nullptr);
    EXPECT_EQ(m_logAuthThread->m_canRun,true);
}

TEST_F(LogAuthThread_UT, handleDpkg_UT_001)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_Logexists);

    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart);
    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString);
    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), dmesgLogreadAllStandardOutput);
    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError);
    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag);
    stub.set(wtmp_close, stub_wtmp_close);
    stub.set(ADDR(QProcess, setProcessChannelMode), stub_setProcessChannelMode);
    stub.set(ADDR(QProcess, exitCode), stub_exitCode);
    stub.set(ADDR(DLDBusHandler, readLog), stub_BootReadLog);
    stub.set((QByteArray(QIODevice::*)(qint64))ADDR(QIODevice, readLine), fileReadLine);
    stub.set(ADDR(QDateTime, toMSecsSinceEpoch), dnfToMSecsSinceEpoch);
    stub.set(ADDR(QRegExp, indexIn), dmesgIndexIn);
    stub.set((QString & (QString::*)(const QRegExp &, const QString &)) ADDR(QString, replace), dmesgReplace);
    stub.set((QStringList(QRegExp::*)())ADDR(QRegExp, capturedTexts), dmesgCapturedTexts);
    m_logAuthThread->m_FilePath = QStringList() << "/test"<< "/xorg.old";
    m_logAuthThread->m_canRun=true;
    m_logAuthThread->handleDkpg();
    EXPECT_NE(m_logAuthThread,nullptr);
    EXPECT_EQ(m_logAuthThread->m_canRun,true);
}

TEST_F(LogAuthThread_UT, handleNormal_UT_001)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_Logexists);

    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart);
    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString);
    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), dmesgLogreadAllStandardOutput);
    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError);
    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag);
    stub.set(wtmp_close, stub_wtmp_close);
    stub.set(ADDR(QProcess, setProcessChannelMode), stub_setProcessChannelMode);
    stub.set(ADDR(QProcess, exitCode), stub_exitCode);
    stub.set(ADDR(DLDBusHandler, readLog), stub_BootReadLog);
    stub.set((QByteArray(QIODevice::*)(qint64))ADDR(QIODevice, readLine), fileReadLine);
    stub.set(ADDR(QDateTime, toMSecsSinceEpoch), dnfToMSecsSinceEpoch);
    stub.set(ADDR(QRegExp, indexIn), dmesgIndexIn);
    stub.set((QString & (QString::*)(const QRegExp &, const QString &)) ADDR(QString, replace), dmesgReplace);
    stub.set((QStringList(QRegExp::*)())ADDR(QRegExp, capturedTexts), dmesgCapturedTexts);
    m_logAuthThread->m_FilePath = QStringList() << "/test"<< "/xorg.old";
    m_logAuthThread->m_canRun=true;
    m_logAuthThread->handleNormal();
    EXPECT_NE(m_logAuthThread,nullptr);
    EXPECT_EQ(m_logAuthThread->m_canRun,true);
}

TEST_F(LogAuthThread_UT, UT_Run_001)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_Logexists);

    stub.set(ADDR(SharedMemoryManager, isAttached), stub_isAttached);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), stub_Logstart);
    stub.set((QString(QDateTime::*)(QStringView) const)ADDR(QDateTime, toString), stub_toString);
    stub.set(ADDR(QProcess, waitForFinished), stub_LogwaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), stub_LogreadAllStandardOutput);
    stub.set(ADDR(QProcess, readAllStandardError), stub_LogreadAllStandardError);
    stub.set(ADDR(SharedMemoryManager, setRunnableTag), stub_LogsetRunnableTag);
    stub.set(wtmp_close, stub_wtmp_close);
    stub.set(ADDR(QProcess, setProcessChannelMode), stub_setProcessChannelMode);
    stub.set(ADDR(QProcess, exitCode), stub_exitCode);
    stub.set(ADDR(DLDBusHandler, readLog), stub_readLog);
    stub.set(ADDR(LogAuthThread, handleDkpg), handleFile);
    stub.set(ADDR(LogAuthThread, handleNormal), handleFile);
    stub.set(ADDR(LogAuthThread, handleDnf), handleFile);
    stub.set(ADDR(LogAuthThread, handleDmesg), handleFile);
    m_logAuthThread->m_type = LOG_FLAG::DPKG;
    m_logAuthThread->run();
    m_logAuthThread->m_type = LOG_FLAG::Normal;
    m_logAuthThread->run();
    m_logAuthThread->m_type = LOG_FLAG::Dnf;
    m_logAuthThread->run();
    m_logAuthThread->m_type = LOG_FLAG::Dmesg;
    m_logAuthThread->run();
    m_logAuthThread->m_type = LOG_FLAG::NONE;
    m_logAuthThread->run();
    EXPECT_NE(m_logAuthThread,nullptr);
    EXPECT_EQ(m_logAuthThread->m_canRun,false);
}

TEST(LogAuthThread_initDnfLevelMap_UT, LogAuthThread_initDnfLevelMap_UT_001)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    p->initDnfLevelMap();
    EXPECT_TRUE(p->m_dnfLevelDict.size());
    EXPECT_TRUE(p->m_transDnfDict.size());
    delete p;
}

TEST(LogAuthThread_initLevelMap_UT, LogAuthThread_initLevelMap_UT_001)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    p->initLevelMap();
    EXPECT_TRUE(p->m_levelMap.size());
    delete p;
}

TEST(LogAuthThread_stopProccess_UT, LogAuthThread_stopProccess_UT_001)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    p->m_isStopProccess = false;
    p->stopProccess();
    EXPECT_TRUE(p->m_isStopProccess);
    EXPECT_FALSE(p->m_canRun);
    delete p;
}

TEST(LogAuthThread_setFilePath_UT, LogAuthThread_setFilePath_UT_001)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    QStringList list;
    list << "test" << "test2";
    p->setFilePath(list);
    EXPECT_EQ(p->m_FilePath, list);
    delete p;
}

TEST(LogAuthThread_getIndex_UT, LogAuthThread_getIndex_UT_001)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    p->m_threadCount = 0;
    p->getIndex();
    EXPECT_EQ(p->getIndex(), p->m_threadCount);
    delete p;
}

TEST(LogAuthThread_startTime_UT, LogAuthThread_startTime_UT_001)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    p->startTime();
    delete p;
}

TEST(LogAuthThread_run_UT, LogAuthThread_run_UT_001)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    p->m_type = LOG_FLAG::KERN;
    Stub stub;
    stub.set(ADDR(LogAuthThread, handleKern), handleFile);
    p->run();
    EXPECT_FALSE(p->m_canRun);
    delete p;
}

TEST(LogAuthThread_run_UT, LogAuthThread_run_UT_002)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    p->m_type = LOG_FLAG::BOOT;
    Stub stub;
    stub.set(ADDR(LogAuthThread, handleBoot), handleFile);
    p->run();
    EXPECT_FALSE(p->m_canRun);
    delete p;
}

TEST(LogAuthThread_run_UT, LogAuthThread_run_UT_003)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    p->m_type = LOG_FLAG::Kwin;
    Stub stub;
    stub.set(ADDR(LogAuthThread, handleKwin), handleFile);
    p->run();
    EXPECT_FALSE(p->m_canRun);
    delete p;
}

TEST(LogAuthThread_run_UT, LogAuthThread_run_UT_004)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    p->m_type = LOG_FLAG::XORG;
    Stub stub;
    stub.set(ADDR(LogAuthThread, handleXorg), handleFile);
    p->run();
    EXPECT_FALSE(p->m_canRun);
    delete p;
}

TEST(LogAuthThread_run_UT, LogAuthThread_run_UT_005)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    p->m_type = LOG_FLAG::DPKG;
    Stub stub;
    stub.set(ADDR(LogAuthThread, handleDkpg), handleFile);
    p->run();
    EXPECT_FALSE(p->m_canRun);
    delete p;
}

TEST(LogAuthThread_run_UT, LogAuthThread_run_UT_006)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    p->m_type = LOG_FLAG::Normal;
    Stub stub;
    stub.set(ADDR(LogAuthThread, handleNormal), handleFile);
    p->run();
    EXPECT_FALSE(p->m_canRun);
    delete p;
}

TEST(LogAuthThread_run_UT, LogAuthThread_run_UT_007)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    p->m_type = LOG_FLAG::Dnf;
    Stub stub;
    stub.set(ADDR(LogAuthThread, handleDnf), handleFile);
    p->run();
    EXPECT_FALSE(p->m_canRun);
    delete p;
}

TEST(LogAuthThread_run_UT, LogAuthThread_run_UT_008)
{
    LogAuthThread *p = new LogAuthThread();
    ASSERT_TRUE(p);
    p->m_type = LOG_FLAG::Dmesg;
    Stub stub;
    stub.set(ADDR(LogAuthThread, handleDmesg), handleFile);
    p->run();
    EXPECT_FALSE(p->m_canRun);
    delete p;
}



