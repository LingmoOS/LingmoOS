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
}

windowStateThread::~windowStateThread()
{

}

void windowStateThread::run()
{
    while (!isInterruptionRequested()) {
//        QMutexLocker locker(&m_mutex);
        //获取当前工作区域内所有的窗口
        auto list = workspaceWindows();

        foreach(DForeignWindow *window, list) {
            if (window && window->windowState() == Qt::WindowState::WindowFullScreen) {
                //去掉全屏的窗口信号处理
                //emit someWindowFullScreen();
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


