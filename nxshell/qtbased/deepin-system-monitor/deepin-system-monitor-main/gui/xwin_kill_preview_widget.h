// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef XWIN_KILL_PREVIEW_WIDGET_H
#define XWIN_KILL_PREVIEW_WIDGET_H

#include "../config.h"
#include <QWidget>
#include <QObject>
#include <QGuiApplication>
#include <QDebug>
#include <QThread>
#include <QTime>
// system
#include <unistd.h>

// 使用 USE_DEEPIN_WAYLAND 宏决定编译时是否支持 wayland
// 如果支持，用全局变量 WaylandCentered 运行时控制
#ifdef USE_DEEPIN_WAYLAND
#ifdef DWAYLAND
#include <DWayland/Client/clientmanagement.h>
#include <DWayland/Client/registry.h>
#include <DWayland/Client/connection_thread.h>
#include <DWayland/Client/event_queue.h>
#else
#include <KF5/KWayland/Client/clientmanagement.h>
#include <KF5/KWayland/Client/registry.h>
#include <KF5/KWayland/Client/connection_thread.h>
#include <KF5/KWayland/Client/event_queue.h>
#endif

using namespace KWayland::Client;
#endif // USE_DEEPIN_WAYLAND

namespace core {
namespace wm {
class WMInfo;
}
} // namespace core
class XWinKillPreviewBackgroundWidget;

/**
 * @brief Preview widget to show when interaction with user while kill application by window requested
 */
class XWinKillPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit XWinKillPreviewWidget(QWidget *parent = nullptr);
    /**
     * @brief Destructor
     */
    ~XWinKillPreviewWidget() override;

#ifdef USE_DEEPIN_WAYLAND
    /**
     * @brief Print current window states
     * @param QVector of window state which contains pid,windowid,resourceName,geometry,etc
     */
    void print_window_states(const QVector<ClientManagement::WindowState> &m_windowStates);
#endif // USE_DEEPIN_WAYLAND

signals:
    /**
     * @brief Window clicked signal
     * @param pid Gui process's pid of whose widgets has been clicked
     */
    void windowClicked(pid_t pid);
    /**
     * @brief Cursor updated signal
     * @param cursor Updated cursor instance
     */
    void cursorUpdated(const QCursor &cursor);

public slots:

protected:
    /**
     * @brief mousePressEvent Mouse press event handler
     * @param event Mouse press event
     */
    void mousePressEvent(QMouseEvent *event) override;
    /**
     * @brief mouseMoveEvent Mouse move event handler
     * @param event Mouse move event
     */
    void mouseMoveEvent(QMouseEvent *event) override;
    /**
     * @brief keyPressEvent Key press event handler
     * @param event Key press event
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    /**
     * @brief Initialize ui components
     */
    void initUI();
    /**
     * @brief Initialize connections
     */
    void initConnections();

#ifdef USE_DEEPIN_WAYLAND
    QVector<ClientManagement::WindowState> getAllWindowStates();
#endif

private:
    // Window manager (x11) instance
    core::wm::WMInfo *m_wminfo;
    // List of background widgets, per screen each
    QList<XWinKillPreviewBackgroundWidget *> m_backgroundList;
    // List of screens
    QList<QScreen *> m_screens;

    // Styled kill cursor
    QCursor m_killCursor;
    // Default cursor style
    QCursor m_defaultCursor;

#ifdef USE_DEEPIN_WAYLAND
    //Vector of window states
    QVector<ClientManagement::WindowState> m_windowStates;
    //Kwayland Client Management
    ClientManagement *m_clientManagement = nullptr;
    //Regist m_clientManagement
    void setupRegistry(Registry *registry);
    QThread *m_connectionThread;
    //Thread connet to wayland
    ConnectionThread *m_connectionThreadObject;
    //Event Queue
    EventQueue *m_eventQueue = nullptr;
    //wayland compositor
    Compositor *m_compositor = nullptr;
    //oringinal Kwayland window management
    PlasmaWindowManagement *m_windowManagement = nullptr;
#endif // USE_DEEPIN_WAYLAND
};

#endif // XWIN_KILL_PREVIEW_WIDGET_H
