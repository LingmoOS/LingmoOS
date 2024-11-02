/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef INDEXSCHEDULER_H
#define INDEXSCHEDULER_H

#include <QObject>
#include <QThreadPool>
#include <QAtomicInt>
#include <QEvent>
#include "file-watcher.h"
#include "index-status-recorder.h"
#include "common.h"
#include "batch-indexer.h"
namespace LingmoUISearch {

class IndexScheduler : public QObject
{
    Q_OBJECT

public:
    enum IndexerState {
        Startup,
        Running,
        Idle,
        Stop
    };
    Q_ENUM(IndexerState)

    explicit IndexScheduler(QObject *parent = nullptr);

    Q_INVOKABLE IndexerState getIndexState();
    /**
     * 强制立刻执行增量更新
     * @param target 需要更新的数据库，只能对已经启动索引的数据库生效
     */
    Q_INVOKABLE void forceUpdate(BatchIndexer::Targets target);

Q_SIGNALS:
    void stateChange(IndexerState);
    void process(IndexType type, uint all, uint finished);
    void basicIndexStart();
    void contentIndexStart();
    void ocrContentIndexStart();
    void aiIndexStart();
    void basicIndexDone(bool success);
    void contentIndexDone(bool success);
    void ocrContentIndexDone(bool success);
    void aiIndexDone(bool success);
    void done();

private Q_SLOTS:
    /**
     * @brief addNewPath
     * @param folders 要添加索引的目录
     * @param blackList 要添加索引的目录下的黑名单
     */
    void addNewPath(const QString &folders, const QStringList& blackList = QStringList());
    /**
     * @brief removeIndex
     * @param folders 要移除索引的目录
     */
    void removeIndex(const QString& folders);
    void start(BatchIndexer::Targets target);
    void stop(BatchIndexer::Targets target);
    void fileIndexEnable(bool enable);
    void contentIndexEnable(bool enable);
    void ocrContentIndexEnable(bool enable);
    void aiIndexEnable(bool enable);
    void updateIndex(const QVector<PendingFile>& files);
    void batchIndexerFinished(BatchIndexer::WorkMode mode, BatchIndexer::Targets targets);
    void updateFinished();
    bool isIdle();
    void onBasicIndexDone(BatchIndexer::WorkMode mode);
    void onContentIndexDone(BatchIndexer::WorkMode mode);
    void onOcrContentIndexDone(BatchIndexer::WorkMode mode);
    void onAiIndexDone(BatchIndexer::WorkMode mode);
    void checkIfStartsWaiting();
    void installWatches();

private:
    /**
     * @brief checkAndRebuild
     * 检查数据库状态，数据库状态处于 IndexStatusRecorder::State::Error 时，开始重建任务。
     * @return 返回需要重建的数据库
     */
    BatchIndexer::Targets checkAndRebuild(BatchIndexer::Targets target = BatchIndexer::All);
    void startIndexJob(const QStringList &folders, const QStringList &blackList, BatchIndexer::WorkMode mode, BatchIndexer::Targets target);
    FileWatcher m_fileWatcher;
    IndexStatusRecorder *m_statusRecorder = nullptr;
    FileIndexerConfig *m_config = nullptr;
    IndexerState m_state;
    QAtomicInt m_indexStop;
    QAtomicInt m_contentIndexStop;
    QAtomicInt m_ocrContentIndexStop;
    QAtomicInt m_aiIndexStop;
    QThreadPool m_threadPool;
    QThreadPool m_threadPoolForRTUpdate; //用于实时更新索引的线程池

    //等待完成的工作数量
    quint64 m_basicIndexPendingWorkCount = 0;
    quint64 m_contentIndexPendingWorkCount = 0;
    quint64 m_ocrContentIndexPendingWorkCount= 0;
    quint64 m_aiIndexPendingWorkCount= 0;
    quint64 m_updatePendingWorkCount = 0;
    quint64 m_addNewPathPendingWorkCount = 0;

    //是否有启动操作正在等待已有工作结束
    bool m_basicIndexStartWaiting = false;
    bool m_contentIndexStartWaiting = false;
    bool m_ocrContentIndexStartWaiting = false;
    bool m_aiIndexStartWaiting = false;

    //监听是否已安装
    bool m_watchInstalled = false;

};
}
#endif // INDEXSCHEDULER_H
