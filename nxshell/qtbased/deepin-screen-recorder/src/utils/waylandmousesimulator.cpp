// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "waylandmousesimulator.h"

#define SCROLL_DOWN 15.0

WaylandMouseSimulator::WaylandMouseSimulator(QObject *parent) : QObject(parent)
    , m_connection(nullptr)
    , m_queue(nullptr)
    , m_registry(nullptr)
    , m_fakeinput(nullptr)
{
    //initWaylandScrollThread();
}

WaylandMouseSimulator::~WaylandMouseSimulator()
{
    releaseWaylandScrollThread();
}

WaylandMouseSimulator *WaylandMouseSimulator::instance()
{
    static WaylandMouseSimulator *waylandMouseSimulator;
    if (!waylandMouseSimulator) {
        waylandMouseSimulator = new WaylandMouseSimulator();
    }
    return waylandMouseSimulator;
}

void WaylandMouseSimulator::setCursorPoint(QPointF pos)
{
    if (m_fakeinput != nullptr) {
        m_fakeinput->requestPointerMoveAbsolute(pos);
    }
}

// 手动滚动模拟
void WaylandMouseSimulator::slotManualScroll(float direction)
{
    if (m_fakeinput != nullptr) {
        // direction 滚动方向
        m_fakeinput->requestPointerAxisForCapture(Qt::Vertical, static_cast<double>(direction));
    }
}

// 自动滚动执行
void WaylandMouseSimulator::doWaylandAutoScroll()
{
    if (m_fakeinput != nullptr) {
        // Qt::Vertical 垂直滚动，15.0 代表向下滚动
        m_fakeinput->requestPointerAxisForCapture(Qt::Vertical, SCROLL_DOWN);
    }
}

// 初始化队列、开启注册器
void WaylandMouseSimulator::setupRegistry()
{
    m_queue = new KWayland::Client::EventQueue(this);
    m_queue->setup(m_connection);

    m_registry = new KWayland::Client::Registry(this);
    qDebug() << "==========setupRegistry=====";
    connect(m_registry, &KWayland::Client::Registry::fakeInputAnnounced, this, &WaylandMouseSimulator::setupFakeinput);
    //connect(m_registry, &KWayland::Client::Registry::fakeInputRemoved, this, &WaylandMonitor::removeOutput);
//    connect(m_registry, &KWayland::Client::Registry::interfacesAnnounced, this, [this] {
//        m_registryInitialized = true;
//    });

    m_registry->create(m_connection);
    m_registry->setEventQueue(m_queue);
    m_registry->setup();
}

// 初始化Fakeinput
void WaylandMouseSimulator::setupFakeinput(quint32 name, quint32 version)
{
    if (m_fakeinput == nullptr) {
        m_fakeinput = new KWayland::Client::FakeInput(this);
        m_fakeinput->setup(m_registry->bindFakeInput(name, version));
        m_fakeinput->authenticate(qAppName(), "wayland scroll monitor");
    }
}

// 滚动模拟初始化流程
void WaylandMouseSimulator::initWaylandScrollThread()
{
    // 初始化ConnectionThread
    m_connection = new KWayland::Client::ConnectionThread(this);
    // 注册器
    connect(m_connection, &KWayland::Client::ConnectionThread::connected, this, &WaylandMouseSimulator::setupRegistry, Qt::QueuedConnection);
    connect(m_connection, &KWayland::Client::ConnectionThread::connectionDied, this, [this] {
        if (m_queue)
        {
            delete m_queue;
            m_queue = nullptr;
        }

        m_connection->deleteLater();
        m_connection = nullptr;
    });
    connect(m_connection, &KWayland::Client::ConnectionThread::failed, this, [this] {
        m_connection->deleteLater();
        m_connection = nullptr;
    });
    m_connection->initConnection();
}

// 释放注册资源
void WaylandMouseSimulator::releaseWaylandScrollThread()
{

    if (m_fakeinput) {
        m_fakeinput->release();
        delete m_fakeinput;
        m_fakeinput = nullptr;
    }

    if (m_registry) {
        m_registry->release();
        delete  m_registry;
        m_registry = nullptr;
    }

    if (m_queue) {
        m_queue->release();
        delete m_queue;
        m_queue = nullptr;
    }

    if (m_connection) {
        m_connection->deleteLater();
        m_connection = nullptr;
    }
}

