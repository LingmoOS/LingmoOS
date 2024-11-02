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
 *
 */
#ifndef SEARCHTASKPLUGINIFACE_H
#define SEARCHTASKPLUGINIFACE_H
#define SearchTaskPluginIface_iid "org.lingmo.lingmo-search.plugin-iface.SearchTaskPluginIface"
#define SEARCH_TASK_PLUGIN_IFACE_VERSION "1.1.0"

#include <QObject>
#include <QtPlugin>
#include "plugin-iface.h"
#include "search-result-property.h"
#include "search-controller.h"


namespace LingmoUISearch {
class SearchTaskPluginIface : public QObject, public PluginInterface
{
    Q_OBJECT
public:
    virtual void setController(const SearchController &searchController) = 0;
    virtual QString getCustomSearchType() = 0;
    virtual SearchProperty::SearchType getSearchType() = 0;
    //Asynchronous,multithread.
    virtual void startSearch() = 0;
    virtual void stop() = 0;
    virtual bool isSearching() = 0;
Q_SIGNALS:
    void searchFinished(size_t searchId);
    void searchError(size_t searchId, QString msg = {});
    void reachInformNum();
};
}
Q_DECLARE_INTERFACE(LingmoUISearch::SearchTaskPluginIface, SearchTaskPluginIface_iid)



#endif // SEARCHTASKPLUGINIFACE_H
