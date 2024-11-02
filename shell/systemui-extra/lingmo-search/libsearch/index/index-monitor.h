/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
#ifndef LINGMO_SEARCH_INDEX_MONITOR_H
#define LINGMO_SEARCH_INDEX_MONITOR_H

#include <QObject>

namespace LingmoUISearch {
/**
 * 索引状态和进度信息查询
 */
class IndexMonitorPrivate;
class IndexMonitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList currentIndexPaths READ currentIndexPaths NOTIFY currentIndexPathsChanged)
    Q_PROPERTY(QString indexState READ indexState NOTIFY indexStateChanged)
    Q_PROPERTY(uint basicIndexSize READ basicIndexSize NOTIFY basicIndexSizeChanged)
    Q_PROPERTY(uint contentIndexSize READ contentIndexSize NOTIFY contentIndexSizeChanged)
    Q_PROPERTY(uint ocrContentIndexSize READ ocrContentIndexSize NOTIFY ocrContentIndexSizeChanged)
    Q_PROPERTY(uint aiIndexSize READ aiIndexSize NOTIFY aiIndexSizeChanged)
    Q_PROPERTY(uint basicIndexProgress READ basicIndexProgress NOTIFY basicIndexProgressChanged)
    Q_PROPERTY(uint contentIndexProgress READ contentIndexProgress NOTIFY contentIndexProgressChanged)
    Q_PROPERTY(uint ocrContentIndexProgress READ ocrContentIndexProgress NOTIFY ocrContentIndexProgressChanged)
    Q_PROPERTY(uint aiIndexProgress READ aiIndexProgress NOTIFY aiIndexProgressChanged)
    Q_PROPERTY(uint basicIndexDocNum READ basicIndexDocNum NOTIFY basicIndexDocNumChanged)
    Q_PROPERTY(uint contentIndexDocNum READ contentIndexDocNum NOTIFY contentIndexDocNumChanged)
    Q_PROPERTY(uint ocrContentIndexDocNum READ ocrContentIndexDocNum NOTIFY ocrContentIndexDocNumChanged)
    Q_PROPERTY(uint basicIndexUpdating READ basicIndexUpdating NOTIFY basicIndexUpdatingChanged)
    Q_PROPERTY(uint contentIndexUpdating READ contentIndexUpdating NOTIFY contentIndexUpdatingChanged)
    Q_PROPERTY(uint ocrContentIndexUpdating READ ocrContentIndexUpdating NOTIFY ocrContentIndexUpdatingChanged)
    Q_PROPERTY(uint aiIndexUpdating READ aiIndexUpdating NOTIFY aiIndexUpdatingChanged)

public:
    static IndexMonitor *self();
    static void stopMonitor();
    static void startMonitor();
    /**
     * @brief currentIndexPaths
     * @return 当前索引的所有目录
     */
    QStringList currentIndexPaths() const;
    /**
     * @brief indexState
     * @return 当前索引调度器的状态
     */
    QString indexState() const;
    /**
     * @brief basicIndexSize
     * @return 当前需要处理的基础索引数量
     */
    uint basicIndexSize() const;
    /**
     * @brief contentIndexSize
     * @return 当前需要处理的内容索引数量
     */
    uint contentIndexSize() const;
    /**
     * @brief ocrContentIndexSize
     * @return 当前需要处理的OCR索引数量
     */
    uint ocrContentIndexSize() const;
    /**
     * @brief aiIndexSize
     * @return 当前需要处理的ai索引数量
     */
    uint aiIndexSize() const;
    /**
     * @brief basicIndexProgress
     * @return 基础索引进度
     */
    uint basicIndexProgress() const;
    /**
     * @brief contentIndexProgress
     * @return 内容索引进度
     */
    uint contentIndexProgress() const;
    /**
     * @brief ocrContentIndexProgress
     * @return ocr索引进度
     */
    uint ocrContentIndexProgress() const;
    /**
     * @brief aiIndexProgress
     * @return ai索引进度
     */
    uint aiIndexProgress() const;
    /**
     * @brief basicIndexDocNum
     * @return 基础索引完成的总文档数
     */
    uint basicIndexDocNum() const;
    /**
     * @brief contentIndexDocNum
     * @return 内容索引完成的总文档数
     */
    uint contentIndexDocNum() const;
    /**
     * @brief contentIndexDocNum
     * @return ocr内容索引完成的总文档数
     */
    uint ocrContentIndexDocNum() const;
    /**
     * basicIndexUpdating
     * @return 基础索引是否正在执行增量更新
     */
    bool basicIndexUpdating() const;
    /**
     * basicIndexUpdating
     * @return 文本内容索引是否正在执行增量更新
     */
    bool contentIndexUpdating() const;
    /**
     * basicIndexUpdating
     * @return ocr索引是否正在执行增量更新
     */
    bool ocrContentIndexUpdating() const;
    /**
     * aiIndexUpdating
     * @return ai索引是否正在执行增量更新
     */
    bool aiIndexUpdating() const;

Q_SIGNALS:
    void currentIndexPathsChanged(QStringList currentIndexPaths);
    void indexStateChanged(QString indexState);
    void basicIndexSizeChanged(uint basicIndexSize);
    void contentIndexSizeChanged(uint contentIndexSize);
    void ocrContentIndexSizeChanged(uint ocrContentIndexSize);
    void aiIndexSizeChanged(uint aiIndexSize);
    void basicIndexProgressChanged(uint basicIndexProgress);
    void contentIndexProgressChanged(uint contentIndexProgress);
    void ocrContentIndexProgressChanged(uint ocrIndexProgress);
    void aiIndexProgressChanged(uint aiIndexProgress);
    void basicIndexDocNumChanged(uint basicIndexDocNum);
    void contentIndexDocNumChanged(uint contentIndexDocNum);
    void ocrContentIndexDocNumChanged(uint ocrContentIndexDocNum);
    void aiIndexDocNumChanged(uint ocrContentIndexDocNum);
    void basicIndexStart();
    void contentIndexStart();
    void ocrContentIndexStart();
    void aiIndexStart();
    void basicIndexDone(bool success);
    void contentIndexDone(bool success);
    void ocrContentIndexDone(bool success);
    void aiIndexDone(bool success);
    void basicIndexUpdatingChanged(bool basicIndexUpdating);
    void contentIndexUpdatingChanged(bool contentIndexUpdating);
    void ocrContentIndexUpdatingChanged(bool ocrContentIndexUpdating);
    void aiIndexUpdatingChanged(bool ocrContentIndexUpdating);

    void serviceReady();
    void serviceOffline();

private:
    explicit IndexMonitor(QObject *parent = nullptr);
    ~IndexMonitor();
    IndexMonitorPrivate *d = nullptr;
};

} // LingmoUISearch

#endif //LINGMO_SEARCH_INDEX_MONITOR_H
