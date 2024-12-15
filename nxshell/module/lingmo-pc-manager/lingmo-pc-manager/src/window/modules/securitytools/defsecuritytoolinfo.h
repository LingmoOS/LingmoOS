// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFSECURITYTOOLINFO_H
#define DEFSECURITYTOOLINFO_H

#include <QDBusArgument>
#include <QMetaType>
#include <QString>

// 定义工具状态信息
enum DEFSECURITYTOOLSTATUS {
    UNINSTALLED,    // 未安装
    INSTALLING,     // 安装中
    INSTALLED,      // 已安装
    INSTALLFAILED,  // 安装失败
    INSTALLSUCCESS, // 安装成功
    WAITFORUPDATE,  // 待更新状态
    UPDATING,       // 更新中
    UPDATEFAIL,     // 更新失败
    UPDATASUCCESS,  // 更新成功
    UNINSTALLING,   // 卸载中
    UNINSTALLFAILED // 卸载失败
};

// 定义工具安装状态
enum TOOLSSTATUS { ALL, INSTALLEDTOOL, UNINSTALLEDTOOL };

// 定义工具分类
enum TOOLCLASSIFY {
    ALLTOOLS,     // 所有
    SYSTEMTOOLS,  // 系统
    NETWORKTOOLS, // 网络
    DATATOOLS,    // 数据
    APPMGRTOOLS,  // 应用管理
    DEVICETOOLS   // 外设
};

// 定义工具信息结构体
typedef struct DefSecurityToolInfo
{
    explicit DefSecurityToolInfo()
        : strPackageName("")
        , strAppName("")
        , strImagePath("")
        , strDescription("")
        , strCompany("")
        , strInallModel("")
        , strCallMethodName("")
        , bSystemTool(false)
        , bHasInstalled(false)
        , bCanUpdate(false)
        , iClassify(ALLTOOLS)
        , strReserve("")
        , strReserve2("")
        , strReserve3("")
        , strReserve4("")
    {
    }

    QString getPackageKey() { return QString("%1_%2").arg(strPackageName).arg(strAppName); }

    QString strPackageName; // 安装包名称
    QString strAppName;     // 应用名称
    QString strImagePath;   // 图标名称
    QString strDescription; // 应用描述
    QString strCompany;
    QString strInallModel;     // 安装模型
    QString strCallMethodName; // 调用方法名称
    bool bSystemTool;          // 是否可卸载
    bool bHasInstalled;        // 已安装
    bool bCanUpdate;           // 是否有更新
    int iClassify;             // 应用分类
    QString strReserve; // 保留参数1，黙认对应用DBus的Service，当有其它调用方式时可以覆盖使用。
    QString strReserve2; // 保留参数2，黙认对应用DBus的Path, 当有其它调用方式时可以覆盖使用。
    QString strReserve3; // 保留参数3，黙认对应用DBus的Interface，当有其它调用方式时可以覆盖使用。
    QString strReserve4; // 保留参数3，黙认对应用DBus的服务总线类型，当有其它调用方式时可以覆盖使用。
} DEFSECURITYTOOLINFO, *LPDEFSECURITYTOOLINFO;

typedef QList<DEFSECURITYTOOLINFO> DEFSECURITYTOOLINFOLIST;

Q_DECLARE_METATYPE(DEFSECURITYTOOLINFO);
Q_DECLARE_METATYPE(DEFSECURITYTOOLINFOLIST);
void registerDefSecurityToolInfo();
void registerDefSecurityToolInfoList();
QDBusArgument &operator<<(QDBusArgument &argument, const DEFSECURITYTOOLINFO &toolInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, DEFSECURITYTOOLINFO &toolInfo);

#endif // DEFSECURITYTOOLINFO_H
