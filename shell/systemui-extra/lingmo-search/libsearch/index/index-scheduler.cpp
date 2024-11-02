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
#include "index-scheduler.h"
#include <QCoreApplication>
#include <QMetaEnum>
#include "index-updater.h"
#include "compatible-define.h"
#include "ai-indexer.h"

using namespace LingmoUISearch;
IndexScheduler::IndexScheduler(QObject *parent) :
        QObject(parent),
        m_statusRecorder(IndexStatusRecorder::getInstance()),
        m_config(FileIndexerConfig::getInstance()),
        m_state(Startup),
        m_indexStop(0),
        m_contentIndexStop(0),
        m_ocrContentIndexStop(0),
        m_aiIndexStop(0)
{
    qRegisterMetaType<IndexerState>("IndexerState");
    qRegisterMetaType<BatchIndexer::WorkMode>("BatchIndexer::WorkMode");
    qRegisterMetaType<BatchIndexer::WorkMode>("WorkMode");
    qRegisterMetaType<BatchIndexer::Targets>("Targets");
    m_threadPool.setMaxThreadCount(1);
    m_threadPoolForRTUpdate.setMaxThreadCount(1);
    connect(&m_fileWatcher, &FileWatcher::filesUpdate, this, &IndexScheduler::updateIndex);
    connect(m_config, &FileIndexerConfig::fileIndexEnableStatusChanged, this, &IndexScheduler::fileIndexEnable);
    connect(m_config, &FileIndexerConfig::contentIndexEnableStatusChanged, this, &IndexScheduler::contentIndexEnable);
    connect(m_config, &FileIndexerConfig::contentIndexEnableOcrStatusChanged, this, &IndexScheduler::ocrContentIndexEnable);
    connect(m_config, &FileIndexerConfig::aiIndexEnableStatusChanged, this, &IndexScheduler::aiIndexEnable);

    connect(m_config, &FileIndexerConfig::appendIndexDir, this, &IndexScheduler::addNewPath);
    connect(m_config, &FileIndexerConfig::removeIndexDir, this, &IndexScheduler::removeIndex);
    m_state = Startup;
    BatchIndexer::Targets targets = BatchIndexer::Target::None;
    if(m_config->isFileIndexEnable()) {
        targets |= BatchIndexer::Target::Basic;
    } else {
        m_indexStop.fetchAndStoreRelaxed(1);
    }
    if(m_config->isContentIndexEnable()) {
        targets |= BatchIndexer::Target::Content;
    } else {
        m_contentIndexStop.fetchAndStoreRelaxed(1);
    }
    if(m_config->isOCREnable()) {
        targets |= BatchIndexer::Target::Ocr;
    } else {
        m_ocrContentIndexStop.fetchAndStoreRelaxed(1);
    }
    if(m_config->isAiIndexEnable()) {
        targets |= BatchIndexer::Target::Ai;
    } else {
        m_aiIndexStop.fetchAndStoreRelaxed(1);
    }
    start(targets);
}

void IndexScheduler::addNewPath(const QString &folders, const QStringList &blackList)
{
    if(m_indexStop.LOAD && m_contentIndexStop.LOAD && m_ocrContentIndexStop.LOAD && m_aiIndexStartWaiting) {
        qDebug() << "Index Scheduler is being stopped, add operation will be executed when started up next time.";
        return;
    }

    BatchIndexer::Targets target = BatchIndexer::Target::None;
    if(m_config->isFileIndexEnable()) {
        target |= BatchIndexer::Target::Basic;
        m_statusRecorder->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Updating);
    }
    if(m_config->isContentIndexEnable()) {
        target |= BatchIndexer::Target::Content;
        m_statusRecorder->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Updating);
    }
    if(m_config->isOCREnable()) {
        target |= BatchIndexer::Target::Ocr;
        m_statusRecorder->setStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Updating);
    }
    if(m_config->isAiIndexEnable()) {
        target |= BatchIndexer::Target::Ai;
        m_statusRecorder->setStatus(AI_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Updating);
    }
    BatchIndexer::WorkMode mode = BatchIndexer::WorkMode::Add;
    startIndexJob(QStringList() << folders, blackList, mode, target);
    if(BatchIndexer::Target::None != target) {
        m_fileWatcher.addWatch(folders, blackList);
    }
}

void IndexScheduler::removeIndex(const QString &folders)
{
    if(m_indexStop.LOAD && m_contentIndexStop.LOAD && m_ocrContentIndexStop.LOAD && m_aiIndexStop.LOAD) {
        qDebug() << "Index Scheduler is being stopped, remove operation will be executed when started up next time.";
        return;
    }
    m_fileWatcher.removeWatch(folders, true);
}

void IndexScheduler::stop(BatchIndexer::Targets target)
{
    if(target & BatchIndexer::Target::Basic) {
        m_indexStop.fetchAndStoreRelaxed(1);
        m_statusRecorder->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Off);
        qDebug() << "File index has been stopped.";
    }
    if(target & BatchIndexer::Target::Content) {
        m_contentIndexStop.fetchAndStoreRelaxed(1);
        m_statusRecorder->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Off);
        qDebug() << "File content index has been stopped.";
    }
    if(target & BatchIndexer::Target::Ocr) {
        m_ocrContentIndexStop.fetchAndStoreRelaxed(1);
        m_statusRecorder->setStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Off);
        qDebug() << "File ocr content index has been stopped.";
    }
    if(target & BatchIndexer::Target::Ai) {
        m_aiIndexStop.fetchAndStoreRelaxed(1);
        m_statusRecorder->setStatus(AI_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Off);
        qDebug() << "Ai index has been stopped.";
    }
    if(m_indexStop.LOAD && m_contentIndexStop.LOAD && m_ocrContentIndexStop.LOAD && m_aiIndexStop.LOAD) {
        m_fileWatcher.removeWatch();
        m_watchInstalled = false;
        m_threadPool.clear();
        m_threadPool.waitForDone(-1);
        m_threadPoolForRTUpdate.clear();
        m_threadPoolForRTUpdate.waitForDone(-1);
        m_state = Stop;
        qDebug() << "Index scheduler has been stopped.";
        Q_EMIT stateChange(m_state);
    }
}

IndexScheduler::IndexerState IndexScheduler::getIndexState()
{
    return m_state;
}

void IndexScheduler::forceUpdate(BatchIndexer::Targets target)
{
    m_config->syncIndexFileTypes();
    stop(target);
    if(target & BatchIndexer::Basic) {
        fileIndexEnable(true);
    }
    if(target & BatchIndexer::Content) {
        contentIndexEnable(true);
    }
    if(target & BatchIndexer::Ocr) {
        ocrContentIndexEnable(true);
    }
    if(target & BatchIndexer::Ai) {
        aiIndexEnable(true);
    }
}

void IndexScheduler::start(BatchIndexer::Targets target)
{
    qDebug() << "Index scheduler start." << QMetaEnum::fromType<BatchIndexer::Targets>().valueToKeys(target) ;

    //打开异步控制开关
    if(target & BatchIndexer::Basic) {
        m_indexStop.fetchAndStoreRelaxed(0);
    }
    if(target & BatchIndexer::Content) {
        m_contentIndexStop.fetchAndStoreRelaxed(0);
    }
    if(target & BatchIndexer::Ocr) {
        m_ocrContentIndexStop.fetchAndStoreRelaxed(0);
    }
    if(target & BatchIndexer::Ai) {
        m_aiIndexStop.fetchAndStoreRelaxed(0);
    }
    if(target == BatchIndexer::None) {
        return;
    }

    //检查是否有数据库需要重建并且执行重建
    BatchIndexer::Targets rebuiltTarget = checkAndRebuild(target);

    BatchIndexer::WorkMode mode = BatchIndexer::WorkMode::Update;
    BatchIndexer::Targets startTarget = BatchIndexer::Target::None;

    //如果数据库被执行过重建，那么跳过增量更新步骤。
    if((target & BatchIndexer::Target::Basic) && !(rebuiltTarget & BatchIndexer::Target::Basic)) {
        startTarget |= BatchIndexer::Target::Basic;
        m_statusRecorder->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Updating);
    }
    if((target & BatchIndexer::Target::Content) && !(rebuiltTarget & BatchIndexer::Target::Content)) {
        startTarget |= BatchIndexer::Target::Content;
        m_statusRecorder->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Updating);
    }
    if(target & BatchIndexer::Ocr && !(rebuiltTarget & BatchIndexer::Target::Ocr)) {
        startTarget |= BatchIndexer::Target::Ocr;
        m_statusRecorder->setStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Updating);
    }
    if(target & BatchIndexer::Ai && !(rebuiltTarget & BatchIndexer::Target::Ai)) {
        startTarget |= BatchIndexer::Target::Ai;
        m_statusRecorder->setStatus(AI_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Updating);
    }
    startIndexJob(m_config->currentIndexableDir(), m_config->currentBlackListOfIndex(), mode, startTarget);

    //启动监听
    installWatches();
}

BatchIndexer::Targets IndexScheduler::checkAndRebuild(BatchIndexer::Targets target)
{
    BatchIndexer::WorkMode mode = BatchIndexer::WorkMode::Rebuild;
    BatchIndexer::Targets rebuildTarget = BatchIndexer::Target::None;
    if((target & BatchIndexer::Target::Basic) && m_config->isFileIndexEnable() &&
            (m_statusRecorder->getStatus(INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::Error
        || !m_statusRecorder->versionCheck(INDEX_DATABASE_VERSION_KEY, INDEX_DATABASE_VERSION))) {
        qDebug() << "Basic database need rebuild";
        rebuildTarget |= BatchIndexer::Target::Basic;
        m_statusRecorder->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Initializing);
    }
    if((target & BatchIndexer::Target::Content) && m_config->isContentIndexEnable() &&
            (m_statusRecorder->getStatus(CONTENT_INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::Error
        || !m_statusRecorder->versionCheck(CONTENT_DATABASE_VERSION_KEY, CONTENT_DATABASE_VERSION))) {
        qDebug() << "Content database need rebuild";
        rebuildTarget |= BatchIndexer::Target::Content;
        m_statusRecorder->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Initializing);
    }
    if((target & BatchIndexer::Target::Ocr) && m_config->isOCREnable() &&
       (m_statusRecorder->getStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::Error
        || !m_statusRecorder->versionCheck(OCR_CONTENT_DATABASE_VERSION_KEY, OCR_CONTENT_DATABASE_VERSION))) {
        qDebug() << "Ocr content database need rebuild";
        rebuildTarget |= BatchIndexer::Target::Ocr;
        m_statusRecorder->setStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Initializing);
    }
    startIndexJob(m_config->currentIndexableDir(), m_config->currentBlackListOfIndex(), mode, rebuildTarget);
    return rebuildTarget;
}

void IndexScheduler::startIndexJob(const QStringList& folders,const QStringList& blackList, BatchIndexer::WorkMode mode, BatchIndexer::Targets target)
{
    if(BatchIndexer::Target::None != target) {
        if(mode == BatchIndexer::WorkMode::Add) {
            m_addNewPathPendingWorkCount++;
        }
        if(target & BatchIndexer::Basic) {
            m_basicIndexPendingWorkCount++;
        }
        if(target & BatchIndexer::Content) {
            m_contentIndexPendingWorkCount++;
        }
        if(target & BatchIndexer::Ocr) {
            m_ocrContentIndexPendingWorkCount++;
        }
        if(target & BatchIndexer::Ai) {
            m_aiIndexPendingWorkCount++;
        }

        m_state = Running;
        Q_EMIT stateChange(m_state);

        auto *indexer = new BatchIndexer(folders, blackList,
                                                 m_indexStop, m_contentIndexStop, m_ocrContentIndexStop, m_aiIndexStop,
                                                 mode, target);
        connect(indexer, &BatchIndexer::done, this, &IndexScheduler::batchIndexerFinished, Qt::QueuedConnection);
        connect(indexer, &BatchIndexer::progress, this, &IndexScheduler::process, Qt::QueuedConnection);
        connect(indexer, &BatchIndexer::basicIndexDone, this, &IndexScheduler::onBasicIndexDone, Qt::QueuedConnection);
        connect(indexer, &BatchIndexer::contentIndexDone, this, &IndexScheduler::onContentIndexDone, Qt::QueuedConnection);
        connect(indexer, &BatchIndexer::ocrContentIndexDone, this, &IndexScheduler::onOcrContentIndexDone, Qt::QueuedConnection);
        connect(indexer, &BatchIndexer::aiIndexDone, this, &IndexScheduler::onAiIndexDone, Qt::QueuedConnection);
        connect(indexer, &BatchIndexer::basicIndexStart, this, &IndexScheduler::basicIndexStart, Qt::QueuedConnection);
        connect(indexer, &BatchIndexer::contentIndexStart, this, &IndexScheduler::contentIndexStart, Qt::QueuedConnection);
        connect(indexer, &BatchIndexer::ocrContentIndexStart, this, &IndexScheduler::ocrContentIndexStart, Qt::QueuedConnection);
        connect(indexer, &BatchIndexer::aiIndexStart, this, &IndexScheduler::aiIndexStart, Qt::QueuedConnection);
        m_threadPool.start(indexer);
    }
}

void IndexScheduler::fileIndexEnable(bool enable)
{
    if(enable) {
        if(m_basicIndexPendingWorkCount == 0) {
            start(BatchIndexer::Basic);
        } else {
            m_basicIndexStartWaiting = true;
        }
    } else {
        stop(BatchIndexer::Basic);
    }
}

void IndexScheduler::contentIndexEnable(bool enable)
{
    if(enable) {
        if(m_contentIndexPendingWorkCount == 0) {
            start(BatchIndexer::Content);
        } else {
            m_contentIndexStartWaiting = true;
        }
    } else {
        stop(BatchIndexer::Content);
    }
}

void IndexScheduler::ocrContentIndexEnable(bool enable)
{
    if(enable) {
        if(m_ocrContentIndexPendingWorkCount == 0) {
            start(BatchIndexer::Ocr);
        } else {
            m_ocrContentIndexStartWaiting = true;
        }
    } else {
        stop(BatchIndexer::Ocr);
    }
}

void IndexScheduler::aiIndexEnable(bool enable)
{
    if(enable) {
        if(m_aiIndexPendingWorkCount == 0) {
            start(BatchIndexer::Ai);
        } else {
            m_aiIndexStartWaiting = true;
        }
    } else {
        stop(BatchIndexer::Ai);
    }
}

void IndexScheduler::updateIndex(const QVector<PendingFile> &files)
{
    qDebug() << "updateIndex=====";
    m_updatePendingWorkCount++;

    m_state = Running;
    Q_EMIT stateChange(m_state);

    auto *updateJob = new IndexUpdater(files, m_indexStop, m_contentIndexStop, m_ocrContentIndexStop, m_aiIndexStop);
    connect(updateJob, &IndexUpdater::done, this, &IndexScheduler::updateFinished, Qt::QueuedConnection);
    m_threadPoolForRTUpdate.start(updateJob);
}

void IndexScheduler::batchIndexerFinished(BatchIndexer::WorkMode mode, BatchIndexer::Targets targets)
{
    if(mode == BatchIndexer::WorkMode::Add) {
        m_addNewPathPendingWorkCount--;
    }
    if(isIdle()) {
        m_state = Idle;
        Q_EMIT stateChange(m_state);
    }
}

void IndexScheduler::updateFinished()
{
    m_updatePendingWorkCount--;
    if(isIdle()) {
        m_state = Idle;
        Q_EMIT stateChange(m_state);
    }
}

bool IndexScheduler::isIdle()
{
    return m_basicIndexPendingWorkCount == 0
           && m_contentIndexPendingWorkCount == 0
           && m_ocrContentIndexPendingWorkCount == 0
           && m_aiIndexPendingWorkCount == 0
           && m_updatePendingWorkCount == 0
           && m_addNewPathPendingWorkCount == 0;
}

void IndexScheduler::onBasicIndexDone(BatchIndexer::WorkMode mode)
{
    Q_UNUSED(mode)
    m_basicIndexPendingWorkCount--;

    bool success = false;
    if(m_statusRecorder->getStatus(INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::Initializing ||
       m_statusRecorder->getStatus(INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::Updating) {
        m_statusRecorder->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Ready);
        success = true;
    } else if (m_statusRecorder->getStatus(INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::RealTimeUpdating ) {
        success = true;
    }
    Q_EMIT basicIndexDone(success);
    checkIfStartsWaiting();
}

void IndexScheduler::onContentIndexDone(BatchIndexer::WorkMode mode)
{
    Q_UNUSED(mode)
    m_contentIndexPendingWorkCount--;

    bool success = false;
    if(m_statusRecorder->getStatus(CONTENT_INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::Initializing ||
       m_statusRecorder->getStatus(CONTENT_INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::Updating) {
        m_statusRecorder->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Ready);
        success = true;
    } else if (m_statusRecorder->getStatus(CONTENT_INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::RealTimeUpdating ) {
        success = true;
    }
    Q_EMIT contentIndexDone(success);
    checkIfStartsWaiting();
}

void IndexScheduler::onOcrContentIndexDone(BatchIndexer::WorkMode mode)
{
    Q_UNUSED(mode)
    m_ocrContentIndexPendingWorkCount--;

    bool success = false;
    if(m_statusRecorder->getStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::Initializing ||
       m_statusRecorder->getStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::Updating) {
        m_statusRecorder->setStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Ready);
        success = true;
    } else if (m_statusRecorder->getStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::RealTimeUpdating ) {
        success = true;
    }
    Q_EMIT ocrContentIndexDone(success);
    checkIfStartsWaiting();
}
void IndexScheduler::onAiIndexDone(BatchIndexer::WorkMode mode)
{
    Q_UNUSED(mode)
    m_aiIndexPendingWorkCount--;

    bool success = false;
    if(m_statusRecorder->getStatus(AI_INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::Initializing ||
       m_statusRecorder->getStatus(AI_INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::Updating) {
        m_statusRecorder->setStatus(AI_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Ready);
        success = true;
    } else if (m_statusRecorder->getStatus(AI_INDEX_DATABASE_STATE_KEY).toInt() == IndexStatusRecorder::State::RealTimeUpdating ) {
        success = true;
    }
    Q_EMIT aiIndexDone(success);
    checkIfStartsWaiting();
}

void IndexScheduler::checkIfStartsWaiting()
{
    BatchIndexer::Targets targets = BatchIndexer::None;
    if(m_basicIndexStartWaiting && !m_basicIndexPendingWorkCount) {
        targets |= BatchIndexer::Basic;
        m_basicIndexStartWaiting = false;
    }
    if(m_contentIndexStartWaiting && !m_contentIndexPendingWorkCount) {
        targets |= BatchIndexer::Content;
        m_contentIndexStartWaiting = false;
    }
    if(m_ocrContentIndexStartWaiting && !m_ocrContentIndexPendingWorkCount) {
        targets |= BatchIndexer::Ocr;
        m_ocrContentIndexStartWaiting = false;
    }
    if(m_aiIndexStartWaiting && !m_aiIndexPendingWorkCount) {
        targets |= BatchIndexer::Ai;
        m_aiIndexStartWaiting = false;
    }
    if(targets != BatchIndexer::None) {
        start(targets);
    }
}

void IndexScheduler::installWatches()
{
    if(m_config->isFileIndexEnable() || m_config->isContentIndexEnable() || m_config->isOCREnable() || m_config->isAiIndexEnable()) {
        if(!m_watchInstalled) {
            m_fileWatcher.installWatches();
            m_watchInstalled = true;
        }
    }
}
