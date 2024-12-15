// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubAll.h"
#include <QStandardPaths>
#include <QList>


QAction *QmenuExec(const QPoint &pos)
{
    return nullptr;
}

int DsettingsdialogExec(void *obj)
{
    return 0;
}

int DiagnostictoolExec(void *obj)
{
    return 0;
}
int createtaskwidgetExec()
{
    return 0;
}

int MessageboxExec()
{
    return 1;
}

void QsystemtrayiconShow()
{
    return;
}

void MainframeOndownloadnewurl(QString url, QString savePath, QString fileName, QString type)
{
    return;
}

bool MainframeShowredownloadmsgbox(const QString sameUrl)
{
    return true;
}

bool QiodeviceOpen(QIODevice::OpenMode flags)
{
    return true;
}

void MainFrameInitWebsocket()
{
    return;
}

double CreatetaskwidgetFormatspeed(QString str)
{
    return 11111111111111111;
}

void BtinfodialogSetwindowtitle(QString &str)
{
    return;
}

void ClipboardtimerCheckclipboardhasurl()
{
    return;
}

QString BtGetSelectedNull()
{
    return "";
}

bool BtGetSelectedIsNull()
{
    return true;
}

bool returnFalse()
{
    return false;
}

bool returnTrue()
{
    return true;
}

int UrlThreadGet405()
{
    return 405;
}

int ApplicationHelperThemeType2()
{
    return 2;
}

void TableDataControlAutoDownloadBt(QString btFilePath)
{
    return;
}

void SearchResoultWidgetHide()
{
    return;
}

bool Aria2RPCInterfaceForceShutdown(QString id)
{
    return true;
}


DRadioButton *mockSender()
{
    static DRadioButton *rad = new DRadioButton;
    return rad;
}

CTimeEdit *mockSenderTime()
{
    static CTimeEdit *time = new CTimeEdit;
    return time;
}

void mockShowAlertMessage(const QString &text, QWidget *follower, int duration)
{

}



QList<QUrl> torrentLink()
{
    QUrl u = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Desktop/seed/123.torrent";
    QList<QUrl> l;
    l << u;
    return l;
}

