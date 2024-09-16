// Copyright (C) 2019 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>

#include "qmap.h"

class SecurityPkgNameSql : public QObject
{
    Q_OBJECT

public:
    explicit SecurityPkgNameSql(QSqlDatabase db, QObject *parent = nullptr);
    ~SecurityPkgNameSql();

public:
    // 查询package_binary_file表中已有的包名
    void queryPackageList();

    // 获取包名及相关联文件
    void getPkgNameLinkFile();
    // 建立desktop file -> package 映射
    void getDesktopPackage();
    // 根据已安装的包名来获取可执行文件路径
    void queryPackageInfo(const QString szPackageName);

    // 插入包名及相关联文件数据
    void insertPackageLinkInfo(QMap<QString, QString> sPkgNameMap);

public Q_SLOTS:
    // 新建(可执行文件->包名)映射数据表
    void createPackageLinkTable();
    // 删除数据库某个包名数据 -- 备用
    void deletePackageLinkInfo(const QString sPkgName);
    // 应用程序安装卸载刷新数据库
    void refreshPackageLinkTable();

    // 预留接口,应付Asan内存泄露检查
    void quit();

Q_SIGNALS:
    void notifyPackageLinkInfo(const QString sBinary, const QString sPkgName);
    void notifySqlInsertFinish();

private:
    QSqlDatabase m_db;

    QStringList m_sPkgNameList;
    QMap<QString, QString> m_sPkgNameMap;
};
