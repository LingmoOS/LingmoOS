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
 *
 */
#ifndef PENDINGAPPINFOQUEUE_H
#define PENDINGAPPINFOQUEUE_H

#include "pending-app-info.h"

#include <QThread>
#include <QMutexLocker>
#include <QTimer>
#include <QVector>

namespace LingmoUISearch{

class PendingAppInfoQueue : public QThread
{
    Q_OBJECT
public:
    static PendingAppInfoQueue &getAppInfoQueue();
    void enqueue(const PendingAppInfo &appInfo);

protected:
    void run() override;

private:
    explicit PendingAppInfoQueue(QObject *parent = nullptr);
    PendingAppInfoQueue(const PendingAppInfoQueue&) = delete;
    PendingAppInfoQueue& operator =(const PendingAppInfoQueue&) = delete;
    ~PendingAppInfoQueue();

    void processCache();

private:
    static QMutex s_mutex;

    QTimer *m_minProcessTimer = nullptr;
    QTimer *m_maxProcessTimer = nullptr;

    QVector<LingmoUISearch::PendingAppInfo> m_cache;
    QVector<LingmoUISearch::PendingAppInfo> m_pendingAppInfos;

    int m_handleTimes;

Q_SIGNALS:
    void startMinProcessTimer();
    void startMaxProcessTimer();
    void stopTimer();
};
}



#endif // PENDINGAPPINFOQUEUE_H
