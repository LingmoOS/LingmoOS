/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "pending-app-info-queue.h"
#include "app-db-manager.h"

using namespace LingmoUISearch;

QMutex PendingAppInfoQueue::s_mutex;

PendingAppInfoQueue &PendingAppInfoQueue::getAppInfoQueue()
{
    static PendingAppInfoQueue instance;
    return instance;
}

void PendingAppInfoQueue::enqueue(const PendingAppInfo &appInfo)
{
    QMutexLocker locker(&s_mutex);
    m_handleTimes++;
    int index = m_cache.lastIndexOf(appInfo);
    if (index == -1) {
        m_cache << appInfo;
    } else {
        //刷新数据库操作直接插入
        if (appInfo.handleType() == PendingAppInfo::RefreshDataBase) {
            m_cache << appInfo;
        }
        //要插入项操作类型为删除，清除之前所有操作，替换为删除
        if (appInfo.handleType() == PendingAppInfo::Delete) {
            m_cache.removeAll(appInfo);
            m_cache << appInfo;
        } else if (m_cache[index].handleType() == PendingAppInfo::Delete) {
            //先删后建，分别处理
            if (appInfo.handleType() == PendingAppInfo::Insert) {
                m_cache << appInfo;
            }
        } else if (appInfo.handleType() == PendingAppInfo::SetValue) {
            m_cache << appInfo;
        } else if (m_cache[index].handleType() <= PendingAppInfo::UpdateLocaleData
                   and appInfo.handleType() <= PendingAppInfo::UpdateLocaleData) {
            //类型为insert, updateall, updatelocaledata时，设置为优先级高的操作类型
            if (m_cache[index].handleType() > appInfo.handleType()) {
                m_cache.remove(index);
                m_cache << appInfo;
            }
        } else {
            m_cache[index].merge(appInfo);
        }

        /*
        //只要操作类型为delete，直接覆盖
        if (m_cache[index].handleType() == PendingAppInfo::Delete
                or appInfo.handleType() == PendingAppInfo::Delete) {
            m_cache[index].setHandleType(PendingAppInfo::Delete);

        //已插入项操作类型为对所有desktop文件相关数据进行操作
        } else if (m_cache[index].handleType() < PendingAppInfo::UpdateLocaleData
                   and appInfo.handleType() < PendingAppInfo::UpdateLocaleData) {
            //设置为优先级高的操作类型
            if (m_cache[index].handleType() > appInfo.handleType()) {
                m_cache[index].setHandleType(appInfo);
            }
        } else {
            m_cache[index].merge(appInfo);
        }
        */
    }

    //启动定时器
    if (!m_maxProcessTimer->isActive()) {
        qDebug() << "start max timer";
        Q_EMIT this->startMaxProcessTimer();
    }
    Q_EMIT this->startMinProcessTimer();
}

void PendingAppInfoQueue::run()
{
    exec();
}

PendingAppInfoQueue::PendingAppInfoQueue(QObject *parent) : QThread(parent)
{
    this->start();

    m_handleTimes = 0;

    m_minProcessTimer = new QTimer;
    m_maxProcessTimer = new QTimer;

    m_minProcessTimer->setInterval(500);
    m_maxProcessTimer->setInterval(5*1000);

    m_minProcessTimer->setSingleShot(true);
    m_maxProcessTimer->setSingleShot(true);

    m_minProcessTimer->moveToThread(this);
    m_maxProcessTimer->moveToThread(this);

    connect(this, SIGNAL(startMinProcessTimer()), m_minProcessTimer, SLOT(start()));
    connect(this, SIGNAL(startMaxProcessTimer()), m_maxProcessTimer, SLOT(start()));

    connect(this, &PendingAppInfoQueue::stopTimer, m_minProcessTimer, &QTimer::stop);
    connect(this, &PendingAppInfoQueue::stopTimer, m_maxProcessTimer, &QTimer::stop);

    connect(m_minProcessTimer, &QTimer::timeout, this, &PendingAppInfoQueue::processCache);
    connect(m_maxProcessTimer, &QTimer::timeout, this, &PendingAppInfoQueue::processCache);
}

PendingAppInfoQueue::~PendingAppInfoQueue()
{
    this->quit();
    this->wait();
    if (m_minProcessTimer) {
        delete m_minProcessTimer;
    }
    if (m_maxProcessTimer) {
        delete m_maxProcessTimer;
    }
}

void PendingAppInfoQueue::processCache()
{
    s_mutex.lock();
    qDebug() << "handle apps:" << m_handleTimes;
    m_handleTimes = 0;
    m_cache.swap(m_pendingAppInfos);
    s_mutex.unlock();

    if (m_pendingAppInfos.isEmpty()) {
        qWarning() << "No App Info will be process, I'll stop process cache of app queue.";
        return;
    }

    for (const PendingAppInfo &info : m_pendingAppInfos) {
        PendingAppInfo::HandleTypes handleTypes = info.handleType();
        if (handleTypes <= PendingAppInfo::UpdateLocaleData || handleTypes == PendingAppInfo::RefreshDataBase || handleTypes == PendingAppInfo::SetValue) {
            switch (handleTypes) {
            case PendingAppInfo::Delete:
                AppDBManager::getInstance()->handleDBItemDelete(info.path());
                break;
            case PendingAppInfo::Insert:
                AppDBManager::getInstance()->handleDBItemInsert(info.path());
                break;
            case PendingAppInfo::UpdateAll:
                AppDBManager::getInstance()->handleDBItemUpdate(info.path());
                break;
            case PendingAppInfo::UpdateLocaleData:
                AppDBManager::getInstance()->handleLocaleDataUpdate(info.path());
                break;
            case PendingAppInfo::RefreshDataBase:
                AppDBManager::getInstance()->handleDataBaseRefresh(info.pathsNeedRefreshData(), info.dbVersionNeedUpdate());
                break;
            case PendingAppInfo::SetValue:
                AppDBManager::getInstance()->handleValueSet(info.value2BSet());
                break;
            default:
                break;
            }
        } else {
            if (handleTypes & PendingAppInfo::Insert) {
                AppDBManager::getInstance()->handleDBItemInsert(info.path());
            }
            if (handleTypes & PendingAppInfo::UpdateAll) {
                AppDBManager::getInstance()->handleDBItemUpdate(info.path());
            }
            if (handleTypes & PendingAppInfo::UpdateLocaleData) {
                AppDBManager::getInstance()->handleLocaleDataUpdate(info.path());
            }
            if (handleTypes & PendingAppInfo::UpdateLaunchTimes) {
                AppDBManager::getInstance()->handleLaunchTimesUpdate(info.path(), info.launchTimes());
            }
            if (handleTypes & PendingAppInfo::UpdateFavorites) {
                AppDBManager::getInstance()->handleFavoritesStateUpdate(info.path(), info.favoritesState());
            }
            if (handleTypes & PendingAppInfo::UpdateTop) {
                AppDBManager::getInstance()->handleTopStateUpdate(info.path(), info.topState());
            }
        }
    }
    Q_EMIT AppDBManager::getInstance()->finishHandleAppDB();

    m_pendingAppInfos.clear();
    m_pendingAppInfos.squeeze();
}

