/*
    SPDX-FileCopyrightText: 2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "BackendsManager.h"

#include "BackendsConfig.h"

// Include backends directly
#include "TestBackend.h"
#include "backends/dbus/DBusHelperProxy.h"

#include <QDebug>
#include <QDir>
#include <QPluginLoader>
#include <QThread>

namespace KAuth
{
AuthBackend *BackendsManager::auth = nullptr;
QHash<QThread *, HelperProxy *> proxiesForThreads = QHash<QThread *, HelperProxy *>();

BackendsManager::BackendsManager()
{
}

void BackendsManager::init()
{
    if (!auth) {
        // Load the test backend
        auth = new TestBackend;
    }

    if (!proxiesForThreads.contains(QThread::currentThread())) {
        // Load the test helper backend
        proxiesForThreads.insert(QThread::currentThread(), new DBusHelperProxy(QDBusConnection::sessionBus()));
    }
}

AuthBackend *BackendsManager::authBackend()
{
    if (!auth) {
        init();
    }

    return auth;
}

HelperProxy *BackendsManager::helperProxy()
{
    if (!proxiesForThreads.contains(QThread::currentThread())) {
        qDebug() << "Creating new proxy for thread" << QThread::currentThread();
        init();
    }

    return proxiesForThreads[QThread::currentThread()];
}

void BackendsManager::setProxyForThread(QThread *thread, HelperProxy *proxy)
{
    qDebug() << "Adding proxy for thread" << thread;

    proxiesForThreads.insert(thread, proxy);
}

} // namespace Auth
