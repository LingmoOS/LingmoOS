// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WAYLANDMOUSESIMULATOR_H
#define WAYLANDMOUSESIMULATOR_H

#include <QObject>
#include <QApplication>
#include <QTimer>
#include <QDebug>

// KWayland
#include <DWayland/Client/connection_thread.h>
#include <DWayland/Client/event_queue.h>
#include <DWayland/Client/registry.h>
#include <DWayland/Client/output.h>
#include <DWayland/Client/remote_access.h>
#include <DWayland/Client/fakeinput.h>

/**
 * @brief The WaylandMouseSimulator class: wayland下鼠标模拟器
 */
class WaylandMouseSimulator : public QObject
{
    Q_OBJECT
public:
    explicit WaylandMouseSimulator(QObject *parent = nullptr);
    ~WaylandMouseSimulator();

    /**
     * @brief 单列模式
     * @return
     */
    static WaylandMouseSimulator *instance();
    /**
     * @brief 初始化wayland的相关链接
     */
    void initWaylandScrollThread();

    /**
     * @brief wayland下设置光标在屏幕中的绝对位置
     * @param pos
     */
    void setCursorPoint(QPointF pos);

    /**
     * @brief 模拟wayland下鼠标滚轮自动滚动
     */
    void doWaylandAutoScroll(); //自动滚动

public slots:
    /**
     * @brief wayland模拟手动滚动鼠标滚动
     * @param direction
     */
    void slotManualScroll(float direction); // 手动滚动

protected:
    /**
     * @brief releaseWaylandScrollThread
     */
    void releaseWaylandScrollThread();

protected slots:
    /**
     * @brief setupRegistry
     */
    void setupRegistry(); //注册
    /**
     * @brief setupFakeinput
     * @param name
     * @param version
     */
    void setupFakeinput(quint32 name, quint32 version); // 初始化fakeinput
private:
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::EventQueue *m_queue;
    KWayland::Client::Registry *m_registry;
    KWayland::Client::FakeInput *m_fakeinput;
};

#endif // WAYLANDMOUSESIMULATOR_H
