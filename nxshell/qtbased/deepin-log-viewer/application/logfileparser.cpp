// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "logfileparser.h"
#include "journalwork.h"
#include "sharedmemorymanager.h"
#include "utils.h"// add by Airy
#include "wtmpparse.h"
#include "logapplicationhelper.h"

#include "parsethread/parsethreadkern.h"
#include "parsethread/parsethreadkwin.h"

#include <DMessageManager>

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QProcess>
#include <QtConcurrent>
#include <QLoggingCategory>

#include <time.h>
#include <utmp.h>
#include <utmpx.h>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logFileParser, "org.deepin.log.viewer.parser")
#else
Q_LOGGING_CATEGORY(logFileParser, "org.deepin.log.viewer.parser", QtInfoMsg)
#endif

int journalWork::thread_index = 0;
int JournalBootWork::thread_index = 0;
DWIDGET_USE_NAMESPACE

LogFileParser::LogFileParser(QWidget *parent)
    : QObject(parent)
{
    qRegisterMetaType<QList<LOG_MSG_KWIN> > ("QList<LOG_MSG_KWIN>");
    qRegisterMetaType<QList<LOG_MSG_XORG> > ("QList<LOG_MSG_XORG>");
    qRegisterMetaType<QList<LOG_MSG_DPKG> > ("QList<LOG_MSG_DPKG>");
    qRegisterMetaType<QList<LOG_MSG_BOOT> > ("QList<LOG_MSG_BOOT>");
    qRegisterMetaType<QList<LOG_MSG_NORMAL> > ("QList<LOG_MSG_NORMAL>");
    qRegisterMetaType<QList<LOG_MSG_DNF>>("QList<LOG_MSG_DNF>");
    qRegisterMetaType<QList<LOG_MSG_DMESG>>("QList<LOG_MSG_DMESG>");
    qRegisterMetaType<QList<LOG_MSG_AUDIT>>("QList<LOG_MSG_AUDIT>");
    qRegisterMetaType<QList<LOG_MSG_JOURNAL>>("QList<LOG_MSG_JOURNAL>");
    qRegisterMetaType<QList<LOG_MSG_COREDUMP>>("QList<LOG_MSG_COREDUMP>");
    qRegisterMetaType<LOG_FLAG> ("LOG_FLAG");

}

LogFileParser::~LogFileParser()
{
    stopAllLoad();
    if (SharedMemoryManager::getInstance()) {
        //释放共享内存
        SharedMemoryManager::instance()->releaseMemory();
    }
}

int LogFileParser::parseByJournal(const QStringList &arg)
{
    stopAllLoad();

    emit stopJournal();
    journalWork *work = new journalWork(this);

    work->setArg(arg);
    auto a = connect(work, &journalWork::journalFinished, this, &LogFileParser::journalFinished,
                     Qt::QueuedConnection);
    auto b = connect(work, &journalWork::journalData, this, &LogFileParser::journalData,
                     Qt::QueuedConnection);

    connect(this, &LogFileParser::stopJournal, work, &journalWork::stopWork);

    int index = work->getIndex();
    QThreadPool::globalInstance()->start(work);
    return index;
}

int LogFileParser::parseByJournalBoot(const QStringList &arg)
{
    stopAllLoad();
    JournalBootWork *work = new JournalBootWork(this);

    work->setArg(arg);
    auto a = connect(work, &JournalBootWork::journalBootFinished, this, &LogFileParser::journalBootFinished,
                     Qt::QueuedConnection);
    auto b = connect(work, &JournalBootWork::journaBootlData, this, &LogFileParser::journaBootlData,
                     Qt::QueuedConnection);

    connect(this, &LogFileParser::stopJournalBoot, work, &JournalBootWork::stopWork);

    int index = work->getIndex();
    QThreadPool::globalInstance()->start(work);
    return index;
}

int LogFileParser::parseByDpkg(const DKPG_FILTERS &iDpkgFilter)
{

    stopAllLoad();
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(DPKG);
    QStringList filePath = DLDBusHandler::instance(this)->getFileInfo("dpkg");
    //    const QString&str="/var/log/kern";
    authThread->setFilePath(filePath);
    authThread->setFileterParam(iDpkgFilter);
    connect(authThread, &LogAuthThread::proccessError, this,
            &LogFileParser::slog_proccessError, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::dpkgFinished, this,
            &LogFileParser::dpkgFinished, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::dpkgData, this,
            &LogFileParser::dpkgData, Qt::UniqueConnection);
    connect(this, &LogFileParser::stopDpkg, authThread, &LogAuthThread::stopProccess);
    int index = authThread->getIndex();
    QThreadPool::globalInstance()->start(authThread);
    return index;
}

int LogFileParser::parseByXlog(const XORG_FILTERS &iXorgFilter)    // modifed by Airy
{
    stopAllLoad();
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(XORG);
    QStringList filePath = DLDBusHandler::instance(this)->getFileInfo("Xorg");
    authThread->setFilePath(filePath);
    authThread->setFileterParam(iXorgFilter);
    connect(authThread, &LogAuthThread::proccessError, this,
            &LogFileParser::slog_proccessError, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::xorgFinished, this,
            &LogFileParser::xlogFinished, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::xorgData, this,
            &LogFileParser::xlogData, Qt::UniqueConnection);
    connect(this, &LogFileParser::stopXlog, authThread, &LogAuthThread::stopProccess);
    int index = authThread->getIndex();
    QThreadPool::globalInstance()->tryStart(authThread);
    return index;
}

int LogFileParser::parseByNormal(const NORMAL_FILTERS &iNormalFiler)
{
    stopAllLoad();
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(Normal);
    authThread->setFileterParam(iNormalFiler);
    connect(authThread, &LogAuthThread::proccessError, this,
            &LogFileParser::slog_proccessError, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::normalFinished, this,
            &LogFileParser::normalFinished, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::normalData, this,
            &LogFileParser::normalData, Qt::UniqueConnection);
    connect(this, &LogFileParser::stopNormal, authThread, &LogAuthThread::stopProccess);
    int index = authThread->getIndex();
    QThreadPool::globalInstance()->tryStart(authThread);
    return index;
}

int LogFileParser::parseByKwin(const KWIN_FILTERS &iKwinfilter)
{
    stopAllLoad();
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(Kwin);
    authThread->setFileterParam(iKwinfilter);
    connect(authThread, &LogAuthThread::kwinFinished, this,
            &LogFileParser::kwinFinished);
    connect(authThread, &LogAuthThread::kwinData, this,
            &LogFileParser::kwinData);
    connect(this, &LogFileParser::stopKwin, authThread, &LogAuthThread::stopProccess);

    int index = authThread->getIndex();
    QThreadPool::globalInstance()->start(authThread);
    return index;
}

int LogFileParser::parseByBoot()
{
    stopAllLoad();
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(BOOT);

    QStringList filePath = DLDBusHandler::instance(this)->getFileInfo("boot");
    authThread->setFilePath(filePath);
    connect(authThread, &LogAuthThread::bootFinished, this,
            &LogFileParser::bootFinished);
    connect(authThread, &LogAuthThread::bootData, this,
            &LogFileParser::bootData);
    connect(this, &LogFileParser::stopBoot, authThread,
            &LogAuthThread::stopProccess);
    int index = authThread->getIndex();
    QThreadPool::globalInstance()->start(authThread);
    return index;
}

int LogFileParser::parse(LOG_FILTER_BASE &filter)
{
    stopAllLoad();

    ParseThreadBase *parseWork = nullptr;
    if (filter.type == KERN)
        parseWork = new ParseThreadKern(this);
    else if (filter.type == Kwin)
        parseWork = new ParseThreadKwin(this);
    if (parseWork) {
        parseWork->setFilter(filter);
        int index = parseWork->getIndex();
        QThreadPool::globalInstance()->start(parseWork);
        return index;
    }

    return -1;
}

int LogFileParser::parseByKern(const KERN_FILTERS &iKernFilter)
{
    stopAllLoad();
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(KERN);
    QStringList filePath = DLDBusHandler::instance(this)->getFileInfo("kern", false);
    authThread->setFileterParam(iKernFilter);
    authThread->setFilePath(filePath);
    connect(authThread, &LogAuthThread::kernFinished, this,
            &LogFileParser::kernFinished);
    connect(authThread, &LogAuthThread::kernData, this,
            &LogFileParser::kernData);
    connect(this, &LogFileParser::stopKern, authThread,
            &LogAuthThread::stopProccess);
    int index = authThread->getIndex();
    QThreadPool::globalInstance()->start(authThread);
    return index;
}

int LogFileParser::parseByApp(const APP_FILTERS &iAPPFilter)
{
    // 根据应用名获取应用日志配置信息
    QString appName = iAPPFilter.app;
    AppLogConfig appLogConfig = LogApplicationHelper::instance()->appLogConfig(appName);

    APP_FILTERSList appFilterList;
    if (appLogConfig.subModules.size() == 1) {
        APP_FILTERS appFilter = iAPPFilter;
        // 子模块名称与应用名称显示一致，并且仅有一个子模块，
        // 则认为该应用与子模块同名称，来源列表显示为应用名称
        if (appLogConfig.subModules[0].name == appLogConfig.name)
            appFilter.submodule = "";
        else
            appFilter.submodule = appLogConfig.subModules[0].name;
        appFilter.logType = appLogConfig.subModules[0].logType;
        appFilter.filter = appLogConfig.subModules[0].filter;
        appFilter.path = appLogConfig.subModules[0].logPath;
        appFilter.execPath = appLogConfig.subModules[0].execPath;
        appFilterList.push_back(appFilter);
    } else if (appLogConfig.subModules.size() > 1) {
        for (auto submodule : appLogConfig.subModules) {
            APP_FILTERS appFilter = iAPPFilter;
            appFilter.submodule = submodule.name;
            appFilter.logType = submodule.logType;
            appFilter.filter = submodule.filter;
            appFilter.path = submodule.logPath;
            appFilter.execPath = submodule.execPath;
            appFilterList.push_back(appFilter);
        }
    }

    if (appFilterList.size() > 0) {
        stopAllLoad();

        m_appThread = new LogApplicationParseThread(this);
        quitLogAuththread(m_appThread);

        disconnect(m_appThread, &LogApplicationParseThread::appFinished, this,
                   &LogFileParser::appFinished);
        disconnect(m_appThread, &LogApplicationParseThread::appData, this,
                   &LogFileParser::appData);
        disconnect(this, &LogFileParser::stopApp, m_appThread,
                   &LogApplicationParseThread::stopProccess);
        m_appThread->setFilters(appFilterList);
        connect(m_appThread, &LogApplicationParseThread::appFinished, this,
                &LogFileParser::appFinished);
        connect(m_appThread, &LogApplicationParseThread::appData, this,
                &LogFileParser::appData);
        connect(this, &LogFileParser::stopApp, m_appThread,
                &LogApplicationParseThread::stopProccess);
        connect(m_appThread, &LogApplicationParseThread::finished, m_appThread,
                &QObject::deleteLater);
        int index = m_appThread->getIndex();
        m_appThread->start();
        return index;
    }

    return -1;
}

void LogFileParser::parseByDnf(DNF_FILTERS iDnfFilter)
{
    stopAllLoad();
    LogAuthThread *authThread = new LogAuthThread(this);
    authThread->setType(Dnf);
    QStringList filePath = DLDBusHandler::instance(this)->getFileInfo("dnf");
    authThread->setFilePath(filePath);
    authThread->setFileterParam(iDnfFilter);
    connect(authThread, &LogAuthThread::proccessError, this,
            &LogFileParser::slog_proccessError, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::dnfFinished, this,
            &LogFileParser::dnfFinished, Qt::UniqueConnection);
    connect(this, &LogFileParser::stopDnf, authThread,
            &LogAuthThread::stopProccess);
    QThreadPool::globalInstance()->start(authThread);
}

void LogFileParser::parseByDmesg(DMESG_FILTERS iDmesgFilter)
{
    stopAllLoad();
    LogAuthThread *authThread = new LogAuthThread(this);
    authThread->setType(Dmesg);
    QStringList filePath = DLDBusHandler::instance(this)->getFileInfo("dmesg");
    authThread->setFilePath(filePath);
    authThread->setFileterParam(iDmesgFilter);
    connect(authThread, &LogAuthThread::proccessError, this,
            &LogFileParser::slog_proccessError, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::dmesgFinished, this,
            &LogFileParser::dmesgFinished, Qt::UniqueConnection);
    connect(this, &LogFileParser::stopDmesg, authThread,
            &LogAuthThread::stopProccess);
    QThreadPool::globalInstance()->start(authThread);
}

int LogFileParser::parseByOOC(const QString &path)
{
    stopAllLoad();

    m_OOCThread = new LogOOCFileParseThread(this);
    m_OOCThread->setParam(path);
    connect(m_OOCThread, &LogOOCFileParseThread::sigFinished, this,
            &LogFileParser::OOCFinished);
    connect(m_OOCThread, &LogOOCFileParseThread::sigData, this,
            &LogFileParser::OOCData);
    connect(this, &LogFileParser::stopOOC, m_OOCThread,
            &LogOOCFileParseThread::stopProccess);
    connect(m_OOCThread, &LogOOCFileParseThread::finished, m_OOCThread,
            &QObject::deleteLater);
    int index = m_OOCThread->getIndex();
    m_OOCThread->start();
    return index;
}

int LogFileParser::parseByAudit(const AUDIT_FILTERS &iAuditFilter)
{
    stopAllLoad();
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(Audit);
    QStringList filePath = DLDBusHandler::instance(this)->getFileInfo("audit", false);
    authThread->setFileterParam(iAuditFilter);
    authThread->setFilePath(filePath);
    connect(authThread, &LogAuthThread::auditFinished, this,
            &LogFileParser::auditFinished);
    connect(authThread, &LogAuthThread::auditData, this,
            &LogFileParser::auditData);
    connect(this, &LogFileParser::stopKern, authThread,
            &LogAuthThread::stopProccess);
    int index = authThread->getIndex();
    QThreadPool::globalInstance()->start(authThread);
    return index;
}

int LogFileParser::parseByCoredump(const COREDUMP_FILTERS &iCoredumpFilter, bool parseMap)
{
    stopAllLoad();
    //qRegisterMetaType<QList<quint16>>("QList<LOG_MSG_COREDUMP>");
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(COREDUMP);
    authThread->setParseMap(parseMap);
    authThread->setFileterParam(iCoredumpFilter);
    connect(authThread, &LogAuthThread::coredumpFinished, this,
            &LogFileParser::coredumpFinished);
    connect(authThread, &LogAuthThread::coredumpData, this,
            &LogFileParser::coredumpData);
    connect(this, &LogFileParser::stopCoredump, authThread, &LogAuthThread::stopProccess);
    int index = authThread->getIndex();
    QThreadPool::globalInstance()->start(authThread);
    return index;
}

void LogFileParser::stopAllLoad()
{
    emit stop();
    emit stopKern();
    emit stopBoot();
    emit stopDpkg();
    emit stopXlog();
    emit stopKwin();
    emit stopApp();
    emit stopJournal();
    emit stopJournalBoot();
    emit stopNormal();
    emit stopDnf();
    emit stopDmesg();
    emit stopOOC();
    emit stopCoredump();
    return;
}

void LogFileParser::quitLogAuththread(QThread *iThread)
{
    if (iThread && iThread->isRunning()) {
        iThread->quit();
        iThread->wait();
    }
}






#include <unistd.h>
#include <QApplication>

/**
 * @brief LogFileParser::slog_proccessError 处理转发日志获取线程的错误信息信号
 * @param iError 错误信息
 */
void LogFileParser::slog_proccessError(const QString &iError)
{
    emit proccessError(iError);
}




