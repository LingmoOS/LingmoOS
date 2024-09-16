// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CAPPLICATION_H
#define CAPPLICATION_H

#include "mainwindow.h"

#include <DVtableHook>
#include <QObject>
#include <DGuiApplicationHelper>

#include <QtSingleApplication>
#include <QProcess>

class CApplication;

#if defined(dApp)
#undef dApp
#endif
#define cApp (static_cast<CApplication *>(QCoreApplication::instance()))
#define CamApp (CApplication::CamApplication())

DWIDGET_USE_NAMESPACE

bool get_application(int &argc, char **argv);

/**
* @brief CApplication　对主窗口事件进行处理
* 主窗口获取与设置，退出时间处理，调用进程等
*/
class CApplication : public QtSingleApplication
{
    Q_OBJECT
public:
    CApplication(int &argc, char **argv);

    /**
     * @brief Application 返回顶层topToolbar
     * @return 返回程序的指针
     */
    static CApplication *CamApplication();

    /**
    * @brief setMainWindow　设置主窗口
    * @param window 主窗口对象
    */
    void setMainWindow(CMainWindow *window);

    /**
     * @brief isPanelEnvironment 是否是平板环境
     * @return
     */
    bool isPanelEnvironment();


    /**
    * @brief getMainWindow　获得主窗口
    */
    CMainWindow *getMainWindow();

signals:
    /**
    * @brief popupConfirmDialog　弹出确认对话框
    */
    void popupConfirmDialog();

protected:
    /**
    * @brief QuitAction　退出事件处理
    */
    void QuitAction();

private:
    static CApplication *cameraCore;
    CMainWindow         *m_mainwindow;
    QList<QProcess *>   m_camprocesslist;
    bool                m_bpanel;//是否是平板设备
};
#endif // CAPPLICATION_H
