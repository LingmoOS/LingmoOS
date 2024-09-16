// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QStringList>

class Utils
{
public:
    Utils();
    static QString file2PackageName(const QString &fileName);
    static QString getModuleFilePath(const QString &moduleName);
    static QString kernelRelease();
    static QString machineArch();
    static bool addModBlackList(const QString &moduleName);
    static bool unInstallPackage(const QString &packageName);
    //判断Deb是否为驱动包
    static bool isDriverPackage(const QString &filepath);
    //更新mod dep
    static bool updateModDeps(bool bquick = true);
    //判断文件是否被占用
    static bool isFileLocked(const QString &filepath, bool bread = false);
    //判断dpkg是否被占用
    static bool isDpkgLocked();
    //获取url
    static QString getUrl();
    static QByteArray executeCmd(const QString& cmd, const QStringList& args = QStringList(), const QString& workPath = QString(), int msecsWaiting = 30000);
};

#endif // UTILS_H
