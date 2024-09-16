// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QStandardPaths>
#include <iostream>
#include "installDebThread.h"

#include <sys/types.h>
#include <unistd.h>

bool isValidInvoker()
{
    bool valid = false;

    // 判断是否存在执行路径
    pid_t pid = getppid();
    QFileInfo f(QString("/proc/%1/exe").arg(pid));
    if (!f.exists()) {
        valid = false;
    } else {
        valid = true;
    }

    // 是否存在于可调用者名单中
    QStringList ValidInvokerExePathList;
    QString invokerPath = f.canonicalFilePath();
    QStringList findPaths;  // 合法调用者查找目录列表
    findPaths << "/usr/bin";
    ValidInvokerExePathList << QStandardPaths::findExecutable("deepin-deb-installer", findPaths);

    if (valid) {
        valid = ValidInvokerExePathList.contains(invokerPath);
    }

    // 非法调用
    if (!valid) {
        qWarning() << QString("(pid: %1)[%2] is not allowed to configrate firewall").arg(pid).arg((invokerPath));
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if (!isValidInvoker()) {
        return 1;
    }

    QCommandLineParser parser;
    parser.process(app);
    const QStringList tParamList = parser.positionalArguments();

    InstallDebThread mThread;
    mThread.setParam(tParamList);
    mThread.run();
    mThread.wait();
    return mThread.m_resultFlag;
}
