// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SECURITYLOGENVIRONMENT_H
#define SECURITYLOGENVIRONMENT_H

#include <gtest/gtest.h>
#include <QFile>
#include <QDir>
#include <QProcess>

#define DATABASENAME "securitylog.db"
#define DATABASE_QRC ":/SecurityLog/testfiles/securitylog.db"

class SecurityLogEnvironment : public testing::Environment
{
public:
    virtual void SetUp()
    {
        // 从资源中拷贝数据库文件到构建目录用于测试
        QFile file(DATABASE_QRC);
        QString path = QString("%1%2%3").arg(QDir::currentPath()).arg(QDir::separator()).arg(DATABASENAME);
        file.copy(path);
        // 数据库文件权限设置为仅所有者读写
        QString cmd = "chmod 600 %1";
        QProcess::execute(cmd.arg(path));
    }
    virtual void TearDown()
    {
    }
};

#endif // SECURITYLOGENVIRONMENT_H
