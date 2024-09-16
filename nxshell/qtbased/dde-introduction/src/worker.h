// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKER_H
#define WORKER_H

#include "model.h"

#include <QObject>
#include <QJsonDocument>
#include <QProcess>

#include <com_deepin_daemon_appearance.h>
#include <com_deepin_wmswitcher.h>
#include <com_deepin_dde_daemon_dock.h>
#include <DWindowManagerHelper>

//这是什么??? 封装好的DBUS接口
using Icon       = com::deepin::daemon::Appearance;
using WMSwitcher = com::deepin::WMSwitcher;
using Dock       = com::deepin::dde::daemon::Dock;

DGUI_USE_NAMESPACE
//发送信息到DBUS改变电脑的桌面样式、运行模式、图标主题类
class Worker : public QObject
{
    Q_OBJECT

public:
    static Worker* Instance();
    /* 判断当前系统是否是wayland系统 */
    static bool isWaylandType();

public slots:
    //设置桌面样式发送到dock栏
    void setDesktopMode(Model::DesktopMode mode);
    //设置运行模式发送DBUS修改电脑的窗口特效打开或关闭
    void setWMMode(Model::WMType type);
    //设置主题图标改变电脑的主题图标配置
    void setIcon(const IconStruct &icon);

private slots:
    //运行模式变化保存到model槽
    void onWMChanged(const QString &wm);
    //目前不使用，等后续研究
    void onWMChang(/*const quint32 &wm*/);
    //桌面样式变化保存到model槽
    void onDisplayModeChanged(int mode);
    //图标刷新槽
    void onIconRefreshed(const QString &name);
    //图标主题列表变化响应槽
    void onIconListChanged(const QString & value);
    //图标图片完成响应槽
    void onIconPixmapFinished(QDBusPendingCallWatcher *w);
    //图片点击刷新响应槽 ut002764 2021.6.1
    void onIconClickChange(QDBusPendingCallWatcher *w);

private:
    explicit Worker(QObject *parent = nullptr);

private:
    Model*      m_model;
    //图标DBUS
    Icon*       m_iconInter;
    //窗口特效DBUS
    WMSwitcher* m_wmInter;
    //Dock栏DBUS
    Dock*       m_dockInter;
    //窗口管理帮助，什么作用???
    DWindowManagerHelper* m_windowManage;
};

#endif // WORKER_H
