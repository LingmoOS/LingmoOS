// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailwidgetmanager.h"
#include <QTimer>
QMutex DetailWidgetManager::mutex;
QAtomicPointer<DetailWidgetManager> DetailWidgetManager::instance;

DetailWidgetManager &DetailWidgetManager::getInstance()
{
    if (instance.testAndSetOrdered(nullptr, nullptr))
    {
        QMutexLocker locker(&mutex);

        instance.testAndSetOrdered(nullptr, new DetailWidgetManager);
    }
    return *instance;
}

void DetailWidgetManager::jumpDetailWidget(const QString &msgCode)
{
    internalMutex.lockForRead();
    QString detailWidgetName = msgCode;

//    QTimer::singleShot(50, this, [=]() {
//        emit sigJumpToProcessWidget("MSG_PROCESS");
//    });
    emit sigJumpToDetailWidget(detailWidgetName);
    internalMutex.unlock();
}

void DetailWidgetManager::jumpProcessWidget(const QString &msgCode) {
    internalMutex.lockForRead();
    QString detailWidgetName = msgCode;
    emit sigJumpToProcessWidget(detailWidgetName);
    internalMutex.unlock();
}


DetailWidgetManager::DetailWidgetManager(QObject *parent)
    :QObject (parent)
{
//    connect(SystemMonitor::instance(), &SystemMonitor::statInfoUpdated, this, &DetailWidgetManager::sigDataUpdate);
}

DetailWidgetManager::~DetailWidgetManager()
{

}
