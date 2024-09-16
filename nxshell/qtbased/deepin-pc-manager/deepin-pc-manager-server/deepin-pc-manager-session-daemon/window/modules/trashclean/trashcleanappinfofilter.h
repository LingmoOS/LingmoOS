// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHCLEANAPPINFOFILTER_H
#define TRASHCLEANAPPINFOFILTER_H

#include "../../deepin-pc-manager/src/window/modules/common/invokers/invokerinterface.h"
#include <QObject>

// 使用该结构体保存记录，由于不打算修改trashcleanwidget逻辑，所以后端处理后依旧使用json传递结果
struct TrashCleanApp {
    bool isInstalled; // 本应用属于已安装应用还是卸载应用
    QString appPkgName; // 应用的包名
    QString appDisplayName; // 应用在系统桌面的显示名称
    QStringList appConfigPaths; // 从配置中获取的应用config路径
    QStringList appCachePaths; // 从配置中获取的应用cache路径
    QStringList browserCookiesPaths; // 浏览器cookies路径
    QStringList whiteList; // 白名单
    QStringList blackList; // 黑名单
    TrashCleanApp()
        : isInstalled(false)
        , appPkgName("")
        , appDisplayName("")
    {
    }
};
typedef QList<TrashCleanApp> TrashCleanAppList;

// 接口声明暂放在这里，等待之后统一建立接口文件目录再移动
class TrashCleanAppInfoFilterInterface
{
public:
    virtual ~TrashCleanAppInfoFilterInterface() = default;
    virtual bool FromJson(QString) = 0;
    virtual TrashCleanAppList ToList() = 0;
};

class TrashCleanAppInfoFilter : public QObject
    , public TrashCleanAppInfoFilterInterface
{
    Q_OBJECT
public:
    explicit TrashCleanAppInfoFilter(QObject *parent = nullptr);
    virtual ~TrashCleanAppInfoFilter() override;
    // 返回结果窗口，由于是同工程类内调用，避免再次做JSON解析
    virtual TrashCleanAppList ToList() override;
    // 从配置文件开始构建过滤器内容
    virtual bool FromJson(QString) override;
    // 检查包名是否存在于配置中
    virtual bool isExistInConfig(const QString &);

    // 通过dbus接口获取已安装应用信息：包名 应用名,
    // 因单元测试需要作为public virtual方法
    virtual QMap<QString, QString> GetInstalledApps();

    // 通过数据库获取已卸载应用信息：包名 应用名
    // 注意写数据库时也将应用名写和表
    // 不要对用map存储，会被重新排序
    virtual QList<QPair<QString, QString>> GetUnInstalledApps();

    virtual TrashCleanAppList InitAppData(QString);

    virtual void MergeAppInfo();

    // 因单元测试需要作为public方法
    void MergeInstalledAppInfo(const QMap<QString, QString> &);
    void MergeUninstalledAppInfo(const QList<QPair<QString, QString>> &);

private:
    void reset();
    // 检查应用元素的config和cache目录配置是否均为空
    // 两项配置为空时认为该元素无效
    virtual bool isValidConfig(const TrashCleanApp &);

    QStringList m_installedAppPaths;
    QStringList m_uninstalledAppPaths;
    // 由dbus接口查询获取已安装app
    // 由数据库查询获取已卸载app
    // 对比配置文件，如果有配置，即加入该list
    TrashCleanAppList m_appInfoList;
    DBusInvokerInterface *m_monitorInterFaceServer; // 系统服务日志添加对象
};

#endif // TRASHCLEANAPPINFOFILTER_H
