/*
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: zhangpengfei <zhangpengfei@kylinos.cn>
 *
 */
#ifndef PENDINGFILEQUEUE_H
#define PENDINGFILEQUEUE_H

#include <QObject>
#include <QVector>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include "pending-file.h"

class RemoteFileEventHelper;

namespace LingmoUISearch {
class PendingFileQueue : public QThread
{
    Q_OBJECT
public:
    static PendingFileQueue *getInstance(QObject *parent = nullptr);

    ~PendingFileQueue();
    //This method will block until current cache has been processed.
    //Do not do enqueue operation in other thread while this method is running.
    void forceFinish();
    void enqueue(const PendingFile& file);
    QTimer *m_cacheTimer = nullptr;
    QTimer *m_minProcessTimer = nullptr;

protected:
    void run() override;

Q_SIGNALS:
    void cacheTimerStart();
    void minProcessTimerStart();
    void timerStop();

    void filesUpdate(const QVector<PendingFile>&);

    void signalFileEvent(int eventType, const QString &arg1, const QString &arg2);
private:
    void processCache();
    explicit PendingFileQueue(QObject *parent = nullptr);

    QVector<PendingFile> m_cache;
    QMutex m_mutex;

    QThread *m_timerThread = nullptr;
    int m_enqueuetimes = 0;

    RemoteFileEventHelper *m_fileEventHelper = nullptr;
};
}
#endif // PENDINGFILEQUEUE_H
