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
#ifndef SEARCHRESULTMANAGER_H
#define SEARCHRESULTMANAGER_H

#include <QQueue>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include "search-plugin-manager.h"

namespace LingmoUISearch {

class ReceiveResultThread : public QThread {
    Q_OBJECT
public:
    ReceiveResultThread(DataQueue<SearchPluginIface::ResultInfo> * resultQueue, QObject * parent = nullptr);
    ~ReceiveResultThread() = default;
    void stop();
protected:
    void run() override;

private:
    DataQueue<SearchPluginIface::ResultInfo> * m_resultQueue;
    QTimer *m_timer = nullptr;

Q_SIGNALS:
    void gotResultInfo(const SearchPluginIface::ResultInfo&);

};

class SearchResultManager : public QObject
{
    Q_OBJECT
public:
    explicit SearchResultManager(const QString &plugin_id, QObject *parent = nullptr);
    ~SearchResultManager() = default;

public Q_SLOTS:
    void startSearch(const QString &);
    void stopSearch();

private:
    void initConnections();
    QString m_pluginId;
    DataQueue<SearchPluginIface::ResultInfo> * m_resultQueue;
    ReceiveResultThread * m_getResultThread = nullptr;

Q_SIGNALS:
    void gotResultInfo(const SearchPluginIface::ResultInfo&);
};
}

#endif // SEARCHRESULTMANAGER_H
