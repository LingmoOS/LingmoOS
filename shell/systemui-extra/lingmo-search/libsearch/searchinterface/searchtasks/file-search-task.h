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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef FILESEARCHTASK_H
#define FILESEARCHTASK_H

#include <QIcon>
#include <QThreadPool>
#include <QRunnable>
#include <xapian.h>

#include "search-task-plugin-iface.h"
#include "search-controller.h"
#include "result-item.h"

namespace LingmoUISearch {
class FileSearchTask : public SearchTaskPluginIface
{
    Q_OBJECT
public:
    explicit FileSearchTask(QObject *parent = nullptr);
    ~FileSearchTask();

    void setController(const SearchController &searchController);
    PluginType pluginType() {return PluginType::SearchTaskPlugin;}
    const QString name();
    const QString description();
    const QIcon icon() {return QIcon::fromTheme("folder");}
    void setEnable(bool enable) {Q_UNUSED(enable)}
    bool isEnable() { return true;}

    SearchProperty::SearchType getSearchType() {return SearchProperty::SearchType::File;}
    QString getCustomSearchType();
    void startSearch();
    void stop();
    bool isSearching();

private:
    QThreadPool *m_pool = nullptr;
    SearchController m_searchController;
};

class FileSearchWorker : public QRunnable
{
    friend class FileSearchFilter;

public:
    explicit FileSearchWorker(FileSearchTask *fileSarchTask, SearchController *searchController);

protected:
    void run();

private:
    /**
     * @brief 通过索引进行搜索，如果搜索过程正常，返回true
     * 如果搜索被打断，返回false.
     * 搜索被打断是指用户使用同一个task发起多次搜索，导致searchId发生变化，那么上一次搜索即被打断。
     * @return
     */
    bool searchWithIndex();
    //同上
    bool directSearch();
    Xapian::Query creatQueryForFileSearch();
    void sendErrorMsg(const QString &msg);

private:
    FileSearchTask *m_FileSearchTask;
    SearchController *m_searchController = nullptr;

    size_t m_currentSearchId = 0;
    int m_resultNum = 0;
    QStringList m_validDirectories;
    QStringList m_blackList;
    QStringList m_labels;
};

class FileSearchFilter : public Xapian::MatchDecider {
public:
    explicit FileSearchFilter(FileSearchWorker *parent);
    bool operator ()(const Xapian::Document &doc) const;

    /**
     * @brief 检查path对应的文件是否包含labels中的标签
     * @param path
     * @param labels
     * @return
     */
    static bool checkFileLabel(const QString &path, const QStringList &labels);

private:
    FileSearchWorker *m_parent = nullptr;
};

}
#endif // FILESEARCHTASK_H
