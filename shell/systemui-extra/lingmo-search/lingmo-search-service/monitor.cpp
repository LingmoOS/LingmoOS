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
#include "monitor.h"
#include <QMetaEnum>
#include "file-indexer-config.h"
using namespace LingmoUISearch;
Monitor::Monitor(IndexScheduler *scheduler, QObject *parent) : MonitorSource(parent),
                                                               m_scheduler(scheduler),
                                                               m_basicDatabase(DataBaseType::Basic),
                                                               m_contentDatabase(DataBaseType::Content),
                                                               m_ocrContentDatabase(DataBaseType::OcrContent)
{
    connect(scheduler, &IndexScheduler::stateChange, this, &Monitor::onIndexStateChanged);
    connect(scheduler, &IndexScheduler::process, this, &Monitor::processUpdate);
    connect(scheduler, &IndexScheduler::basicIndexDone, this, [&](bool success){
        Q_EMIT basicIndexDone(success);
        m_basicIndexUpdating = false;
        Q_EMIT basicIndexUpdatingChanged(m_basicIndexUpdating);
    });
    connect(scheduler, &IndexScheduler::contentIndexDone, this, [&](bool success){
        Q_EMIT contentIndexDone(success);
        m_contentIndexUpdating = false;
        Q_EMIT contentIndexUpdatingChanged(m_contentIndexUpdating);
    });
    connect(scheduler, &IndexScheduler::ocrContentIndexDone, this, [&](bool success){
        Q_EMIT ocrContentIndexDone(success);
        m_ocrContentIndexUpdating = false;
        Q_EMIT ocrContentIndexUpdatingChanged(m_ocrContentIndexUpdating);
    });
    connect(scheduler, &IndexScheduler::aiIndexDone, this, [&](bool success){
        Q_EMIT aiIndexDone(success);
        m_aiIndexUpdating = false;
        Q_EMIT aiIndexUpdatingChanged(m_aiIndexUpdating);
    });
    connect(scheduler, &IndexScheduler::basicIndexStart, this, [&](){
        Q_EMIT basicIndexStart();
        m_basicIndexUpdating = true;
        Q_EMIT basicIndexUpdatingChanged(m_basicIndexUpdating);
    });
    connect(scheduler, &IndexScheduler::contentIndexStart, this, [&](){
        Q_EMIT contentIndexStart();
        m_contentIndexUpdating = true;
        Q_EMIT contentIndexUpdatingChanged(m_contentIndexUpdating);
    });
    connect(scheduler, &IndexScheduler::ocrContentIndexStart, this, [&](){
        Q_EMIT ocrContentIndexStart();
        m_ocrContentIndexUpdating = true;
        Q_EMIT ocrContentIndexUpdatingChanged(m_ocrContentIndexUpdating);
    });
    connect(scheduler, &IndexScheduler::aiIndexStart, this, [&](){
        Q_EMIT aiIndexStart();
        m_aiIndexUpdating = true;
        Q_EMIT aiIndexUpdatingChanged(m_aiIndexUpdating);
    });
    connect(FileIndexerConfig::getInstance(), &FileIndexerConfig::appendIndexDir, this, [&](){
        m_currentIndexPaths = FileIndexerConfig::getInstance()->currentIndexableDir();
        Q_EMIT currentIndexPathsChanged(m_currentIndexPaths);
    });
    connect(FileIndexerConfig::getInstance(), &FileIndexerConfig::removeIndexDir, this, [&](){
        m_currentIndexPaths = FileIndexerConfig::getInstance()->currentIndexableDir();
        Q_EMIT currentIndexPathsChanged(m_currentIndexPaths);
    });
}

QStringList Monitor::currentIndexPaths() const
{
    return FileIndexerConfig::getInstance()->currentIndexableDir();
}

QString Monitor::indexState() const
{
    QMetaEnum metaEnum = QMetaEnum::fromType<IndexScheduler::IndexerState>();
    return QString::fromLocal8Bit(metaEnum.valueToKey(m_scheduler->getIndexState()));
}

uint Monitor::basicIndexSize() const
{
    return m_basicIndexSize;
}

uint Monitor::contentIndexSize() const
{
    return m_contentIndexSize;
}

uint Monitor::ocrContentIndexSize() const
{
    return m_ocrContentIndexSize;
}

uint Monitor::aiIndexSize() const
{
    return m_aiIndexSize;
}

uint Monitor::basicIndexProgress() const
{
    return m_basicIndexProgress;
}

uint Monitor::contentIndexProgress() const
{
    return m_contentIndexProgress;
}

uint Monitor::ocrContentIndexProgress() const
{
    return m_ocrContentIndexProgress;
}

uint Monitor::aiIndexProgress() const
{
    return m_aiIndexProgress;
}


uint Monitor::basicIndexDocNum() const
{
    return m_basicIndexDocNum;
}

uint Monitor::contentIndexDocNum() const
{
    return m_contentIndexDocNum;
}

uint Monitor::ocrContentIndexDocNum() const
{
    return m_ocrContentIndexDocNum;
}

uint Monitor::aiIndexDocNum() const
{
    return m_aiIndexDocNum;
}

void Monitor::onIndexStateChanged(IndexScheduler::IndexerState state)
{
    if(state == IndexScheduler::IndexerState::Idle || state == IndexScheduler::IndexerState::Stop) {
        m_basicIndexDocNum = m_basicDatabase.getIndexDocCount();
        Q_EMIT basicIndexDocNumChanged(m_basicIndexDocNum);
        m_contentIndexDocNum = m_contentDatabase.getIndexDocCount();
        Q_EMIT contentIndexDocNumChanged(m_contentIndexDocNum);
        m_ocrContentIndexDocNum = m_ocrContentDatabase.getIndexDocCount();
        Q_EMIT ocrContentIndexDocNumChanged(m_ocrContentDatabase.getIndexDocCount());
    }
    QMetaEnum metaEnum = QMetaEnum::fromType<IndexScheduler::IndexerState>();
    Q_EMIT indexStateChanged(QString::fromLocal8Bit(metaEnum.valueToKey(state)));
}

void Monitor::processUpdate(IndexType type, uint all, uint finished)
{
    switch (type) {
        case IndexType::Basic: {
            if(m_basicIndexSize != all) {
                m_basicIndexSize = all;
                Q_EMIT basicIndexSizeChanged(m_basicIndexSize);
            }
            if(m_basicIndexProgress != finished) {
                m_basicIndexProgress = finished;
                Q_EMIT basicIndexProgressChanged(m_basicIndexProgress);
            }
            uint count = m_basicDatabase.getIndexDocCount();
            if(m_basicIndexDocNum != count) {
                m_basicIndexDocNum = count;
                Q_EMIT basicIndexDocNumChanged(m_basicIndexDocNum);
            }
            break;
        }
        case IndexType::Contents: {
            if(m_contentIndexSize != all) {
                m_contentIndexSize = all;
                Q_EMIT contentIndexSizeChanged(m_contentIndexSize);
            }
            if(m_contentIndexProgress != finished) {
                m_contentIndexProgress = finished;
                Q_EMIT contentIndexProgressChanged(m_contentIndexProgress);
            }
            uint count = m_contentDatabase.getIndexDocCount();
            if(m_contentIndexDocNum != count) {
                m_contentIndexDocNum = count;
                Q_EMIT contentIndexDocNumChanged(m_contentIndexDocNum);
            }
            break;
        }
        case IndexType::OCR: {
            if(m_ocrContentIndexSize != all) {
                m_ocrContentIndexSize = all;
                Q_EMIT ocrContentIndexSizeChanged(m_ocrContentIndexSize);
            }
            if(m_ocrContentIndexProgress != finished) {
                m_ocrContentIndexProgress = finished;
                Q_EMIT ocrContentIndexProgressChanged(m_ocrContentIndexProgress);
            }
            uint count = m_contentDatabase.getIndexDocCount();
            if(m_ocrContentIndexDocNum != count) {
                m_ocrContentIndexDocNum = count;
                Q_EMIT ocrContentIndexDocNumChanged(m_ocrContentDatabase.getIndexDocCount());
            }
            break;
        }
        case IndexType::Ai: {
            if(m_aiIndexSize != all) {
                m_aiIndexSize = all;
                Q_EMIT aiIndexSizeChanged(m_aiIndexSize);
            }
            if(m_aiIndexProgress != finished) {
                m_aiIndexProgress = finished;
                Q_EMIT aiIndexProgressChanged(m_aiIndexProgress);
            }
            //AI索引暂不支持文档数量信号
            break;
        }
        default:
            break;
    }
}

bool Monitor::basicIndexUpdating() const
{
    return m_basicIndexUpdating;
}

bool Monitor::contentIndexUpdating() const
{
    return m_contentIndexUpdating;
}

bool Monitor::ocrContentIndexUpdating() const
{
    return m_ocrContentIndexUpdating;
}

bool Monitor::aiIndexUpdating() const
{
    return m_aiIndexUpdating;
}
