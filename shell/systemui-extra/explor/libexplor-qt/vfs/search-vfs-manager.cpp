/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "search-vfs-manager.h"

using namespace Peony;

static SearchVFSManager* global_manager = nullptr;

SearchVFSManager *SearchVFSManager::getInstance()
{
    if (!global_manager) {
        global_manager = new SearchVFSManager;
    }
    return global_manager;
}

SearchVFSManager::SearchVFSManager(QObject *parent) : QObject(parent)
{

}

SearchVFSManager::~SearchVFSManager()
{
    m_search_dir_results_hash.clear();
}

void SearchVFSManager::clearHistory()
{
    m_mutex.lock();
    m_search_dir_results_hash.clear();
    m_mutex.unlock();
}

void SearchVFSManager::clearHistoryOne(const QString &searchUri)
{
    m_mutex.lock();
    m_search_dir_results_hash.remove(searchUri);
    m_mutex.unlock();
}

bool SearchVFSManager::hasHistory(const QString &searchUri)
{
    return m_search_dir_results_hash.contains(searchUri);
}

void SearchVFSManager::addHistory(const QString &searchUri, const QStringList &results)
{
    m_mutex.lock();
    m_search_dir_results_hash.insert(searchUri, results);
    m_mutex.unlock();
}

QStringList SearchVFSManager::getHistroyResults(const QString &searchUri)
{
    return m_search_dir_results_hash.value(searchUri);
}
