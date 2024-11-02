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

#ifndef LINGMO_SEARCH_FILE_CONTENT_SEARCH_TASK_H
#define LINGMO_SEARCH_FILE_CONTENT_SEARCH_TASK_H

//lingmo-search
#include "search-task-plugin-iface.h"

//Qt
#include <QThreadPool>
#include <QRunnable>

//Xapian
#include <xapian.h>

namespace LingmoUISearch {

class FileContentSearchTask : public SearchTaskPluginIface
{
    Q_OBJECT
public:
    explicit FileContentSearchTask(QObject *parent = nullptr);

    ~FileContentSearchTask() override;
    void setController(const SearchController &searchController) override;
    PluginType pluginType() override;
    const QString name() override;
    const QString description() override;
    const QIcon icon() override;
    void setEnable(bool enable) override;
    bool isEnable() override;

    QString getCustomSearchType() override;
    SearchProperty::SearchType getSearchType() override;
    void startSearch() override;
    void stop() override;
    bool isSearching() override;

private:
    QThreadPool *m_pool = nullptr;
    bool m_enable = true;
    SearchController m_searchController;
};

class FileContentSearchWorker : public QRunnable
{
    friend class FileContentSearchFilter;

public:
    explicit FileContentSearchWorker(FileContentSearchTask *fileContentSearchTask, SearchController *searchController);
    void run() override;

private:
    bool execSearch();
    inline Xapian::Query createQuery();

    void sendErrorMsg(const QString &msg);

private:
    FileContentSearchTask *m_fileContentSearchTask = nullptr;
    SearchController *m_searchController = nullptr;
    QStringList m_validDirectories;
    size_t m_currentSearchId = 0;
    int m_resultNum = 0;
};

class FileContentSearchFilter : public Xapian::MatchDecider {
public:
    explicit FileContentSearchFilter(FileContentSearchWorker*);
    bool operator ()(const Xapian::Document &doc) const override;

private:
    FileContentSearchWorker *m_worker;
};

}

#endif //LINGMO_SEARCH_FILE_CONTENT_SEARCH_TASK_H
