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
#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include "file-system-watcher.h"
#include "file-indexer-config.h"
#include "pending-file-queue.h"
namespace LingmoUISearch {
/**
 * @brief The FileWatcher class
 * 文件监听与信号处理控制中心
 */
class FileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit FileWatcher(QObject *parent = nullptr);
    ~FileWatcher();

public Q_SLOTS:
    /**
     * @brief addWatch
     * 增加监听目录
     * @param 要增加的目录和黑名单
     */
    void addWatch(const QString& path, const QStringList& blackList);
    void removeWatch(const QString& path, bool updateIndex = true);

    /**
     * @brief installWatches
     * 安装监听
     */
    void installWatches();
    /**
     * @brief removeWatch
     * 移除所有监听
     */
    void removeWatch();

Q_SIGNALS:
    void filesUpdate(const QVector<PendingFile>&);
    void installedWatches();

private:
    void onFileCreated(const QString& path, bool isDir);
    void onFileMoveTo(const QString& path, bool isDir);
    void onFileModified(const QString& path);
    void onFileDeletedOrMoved(const QString& path, bool isDir);
    void onFileAccessibleChanged(const QString& path, bool isDir, bool accessible);
    FileSystemWatcher *m_watcher = nullptr;
    FileIndexerConfig *m_config = nullptr;
    PendingFileQueue *m_pendingFileQueue = nullptr;

};
}
#endif // FILEWATCHER_H
