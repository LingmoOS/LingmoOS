// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifythread.h"
#include "DDLog.h"
#include "utils.h"

#include <QTimer>
#include <QtDebug>
#include <QProcess>

using namespace DDLog;

NotifyThread::NotifyThread(QObject *parent) : QThread(parent)
{

}

void NotifyThread::run()
{
    QString strUsername("");
    QProcess process;
    forever {
        //查看dde-desktop进程是否启动
        QString outInfo = Utils::executeCmd("ps", QStringList() << "aux", QString(), -1);

        // Split the output  search for the 'grep desktop | grep -v grep' pattern
        QStringList retLst;
        foreach (QString out, outInfo.split("\n")) {
            if (out.contains("desktop") && !out.contains("grep")) {
                retLst << out;
            }
        }
        bool bDeskboot = true;
        if (retLst.size() < 1) {
            sleep(2);
            continue;
        }
        //查看用户名
        process.start("who");
        process.waitForFinished(-1);
        strUsername = process.readAll().split(' ')[0];
        qCInfo(appLog) << strUsername;
        if (strUsername.isEmpty()) {
            sleep(2);
            continue;
        }

        QString name = QLocale::system().name();

        //通知前端 example: runuser -l Username -c "XDG_RUNTIME_DIR=\"/run/user/\$(id -u $Username)\" /usr/bin/deepin-devicemanager notify zh_CN"
        QString strCmd=QString("XDG_RUNTIME_DIR=\"/run/user/\$(id -u %1)\" /usr/bin/deepin-devicemanager notify %2").arg(strUsername).arg(name);
        process.start("runuser", QStringList() << "-l"  << strUsername << "-c" << strCmd);
        process.waitForFinished(-1);
        break;
    }
}

