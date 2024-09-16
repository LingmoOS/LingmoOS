// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLEANERENVIRONMENT_H
#define CLEANERENVIRONMENT_H

#include <gtest/gtest.h>

#include <QProcess>
#include <QDir>

class CleanerEnvironment : public testing::Environment
{
public:
    virtual void SetUp()
    {
        std::cout << " Cleaner Environment Setup " << std::endl;
        /* 创建垃圾清理所需要使用的被扫描文件
         * 目录结构如下所示
         * |--cleanTestDir
         *  |--app
         *      |--installEmpty
         *      |--installNotEmpty
         *          ----cache
         *          ----config
         *      |--uninstallEmpty
         *      |--uninstallNotEmpty
         *          ----cache
         *          ----config
         *  |--cookies
         *      |--browser
         *          ----cookies
         *  |--deb
         *      ----file1
         *      ----file2
         *  |--history
         *      ----file1
         *  |--log
         *      ----file1
         *      ----file2
         *      ----file3
         *  |--trash
         *      ----file1
         *      ----file2
         *      ----file3
         *      ----file4
        */

        QString currentPath = QDir::currentPath();
        QString testDirPath = currentPath + QDir::separator() + "cleanerTestDir";
        QString app = testDirPath + QDir::separator() + "app";
        QString cookies = testDirPath + QDir::separator() + "cookies";

        QString deb = testDirPath + QDir::separator() + "deb";
        QString history = testDirPath + QDir::separator() + "history";
        QString log = testDirPath + QDir::separator() + "log";
        QString trash = testDirPath + QDir::separator() + "trash";

        QString appSubInstallNotEmpty = app + QDir::separator() + "installNotEmpty";
        QString appSubuninstallNotEmpty = app + QDir::separator() + "uninstallNotEmpty";
        QString appSubInstallEmpty = app + QDir::separator() + "installEmpty";
        QString appSubuninstallEmpty = app + QDir::separator() + "uninstallEmpty";
        QString cookiesSub = cookies + QDir::separator() + "browser";

        //建立目录
        QStringList pathList = QStringList()
                               << deb << history << log << trash
                               << appSubInstallNotEmpty << appSubuninstallNotEmpty
                               << appSubInstallEmpty << appSubuninstallEmpty
                               << cookiesSub;
        QDir dir;
        foreach (auto &path, pathList) {
            dir.mkpath(path);
        }

        // 产生文件模板
        QProcess::execute("dd if=/dev/zero of=template bs=1K count=1");
        QString copyFormat = "cp %1 %2";
        // 拷贝文件到各目录
        QString cmd;

        // 应用目录
        cmd = QString(copyFormat).arg("template").arg(appSubInstallNotEmpty + "/cache");
        QProcess::execute(cmd);
        cmd = QString(copyFormat).arg("template").arg(appSubInstallNotEmpty + "/config");
        QProcess::execute(cmd);
        cmd = QString(copyFormat).arg("template").arg(appSubuninstallNotEmpty + "/cache");
        QProcess::execute(cmd);
        cmd = QString(copyFormat).arg("template").arg(appSubuninstallNotEmpty + "/config");
        QProcess::execute(cmd);

        // cookies目录
        cmd = QString(copyFormat).arg("template").arg(cookiesSub + "/cookies");
        QProcess::execute(cmd);

        // log 目录
        cmd = QString(copyFormat).arg("template").arg(log + "/file1");
        QProcess::execute(cmd);
        cmd = QString(copyFormat).arg("template").arg(log + "/file2");
        QProcess::execute(cmd);
        cmd = QString(copyFormat).arg("template").arg(log + "/file3");
        QProcess::execute(cmd);

        // trash 目录
        cmd = QString(copyFormat).arg("template").arg(trash + "/file1");
        QProcess::execute(cmd);
        cmd = QString(copyFormat).arg("template").arg(trash + "/file2");
        QProcess::execute(cmd);
        cmd = QString(copyFormat).arg("template").arg(trash + "/file3");
        QProcess::execute(cmd);
        cmd = QString(copyFormat).arg("template").arg(trash + "/file4");
        QProcess::execute(cmd);

        // history 目录
        cmd = QString(copyFormat).arg("template").arg(history + "/file1");
        QProcess::execute(cmd);

        // deb目录
        cmd = QString(copyFormat).arg("template").arg(deb + "/file1");
        QProcess::execute(cmd);
        cmd = QString(copyFormat).arg("template").arg(deb + "/file2");
        QProcess::execute(cmd);
    }

    virtual void TearDown()
    {
    }
};

#endif // CLEANERENVIRONMENT_H
