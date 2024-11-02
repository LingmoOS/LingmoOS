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
#ifndef LINGMOSEARCH_H
#define LINGMOSEARCH_H

#include "result-item.h"
#include "data-queue.h"
#include "search-result-property.h"
namespace LingmoUISearch {
class LingmoUISearchTaskPrivate;
class LingmoUISearchTask : public QObject
{
    Q_OBJECT
public:
    explicit LingmoUISearchTask(QObject *parent = nullptr);
    ~LingmoUISearchTask();
    DataQueue<ResultItem>* init();
    void addSearchDir(const QString &path);
    void setRecurse(bool recurse = true);
    void addKeyword(const QString &keyword);
    /**
     * 是否在未添加关键词的情况下进行所有文件名匹配
     * @brief setMatchAllIfNoKeyword
     * @param matchAll
     */
    void setMatchAllIfNoKeyword(bool matchAll);
    void addFileLabel(const QString &label);
    void setOnlySearchFile(bool onlySearchFile);
    void setOnlySearchDir(bool onlySearchDir);
    void setSearchOnlineApps(bool searchOnlineApps);
    /**
     * 是否搜索隐藏文件
     * @brief setSearchHiddenFiles
     * @param searchHiddenFiles
     */
    void setSearchHiddenFiles(bool searchHiddenFiles);
    /**
     * @brief initSearchPlugin 初始化搜索插件
     * @param searchType
     * @param customSearchType
     */
    void initSearchPlugin(SearchProperty::SearchType searchType, const QString& customSearchType = QString());
    /**
     * @brief setResultDataType
     * @param searchType
     * @param dataType
     * @return
     */
    bool setResultProperties(SearchProperty::SearchType searchType, SearchResultProperties searchResultProperties);
    void setCustomResultDataType(QString customSearchType, QStringList dataType);

    void clearAllConditions();
    void clearKeyWords();
    void clearSearchDir();
    void clearFileLabel();
    /**
     * @brief setMaxResultNum 设置最大结果数量
     * @param maxResults
     */
    void setMaxResultNum(unsigned int maxResults = 99999999);
    /**
     * @brief setInformNum 设置搜索结果提醒数量
     * @param num
     */
    void setInformNum(int num);

    /**
     * @brief startSearch 启动搜索
     * @param searchtype 搜索插件
     * @param customSearchType 外部插件类型，当searchType为Custom时可用
     * @return
     */
    size_t startSearch(SearchProperty::SearchType searchtype, QString customSearchType = QString());
    /**
     * @brief stop 停止搜索
     */
    void stop();
    /**
     * @brief isSearching 查询某个插件是否处于搜索中
     */
    bool isSearching(SearchProperty::SearchType searchtype, QString customSearchType = {});

Q_SIGNALS:
    void searchFinished(size_t searchId);
    void searchError(size_t searchId, QString msg);
    void reachInformNum();

private:
    LingmoUISearchTaskPrivate* d = nullptr;
};
}

#endif // LINGMOSEARCH_H
