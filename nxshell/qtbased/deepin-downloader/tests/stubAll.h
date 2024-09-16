// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STUBALL_H
#define STUBALL_H

#include <QThread>
#include <QThreadPool>
#include <QDebug>
//#include <DDialog>
#include <QMenu>
#include <QCursor>
#include <QAction>
#include <QFileDialog>
#include <dtkwidget_global.h>
#include "DRadioButton"
#include "timeEdit.h"
DWIDGET_USE_NAMESPACE


void QthreadpoolStart(QRunnable *runnable, int priority = 0);

QAction *QmenuExec(const QPoint &pos);

int DsettingsdialogExec(void *obj);

int DiagnostictoolExec(void *obj);

int MessageboxExec();

int createtaskwidgetExec();

void QsystemtrayiconShow();

void MainframeOndownloadnewurl(QString url, QString savePath, QString fileName, QString type);

bool MainframeShowredownloadmsgbox(const QString sameUrl);

bool QiodeviceOpen(QIODevice::OpenMode flags);

double CreatetaskwidgetFormatspeed(QString str);

void ClipboardtimerCheckclipboardhasurl();

void MainFrameInitWebsocket();

void BtinfodialogSetwindowtitle(QString &str);

QString BtGetSelectedNull();

bool BtGetSelectedIsNull();

bool returnFalse();

bool returnTrue();

int UrlThreadGet405();

int ApplicationHelperThemeType2();

void TableDataControlAutoDownloadBt(QString btFilePath);

void SearchResoultWidgetHide();

bool Aria2RPCInterfaceForceShutdown(QString id = "");

DRadioButton *mockSender();

CTimeEdit *mockSenderTime();

QList<QUrl> torrentLink();
void mockShowAlertMessage(const QString &text, QWidget *follower, int duration = 3000);


#endif // STUBALLTHREAD_H
