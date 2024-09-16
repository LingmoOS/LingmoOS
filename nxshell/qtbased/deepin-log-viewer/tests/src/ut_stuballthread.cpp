// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_stuballthread.h"
void QThread_start(QThread::Priority)
{
    qDebug() << "QThread_start--";
}

void QThreadPool_start(QRunnable *runnable, int priority)
{
    qDebug() << "QThreadPool_start--";
}

int LogFileParser_parseByJournal(QStringList arg)
{
    return 0;
}

int LogFileParser_parseByJournalBoot(QStringList arg)
{
    return 0;
}

void LogFileParser_parseByDpkg(DKPG_FILTERS &iDpkgFilter)
{
}

void LogFileParser_parseByXlog(XORG_FILTERS &iXorgFilter)
{
}

void LogFileParser_parseByBoot()
{
}

void LogFileParser_parseByKern(KERN_FILTERS &iKernFilter)
{
}

void LogFileParser_parseByApp(APP_FILTERS &iAPPFilter)
{
}

void LogFileParser_parseByNormal(QList<LOG_MSG_NORMAL> &nList, NORMAL_FILTERS &iNormalFiler)
{
}

void LogFileParser_parseByKwin(KWIN_FILTERS iKwinfilter)
{
}

void LogFileParser_parseByOOC(QString & path)
{
    Q_UNUSED(path)
}

//void QRunnable_run()
//{
//}

//void Doc_Document_save(const QString &path)
//{
//}
