// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STUBALLTHREAD_H
#define STUBALLTHREAD_H
#include "structdef.h"

#include <QThread>
#include <QThreadPool>
#include <QDebug>
void QThreadPool_start(QRunnable *runnable, int priority = 0);

void QThread_start(QThread::Priority = QThread::InheritPriority);
int LogFileParser_parseByJournal(QStringList arg = QStringList());
int LogFileParser_parseByJournalBoot(QStringList arg = QStringList());

void LogFileParser_parseByDpkg(DKPG_FILTERS &iDpkgFilter);
void LogFileParser_parseByXlog(XORG_FILTERS &iXorgFilter);
void LogFileParser_parseByBoot();
void LogFileParser_parseByKern(KERN_FILTERS &iKernFilter);
void LogFileParser_parseByApp(APP_FILTERS &iAPPFilter);
void LogFileParser_parseByNormal(QList<LOG_MSG_NORMAL> &nList, NORMAL_FILTERS &iNormalFiler); // add by Airy
void LogFileParser_parseByKwin(KWIN_FILTERS iKwinfilter);
void LogFileParser_parseByOOC(QString & path);
void Doc_Document_save(const QString &path);
void QRunnable_run();
#endif // STUBALLTHREAD_H
