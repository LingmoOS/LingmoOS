// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "watcher.h"
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <QDebug>
#include <QTime>
#include <QThread>
#include <QProcess>
#include <QString>
#include <QCoreApplication>


namespace DiskManager {

/**
 * @brief 执行外部命令
 * @param strCmd:外部命令字符串
 * @param outPut:命令控制台输出
 * @param error:错误信息
 * @return exitcode:退出码
 */
void Watcher::executCmd(const QString &strCmd, QString &outPut, QString &error)
{
    // QProcess proc;
    // proc.start("b-a-s-h", QStringList() << "-c" << strCmd);
    // proc.waitForFinished(-1);
    // outPut = proc.readAllStandardOutput();
}

void Watcher::exit()
{
    qDebug() << "Watcher wait for exit now!";
    stoped = true;
    this->wait();
}

void Watcher::run()
{
    bool isrun = false;
    QString cmd, outPut, error;
    //先判断后台服务进程是否存在,如果存在可能是强制退出导致,应先退出后台程序再重新启动磁盘管理器
    cmd = QString("ps -eo pid,cmd |awk '{print $2}' |grep -w deepin-diskmanager$");

    while (!stoped) {
        QThread::msleep(500);  //0.5 second
        executCmd(cmd, outPut, error);
        int ret = 0;
        ret = outPut.length();
        if (ret) {
            //这里表示前端在运行当中
           // qDebug() << "Set to true!!!!!";
            isrun = true;
        } else {
            //这里表示，前端启动过，但是现在已经关闭了
            if (isrun) {
                qDebug() << "Need to quit now";
                _exit(0);
            }
        }
    // qDebug() << "Sleep !!!  == " << ret << " " << outPut << " " << outPut.length() << " "<< error;
    }
}

} //endof namespace
