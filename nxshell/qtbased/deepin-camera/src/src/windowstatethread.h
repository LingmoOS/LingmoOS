// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWSTATETHREAD_H
#define WINDOWSTATETHREAD_H

#include <DMainWindow>
#include <DForeignWindow>

#include <thread>
#include <QThread>
#include <QMutex>

#ifdef USE_DEEPIN_WAYLAND
#ifdef DWAYLAND
#include <DWayland/Client/clientmanagement.h>
#include <DWayland/Client/registry.h>
#include <DWayland/Client/connection_thread.h>
#include <DWayland/Client/event_queue.h>
#else
#include <KF5/KWayland/Client/connection_thread.h>
#include <KF5/KWayland/Client/clientmanagement.h>
#include <KF5/KWayland/Client/event_queue.h>
#include <KF5/KWayland/Client/registry.h>
#endif

using namespace KWayland::Client;
#endif // USE_DEEPIN_WAYLAND

DGUI_USE_NAMESPACE

class windowStateThread : public QThread
{
    Q_OBJECT

public:
    explicit windowStateThread(bool isWayland, QObject *parent = nullptr);
    ~windowStateThread();
    QList<DForeignWindow *> workspaceWindows() const;
    void run();

signals:
    void someWindowFullScreen();

private:
#ifdef USE_DEEPIN_WAYLAND
    /**
     * @brief wayland获取屏幕窗口信息的安装注册函数
     * @param registry
     */
    void setupRegistry(Registry *registry);

    /**
     * @brief wayland获取屏幕窗口信息
     * @param m_windowStates
     */
    void waylandwindowinfo(const QVector<ClientManagement::WindowState> &m_windowStates);
#endif

private:
#ifdef USE_DEEPIN_WAYLAND
    // 获取wayland窗口信息相关。 wayland获取窗口的方法对于x11有很大的区别
    QThread *m_connectionThread = nullptr;
    EventQueue *m_eventQueue = nullptr;
    ConnectionThread *m_connectionThreadObject;
    Compositor *m_compositor = nullptr;
    PlasmaWindowManagement *m_windowManagement = nullptr;
    ClientManagement *m_clientManagement = nullptr;
    QVector<ClientManagement::WindowState> m_windowStates;
    /**
      * @brief mips平台创建缓存文件的路径
      */
    std::string m_tempPath;
#endif

};

#endif  // WINDOWSTATETHREAD_H
