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
#ifndef MONITOR_H
#define MONITOR_H

#include <QObject>
#include "index-scheduler.h"
#include "database.h"
#include "rep_monitor_source.h"
namespace LingmoUISearch {
/**
 * @brief The Monitor class
 * 用于监控索引状态
 */
class Monitor : public MonitorSource
{
    Q_OBJECT

public:
    explicit Monitor(IndexScheduler* scheduler, QObject *parent = nullptr);
    /**
     * @brief currentIndexPaths
     * @return 当前索引的所有目录
     */
    QStringList currentIndexPaths() const override;
    /**
     * @brief indexState
     * @return 当前索引调度器的状态
     */
    QString indexState() const override;
    /**
     * @brief basicIndexSize
     * @return 当前需要处理的基础索引数量
     */
    uint basicIndexSize() const override;
    /**
     * @brief contentIndexSize
     * @return 当前需要处理的内容索引数量
     */
    uint contentIndexSize() const override;
    /**
     * @brief ocrContentIndexSize
     * @return 当前需要处理的OCR索引数量
     */
    uint ocrContentIndexSize() const override;
    /**
     * @brief aiIndexSize
     * @return 当前需要处理的AI索引数量
     */
    uint aiIndexSize() const override;
    /**
     * @brief basicIndexProgress
     * @return 基础索引进度
     */
    uint basicIndexProgress() const override;
    /**
     * @brief contentIndexProgress
     * @return 内容索引进度
     */
    uint contentIndexProgress() const override;
    /**
     * @brief ocrContentIndexProgress
     * @return ocr索引进度
     */
    uint ocrContentIndexProgress() const override;
    /**
     * @brief aiIndexProgress
     * @return ai索引进度
     */
    uint aiIndexProgress() const override;
    /**
     * @brief basicIndexDocNum
     * @return 基础索引完成的总文档数
     */
    uint basicIndexDocNum() const override;
    /**
     * @brief contentIndexDocNum
     * @return 内容索引完成的总文档数
     */
    uint contentIndexDocNum() const override;
    /**
     * @brief contentIndexDocNum
     * @return ocr容索引完成的总文档数
     */
    uint ocrContentIndexDocNum() const override;
    /**
     * @brief aiIndexDocNum
     * @return ai索引完成的总文档数
     */
    uint aiIndexDocNum() const override;
    /**
     * basicIndexUpdating
     * @return 基础索引是否正在执行增量更新
     */
    bool basicIndexUpdating() const override;
    /**
     * basicIndexUpdating
     * @return 文本内容索引是否正在执行增量更新
     */
    bool contentIndexUpdating() const override;
    /**
     * basicIndexUpdating
     * @return ocr索引是否正在执行增量更新
     */
    bool ocrContentIndexUpdating() const override;
    /**
     * aiIndexUpdating
     * @return ai索引是否正在执行增量更新
     */
    bool aiIndexUpdating() const override;

private Q_SLOTS:
    void onIndexStateChanged(IndexScheduler::IndexerState);
    void processUpdate(LingmoUISearch::IndexType type, uint all, uint finished);

private:
    IndexScheduler *m_scheduler = nullptr;
    QStringList m_currentIndexPaths;
    Database m_basicDatabase;
    Database m_contentDatabase;
    Database m_ocrContentDatabase;
    uint m_basicIndexSize = 0;
    uint m_contentIndexSize = 0;
    uint m_ocrContentIndexSize = 0;
    uint m_aiIndexSize = 0;
    uint m_basicIndexProgress = 0;
    uint m_contentIndexProgress = 0;
    uint m_ocrContentIndexProgress = 0;
    uint m_aiIndexProgress = 0;
    uint m_basicIndexDocNum = 0;
    uint m_contentIndexDocNum = 0;
    uint m_ocrContentIndexDocNum = 0;
    uint m_aiIndexDocNum = 0;
    bool m_basicIndexUpdating = false;
    bool m_contentIndexUpdating = false;
    bool m_ocrContentIndexUpdating = false;
    bool m_aiIndexUpdating = false;

};
}
#endif // MONITOR_H
