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
#ifndef LINGMOSEARCHPRIVATE_H
#define LINGMOSEARCHPRIVATE_H

#include "lingmo-search-task.h"
#include <QThreadPool>
#include <QMutex>
#include <memory>
#include <QUuid>

#include "data-queue.h"
#include "search-controller.h"
#include "search-task-plugin-iface.h"

namespace LingmoUISearch {
class LingmoUISearchTaskPrivate : public QObject
{
    Q_OBJECT
public:
    explicit LingmoUISearchTaskPrivate(LingmoUISearchTask* parent);
    ~LingmoUISearchTaskPrivate();
    DataQueue<ResultItem>* init();
    void addSearchDir(const QString &path);
    void setRecurse(bool recurse = true);
    void addKeyword(const QString &keyword);
    void addFileLabel(const QString &label);
    void setOnlySearchFile(bool onlySearchFile);
    void setOnlySearchDir(bool onlySearchDir);
    void setSearchOnlineApps(bool searchOnlineApps);
    void initSearchPlugin(SearchType searchType, const QString& customSearchType = QString());
    bool setResultDataType(SearchType searchType, ResultDataTypes dataType);
    bool setCustomResultDataType(QString customSearchType, QStringList dataType);
    void clearAllConditions();
    void clearKeyWords();
    void clearSearchDir();
    void clearFileLabel();
    void setPagination(unsigned int first, unsigned int maxResults);

    size_t startSearch(SearchType searchtype, const QString& customSearchType = QString());
    void stop();

private:
    std::shared_ptr<SearchController> m_searchCotroller = nullptr;
    size_t m_searchId = 0;
    LingmoUISearchTask* q = nullptr;
    QUuid m_uuid;

Q_SIGNALS:
    void searchFinished(size_t searchId);
    void searchError(size_t searchId, QString msg);
};
}

#endif // LINGMOSEARCHPRIVATE_H
