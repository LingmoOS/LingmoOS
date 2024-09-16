// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QThread>
#include <QMutex>
/*************************************************************************
 <Class>         FontManagerCore
 <Description>   字体管理线程类-线程执行字体安装、批量安装、中途安装、重新安装、删除、批量删除等操作
 <Author>
 <Note>          null
*************************************************************************/
class FontManagerCore : public QThread
{
    Q_OBJECT

public:
    /*************************************************************************
     <Enum>          Type
     <Description>   操作类型
     <Author>
     <Value>
        <Value1>     Install                 Description:安装
        <Value2>     ReInstall               Description:重装
        <Value2>     UnInstall               Description:卸载
        <Value2>     HalfwayInstall          Description:中途安装
     <Note>          null
    *************************************************************************/
    enum Type { Install, ReInstall, UnInstall, HalfwayInstall, DoCache, DefaultNullType};
    /*************************************************************************
     <Enum>          InstallStatus
     <Description>   安装状态
     <Author>
     <Value>
        <Value1>     InstallSuccess        Description:安装成功
        <Value2>     HalfwayInstallSuccess Description:中途安装成功
        <Value2>     Failed                Description:安装失败
     <Note>          null
    *************************************************************************/
    enum InstallStatus {InstallSuccess, HalfwayInstallSuccess, Failed};
    /*************************************************************************
     <Enum>          CacheStatus
     <Description>   fc-cache命令执行状态
     <Author>
     <Value>
        <Value1>     CacheNow               Description:立即执行
        <Value2>     CacheLater             Description:稍后执行
        <Value2>     NoNewFonts             Description:不执行
     <Note>          null
    *************************************************************************/
    enum CacheStatus {CacheNow, CacheLater, NoNewFonts};
    static FontManagerCore *instance();

    explicit FontManagerCore(QObject *parent = nullptr);
    ~FontManagerCore();
    //设置线程执行类型
    void setType(Type type);
    //传入待安装字体列表
    void setInstallFileList(const QStringList &list);
    //传入待删除字体列表
    void setUnInstallFile(const QStringList &filePath);
    //是否需要做fc-cache
    inline bool needCache()
    {
        return ((m_type == Install && m_CacheStatus != NoNewFonts && !m_installCanceled)
                || (m_type == ReInstall && (m_CacheStatus != NoNewFonts || !m_instFileList.isEmpty())));

    }
    //执行fc-cache命令
    void doCache();
    //设置fc-cache命令执行的状态
    void setCacheStatus(const CacheStatus &CacheStatus);
    //取消安装
    void cancelInstall();

signals:
    //批量安装信号
    void batchInstall(const QString &filePath, const double &percent);
    //安装完成信号
    void installFinished(int state, const QStringList &fileList);
    //重装完成信号
    void reInstallFinished(int state, const QStringList &fileList);
    //卸载完成信号
    void uninstallFontFinished(QStringList &uninstallIndex);
    //卸载fc-cache命令执行完成信号
    void uninstallFcCacheFinish();
    //fc-cache命令完成信号
    void cacheFinish();
    //取消安装
    void requestCancelInstall();
    //处理删除tcc
    void handleDeleteTTC(QString file, bool &isDelete, bool &isAapplyToAll);

public slots:
    //安装拷贝字体文件槽函数
    void onInstallResult(const QString &familyName, const QString &target);

protected:
    //线程执行入口函数-安装、中途安装、重装与卸载
    void run();

private:
    //线程执行函数
    void doCmd(QStringList &arguments);
    //字体安装-函数入口
    void handleInstall();
    //字体卸载-函数入口
    void handleUnInstall();
    //字体安装-具体执行函数
    void doInstall(QStringList &fileList);
    //字体卸载-具体执行函数
    void doUninstall(const QStringList &fileList);

private:
    QStringList m_instFileList;
    QStringList m_installOutList;
    QStringList m_uninstFile;

    volatile bool m_installCanceled = false;
    Type m_type{Type::DefaultNullType};
    CacheStatus m_CacheStatus {CacheStatus::CacheNow};
    int m_installedCount = 0;
};

