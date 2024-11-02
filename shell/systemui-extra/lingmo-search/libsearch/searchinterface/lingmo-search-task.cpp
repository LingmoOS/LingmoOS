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
#include "lingmo-search-task.h"
#include "search-task-plugin-manager.h"
#include <QDebug>
using namespace LingmoUISearch;
namespace LingmoUISearch {

class LingmoUISearchTaskPrivate
{
    friend class LingmoUISearchTask;
private:
    SearchController m_searchCotroller;
    size_t m_searchId = 0;
    QUuid m_uuid;
};
}

LingmoUISearchTask::LingmoUISearchTask(QObject *parent) : QObject(parent), d(new LingmoUISearchTaskPrivate())
{
    d->m_uuid = QUuid::createUuid();
}

LingmoUISearchTask::~LingmoUISearchTask()
{
    stop();
    SearchTaskPluginManager::getInstance()->destroyPlugins(d->m_uuid);
}

DataQueue<ResultItem> *LingmoUISearchTask::init()
{
    return d->m_searchCotroller.initDataQueue();
}

void LingmoUISearchTask::addSearchDir(const QString &path)
{
    d->m_searchCotroller.addSearchDir(path);
}

void LingmoUISearchTask::setRecurse(bool recurse)
{
    d->m_searchCotroller.setRecurse(recurse);
}

void LingmoUISearchTask::addKeyword(const QString &keyword)
{
    d->m_searchCotroller.addKeyword(keyword);
}

void LingmoUISearchTask::setMatchAllIfNoKeyword(bool matchAll)
{
    d->m_searchCotroller.setMatchAllIfNoKeyword(matchAll);
}

void LingmoUISearchTask::addFileLabel(const QString &label)
{
    d->m_searchCotroller.addFileLabel(label);
}

void LingmoUISearchTask::setOnlySearchFile(bool onlySearchFile)
{
    d->m_searchCotroller.setOnlySearchFile(onlySearchFile);
}

void LingmoUISearchTask::setOnlySearchDir(bool onlySearchDir)
{
    d->m_searchCotroller.setOnlySearchDir(onlySearchDir);
}

void LingmoUISearchTask::setSearchOnlineApps(bool searchOnlineApps)
{
    d->m_searchCotroller.setSearchOnlineApps(searchOnlineApps);
}

void LingmoUISearchTask::setSearchHiddenFiles(bool searchHiddenFiles)
{
    d->m_searchCotroller.setSearchHiddenFiles(searchHiddenFiles);
}

void LingmoUISearchTask::initSearchPlugin(SearchProperty::SearchType searchType, const QString &customSearchType)
{
    SearchTaskPluginIface *plugin = SearchTaskPluginManager::getInstance()->initPlugins(d->m_uuid, searchType, customSearchType);
    if (plugin) {
        plugin->setController(d->m_searchCotroller);
        connect(plugin, &SearchTaskPluginIface::searchFinished,this, &LingmoUISearchTask::searchFinished);
        connect(plugin, &SearchTaskPluginIface::searchError,this, &LingmoUISearchTask::searchError);
        connect(plugin, &SearchTaskPluginIface::reachInformNum,this, &LingmoUISearchTask::reachInformNum);
    } else {
        qWarning() << "The plugin has been initialized or the plugin failed to load.";
    }
}

bool LingmoUISearchTask::setResultProperties(SearchProperty::SearchType searchType, SearchResultProperties searchResultProperties)
{
    return d->m_searchCotroller.setResultProperties(searchType, searchResultProperties);
}

void LingmoUISearchTask::setCustomResultDataType(QString customSearchType, QStringList dataType)
{
    return d->m_searchCotroller.setCustomResultDataType(customSearchType, dataType);
}

size_t LingmoUISearchTask::startSearch(SearchProperty::SearchType searchtype, QString customSearchType)
{
    d->m_searchId = d->m_searchCotroller.refreshSearchId();
    if(d->m_searchCotroller.getDataQueue() == nullptr) {
        qWarning() << "the date queue has not been initialized, you need run init first!";
    }

    d->m_searchCotroller.refreshDataqueue();
    //plugin manager do async search here
    if (!SearchTaskPluginManager::getInstance()->startSearch(d->m_uuid, searchtype, customSearchType)) {
        Q_EMIT searchError(d->m_searchCotroller.getCurrentSearchId(), tr("Current task uuid error or an unregistered plugin is used!"));
    }

    return d->m_searchId;
}

void LingmoUISearchTask::stop()
{
    d->m_searchCotroller.stop();
}

bool LingmoUISearchTask::isSearching(SearchProperty::SearchType searchtype, QString customSearchType)
{
    return SearchTaskPluginManager::getInstance()->isSearching(d->m_uuid, searchtype, customSearchType);
}

void LingmoUISearchTask::clearAllConditions()
{
    d->m_searchCotroller.clearAllConditions();
}

void LingmoUISearchTask::clearKeyWords()
{
    d->m_searchCotroller.clearKeyWords();
}

void LingmoUISearchTask::clearSearchDir()
{
    d->m_searchCotroller.clearSearchDir();
}

void LingmoUISearchTask::clearFileLabel()
{
    d->m_searchCotroller.clearFileLabel();
}

void LingmoUISearchTask::setMaxResultNum(unsigned int maxResults)
{
    d->m_searchCotroller.setMaxResultNum(maxResults);
}

void LingmoUISearchTask::setInformNum(int num)
{
    d->m_searchCotroller.setInformNum(num);
}
