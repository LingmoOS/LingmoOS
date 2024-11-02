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
#ifndef FIRSTRUNINDEXER_H
#define FIRSTRUNINDEXER_H

#include <QRunnable>
#include <QObject>
#include <QAtomicInt>
#include "common.h"

namespace LingmoUISearch {
class BatchIndexer : public QObject, public QRunnable
{
    Q_OBJECT
public:
    /**
     * @brief The WorkMode enum
     * Update 增量更新
     * Add 增加索引目录
     * Rebuild 删除并重建数据库
     */
    enum WorkMode{
        Update = 0,
        Add    = 1,
        Rebuild
    };
    Q_ENUM(WorkMode)
    /**
     * @brief The Target enum
     * 要进行索引的数据库
     * All 所有数据库
     * Basic 基础索引数据库
     * Content 内容索引数据库
     */
    enum Target{
        None     = 0,
        Basic    = 1u << 0,
        Content  = 1u << 1,
        Ocr      = 1u << 2,
        Ai       = 1u << 3,
        All      = Basic | Content | Ocr | Ai
    };
    Q_ENUM(Target)
    Q_DECLARE_FLAGS(Targets, Target)
    Q_FLAG(Targets)

    BatchIndexer(const QStringList& folders,
                 const QStringList& blackList,
                 QAtomicInt& indexStop,
                 QAtomicInt& contentIndexStop,
                 QAtomicInt& contentIndexOcrStop,
                 QAtomicInt& aiIndexStop,
                 WorkMode mode = WorkMode::Update,
                 Targets target = Target::All);
    void run() override;

Q_SIGNALS:
    void progress(IndexType type, uint all, uint finished);
    void basicIndexStart(WorkMode);
    void contentIndexStart(WorkMode);
    void ocrContentIndexStart(WorkMode);
    void aiIndexStart(WorkMode);
    void basicIndexDone(WorkMode);
    void contentIndexDone(WorkMode);
    void ocrContentIndexDone(WorkMode);
    void aiIndexDone(WorkMode);
    void done(WorkMode, Targets);

private:
    void fetch();
    void basicIndex();
    void contentIndex();
    void ocrIndex();
    void aiIndex();

    QStringList m_folders;
    QStringList m_blackList;
    QAtomicInt *m_indexStop = nullptr;
    QAtomicInt *m_contentIndexStop = nullptr;
    QAtomicInt *m_contentIndexOcrStop = nullptr;
    QAtomicInt *m_aiIndexStop = nullptr;
    WorkMode m_mode;
    Targets m_target;
    QStringList m_cache;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(BatchIndexer::Targets)
}
#endif // FIRSTRUNINDEXER_H
