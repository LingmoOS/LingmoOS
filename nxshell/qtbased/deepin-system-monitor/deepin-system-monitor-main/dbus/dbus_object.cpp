// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbus_object.h"
#include "gui/main_window.h"
#include "application.h"
#include "ddlog.h"
#include <QDBusConnection>
#include <QDebug>

#define DBUS_SERVER "com.deepin.systemMonitor"
#define DBUS_SERVER_PATH "/com/deepin/systemMonitor"
using namespace DDLog;
QMutex DBusObject::mutex;
QAtomicPointer<DBusObject> DBusObject::instance;

DBusObject &DBusObject::getInstance()
{
    if (instance.testAndSetOrdered(nullptr, nullptr)) {
        QMutexLocker locker(&mutex);

        instance.testAndSetOrdered(nullptr, new DBusObject);
    }
    return *instance;
}

bool DBusObject::registerOrNotify()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (!dbus.registerService(DBUS_SERVER)) {
        QDBusInterface notification(DBUS_SERVER, DBUS_SERVER_PATH, DBUS_SERVER, QDBusConnection::sessionBus());
        QList<QVariant> args;
        QString error = notification.callWithArgumentList(QDBus::Block, "handleWindow", args).errorMessage();
        if (!error.isEmpty())
            qCInfo(app) << error;
        return false;
    }

    dbus.registerObject(DBUS_SERVER_PATH, this, QDBusConnection::ExportScriptableSlots);

    return true;
}

void DBusObject::unRegister()
{
    internalMutex.lockForRead();
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterService(DBUS_SERVER);
    internalMutex.unlock();
}

void DBusObject::handleWindow()
{
    internalMutex.lockForRead();
    MainWindow *mw = gApp->mainWindow();
    mw->setWindowState(mw->windowState() & ~Qt::WindowMinimized);
    mw->raise();
    mw->activateWindow();
    internalMutex.unlock();
}

DBusObject::DBusObject(QObject *parent)
    : QObject(parent)
{
}

DBusObject::~DBusObject()
{
}
