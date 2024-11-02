/*
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */
#include "search-result-manager.h"
#include <QDeadlineTimer>

using namespace LingmoUISearch;
SearchResultManager::SearchResultManager(const QString& plugin_id, QObject *parent) : QObject(parent)
{
    m_pluginId = plugin_id;
    m_resultQueue = new DataQueue<SearchPluginIface::ResultInfo>;
    m_getResultThread = new ReceiveResultThread(m_resultQueue, this);
    initConnections();
}

void SearchResultManager::startSearch(const QString &keyword)
{
    qDebug()<<m_pluginId<<"started";
    if(! m_getResultThread->isRunning()) {
        m_getResultThread->start();
    }
    m_resultQueue->clear();
    SearchPluginIface *plugin = SearchPluginManager::getInstance()->getPlugin(m_pluginId);
    plugin->KeywordSearch(keyword, m_resultQueue);
}

/**
 * @brief SearchResultManager::stopSearch 停止搜索，开始一次新搜索前或主界面退出时执行
 */
void SearchResultManager::stopSearch()
{
    if(m_getResultThread->isRunning()) {
        m_getResultThread->stop();
    }
    SearchPluginIface *plugin = SearchPluginManager::getInstance()->getPlugin(m_pluginId);
    plugin->stopSearch();
    qDebug() << m_pluginId << "stopped";
    m_resultQueue->clear();
}

void SearchResultManager::initConnections()
{
    connect(m_getResultThread, &ReceiveResultThread::gotResultInfo, this, &SearchResultManager::gotResultInfo);
}

ReceiveResultThread::ReceiveResultThread(DataQueue<SearchPluginIface::ResultInfo> * resultQueue, QObject *parent): QThread(parent)
{
    m_resultQueue = resultQueue;
}

void ReceiveResultThread::stop()
{
    this->requestInterruption();
    this->wait();
}

void ReceiveResultThread::run()
{
    QDeadlineTimer deadline(25000);
    while(!isInterruptionRequested()) {
        SearchPluginIface::ResultInfo oneResult = m_resultQueue->tryDequeue();
        if(oneResult.name.isEmpty()) {
            if(deadline.remainingTime()) {
                msleep(100);
            } else {
                this->requestInterruption();
            }
        } else {
            deadline.setRemainingTime(25000);
            Q_EMIT gotResultInfo(oneResult);
        }
    }
    m_resultQueue->clear();
}
