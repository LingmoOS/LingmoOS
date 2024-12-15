// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logviewerwatcher.h"

#include <QProcess>
#include <QCoreApplication>
#include <QDebug>

LogViewerWatcher::LogViewerWatcher():m_Timer(new QTimer (this))
{

    connect(m_Timer,&QTimer::timeout,this,&LogViewerWatcher::onTimeOut);
    m_Timer->start(1000);
}

/**
 * @brief 定时监控客户端
 */
void LogViewerWatcher::onTimeOut()
{
    QString cmd, outPut;
    QStringList args;
    //判断lingmo-log-viewer客户端是否存在，如果不存在退出服务。
    cmd = "ps";
    args << "aux";
    outPut= executCmd(cmd, args);
    bool bHasLogViewer = false;
    QStringList rows = outPut.split('\n');
    for (auto line : rows) {
        QStringList items = line.split(' ');
        if (items.contains("lingmo-log-viewer")) {
            bHasLogViewer = true;
            break;
        }
    }
    if (!bHasLogViewer)
        QCoreApplication::exit(0);
}

/**
 * @brief 执行外部命令
 * @param strCmd:外部命令字符串
 */
QString LogViewerWatcher::executCmd(const QString &strCmd, const QStringList &args)
{
     QProcess proc;
     proc.start(strCmd, args);
     proc.waitForFinished(-1);
     return  proc.readAllStandardOutput();
}
