/*
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
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include <QStringList>
#include <memory>
#include "data-queue.h"
#include "search-result-property.h"
//todo: url parser?
namespace LingmoUISearch {
class LingmoUISearchTask;
class ResultItem;
class SearchControllerPrivate;
/*
 *搜索控制，用于传递搜索条件，搜索唯一ID，以及管理队列等。
 */
class SearchController
{
public:
    SearchController();
    SearchController(const SearchController &other);
    SearchController &operator=(const SearchController &other);
    SearchController &operator=(SearchController &&other) Q_DECL_NOEXCEPT;

    ~SearchController();
    DataQueue<ResultItem>* refreshDataqueue();
    size_t refreshSearchId();
    DataQueue<ResultItem>* initDataQueue();
    void stop();
    void addSearchDir(const QString &path);
    void setRecurse(bool recurse = true);
    void addKeyword(const QString &keyword);
    void setMatchAllIfNoKeyword(bool matchAll);
    void setActiveKeywordSegmentation(bool active);
    void addFileLabel(const QString &label);
    void setOnlySearchFile(bool onlySearchFile);
    void setOnlySearchDir(bool onlySearchDir);
    void setSearchOnlineApps(bool searchOnlineApps);
    void setSearchHiddenFiles(bool searchHiddenFiles);
    void setMaxResultNum(unsigned int maxResults);
    void setInformNum(int num = 0);
    //以上方法插件请不要调用

    //以下方法插件可以调用
    size_t getCurrentSearchId();
    DataQueue<ResultItem>* getDataQueue();
    SearchResultProperties getResultProperties(SearchProperty::SearchType searchType);
    QStringList getCustomResultDataType(QString customSearchType);
    bool beginSearchIdCheck(size_t searchId);
    void finishSearchIdCheck();

    QStringList getSearchDir();
    bool isRecurse();
    QStringList getKeyword();
    bool getMatchAllIfNoKeyword();
    bool isKeywordSegmentationActived();
    QStringList getFileLabel();
    bool isSearchFileOnly();
    bool isSearchDirOnly();
    bool isSearchOnlineApps();
    bool searchHiddenFiles();
    void clearAllConditions();
    void clearKeyWords();
    void clearSearchDir();
    void clearFileLabel();

    unsigned int maxResults() const;
    int informNum() const;

    bool setResultProperties(SearchProperty::SearchType searchType, LingmoUISearch::SearchResultProperties searchResultProperties);
    void setCustomResultDataType(QString customSearchType, QStringList dataType);
private:
    std::shared_ptr<SearchControllerPrivate> d;

};
}

#endif // SEARCHCONTROLLER_H
