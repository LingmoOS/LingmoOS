// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGFILEPARSER_H
#define LOGFILEPARSER_H

#include "journalwork.h"
#include "journalbootwork.h"
#include "logapplicationparsethread.h"
#include "logauththread.h"
#include "dbusproxy/dldbushandler.h"
#include "dbusproxy/dldbusinterface.h"
#include "logoocfileparsethread.h"

#include <QMap>
#include <QThread>
#include <QDebug>

class LogFileParser : public QObject
{
    Q_OBJECT
public:
    explicit LogFileParser(QWidget *parent = nullptr);
    ~LogFileParser();


    int parseByJournal(const QStringList &arg = QStringList());
    int parseByJournalBoot(const QStringList &arg = QStringList());

    int parseByDpkg(const DKPG_FILTERS &iDpkgFilter);

    int parseByXlog(const XORG_FILTERS &iXorgFilter);
    int parseByBoot();
    // 通用解析入口，传入筛选参数，解析线程根据筛选参数吐出日志数据
    int parse(LOG_FILTER_BASE &filter);
    int parseByKern(const KERN_FILTERS &iKernFilter);
    int parseByApp(const APP_FILTERS &iAPPFilter);
    void parseByDnf(DNF_FILTERS iDnfFilter);
    void parseByDmesg(DMESG_FILTERS iDmesgFilter);
    int parseByNormal(const NORMAL_FILTERS &iNormalFiler);   // add by Airy

    int parseByKwin(const KWIN_FILTERS &iKwinfilter);
    int parseByOOC(const QString &path);

    int parseByAudit(const AUDIT_FILTERS &iAuditFilter);

    int parseByCoredump(const COREDUMP_FILTERS &iCoredumpFilter, bool parseMap = false);

    void stopAllLoad();

signals:
    void parseFinished(int index, LOG_FLAG type, int status);
    void logData(int index, const QList<QString> &dataList, LOG_FLAG type);
    void stop();

    void dpkgFinished(int index);
    void dpkgData(int index, QList<LOG_MSG_DPKG>);
    void xlogFinished(int index);
    void xlogData(int index, QList<LOG_MSG_XORG>);
    void bootFinished(int index);
    void bootData(int index, QList<LOG_MSG_BOOT>);
    void kernFinished(int index);
    void kernData(int index, QList<LOG_MSG_JOURNAL>);
    void dnfFinished(QList<LOG_MSG_DNF>);
    void dmesgFinished(QList<LOG_MSG_DMESG>);
    void journalFinished(int index);
    void journalBootFinished(int index);
    void journalData(int index, QList<LOG_MSG_JOURNAL>);
    void journaBootlData(int index, QList<LOG_MSG_JOURNAL>);

    //void normalFinished();  // add by Airy

    void normalFinished(int index);
    void normalData(int index, QList<LOG_MSG_NORMAL>);
    void kwinFinished(int index);
    void kwinData(int index, QList<LOG_MSG_KWIN> iKwinList);
    /**
     * @brief appFinished 获取数据结束信号
     */
    void appFinished(int index);
    void appData(int index, QList<LOG_MSG_APPLICATOIN> iDataList);
    void OOCFinished(int index, int error = 0);
    void OOCData(int index, const QString &data);

    void auditFinished(int index, bool bShowTip = false);
    void auditData(int index, QList<LOG_MSG_AUDIT>);

    void coredumpFinished(int index);
    void coredumpData(int index, QList<LOG_MSG_COREDUMP> iDataList);

    void stopKern();
    void stopBoot();
    void stopDpkg();
    void stopXlog();
    void stopNormal();
    void stopKwin();
    void stopApp();
    void stopJournal();
    void stopJournalBoot();
    void stopDnf();
    void stopDmesg();
    void stopOOC();
    void stopCoredump();
    /**
     * @brief proccessError 获取日志文件失败错误信息传递信号，传递到主界面显示 DMessage tooltip
     * @param iError 错误字符
     */
    void proccessError(const QString &iError);

private:
    void quitLogAuththread(QThread *iThread);
signals:

public slots:
    void slog_proccessError(const QString &iError);
private:
    LogOOCFileParseThread *m_OOCThread {nullptr};
    LogApplicationParseThread *m_appThread {nullptr};
};

#endif  // LOGFILEPARSER_H
