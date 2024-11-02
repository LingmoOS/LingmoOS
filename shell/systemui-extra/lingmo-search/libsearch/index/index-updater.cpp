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
#include "index-updater.h"
#include <malloc.h>
#include <QJsonArray>
#include "writable-database.h"
#include "basic-indexer.h"
#include "file-indexer-config.h"
#include "file-content-indexer.h"
#include "common.h"
#include "file-utils.h"
#include "compatible-define.h"
#include "index-status-recorder.h"
#include "ai-indexer.h"
#include "pending-file-queue.h"
using namespace LingmoUISearch;
IndexUpdater::IndexUpdater(const QVector<PendingFile>& files,
                           QAtomicInt& indexstop,
                           QAtomicInt& contentIndexstop,
                           QAtomicInt& contentIndexOcrStop,
                           QAtomicInt& aiIndexStop)
        : m_cache(files),
          m_indexStop(&indexstop),
          m_contentIndexStop(&contentIndexstop),
          m_contentIndexOcrStop(&contentIndexOcrStop),
          m_aiIndexStop(&aiIndexStop)
{
    connect(this, &IndexUpdater::indexUpdated, PendingFileQueue::getInstance(), &PendingFileQueue::signalFileEvent);
}
void IndexUpdater::updateIndex()
{
    //fix me: How should I delete metadata of files below a folder
    //which has been deleted(When a file watcher signal comes which only contains folder info)?
    if(FileIndexerConfig::getInstance()->isFileIndexEnable() && !m_indexStop->LOAD) {
        IndexStatusRecorder::getInstance()->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::RealTimeUpdating);
        WritableDatabase basicDb(DataBaseType::Basic);
        if(!basicDb.open()) {
            qWarning() << "Basic db open failed, fail to update index";
            return;
        }
        qDebug() << "===update basic index===";
        for(const PendingFile& file : m_cache) {
            // notify lingmo-idm
            if (file.isCreated()) {
                Q_EMIT indexUpdated(1, file.path(), nullptr);
            }

            if(file.shouldRemoveIndex()) {
                qDebug() << "| remove:" <<file.path();
                basicDb.removeDocument(file.path());
                if(file.isDir()) {
                    basicDb.removeChildrenDocument(file.path());
                }
            } else {
                qDebug() << "| index:" <<file.path();
                BasicIndexer indexer(file.path());
                if(indexer.index()) {
                    basicDb.addDocument(indexer.document());
                }
            }

        }
        basicDb.commit();
        IndexStatusRecorder::getInstance()->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Ready);
        qDebug() << "===finish update basic index===";
    }
}

void IndexUpdater::run()
{
    updateIndex();
    if(IndexStatusRecorder::getInstance()->getStatus(INDEX_DATABASE_STATE_KEY) == IndexStatusRecorder::State::RealTimeUpdating) {
        IndexStatusRecorder::getInstance()->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Ready);
    }
    updateContentIndex();
    if(IndexStatusRecorder::getInstance()->getStatus(CONTENT_INDEX_DATABASE_STATE_KEY) == IndexStatusRecorder::State::RealTimeUpdating) {
        IndexStatusRecorder::getInstance()->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Ready);
    }
    updateOcrContentIndex();
    if(IndexStatusRecorder::getInstance()->getStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY) == IndexStatusRecorder::State::RealTimeUpdating) {
        IndexStatusRecorder::getInstance()->setStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Ready);
    }
    updateAiIndex();
    if(IndexStatusRecorder::getInstance()->getStatus(AI_INDEX_DATABASE_STATE_KEY) == IndexStatusRecorder::State::RealTimeUpdating) {
        IndexStatusRecorder::getInstance()->setStatus(AI_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Ready);
    }
    m_cache.clear();
    m_cache.shrink_to_fit();
    malloc_trim(0);
    Q_EMIT done();
}

void IndexUpdater::updateContentIndex()
{
    if(FileIndexerConfig::getInstance()->isContentIndexEnable() && !m_contentIndexStop->LOAD) {
        IndexStatusRecorder::getInstance()->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::RealTimeUpdating);
        WritableDatabase contentDb(DataBaseType::Content);
        if(!contentDb.open()) {
            qWarning() << "Content db open failed, fail to update index";
            return;
        }

        qDebug() << "===update content index===";
        int size = 0;
        for(PendingFile file : m_cache) {
            if(m_contentIndexStop->LOAD) {
                qDebug() << "Content index update interrupted";
                return;
            }
            QString suffix = QFileInfo(file.path()).suffix();
            if(file.shouldRemoveIndex()) {
                qDebug() << "| remove:" <<file.path();
                if(file.isDir()) {
                    contentDb.removeChildrenDocument(file.path());
                } else if(FileIndexerConfig::getInstance()->contentIndexTarget()[suffix]) {
                    contentDb.removeDocument(file.path());
                }
            } else if(FileIndexerConfig::getInstance()->contentIndexTarget()[suffix] && !file.isDir()) {
                if(FileUtils::isEncrypedOrUnsupport(file.path(), suffix)) {
                    if(file.isModified() || file.isMoveTo()) {
                        contentDb.removeDocument(file.path());
                    }
                    continue;
                }
                qDebug() << "| index:" <<file.path();
                fileContentIndexer indexer(file.path());
                if(indexer.index()) {
                    contentDb.addDocument(indexer.document());
                    ++size;
                } else if(file.isModified() || file.isMoveTo()){
                    contentDb.removeDocument(file.path());
                }
            }
            if(size >= 30) {
                contentDb.commit();
                qDebug() << "30 finished.";
                size = 0;
            }
        }
        contentDb.commit();
        qDebug() << "===finish update content index===";
    }
}

void IndexUpdater::updateOcrContentIndex()
{
    if(FileIndexerConfig::getInstance()->isOCREnable() && !m_contentIndexOcrStop->LOAD) {
        IndexStatusRecorder::getInstance()->setStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::RealTimeUpdating);
        WritableDatabase contentDb(DataBaseType::OcrContent);
        if(!contentDb.open()) {
            qWarning() << "Ocr content db open failed, fail to update index";
            return;
        }

        qDebug() << "===update ocr content index===";
        int size = 0;
        for(PendingFile file : m_cache) {
            if(m_contentIndexOcrStop->LOAD) {
                qDebug() << "Ocr content index update interrupted";
                return;
            }
            QString suffix = QFileInfo(file.path()).suffix();
            if(file.shouldRemoveIndex()) {
                qDebug() << "| remove:" <<file.path();
                if(file.isDir()) {
                    contentDb.removeChildrenDocument(file.path());
                } else if(FileIndexerConfig::getInstance()->ocrContentIndexTarget()[suffix]) {
                    contentDb.removeDocument(file.path());
                }
            } else if(FileIndexerConfig::getInstance()->ocrContentIndexTarget()[suffix] && !file.isDir()) {
                if(FileUtils::isEncrypedOrUnsupport(file.path(), suffix)) {
                    if(file.isModified() || file.isMoveTo()) {
                        contentDb.removeDocument(file.path());
                    }
                    continue;
                }
                qDebug() << "| index:" <<file.path();
                fileContentIndexer indexer(file.path());
                if(indexer.index()) {
                    contentDb.addDocument(indexer.document());
                    ++size;
                } else if(file.isModified() || file.isMoveTo()){
                    contentDb.removeDocument(file.path());
                }
            }
            if(size >= 10) {
                contentDb.commit();
                qDebug() << "10 finished.";
                size = 0;
            }
        }
        contentDb.commit();
        qDebug() << "===finish update ocr content index===";
    }
}

void IndexUpdater::updateAiIndex()
{
    if(FileIndexerConfig::getInstance()->isAiIndexEnable() && !m_aiIndexStop->LOAD) {
        IndexStatusRecorder::getInstance()->setStatus(AI_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::RealTimeUpdating);
        AiIndexer indexer;
        if(!indexer.createSession()) {
            qWarning() << "Creat session failed, fail to update ai index";
            return;
        }
        qDebug() << "===update ai index===";
        for(PendingFile file : m_cache) {
            if(m_aiIndexStop->LOAD) {
                qDebug() << "Ai index update interrupted";
                return;
            }

            auto update = [&](bool(AiIndexer::*creat)(const QJsonArray &)){
                QString type;
                if(!indexer.checkFileSupported(file.path(), type)) {
                    if(file.isModified() || file.isMoveTo()) {
                        qDebug() << "| remove:" <<file.path();
                        indexer.deleteFileIndex({file.path()});
                    }
                    return;
                }
                qDebug() << "| index:" <<file.path();
                if(!(indexer.*creat)(QJsonArray{
                        QJsonObject{
                                {QStringLiteral("filepath"), file.path()},
                                {QStringLiteral("fileformat"), type}
                        }})){
                    qWarning() << "Ai index failed for" << file.path();
                }
            };

            QString suffix = QFileInfo(file.path()).suffix();
            if(file.shouldRemoveIndex()) {
                if(file.isDir() || (FileIndexerConfig::getInstance()->aiIndexFileTarget()[suffix] || FileIndexerConfig::getInstance()->aiIndexImageTarget()[suffix])) {
                    qDebug() << "| remove:" <<file.path();
                    indexer.deleteFileIndex({file.path()});
                }
            } else if(file.isModified() || file.isMoveTo()) {
                qDebug() << file.path() << "Modified.";
                update(&AiIndexer::update);
            } else if(FileIndexerConfig::getInstance()->aiIndexFileTarget()[suffix] && !file.isDir()) {
                update(&AiIndexer::addTextFileIndex);
            } else if (FileIndexerConfig::getInstance()->aiIndexImageTarget()[suffix] && !file.isDir()) {
                update(&AiIndexer::addImageFileIndex);
            }
        }
        qDebug() << "===finish update ai content index===";
    }
}
