// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowstatethread.h"
#include "qapplication.h"

#include <DWindowManagerHelper>
#include <QMutexLocker>

windowStateThread::windowStateThread(bool isWayland, QObject *parent /*= nullptr*/)
    : QThread (parent)
{
    setObjectName("windowStateThread");
#ifdef USE_DEEPIN_WAYLAND
    if (isWayland) {
        qInfo() << __FUNCTION__ << __LINE__  << "KF5_WAYLAND_FLAGE_ON is open!!";
        //wayland自动识别窗口
        m_connectionThread = new QThread(this);
        m_connectionThreadObject = new ConnectionThread();
        connect(m_connectionThreadObject, &ConnectionThread::connected, this,
        [this] {
            m_eventQueue = new EventQueue(this);
            m_eventQueue->setup(m_connectionThreadObject);

            Registry *registry = new Registry(this);
            setupRegistry(registry);
        },
        Qt::QueuedConnection
               );
        m_connectionThreadObject->moveToThread(m_connectionThread);
        m_connectionThread->start();
        m_connectionThreadObject->initConnection();
    }
#endif
}

windowStateThread::~windowStateThread()
{
#ifdef USE_DEEPIN_WAYLAND
    if (m_connectionThread != nullptr) {
        m_connectionThread->terminate();
        m_connectionThread->wait();
    }
#endif
}

void windowStateThread::run()
{
    while (!isInterruptionRequested()) {
//        QMutexLocker locker(&m_mutex);
        //获取当前工作区域内所有的窗口
        auto list = workspaceWindows();

        foreach(DForeignWindow *window, list) {
            if (window && window->windowState() == Qt::WindowState::WindowFullScreen) {
                emit someWindowFullScreen();
            }
        }
        //线程休眠1秒
        std::this_thread::sleep_for(std::chrono::seconds(1));
#ifdef UNITTEST
        break;
#endif
    }
    qInfo() << "windowStateThread end";
}

QList<DForeignWindow *> windowStateThread::workspaceWindows() const
{
    QList<DForeignWindow *> windowList;
    QList<WId> currentApplicationWindowList;
    const QWindowList &list = qApp->allWindows();

    currentApplicationWindowList.reserve(list.size());

    for (auto window : list) {
        if (window->property("_q_foreignWinId").isValid()) {
            continue;
        }

        currentApplicationWindowList.append(window->winId());
    }

    QVector<quint32> wmClientList = DWindowManagerHelper::instance()->currentWorkspaceWindowIdList();

    bool currentWindow = false;
    for (WId wid : wmClientList) {
        if (currentApplicationWindowList.contains(wid)){
            currentWindow = true;
            continue;
        }
        if (false == currentWindow){
            continue;
        }
        if (DForeignWindow *w = DForeignWindow::fromWinId(wid)) {
            windowList << w;
        }
    }
    return windowList;
}

#ifdef USE_DEEPIN_WAYLAND
void windowStateThread::setupRegistry(Registry *registry)
{
    connect(registry, &Registry::compositorAnnounced, this,
    [this, registry](quint32 name, quint32 version) {
        m_compositor = registry->createCompositor(name, version, this);
    }
           );

    connect(registry, &Registry::clientManagementAnnounced, this,
    [this, registry](quint32 name, quint32 version) {
        m_clientManagement = registry->createClientManagement(name, version, this);
        connect(m_clientManagement, &ClientManagement::windowStatesChanged, this,
        [this] {
            m_windowStates = m_clientManagement->getWindowStates();
            this->waylandwindowinfo(m_windowStates);
        }
               );
    }
           );

    connect(registry, &Registry::interfacesAnnounced, this,
    [this] {
        Q_ASSERT(m_compositor);
        Q_ASSERT(m_clientManagement);
        m_windowStates = m_clientManagement->getWindowStates();
        this->waylandwindowinfo(m_windowStates);
    }
           );

    registry->setEventQueue(m_eventQueue);
    registry->create(m_connectionThreadObject);
    registry->setup();
}

void windowStateThread::waylandwindowinfo(const QVector<ClientManagement::WindowState> &windowStates)
{
    if (windowStates.count() == 0) {
        return;
    }

    bool aheandInCamera = false;
    QRect rect;
    for (int i = 0; i < windowStates.count(); ++i) {
        if (QString(windowStates[i].resourceName) != "deepin-camera") {
            if (!aheandInCamera)
                continue;
            else {
                if (windowStates[i].isFullScreen) {
                    QRect fullscreenRect(windowStates[i].geometry.x, windowStates[i].geometry.y,
                                         windowStates[i].geometry.width, windowStates[i].geometry.height);
                    if (fullscreenRect.contains(rect))
                        emit someWindowFullScreen();
                }
            }
        } else {
            aheandInCamera = true;
            rect = QRect(windowStates[i].geometry.x, windowStates[i].geometry.y,
                         windowStates[i].geometry.width, windowStates[i].geometry.height);
        }
    }
}
#endif

