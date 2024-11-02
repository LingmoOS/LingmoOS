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
#include "search-controller.h"
#include <QMutex>
#include <QMap>
#include <QDebug>
#include "search-result-property.h"
#include "lingmo-search-task.h"
namespace LingmoUISearch {

class SearchControllerPrivate
{
public:
    void clearAllConditions();

    std::shared_ptr<DataQueue<ResultItem>> m_sharedDataQueue = nullptr;
    size_t m_searchId = 0;
    QMutex m_searchIdMutex;
    QStringList m_keywords;
    QStringList m_searchDirs;
    QStringList m_FileLabels;
    bool m_recurse = true;
    bool m_matchAllIfNoKeyword = false;
    bool m_activeKeywordSegmentation = false;
    bool m_onlySearchFile = false;
    bool m_onlySearchDir = false;
    bool m_searchOnlineApps = false;
    bool m_searchHiddenFiles = false;
    unsigned int m_maxResults = 100;        //默认取100条结果
    int m_informNum = 0;
    QMap<SearchProperty::SearchType, SearchResultProperties> m_searchType2ResultProperties;
    QMap<QString, QStringList> m_customSearchType2ResultDataType;
};

void SearchControllerPrivate::clearAllConditions()
{
    m_keywords.clear();
    m_searchDirs.clear();
    m_FileLabels.clear();
    m_recurse = true;
    m_activeKeywordSegmentation = false;
    m_onlySearchFile = false;
    m_onlySearchDir = false;
    m_searchOnlineApps = false;
}

SearchController::SearchController(): d(new SearchControllerPrivate)
{
}

SearchController::SearchController(const SearchController &other):d(other.d)
{
}

SearchController &SearchController::operator =(const SearchController &other)
{
    d = other.d;
    return *this;
}
SearchController &SearchController::operator=(SearchController &&other) Q_DECL_NOEXCEPT
{
    d = other.d;
    other.d.reset();
    return *this;
}

SearchController::~SearchController()
{
    d.reset();
}

DataQueue<ResultItem> *SearchController::refreshDataqueue()
{
    if(!d->m_sharedDataQueue.get()) {
//        m_dataQueue = new DataQueue<ResultItem>;
        d->m_sharedDataQueue = std::make_shared<DataQueue<ResultItem>>();
        return d->m_sharedDataQueue.get();
    }
    d->m_sharedDataQueue.get()->clear();
    return d->m_sharedDataQueue.get();
}

size_t SearchController::refreshSearchId()
{
    d->m_searchIdMutex.lock();
    d->m_searchId += 1;
    d->m_searchIdMutex.unlock();
    return d->m_searchId;
}

DataQueue<ResultItem> *SearchController::initDataQueue()
{
    if(!d->m_sharedDataQueue.get()) {
        d->m_sharedDataQueue = std::make_shared<DataQueue<ResultItem>>();
        return d->m_sharedDataQueue.get();
    }
    return d->m_sharedDataQueue.get();
}

void SearchController::addSearchDir(const QString &path)
{
    d->m_searchDirs.append(path);
}

void SearchController::setRecurse(bool recurse)
{
    d->m_recurse = recurse;;
}

void SearchController::addKeyword(const QString &keyword)
{
    d->m_keywords.append(keyword);
}

void SearchController::setMatchAllIfNoKeyword(bool matchAll)
{
    d->m_matchAllIfNoKeyword = matchAll;
}

size_t SearchController::getCurrentSearchId()
{
    d->m_searchIdMutex.lock();
    size_t searchId = d->m_searchId;
    d->m_searchIdMutex.unlock();

    return searchId;
}

DataQueue<ResultItem> *SearchController::getDataQueue()
{
    return d->m_sharedDataQueue.get();
}

SearchResultProperties SearchController::getResultProperties(SearchProperty::SearchType searchType)
{
    return d->m_searchType2ResultProperties[searchType];;
}

QStringList SearchController::getCustomResultDataType(QString customSearchType)
{
    return d->m_customSearchType2ResultDataType[customSearchType];
}

void SearchController::setActiveKeywordSegmentation(bool active)
{
    d->m_activeKeywordSegmentation = active;;
}

void SearchController::addFileLabel(const QString &label)
{
    d->m_FileLabels.append(label);;
}

void SearchController::setOnlySearchFile(bool onlySearchFile)
{
    d->m_onlySearchFile = onlySearchFile;
}

void SearchController::setOnlySearchDir(bool onlySearchDir)
{
    d->m_onlySearchDir = onlySearchDir;
}

void SearchController::setSearchOnlineApps(bool searchOnlineApps)
{
    d->m_searchOnlineApps = searchOnlineApps;
}

void SearchController::setSearchHiddenFiles(bool searchHiddenFiles)
{
    d->m_searchHiddenFiles = searchHiddenFiles;
}

bool SearchController::beginSearchIdCheck(size_t searchId)
{
    d->m_searchIdMutex.lock();
    return d->m_searchId == searchId;
}

void SearchController::finishSearchIdCheck()
{
    d->m_searchIdMutex.unlock();
    return;
}

QStringList SearchController::getSearchDir()
{
    return d->m_searchDirs;
}

bool SearchController::isRecurse()
{
    return d->m_recurse;
}

QStringList SearchController::getKeyword()
{
    return d->m_keywords;
}

bool SearchController::getMatchAllIfNoKeyword()
{
    return d->m_matchAllIfNoKeyword;
}

bool SearchController::isKeywordSegmentationActived()
{
    return d->m_activeKeywordSegmentation;
}

QStringList SearchController::getFileLabel()
{
    return d->m_FileLabels;
}

bool SearchController::isSearchFileOnly()
{
    return d->m_onlySearchFile;
}

bool SearchController::isSearchDirOnly()
{
    return d->m_onlySearchDir;
}

bool SearchController::isSearchOnlineApps()
{
    return d->m_searchOnlineApps;
}

bool SearchController::searchHiddenFiles()
{
    return d->m_searchHiddenFiles;
}

void SearchController::stop()
{
    d->m_searchIdMutex.lock();
    d->m_searchId += 1;
    d->m_searchIdMutex.unlock();
}

void SearchController::clearAllConditions()
{
    d->clearAllConditions();
}

void SearchController::clearKeyWords()
{
    d->m_keywords.clear();
}

void SearchController::clearSearchDir()
{
    d->m_searchDirs.clear();
}

void SearchController::clearFileLabel()
{
    d->m_FileLabels.clear();
}

void SearchController::setMaxResultNum(unsigned int maxResults)
{
    d->m_maxResults = maxResults;
}

void SearchController::setInformNum(int num)
{
    if(num >= 0) {
        d->m_informNum = num;
    }
}

unsigned int SearchController::maxResults() const
{
    return d->m_maxResults;
}

int SearchController::informNum() const
{
    return d->m_informNum;
}

bool SearchController::setResultProperties(SearchProperty::SearchType searchType, SearchResultProperties searchResultProperties)
{
    d->m_searchType2ResultProperties[searchType] = searchResultProperties;
    return true;
}

void SearchController::setCustomResultDataType(QString customSearchType, QStringList dataType)
{
    d->m_customSearchType2ResultDataType[customSearchType] = dataType;
}
}
